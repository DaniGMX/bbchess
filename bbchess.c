
// system headers
#include <stdio.h>
#include <time.h>
#include <memory.h>

#pragma region Type Definitions

#define u64 unsigned long long

#pragma endregion

#pragma region Enumerations

// define bitboard squares
enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1
};

// define square coordinates array to access by square index
const char* square_to_coordinates[] = {
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

// define colors enumerations
enum { white, black };

// define sliding pieces enumerations
enum { rook, bishop };

#pragma endregion

#pragma region Random Number Generation

/** Seed for random number generation. */
unsigned int seed = 1804289383;

/**
 * Generates a 32-bit random unsigned integer.
 * @returns A random 32-bit unsigned integer.
 */
unsigned int psrandom_u32() {
	// get current state
	unsigned int num = seed;

	// XOR shift algorithm
	num ^= num << 13;
	num ^= num >> 17;
	num ^= num << 5;

	// update state
	seed = num;

	// return random number
	return num;	
}

/**
 * Generates a 64-bit random unsigned integer.
 * @returns A random 64-bit unsigned integer.
 */
u64 psrandom_u64() {
	// define 4 random numbers
	u64 n1, n2, n3, n4;

	// initialize random numbers
	n1 = (u64)(psrandom_u32()) & 0xFFFF;
	n2 = (u64)(psrandom_u32()) & 0xFFFF;
	n3 = (u64)(psrandom_u32()) & 0xFFFF;
	n4 = (u64)(psrandom_u32()) & 0xFFFF;

	//return random number
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

#pragma endregion

#pragma region Bit Manipulations

#define get_bit(bitboard, square) (bitboard &  (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

/**
 * Counts the number of bits set in a bitboard.
 * @param bitboard The bitboard to count the bits from.
 * @return The number of bits set in the bitboard.
 */
static inline int count_bits(u64 bitboard) {
	int count = 0;

	// Continuously shift the bitboard to the right until it is zero
	while (bitboard) {
		bitboard &= (bitboard - 1);
		count++;
	}
	return count;
}

/**
 * Gets the least significant bit index set in a bitboard.
 * @param bitboard The bitboard to get the least significant bit index from.
 * @return The least significant bit index set in the bitboard.
 */
static inline int get_lsb_index(u64 bitboard) {
	if (bitboard) {
		return count_bits((bitboard & -bitboard) - 1);
	} else {
		return -1;
	}
}

#pragma endregion

#pragma region Input & Output

/**
 * Bitboard representation of a chess board.
 * @param bitboard The bitboard to represent.
 * @param msg Description of the bitboard to print.
 */
void print_bitboard(u64 bitboard, char* msg) {
	printf("\n=====[ Printing Bitboard ]=====\n");
	// print description of bitboard
	if (msg)
		printf("- Description:\t%s\n", msg);

	// print decimal value of bitboard
	printf("- Base 10:\t%020llud\n", bitboard);

	// print the number of bits of the bitboard
	printf("- Bit count:\t%d\n", count_bits(bitboard));

	// print the LSB index in the bitboard
	printf("- LSB index:\t%d\n", get_lsb_index(bitboard));

	// print the LSB index in the bitboard
	printf("- LSB coords:\t%s\n", square_to_coordinates[get_lsb_index(bitboard)]);

	// print bitboard as a board of bits
	printf("- Board:\t");

	// loop over rank and files
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			// convert file and rank into square index
			int square = rank * 8 + file;
			
			if (!file)
				printf("%d ", 8 - rank);

			// print bit state (either 1 or 0)
			printf(" %d", get_bit(bitboard, square) ? 1 : 0);
		}
		// print new line
		printf("\n\t\t");
	}

	printf("   A B C D E F G H\n");
}

#pragma endregion

#pragma region Attack Precalculations

/**
 * Bitboard with all bits set except for the A file.
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 * 0 1 1 1 1 1 1 1
 */
const u64 not_a_file = 18374403900871474942ULL;

