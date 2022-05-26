
// system headers
#include <stdio.h>
#include <time.h>
#include <memory.h>

#pragma region Type Definitions

#define u64 unsigned long long

#pragma endregion

#pragma region Constants & Enumerations

// define bitboard squares
enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, none
};

// castling enumerations
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

/** Square coordinates array to access by square index */
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
enum { white, black, both };

// define sliding pieces enumerations
enum { rook, bishop };

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// ASCII pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";

// Unicode pieces
char* unicode_pieces[12] = { "♙", "♘", "♗", "♖", "♕", "♔", "♟", "♞", "♝", "♜", "♛", "♚" };

// convert ASCII character pieces to encoded constants
int char_pieces[] = {
	['P'] = P,
	['N'] = N,
	['B'] = B,
	['R'] = R,
	['Q'] = Q,
	['K'] = K,
	['p'] = p,
	['n'] = n,
	['b'] = b,
	['r'] = r,
	['q'] = q,
	['k'] = k
};

#define empty_fen "8/8/8/8/8/8/8/8 w - -"
#define fen_starting_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define fen_tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define fen_killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define fen_cmk_position "2rq1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"

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

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square))) // old -> (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

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

	printf("\n\t\t   A B C D E F G H\n");
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

/** Table containing the bitboards of both colors pawn attacks from each square. */
u64 pawn_attacks[2][64];

/** Array containing the bitboards of knight attacks from each square. */
u64 knight_attacks[64];

/** Array containing the bitboards of king attacks from each square. */
u64 king_attacks[64];

/** Bishop attacks masks */
u64 bishop_masks[64];

/** Rook attacks masks */
u64 rook_masks[64];

/** Bishop attacks table [square][occupancies] */
u64 bishop_attacks[64][512];

/** Rook attacks table [square][occupancies] */
u64 rook_attacks[64][4096];

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
u64 bishop_attacks_on_the_go(int square, u64 blocked) {
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
u64 rook_attacks_on_the_go(int square, u64 blocked) {
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

/**
 * Initialize the sliding piece attacks.
 */
void init_sliders_attacks(int bishop) {
	// loop over all squares
	for (int square = 0; square < 64; square++) {
		// init bishop & rook masks
		bishop_masks[square] = mask_bishop_attacks(square);
		rook_masks[square] = mask_rook_attacks(square);

		// init current mask
		u64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

		// init relevant occupancy bit count
		int relevant_bits_count = count_bits(attack_mask);

		// init occupancy mask
		int occupancy_indices = (1 << relevant_bits_count);

		// loop over occupancy indices
		for (int index = 0; index < occupancy_indices; index++) {
			// initialize current occupancy variation
			u64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

			// initialize magic index
			int magic_index = bishop ? 
				(occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]) :
				(occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
			
			if (bishop) {
				// init bishop attacks
				bishop_attacks[square][magic_index] = bishop_attacks_on_the_go(square, occupancy);
			} else {
				rook_attacks[square][magic_index] = rook_attacks_on_the_go(square, occupancy);
			}
		}
	}
}

/** 
 * Get bishop attacks for a given square. This function must be inline for better performance 
 * because it will be called many times during move generation.
 */
static inline u64 get_bishop_attacks(int square, u64 occupancy) {
	// get bishop attacks assuming current board occupancy
	occupancy &= bishop_masks[square];
	occupancy *= bishop_magic_numbers[square];
	occupancy >>= 64 - bishop_relevant_bits[square];

	return bishop_attacks[square][occupancy];
}

/** 
 * Get rook attacks for a given square. This function must be inline for better performance 
 * because it will be called many times during move generation.
 */
static inline u64 get_rook_attacks(int square, u64 occupancy) {
	// get rook attacks assuming current board occupancy
	occupancy &= rook_masks[square];
	occupancy *= rook_magic_numbers[square];
	occupancy >>= 64 - rook_relevant_bits[square];

	return rook_attacks[square][occupancy];
}

/** 
 * Get queen attacks for a given square. This function must be inline for better performance 
 * because it will be called many times during move generation.
 */
static inline u64 get_queen_attacks(int square, u64 occupancy) {
	// initialize result attacks bitboard
	return (get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy));
}



// get rook attacks

#pragma endregion

#pragma region Magic Numbers

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
		attacks[index] = bishop ? bishop_attacks_on_the_go(square, occupancies[index]) : 
									rook_attacks_on_the_go(square, occupancies[index]);
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

	printf("MAGIC NUMBERS NOT FOUND!\n");
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
void init_all() {
	// initialize leaper pieces attacks table
	init_leapers_attacks();

	// initialize slider pieces attacks
	init_sliders_attacks(bishop);
	init_sliders_attacks(rook);

	/// NOTE: this initialization was made in order to get the values for the magic numbers,
	/// it is nor needed anymore as now the magic numbers are precomputed and hardcoded
	/// for intantaneous initialization of the magic numbers.

	// magic number initialization
	// init_magic_numbers();
}

#pragma endregion

#pragma region Chess Board Representation

/**
 * Array of bitboards containing each piece's bitboard. There are 12 bitboards,
 * one for each piece and color combination.
*/
u64 bitboards[12];

/**
 * Array of bitboards containing the occupancies of the White, Black and Both color pieces, in that order.
 * Access to each occupancy by the enum { white, black, both }.
 */
u64 occupancies[3];

/** Defines who plays next (White or Black) */
int side;

/** Stores the possible en-passant move for the next turn. */
int enpassant = none;

/** 
 * Castling rights. They are defined by the macros [wk = 1, wq = 2, bq = 4, bq = 8].
 * This representatn helps denote each castling right as a mask:
 * 
 * wk = 1 ->	0001
 * wq = 2 -> 	0010
 * bk = 4 ->	0100
 * bq = 8 -> 	1000
 * 
 * This way, toggling their values can be simply done by applying logic operations with them.
 */
int castling_rights;

/** Print the chess board */
void print_board() {
 	printf("\n");

	// loop over rank and files
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			// init square we are at
			int square = rank * 8 + file;

			// print ranks
			if (!file) 
				printf(" %d ", 8 - rank);

			//define piece
			int piece = -1;

			// loop over all piece bitboards
			for (int bb = P; bb <= k; bb++) {
				if (get_bit(bitboards[bb], square))
					piece = bb;
			}

			#ifdef WIN64
				printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
			#else
				printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
			#endif
		}
		printf("\n");
	}
	printf("\n    A B C D E F G H\n");

	// print side to move
	printf("\n > %s to move.\n", (!side) ? "White" : "Black");

	// print en passant
	if (enpassant != none) 
		printf(" > En passant open at %s\n", square_to_coordinates[enpassant]);

	// print castling rights
	printf(" > Available castlings: %c%c%c%c\n",
		(castling_rights & wk) ? 'K' : '-',
		(castling_rights & wq) ? 'Q' : '-',
		(castling_rights & bk) ? 'k' : '-',
		(castling_rights & bq) ? 'q' : '-'
	);
}

