
// system headers
#include <stdio.h>

// define bitboard types
#define u64 unsigned long long

// print bitboard function
void print_bitboard(u64 bitboard) {
	// loop over the board ranks
	for (int rank = 0; rank < 8; rank++) {
		// loop over the board files
		for (int file = 0; file < 8; file++) {
			// convert file and rank into square index
			int square = rank * 8 + file;
			
			// print the square
			printf(" %2d ", bitboard & (1ULL << square) ? 1 : 0);
		}
		// print new line
		printf("\n");
		printf("\n");
	}
}

int main() {
	printf("--- Bitboard Chess ---\n");
	
	u64 bitboard = 0ULL;
	print_bitboard(bitboard);

	return 0;
}