/**
 * Bitboard with all bits set except for the A and B files.
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 * 0 0 1 1 1 1 1 1
 */
const u64 not_ab_files = 18229723555195321596ULL;

/**
 * Bitboard with all bits set except for the H file.
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 * 1 1 1 1 1 1 1 0 
 */
const u64 not_h_file = 9187201950435737471ULL;

/**
 * Bitboard with all bits set except for the G and H files.
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 * 1 1 1 1 1 1 0 0
 */
const u64 not_gh_files = 4557430888798830399ULL;

/** Bishop relevant occupancy bit count for every square on board */
const int bishop_relevant_bits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

/** Rook relevant occupancy bit count for every square on board */
const int rook_relevant_bits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

/** Table containing the bitboards of both colors pawn attacks from each square. */
u64 pawn_attacks[2][64];

/** Array containing the bitboards of knight attacks from each square. */
u64 knight_attacks[64];

/** Array containing the bitboards of king attacks from each square. */
u64 king_attacks[64];

/**
 * Precalculate pawn attacks.
 * @param color The color of the pawn to calculate its attacks from.
 * @param square The square from which to calculate its attacks from.
 * @return The bitboard with the pawn attacks from the given square.
 */
u64 mask_pawn_attacks(int color, int square) {
	// pawn bitboard
	u64 pawn = 0ULL;
	set_bit(pawn, square);

	// pawn attacks bitboard
	u64 attacks = 0ULL;
	
	// white pawn attacks
	if (color == white) {
		// generate white pawn attacks
		if ((pawn >> 7) & not_a_file) attacks |= (pawn >> 7);
		if ((pawn >> 9) & not_h_file) attacks |= (pawn >> 9);
	}
	// black pawn attacks
	else {
		// generate black pawn attacks
		if ((pawn << 7) & not_h_file) attacks |= (pawn << 7);
		if ((pawn << 9) & not_a_file) attacks |= (pawn << 9);
	}

	return attacks;
}

/**
 * Precalculate knight attacks.
 * @param square The square from which to calculate the knight attacks from.
 * @return The bitboard with the knight attacks from the given square.
 */
u64 mask_knight_attacks(int square) {
	// knight bitboard
	u64 knight = 0ULL;
	set_bit(knight, square);

	// knight attacks bitboard
	u64 attacks = 0ULL;

	// generate knight attacks (17, 15, 10, 6, -17, -15, -10, -6)
	if ((knight >> 17) & not_h_file) 	attacks |= (knight >> 17);
	if ((knight >> 15) & not_a_file) 	attacks |= (knight >> 15);
	if ((knight >> 10) & not_gh_files) 	attacks |= (knight >> 10);
	if ((knight >> 6)  & not_ab_files) 	attacks |= (knight >> 6);

	if ((knight << 17) & not_a_file) 	attacks |= (knight << 17);
	if ((knight << 15) & not_h_file) 	attacks |= (knight << 15);
	if ((knight << 10) & not_ab_files) 	attacks |= (knight << 10);
	if ((knight << 6)  & not_gh_files) 	attacks |= (knight << 6);

	return attacks;
}

/**
 * Precalculate king attacks.
 * @param square The square from which to calculate the king attacks from.
 * @return The bitboard with the king attacks from the given square.
 */
u64 mask_king_attacks(int square) {
	// king bitboard
	u64 king = 0ULL;
	set_bit(king, square);

	// king attacks bitboard
	u64 attacks = 0ULL;

	// generate king attacks (8, -8, 1, -1, 7, -7, 9, -9)
	if (king >> 8)				attacks |= (king >> 8);
	if (king >> 9 & not_h_file)	attacks |= (king >> 9);
	if (king >> 7 & not_a_file)	attacks |= (king >> 7);
	if (king >> 1 & not_h_file)	attacks |= (king >> 1);

	if (king << 8)				attacks |= (king << 8);
	if (king << 9 & not_a_file)	attacks |= (king << 9);
	if (king << 7 & not_h_file)	attacks |= (king << 7);
	if (king << 1 & not_a_file)	attacks |= (king << 1);

	return attacks;
}

