
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

/*
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
*/

// define colors
enum { white, black };

#pragma endregion

#pragma region Macros

#define get_bit(bitboard, square) (bitboard &  (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

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
	printf("- Base 10:\t%llud\n", bitboard);

	// print bitboard as a board of bits
	printf("- Board:\t   A B C D E F G H\n");

	// loop over rank and files
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			// convert file and rank into square index
			int square = rank * 8 + file;
			
			if (!file)
				printf("\t\t%d ", 8 - rank);

			// print bit state (either 1 or 0)
			printf(" %d", get_bit(bitboard, square) ? 1 : 0);
		}
		// print new line
		printf("\n");
	}
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
 * Bitboard with all bits set except for the A file.
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

// Pawn attacks table [color][square]
u64 pawn_attacks[2][64];

/**
 * Precalculate pawn attacks.
 * @param color The color of the pawn to calculate the attacks from.
 * @param square The square from which to calculate the attacks from.
 */
u64 mask_pawn_attacks(int color, int square) {
	// result attack bitboard
	u64 attacks = 0ULL;
	
	// piece bitboard
	u64 bitboard = 0ULL;
	
	// set piece
	set_bit(bitboard, square);

	// white pawn attacks
	if (color == white) {
		if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
		if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
	}
	// black pawn attacks
	else {
		if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
		if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
	}

	// return attack map
	return attacks;
}

#pragma endregion

// main function
int main() {
	print_bitboard(mask_pawn_attacks(white, e4), "Available white pawn attacks from e4");
	print_bitboard(mask_pawn_attacks(black, e4), "Available black pawn attacks from e4");
	return 0;
}