/**
 * Parses a given FEN string and initializes the board from it.
 */
void parse_fen(char* fen) {
	// reset board position (bitboards)
	memset(bitboards, 0ULL, sizeof(bitboards));

	// reset occupancies (bitboards)
	memset(occupancies, 0ULL, sizeof(occupancies));

	// reset game state variables
	side = 0;
	enpassant = none;
	castling_rights = 0;

	// loop over board ranks
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			// initialize current square
			int square = rank * 8 + file;

			// match ascii pieces within FEN string
			if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
				// initialize piece type
				int piece = char_pieces[*fen];

				// set piece on corresponding bitboard
				set_bit(bitboards[piece], square);

				// increment pointer of FEN string
				fen++;
			}

			// match empty square numbers within FEN string
			if (*fen >= '0' && *fen <= '9') {
				// init offset variable (convert digit char to int)
				int offset = *fen - '0';

				//define piece
				int piece = -1;

				// loop over all piece bitboards
				for (int bb = P; bb <= k; bb++) {
					// if there is a piece on current square
					if (get_bit(bitboards[bb], square))
						// get piece code
						piece = bb;
				}
				
				// on empty current square, decrement file
				if (piece == -1)
					file--;

				// adjust file counter
				file += offset;

				// increment pointer of FEN string
				fen++;
			}

			// match rank separator
			if (*fen == '/') 
				fen++;
		}
	}

	// skip empty space in FEN string
	fen++;

	// parse side to move
	side = (*fen == 'w') ? white : black;

	// skip empty space
	fen += 2;

	// parse castling righs
	while (*fen != ' ') {
		switch(*fen) {
			case 'K': castling_rights |= wk; break;
			case 'Q': castling_rights |= wq; break;
			case 'k': castling_rights |= bk; break;
			case 'q': castling_rights |= bq; break;
			case '-': break;
		}
		fen++;
	}

	// skip blank space
	fen++;

	// parse en-passant square
	if (*fen != '-') {
		// parse en-passan file and rank
		int file = fen[0] - 'a';
		int rank = 8 - (fen[1] - '0');

		// initialize en-passant square
		enpassant = rank * 8 + file;
	}
	else {
		enpassant = none;
	}

	// initialize white occupancies
	for (int piece = P; piece <= K; piece++) {
		// populate white occupancy bitboard
		occupancies[white] |= bitboards[piece];
	}

	// initialize black occupancies
	for (int piece = p; piece <= k; piece++) {
		// populate white occupancy bitboard
		occupancies[black] |= bitboards[piece];
	}

	// initialize all occupancy
	occupancies[both] = occupancies[white] | occupancies[black];
}

#pragma endregion

// main function
int main() {
	// init all
	init_all();

	// init occupancy bitboard
	u64 occ = 0ULL;

	set_bit(occ, b6);
	set_bit(occ, d6);
	set_bit(occ, f4);

	// get queen attacks
	print_bitboard(get_queen_attacks(d4, occ), "Queen attacks on d4");

	return 0;
} 