/**
 * Initialize attacks for all leaper pieces (pawns, knights & kings).
 */
void init_leapers_attacks() {
	// loop over all squares
	for (int square = 0; square < 64; square++) {
		// initialize pawn attacks
		pawn_attacks[white][square] = mask_pawn_attacks(white, square);
		pawn_attacks[black][square] = mask_pawn_attacks(black, square);

		// initialize knight attacks
		knight_attacks[square] = mask_knight_attacks(square);

		// initialize king attacks
		king_attacks[square] = mask_king_attacks(square);
	}
}

/**
 * Precalculate bishop attacks.
 * @param square The square from which to calculate the bishop attacks from.
 * @return The bitboard with the bishop attacks from the given square.
 */
u64 mask_bishop_attacks(int square) {
	// bishop bitboard
	u64 bishop = 0ULL;
	set_bit(bishop, square);

	// bishop attacks bitboard
	u64 attacks = 0ULL;

	// init ranks and files	
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant bishop occupancy bits
	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f)); 
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

	return attacks;
}

/**
 * Precalculate rook attacks.
 * @param square The square from which to calculate the rook attacks from.
 * @return The bitboard with the rook attacks from the given square.
 */
u64 mask_rook_attacks(int square) {
	// rook bitboard
	u64 rook = 0ULL;
	set_bit(rook, square);

	// rook attacks bitboard
	u64 attacks = 0ULL;

	// init ranks and files	
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant rook occupancy bits
	for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
	for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
	for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
	for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

	return attacks;
}

/**
 * Calculate bishop attacks in real time, given a blocked bitboard.
 * @param square The square from which to calculate the real-time bishop attacks from.
 * @param blocked The bitboard with the blocked squares.
 * @return The bitboard with the bishop attacks from the given square.
 */
u64 real_time_bishop_attacks(int square, u64 blocked) {
	// bishop bitboard
	u64 bishop = 0ULL;
	set_bit(bishop, square);

	// bishop attacks bitboard
	u64 attacks = 0ULL;

	// init ranks and files	
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// generate bishop attacks
	for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blocked) break;
	}
	for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blocked) break;
	}
	for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blocked) break;
	}
	for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blocked) break;
	}

	return attacks;
}

/**
 * Calculate rook attacks in real time, given a blocked bitboard.
 * @param square The square from which to calculate the real-time rook attacks from.
 * @param blocked The bitboard with the blocked squares.
 * @return The bitboard with the rook attacks from the given square.
 */
u64 real_time_rook_attacks(int square, u64 blocked) {
	// rook bitboard
	u64 rook = 0ULL;
	set_bit(rook, square);

	// rook attacks bitboard
	u64 attacks = 0ULL;

	// init ranks and files	
	int r, f;

	// init target rank & files
	int tr = square / 8;
	int tf = square % 8;

	// mask relevant rook occupancy bits
	for (r = tr + 1; r <= 7; r++) {
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & blocked) break;
	}
	for (r = tr - 1; r >= 0; r--) {
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & blocked) break;
	}
	for (f = tf + 1; f <= 7; f++) {
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & blocked) break;
	}
	for (f = tf - 1; f >= 0; f--) {
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & blocked) break;
	}

	return attacks;
}

/**
 * Calculate a combination of occupancies for a sliding piece at a given square.
 * @param index To be determined.
 * @param bits_in_mask To be determined.
 * @param attack_mask To be determined.
 * @return To be determined.
 */
u64 set_occupancy(int index, int bits_in_mask, u64 attack_mask) {
	// occupancy mask
	u64 occupancy = 0ULL;

	// loop over the range of bits within the attack mask
	for (int count = 0; count < bits_in_mask; count++) {
		// get LSB index of attack mask
		int square = get_lsb_index(attack_mask);

		// pop LSB in attack mask
		pop_bit(attack_mask, square);

		// make sure occupancy is on board
		if (index & (1 << count))
			occupancy |= (1ULL << square);
	}

	// return occupancy map
	return occupancy;
}

