
// system headers
#include <stdio.h>
#include <time.h>

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

// define colors
enum { white, black };

#pragma endregion

#pragma region Random Number Generation

// generate 32-bit pseudo legal numbers
unsigned int psrandom_u32() {
	// get current state
	unsigned int num = state;

	// XOR shift algorithm
	num ^= num << 13;
	num ^= num >> 17;
	num ^= num << 5;

	// update state
	state = num;

	// return random number
	return num;	
}

// generate 64-bit pseudo legal numbers
u64 psrandom_u64() {
	// define 4 random numbers
	u64 n1, n2, n3, n4;

	// initialize random numbers
	n1 = (u64)(psrandom_u32() & 0xFFFF);
	n2 = (u64)(psrandom_u32() & 0xFFFF);
	n3 = (u64)(psrandom_u32() & 0xFFFF);
	n4 = (u64)(psrandom_u32() & 0xFFFF);

	//return random number
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
u64 magic_number() {
	return psrandom_u64() & psrandom_u64() & psrandom_u64();
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

// pseudorandom number state
unsigned int state = 1804289383;


// main function
int main() {
	// initialize leaper pieces attacks
	init_leapers_attacks();

	print_bitboard((u64)psrandom_u32(), "idk");
	print_bitboard((u64)psrandom_u32() & 0xFFFF, "idk");
	print_bitboard(psrandom_u64() & 0xFFFF, "idk");
	print_bitboard(magic_number(), "pseduo random u64");

	return 0;
}