#pragma endregion

#pragma region Magic Numbers

/** Pre-calculated rook magic numbers */
u64 rook_magic_numbers[64] = {
	0xa080041440042080ULL,
	0xa840200410004001ULL,
	0xc800c1000200081ULL,
	0x100081001000420ULL,
	0x200020010080420ULL,
	0x3001c0002010008ULL,
	0x8480008002000100ULL,
	0x2080088004402900ULL,
	0x800098204000ULL,
	0x2024401000200040ULL,
	0x100802000801000ULL,
	0x120800800801000ULL,
	0x208808088000400ULL,
	0x2802200800400ULL,
	0x2200800100020080ULL,
	0x801000060821100ULL,
	0x80044006422000ULL,
	0x100808020004000ULL,
	0x12108a0010204200ULL,
	0x140848010000802ULL,
	0x481828014002800ULL,
	0x8094004002004100ULL,
	0x4010040010010802ULL,
	0x20008806104ULL,
	0x100400080208000ULL,
	0x2040002120081000ULL,
	0x21200680100081ULL,
	0x20100080080080ULL,
	0x2000a00200410ULL,
	0x20080800400ULL,
	0x80088400100102ULL,
	0x80004600042881ULL,
	0x4040008040800020ULL,
	0x440003000200801ULL,
	0x4200011004500ULL,
	0x188020010100100ULL,
	0x14800401802800ULL,
	0x2080040080800200ULL,
	0x124080204001001ULL,
	0x200046502000484ULL,
	0x480400080088020ULL,
	0x1000422010034000ULL,
	0x30200100110040ULL,
	0x100021010009ULL,
	0x2002080100110004ULL,
	0x202008004008002ULL,
	0x20020004010100ULL,
	0x2048440040820001ULL,
	0x101002200408200ULL,
	0x40802000401080ULL,
	0x4008142004410100ULL,
	0x2060820c0120200ULL,
	0x1001004080100ULL,
	0x20c020080040080ULL,
	0x2935610830022400ULL,
	0x44440041009200ULL,
	0x280001040802101ULL,
	0x2100190040002085ULL,
	0x80c0084100102001ULL,
	0x4024081001000421ULL,
	0x20030a0244872ULL,
	0x12001008414402ULL,
	0x2006104900a0804ULL,
	0x1004081002402ULL
};

/** Pre-calculated bishop magic numbers */
u64 bishop_magic_numbers[64] = {
	0x40040822862081ULL,
	0x40810a4108000ULL,
	0x2008008400920040ULL,
	0x61050104000008ULL,
	0x8282021010016100ULL,
	0x41008210400a0001ULL,
	0x3004202104050c0ULL,
	0x22010108410402ULL,
	0x60400862888605ULL,
	0x6311401040228ULL,
	0x80801082000ULL,
	0x802a082080240100ULL,
	0x1860061210016800ULL,
	0x401016010a810ULL,
	0x1000060545201005ULL,
	0x21000c2098280819ULL,
	0x2020004242020200ULL,
	0x4102100490040101ULL,
	0x114012208001500ULL,
	0x108000682004460ULL,
	0x7809000490401000ULL,
	0x420b001601052912ULL,
	0x408c8206100300ULL,
	0x2231001041180110ULL,
	0x8010102008a02100ULL,
	0x204201004080084ULL,
	0x410500058008811ULL,
	0x480a040008010820ULL,
	0x2194082044002002ULL,
	0x2008a20001004200ULL,
	0x40908041041004ULL,
	0x881002200540404ULL,
	0x4001082002082101ULL,
	0x8110408880880ULL,
	0x8000404040080200ULL,
	0x200020082180080ULL,
	0x1184440400114100ULL,
	0xc220008020110412ULL,
	0x4088084040090100ULL,
	0x8822104100121080ULL,
	0x100111884008200aULL,
	0x2844040288820200ULL,
	0x90901088003010ULL,
	0x1000a218000400ULL,
	0x1102010420204ULL,
	0x8414a3483000200ULL,
	0x6410849901420400ULL,
	0x201080200901040ULL,
	0x204880808050002ULL,
	0x1001008201210000ULL,
	0x16a6300a890040aULL,
	0x8049000441108600ULL,
	0x2212002060410044ULL,
	0x100086308020020ULL,
	0x484241408020421ULL,
	0x105084028429c085ULL,
	0x4282480801080cULL,
	0x81c098488088240ULL,
	0x1400000090480820ULL,
	0x4444000030208810ULL,
	0x1020142010820200ULL,
	0x2234802004018200ULL,
	0xc2040450820a00ULL,
	0x2101021090020ULL
};

/**
 * Generates a magic number candidate.
 * @returns A magic number candidate.
 */
u64 generate_magic_number() {
	return psrandom_u64() & psrandom_u64() & psrandom_u64();
}

/** 
 * Find an appropriate magic number for either a bishop or a rook, given its square
 * and the relevant bits of the occupancy bitboard.
 * @param square The square of the piece.
 * @param relevant_bits The relevant bits of the occupancy bitboard.
 * @param bishop Whether the piece is a bishop or a rook. On usage, use the macro value bishop or rook.
 * @returns The generated magic number.
 */
u64 find_magic_number(int square, int relevant_bits, int bishop) {
	// initialize occupancies
	u64 occupancies[4096];

	// initialize attack tables
	u64 attacks[4096];

	// initialize used attacks
	u64 used_attacks[4096];

	// initialize attack mask for current piece
	u64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

	// init occupancy indices
	int occupancy_indices = 1 << relevant_bits;

	// loop over occupancy indices
	for (int index = 0; index < occupancy_indices; index++) {
		// initialize occupancies
		occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);

		// initialize attacks
		attacks[index] = bishop ? real_time_bishop_attacks(square, occupancies[index]) : 
									real_time_rook_attacks(square, occupancies[index]);
	}

	// test magic numbers
	for (int random_count = 0; random_count < 100000000; random_count++) {
		// generate magic number candidate
		u64 magic_number = generate_magic_number();

		// skip inappropriate magic numbers
		if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

		// init used attacks
		memset(used_attacks, 0ULL, sizeof(used_attacks));

		// init index and fail flag
		int index, fail;

		// text magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indices; index++)
        {
            // init magic index
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            
            // if magic index works
            if (used_attacks[magic_index] == 0ULL)
                // init used attacks
                used_attacks[magic_index] = attacks[index];
            
            // otherwise
            else if (used_attacks[magic_index] != attacks[index])
                // magic index doesn't work
                fail = 1;
        }

		// if magic number works return it
		if (!fail) return magic_number;
	}

	printf("MAGIC NUMBER NOT FOUND!\n");
	return 0ULL;
}

/** 
 * Initialize magic numbers for bishops and rooks.
 */
void init_magic_numbers() {
	for (int square = 0; square < 64; square++) {
		// init bishop magic numbers
		bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
	}
	for (int square = 0; square < 64; square++) {
		// init rook magic numbers
		rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
	}
}

#pragma endregion

#pragma region Initialize all

/**
 * Itialize all necessary data structures.
 */
 */
void init_all() {
	// leapers attack initialization
	init_leapers_attacks();

	/// NOTE: this initialization was made in order to get the values for the magic numbers,
	/// it is nor needed anymore as now the magic numbers are precomputed and hardcoded
	/// for intantaneous initialization of the magic numbers.

	// magic number initialization
	// init_magic_numbers();
}

#pragma endregion

// main function
int main() {
	// init all
	init_all();

	return 0;
}
