
// system headers
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#ifdef WIN64
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

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

#define empty_fen "8/8/8/8/8/8/8/8 b - -"
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
static inline int lsb_index(u64 bitboard) {
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
void print_bitboard(u64 bitboard) {
	printf("\n     - Bitboard - \n\n");
	// loop over rank and files
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			// convert file and rank into square index
			int square = rank * 8 + file;
			
			if (!file)
				printf(" %d ", 8 - rank);

			// print bit state (either 1 or 0)
			printf(" %d", get_bit(bitboard, square) ? 1 : 0);
		}
		// print new line
		printf("\n");
	}

	printf("\n    A B C D E F G H\n");

		// print decimal value of bitboard
	printf("\n > Base 10:\t%020llud\n", bitboard);

	// print the number of bits of the bitboard
	printf(" > Bit count:\t%d\n", count_bits(bitboard));

	// print the LSB index in the bitboard
	printf(" > LSB index:\t%d\n", lsb_index(bitboard));

	// print the LSB index in the bitboard
	printf(" > LSB coords:\t%s\n", square_to_coordinates[lsb_index(bitboard)]);

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
int open_enpassant = none;

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
int available_castlings;

/** 
 * Print the chess board 
 */
void print_board() {
 	printf("\n   - Chess Board - \n\n");

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
	if (open_enpassant != none) 
		printf(" > En passant open at %s\n", square_to_coordinates[open_enpassant]);

	// print castling rights
	printf(" > Available castlings: %c%c%c%c\n",
		(available_castlings & wk) ? 'K' : '-',
		(available_castlings & wq) ? 'Q' : '-',
		(available_castlings & bk) ? 'k' : '-',
		(available_castlings & bq) ? 'q' : '-'
	);
	printf("\n");
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
	open_enpassant = none;
	available_castlings = 0;

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
			case 'K': available_castlings |= wk; break;
			case 'Q': available_castlings |= wq; break;
			case 'k': available_castlings |= bk; break;
			case 'q': available_castlings |= bq; break;
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
		open_enpassant = rank * 8 + file;
	}
	else {
		open_enpassant = none;
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

#pragma region Attacks

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
		int square = lsb_index(attack_mask);

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
 * @param bishop Whether we are calculating the slider attacks for the bishop or not
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
 * @param square The square to get the bishop attacks from.
 * @param occupancy The occupancy of the board.
 * @return The bishop attacks for the given square.
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
 * @param square The square to get the rook attacks for.
 * @param occupancy The occupancy of the board.
 * @return The rook attacks for the given square.
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
 * @param square The square to get the queen attacks for.
 * @param occupancy The occupancy of the board.
 * @return The queen attacks for the given square.
 */
static inline u64 get_queen_attacks(int square, u64 occupancy) {
	// initialize result attacks bitboard
	return (get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy));
}

/** 
 * Determines whether the given square is being attacked by any piece of the opposite side
 * @param square The square to check.
 * @param side The side to check for.
 * @return Whether the given square is being attacked by any piece of the opposite side.
 */
static inline int is_square_attacked(int square, int side) {
	// attacked by white pawns
	if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;

	// attacked by black pawns
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

	// attacked by knights
	if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
	
	// attacked by bishops
	if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;
	
	// attacked by rooks
	if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;
	
	// attacked by queens
	if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

	// attacked by kings
	if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

	// by default return false
	return 0;
}

/**
 * Print all the attacked squares in the board
 * @param side The side to print the attacked squares for.
 */
void print_attacked_squares(int side) {
	printf("\n     - Attacks -\n\n");
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			if (!file)
				printf(" %d ", 8 - rank);
			printf(" %d", is_square_attacked(square, side) ? 1 : 0);
		}
		printf("\n");
	}

	printf("\n    A B C D E F G H\n");
}

#pragma endregion

#pragma region Board State Preservation

// TODO: take thiese macros to inline funcitons
#define save_board() 																					\
	u64 bitboards_copy[12], occupancies_copy[3]; 														\
	int side_copy, enpassant_copy, available_castlings_copy;											\
	memcpy(bitboards_copy, bitboards, sizeof(bitboards)); 												\
	memcpy(occupancies_copy, occupancies, sizeof(occupancies)); 										\
	side_copy = side, enpassant_copy = open_enpassant, available_castlings_copy = available_castlings; 	\

#define restore_board() 																				\
	memcpy(bitboards, bitboards_copy, sizeof(bitboards_copy)); 											\
	memcpy(occupancies, occupancies_copy, sizeof(occupancies_copy)); 									\
	side = side_copy, open_enpassant = enpassant_copy, available_castlings = available_castlings_copy; 	\

#pragma endregion

#pragma region Move Generation

/* Move encoding
	Binary move representation											Hexadecimal constants

	0000 0000 0000 0000 0011 1111 	source square			6 bits		0x00003F
	0000 0000 0000 1111 1100 0000 	target square			6 bits		0x000FC0
	0000 0000 1111 0000 0000 0000 	piece					4 bits		0x00F000
	0000 1111 0000 0000 0000 0000 	promoted piece			4 bits		0x0F0000
	0001 0000 0000 0000 0000 0000 	capture flag			1 bit		0x100000
	0010 0000 0000 0000 0000 0000 	double pawn push flag	1 bit		0x200000
	0100 0000 0000 0000 0000 0000 	enpassant flag			1 bit		0x400000
	1000 0000 0000 0000 0000 0000 	castling flag			1 bit		0x800000

*/

// encode move macro
#define encode_move(source, target, piece, promoted, capture, double_pawn, enpassant, castle) \
	(source) | 				\
	(target << 6) | 		\
	(piece << 12) | 		\
	(promoted << 16) | 		\
	(capture << 20) | 		\
	(double_pawn << 21) |	\
	(enpassant << 22) |		\
	(castle << 23)

// decode move macros
#define decode_move_source_square(move) 		((move) & 0x3F)
#define decode_move_target_square(move) 		(((move) & 0xFC0) >> 6)
#define decode_move_piece(move) 				(((move) & 0xF000) >> 12)
#define decode_move_promoted_piece(move) 		(((move) & 0xF0000) >> 16)
#define decode_move_capture(move) 				(((move) & 0x100000) >> 20)
#define decode_move_double_pawn_push(move) 		(((move) & 0x200000) >> 21)
#define decode_move_enpassant(move) 			(((move) & 0x400000) >> 22)
#define decode_move_castle(move) 				(((move) & 0x800000) >> 23)

/**
 * Move list holding all the generated moves.
 */
typedef struct {
	int arr[256];
	int last;
} move_list;

/**
 * Adds a move to the move list.
 * @param move_list The move list to add the move to.
 * @param move The move to add.
 */
static inline void add_move(move_list* move_list, int move) {
	move_list->arr[move_list->last++] = move;
}

/** Map to retrieve the promoted piece character from the promoted piece enum. */
char promoted_pieces[] = {
	[Q] = 'q',
	[R] = 'r',
	[B] = 'b',
	[N] = 'n',
	[q] = 'q',
	[r] = 'r',
	[b] = 'b',
	[n] = 'n',
};

/** 
 * Print the UCI move representation of a move.
 * @param move The move to print.
 */
void print_uci_move(int move) {
	printf("%s%s%c", 
		square_to_coordinates[decode_move_source_square(move)],
		square_to_coordinates[decode_move_target_square(move)],
		promoted_pieces[decode_move_promoted_piece(move)]);
}

/**
 * Print all the moves from the move list
 * @param move_list The move list to print.
 */
void print_move_list(move_list* move_list) {
	// do nothing on empty move list
	if (!move_list->last) {
		printf("\nNo moves found in the move list.\n");
		return;
	}


	printf("\nmove\tpiece\tcapture\tdouble\tenpass\tcastling\n\n");

	//loop over the moves in the move list
	for (int move_count = 0; move_count < move_list->last; move_count++) {
		int move = move_list->arr[move_count];
		#ifdef WIN64
			printf("%s%s%c\t%c\t%d\t%d\t%d\t\t%d\n", 
				square_to_coordinates[decode_move_source_square(move)],
				square_to_coordinates[decode_move_target_square(move)],
				promoted_pieces[decode_move_promoted_piece(move)],
				ascii_pieces[decode_move_piece(move)],
				decode_move_capture(move),
				decode_move_double_pawn_push(move),
				decode_move_enpassant(move),
				decode_move_castle(move));
		#else
			printf("%s%s%c\t%s\t%d\t%d\t%d\t%d\n", 
				square_to_coordinates[decode_move_source_square(move)],
				square_to_coordinates[decode_move_target_square(move)],
				promoted_pieces[decode_move_promoted_piece(move)],
				unicode_pieces[decode_move_piece(move)],
				decode_move_capture(move),
				decode_move_double_pawn_push(move),
				decode_move_enpassant(move),
				decode_move_castle(move));
		#endif

	}

	// print the move count in the move list
	printf("\nMove count: %d\n", move_list->last);
}

/**
 * Returns a bitboard containing all the squares attacked by a piece on a given square.
 * @param piece The piece to check.
 * @param square The square to check.
 * @return The bitboard containing all the attacked squares.
 */
static inline u64 attacks_for_piece(int piece, int source_square) {
	u64 attacks;
	switch (piece) {
		case N: case n:
			attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
			break;
		case B: case b:
			attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
			break;
		case R: case r:
			attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
			break;
		case Q: case q:
			attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
			break;
		case K: case k:
			attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
			break;
	}

	return attacks;
}

/**
 * Generates all the moves for a given piece given a bitboard of said piece, and adds them to the move list.
 * @param move_list Move list to add the moves to, if any.
 * @param piece The piece to generate the moves for.
 * @param bitboard The bitboard of the given piece.
 */
static inline void generate_moves_for_piece(move_list* _move_list, int piece, u64 bitboard) {
	// initialize source and target squares
	int source_square, target_square;

	// loop over bitboard for source squares
	while (bitboard) {
		source_square = lsb_index(bitboard);

		// initialize piece attacks
		u64 attacks = attacks_for_piece(piece, source_square);

		// loop over attacks bitboard for target squares
		while (attacks) {
			// initialize target square
			target_square = lsb_index(attacks);

			// quiet move
			if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
				add_move(_move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
			} 
			// capture
			else {
				add_move(_move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
			}

			// pop last target square (attacks lsb)
			pop_bit(attacks, target_square);
		}

		// pop last source square (bitboards lsb)
		pop_bit(bitboard, source_square);
	}
}

/**
 * Generates all pawn specific moves given the pawn bitboard, and adds them to the move list.
 * @param move_list Move list to add the moves to, if any.
 * @param pawn The pawn to generate the moves for.
 * @param bitboard The pawn bitboard.
 */
static inline void generate_pawn_moves(move_list* _move_list, int pawn, u64 bitboard) {
	int queen 		= (side == white) ? Q : q;
	int rook 		= (side == white) ? R : r;
	int bishop 		= (side == white) ? B : b;
	int knight 		= (side == white) ? N : n;
	int opponent 	= (side == white) ? black : white;
	
	while (bitboard) {
		int source_square = lsb_index(bitboard);
		int target_square = (side == white) ? 
			source_square - 8 : 
			source_square + 8 ;

		// quiet pawn moves
		if ((side == white) ? 
				!(target_square < a8) && !get_bit(occupancies[both], target_square) :
				!(target_square > h1) && !get_bit(occupancies[both], target_square)) {
			// handle promotions
			if ((side == white) ?
					(source_square >= a7 && source_square <= h7) :
					(source_square >= a2 && source_square <= h2)) {
				add_move(_move_list, encode_move(source_square, target_square, pawn, queen,  0, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, rook,   0, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, bishop, 0, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, knight, 0, 0, 0, 0));
			} else {
				// handle single pawn move
				add_move(_move_list, encode_move(source_square, target_square, pawn, 0, 0, 0, 0, 0));

				//handle double pawn push
				if ((side == white) ?
						(source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square -= 8) :
						(source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square += 8) ) {
					add_move(_move_list, encode_move(source_square, target_square, pawn, 0, 0, 1, 0, 0));
				}
			}
		}

		// pawn captures
		u64 attacks = pawn_attacks[side][source_square] & occupancies[opponent];

		while (attacks) {
			target_square = lsb_index(attacks);

			// handle promotion captures
			if ((side == white) ?
					(source_square >= a7 && source_square <= h7) :
					(source_square >= a2 && source_square <= h2)) {
				add_move(_move_list, encode_move(source_square, target_square, pawn, queen,  1, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, rook,   1, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, bishop, 1, 0, 0, 0));
				add_move(_move_list, encode_move(source_square, target_square, pawn, knight, 1, 0, 0, 0));
			} 
			// handle regular pawn captures
			else {
				add_move(_move_list, encode_move(source_square, target_square, pawn, 0, 1, 0, 0, 0));
			}

			// pop last tried attack
			pop_bit(attacks, target_square);
		}

		// handle en-passant capture
		if (open_enpassant != none) {
			u64 open_enpassant_attack = pawn_attacks[side][source_square] & (1ULL << open_enpassant);

			// make sure en-passant is available
			if (open_enpassant_attack) {
				target_square = lsb_index(open_enpassant_attack);
				add_move(_move_list, encode_move(source_square, target_square, pawn, 0, 1, 0, 1, 0));
			}
		}

		// pop last tried initial position from bitboard
		pop_bit(bitboard, source_square);
	}
}

/**
 * Generates all caslting moves for the given king.
 * @param move_list Move list to add the moves to, if any.
 * @param pawn The king to generate the castlings for.
 * @param bitboard The king bitboard.
 */
static inline void genreate_castling_moves(move_list* _move_list, int king, u64 bitboard) {
	int king_side_castle	 	= available_castlings & ((side == white) ? wk : bk);
	int queen_side_castle	 	= available_castlings & ((side == white) ? wq : bq);
	int king_side_connected  	= (side == white) ?
		!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1) :
		!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8);
	int queen_side_connected 	= (side == white) ?
		!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1) :
		!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8);
	int no_king_side_checks  	= (side == white) ?
		!is_square_attacked(e1, black) && !is_square_attacked(f1, black) :
		!is_square_attacked(e8, white) && !is_square_attacked(f8, white);
	int no_queen_side_checks 	= (side == white) ?
		!is_square_attacked(e1, black) && !is_square_attacked(d1, black) :
		!is_square_attacked(e8, white) && !is_square_attacked(d8, white);
	int source_square 			= (side == white) ? e1 : e8;
	int ks_target_square 		= (side == white) ? g1 : g8;
	int qs_target_square		= (side == white) ? c1 : c8;

	// handle king-side castling
	if (king_side_castle && king_side_connected && no_king_side_checks) {
		add_move(_move_list, encode_move(source_square, ks_target_square, king, 0, 0, 0, 0, 1));
	}
	//handle queen-side castling
	if (queen_side_castle && queen_side_connected && no_queen_side_checks) {
		add_move(_move_list, encode_move(source_square, qs_target_square, king, 0, 0, 0, 0, 1));
	}
}

/**
 * Generate all pseudo legal moves for the given side.
 * @param _move_list Move list to add the generated moves to, if any.
 */
static inline void generate_moves(move_list* _move_list) {
	// init move count
	_move_list->last = 0;

	int source_square, target_square;
	u64 attacks;

	// loop over all bitboards
	for (int piece = P; piece <= k; piece++) {
		// initialize piece bitboard copy
		u64 bitboard = bitboards[piece];

		// generate pawn moves
		if ((side == white) ? piece == P : piece == p)
			generate_pawn_moves(_move_list, piece, bitboard);

		// generate knight moves
		if ((side == white) ? piece == N : piece == n)
			generate_moves_for_piece(_move_list, piece, bitboard);

		// generate bishop moves
		if ((side == white) ? piece == B : piece == b)
			generate_moves_for_piece(_move_list, piece, bitboard);

		// generate rook moves
		if ((side == white) ? piece == R : piece == r)
			generate_moves_for_piece(_move_list, piece, bitboard);

		// generate queen moves
		if ((side == white) ? piece == Q : piece == q)
			generate_moves_for_piece(_move_list, piece, bitboard);

		// generate king moves
		if ((side == white) ? piece == K : piece == k) {
			generate_moves_for_piece(_move_list, piece, bitboard);
			genreate_castling_moves(_move_list, piece, bitboard);
		}
	}
}

// enum to determine if the search will be done by all moves or only the captures
enum { all_moves, only_captures };

// caslting rights
const int castling_rights[64] = {
	 7, 15, 15, 15,  3, 15, 15, 11,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	13, 15, 15, 15, 12, 15, 15, 14
};

/**
 * Performs a move on the board, if legal.
 * @param move The move to perform.
 * @param move_flag Whether to perform all moves or only captures.
 * @return Whether the move was legal.
 */
static inline int make_move(int move, int moves_flag) {
	// quiet moves
	if (moves_flag == all_moves) {
		// preserve the board state
		save_board();

		// decode move
		int source_square = decode_move_source_square(move);
		int target_square = decode_move_target_square(move);
		int piece = decode_move_piece(move);
		int promoted_piece = decode_move_promoted_piece(move);
		int capture_flag = decode_move_capture(move);
		int double_pawn_push_flag = decode_move_double_pawn_push(move);
		int enpassant_flag = decode_move_enpassant(move);
		int castling_flag = decode_move_castle(move);

		// move piece
		pop_bit(bitboards[piece], source_square);
		set_bit(bitboards[piece], target_square);

		// handle captures
		if (capture_flag) {
			// pick bitboards ranges depending on side
			// int start_piece = (side == white) ? p : P;
			// int end_piece = (side == white) ? k : K;
			int start_piece, end_piece;

			if (side == white) {
				start_piece = p;
				end_piece = k;
			} else {
				start_piece = P;
				end_piece = K;
			}

			// loop over bitboards opposite to the side to move
			for (int opp_piece = start_piece; opp_piece <= end_piece; opp_piece++) {
				// if there´s a piece on the target square, pop that bit and break
				if (get_bit(bitboards[opp_piece], target_square)) {
					pop_bit(bitboards[opp_piece], target_square);
					break;
				}
			}
		}

		// handle pawn promotions
		if (promoted_piece) {
			// pop the pawn and set the piece into the appropriate bitboards
			pop_bit(bitboards[(side == white) ? P : p], target_square);
			set_bit(bitboards[promoted_piece], target_square);
		}

		// hanld en-passant captures
		if (enpassant_flag) {
			(side == white) ? 
				pop_bit(bitboards[p], target_square + 8) : 
				pop_bit(bitboards[P], target_square - 8);
		}

		// clear open en-passant
		open_enpassant = none;

		// handle double pawn pushes
		if (double_pawn_push_flag) {
			open_enpassant = (side == white) ? 
				target_square + 8 : 
				target_square - 8;
		}

		// handle castlings
		if (castling_flag) {
			switch (target_square) {
				case g1:
					pop_bit(bitboards[R], h1);
					set_bit(bitboards[R], f1);
					break;
				case c1:
					pop_bit(bitboards[R], a1);
					set_bit(bitboards[R], d1);
					break;
				case g8:
					pop_bit(bitboards[r], h8);
					set_bit(bitboards[r], f8);
					break;
				case c8:
					pop_bit(bitboards[r], a8);
					set_bit(bitboards[r], d8);
					break;
			}
		}

		// update castling rights
		available_castlings &= castling_rights[source_square];
		available_castlings &= castling_rights[target_square];

		// reset occupancies
		memset(occupancies, 0ULL, sizeof(occupancies));

		// loop over pieces to set occupancies and update afterwards
		for (int piece = P; piece <= K; piece++)
			occupancies[white] |= bitboards[piece];

		for (int piece = p; piece <= k; piece++)
			occupancies[black] |= bitboards[piece];
			
		occupancies[both] |= occupancies[white];
		occupancies[both] |= occupancies[black];

		// change side (this is done with an XOR with 1 because white = 00 and black = 01 in binary)
		side ^= 1;

		// check if the king is not being exposed into check
		if (is_square_attacked(
				((side == white) ? 
					lsb_index(bitboards[k]) : 
					lsb_index(bitboards[K])),
				side)) {
			// move is illegal
			restore_board();

			// return illegal move
			return 0;
		} else {
			// return legal move
			return 1;
		}
	}
	// capture moves
	else {
		// check if the move is a capture
		if (decode_move_capture(move)) {
			make_move(move, all_moves);
		}
		// If the move is not a capture, don´t make the move
		else
			return 0;
	}

	return 0;
}

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

#pragma region Initialize All

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

#pragma region Performace Testing

/**
 * Returns the current time in milliseconds.
 */
int get_time_millis() {
#ifdef WIN64
	return GetTickCount();
#else
	struct timeval time_val;
	gettimeofday(&time_val, NULL);
	return time_val.tv_sec * 1000 + time_val.tv_usec / 1000;
#endif
}

/** Leaf nodes (number of positions reached during the last test of te move generator for a given depth) */
long nodes;

/**
 * Perft debugging function to walk the move generation tree
 * of strictly legal moves to count all the leaf nodes at a
 * certain depth, which can be compared to predetermined
 * values and used to isolate bugs.
 * @param depth Maximum depth of the tree to count the leafs from.
 */
static inline void perft_driver(int depth) {
	// recursion escape condition
	if (depth == 0) {
		// increment nodes count (count reached positions)
		nodes++;
		return;
	}

	// create move list and populate it
	move_list _move_list[1];
	generate_moves(_move_list);

	// loop over generated moves
	for (int i = 0; i < _move_list->last; i++) {
		// preserve board state
		save_board();

		// make move
		if (!make_move(_move_list->arr[i], all_moves)) 
			continue;

		// call perft driver recursively
		perft_driver(depth - 1);

		// restore board state
		restore_board();
	}
}

/**
 * Performance test function to test the efficiency and 
 * accuracy of the engine.
 * @param depth Maximum depth of search to perform the test.
 */
void perft_test(int depth) {
	printf("\n - Performance test - \n");


	// create move list and populate it
	move_list _move_list[1];
	generate_moves(_move_list);

	int start_time = get_time_millis();

	// loop over generated moves
	for (int i = 0; i < _move_list->last; i++) {
		// preserve board state
		save_board();

		int move = _move_list->arr[i];

		// make move
		if (!make_move(move, all_moves)) 
			continue;

		// cummulative nodes
		long prev_nodes = nodes;

		// call perft driver recursively
		perft_driver(depth - 1);

		long curr_nodes = nodes - prev_nodes;

		// restore board state
		restore_board();

		// print the move
		printf(" move: ");
		print_uci_move(move);
		printf("     node: %ld\n", curr_nodes);
	}

	int elapsed = get_time_millis() - start_time;

	// print results
	printf("\n   - Stats - \n");
	printf(  "   depth:      %d\n", depth);
	printf(  "   nodes:      %ld\n", nodes);
	printf(  "   elapsed t:  %d\n", elapsed);
}

#pragma endregion

#pragma region Evaluation

int material_score[12] = {
	   100,	// white pawn
	   300,	// white knight
	   350,	// white bishop
	   500,	// white rook
	  1000,	// white queen
	 10000, // white king
	  -100, // black pawn
	  -300,	// black knight
	  -350,	// black bishop
	  -500,	// black rook
	 -1000,	// black queen
	-10000,	// black king
};

const int pawn_scores[64] = {
	90, 90, 90, 90, 90, 90, 90, 90,
	30, 30, 30, 40, 40, 30, 30, 30,
	20, 20, 20, 30, 30, 30, 20, 20,
	10, 10, 10, 20, 20, 10, 10, 10,
	 5,  5, 10, 20, 20,  5,  5,  5,
	 0,  0,  0,  5,  5,  0,  0,  0,
	 0,  0,  0,-10,-10,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0
};

const int knight_scores[64] = {
	-5,  0,  0,  0,  0,  0,  0, -5,
	-5,  0,  0, 10, 10,  0,  0, -5,
	-5,  5, 20, 20, 20, 20,  5, -5,
	-5, 10, 20, 30, 30, 20, 10, -5,
	-5, 10, 20, 30, 30, 20, 10, -5,
	-5,  5, 20, 10, 10, 20,  5, -5,
	-5,  0,  0,  0,  0,  0,  0, -5,
	-5,-10,  0,  0,  0,  0,-10, -5
};

const int bishop_scores[64] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0, 10, 10,  0,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0, 10,  0,  0,  0,  0, 10,  0,
	 0, 30,  0,  0,  0,  0, 30,  0,
	 0,  0,-10,  0,  0,-10,  0,  0,
};

const int rook_scores[64] = {
	50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0, 10, 20, 20, 10,  0,  0,
	 0,  0,  0, 20, 20,  0,  0,  0,
};

const int king_scores[64] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  5,  5,  5,  5,  0,  0,
	 0,  5,  5, 10, 10,  5,  5,  0,
	 0,  5, 10, 20, 20, 10,  5,  0,
	 0,  5, 10, 20, 20, 10,  5,  0,
	 0,  0,  5, 10, 10,  5,  0,  0,
	 0,  5,  5, -5, -5,  0,  5,  0,
	 0,  0,  5,  0,-15,  0, 10,  0,
};

const int mirror_square[128] = {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

/**
 * Evaluate the position of the board.
 * @return The score of the position.
 */
static inline int evaluate() {
	// static evaluation score
	int score = 0;

	// current pieces bitboard copy
	u64 bitboard;

	int piece, square;

	// loop over all piece bitboards
	for (int bb_piece = P; bb_piece <= k; bb_piece++) {
		// initialize piece bitboard copy
		bitboard = bitboards[bb_piece];

		// loop over all pieces within a bitboard
		while (bitboard) {
			piece = bb_piece;
			square = lsb_index(bitboard);

			// evaluate material weight
			score += material_score[piece];

			// score positional piece scores
			switch (piece) {
				case P: score += pawn_scores[square]; 					break;
				case N: score += knight_scores[square]; 				break;
				case B: score += bishop_scores[square]; 				break;
				case R: score += rook_scores[square]; 					break;
				case K: score += king_scores[square]; 					break;

				case p: score -= pawn_scores[mirror_square[square]]; 	break;
				case n: score -= knight_scores[mirror_square[square]]; 	break;
				case b: score -= bishop_scores[mirror_square[square]]; 	break;
				case r: score -= rook_scores[mirror_square[square]]; 	break;
				case k: score -= king_scores[mirror_square[square]]; 	break;
			}

			// pop lsb
			pop_bit(bitboard, square);
		}
	}

	// return the final evaluation based on side
	return (side == white) ? score : -score;
}

#pragma endregion

#pragma region Search

// most valuable victim & less valuable attacker

/*
                          
    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/
static int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

// killer moves [id][ply]
int killer_moves[2][64];

// history moves [piece][square]
int history_moves[12][64];

// half move counter
int ply;

// best move. This will be replace for the PV (Principal Variation)
int best_move;

static inline int score_move(int move) {
	// score capture move
	if (decode_move_capture(move)) {
        // init target piece
        int target_piece = P;
        
        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;
        
        // pick up side to move
        if (side == white) { start_piece = p; end_piece = k; }
        else { start_piece = P; end_piece = K; }
        
        // loop over bitboards opposite to the current side to move
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
        {
            // if there's a piece on the target square
            if (get_bit(bitboards[bb_piece], decode_move_target_square(move)))
            {
                // remove it from corresponding bitboard
                target_piece = bb_piece;
                break;
            }
        }

		return mvv_lva[decode_move_piece(move)][target_piece] + 10000;
	}

	// socre quiet move
	else {
		// score first killer move
		if (killer_moves[0][ply] == move)
			return 9000;

		// score second killer move
		else if (killer_moves[1][ply] == move)
			return 8000;

		// score history moves
		else 
			return history_moves[decode_move_piece(move)][decode_move_target_square(move)];
	}

	return 0;
}

static inline void sort_moves(move_list *_move_list)
{
    // move scores
    int move_scores[_move_list->last];
	
    // score all the moves within a move list
    for (int count = 0; count < _move_list->last; count++)
        // score move
        move_scores[count] = score_move(_move_list->arr[count]);
    
    // loop over current move within a move list
    for (int current_move = 0; current_move < _move_list->last; current_move++)
    {
        // loop over next move within a move list
        for (int next_move = current_move + 1; next_move < _move_list->last; next_move++)
        {
            // compare current and next move scores
            if (move_scores[current_move] < move_scores[next_move])
            {
                // swap scores
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;
                
                // swap moves
                int temp_move = _move_list->arr[current_move];
                _move_list->arr[current_move] = _move_list->arr[next_move];
                _move_list->arr[next_move] = temp_move;
            }
        }
    }
}

// print move (for UCI purposes)
void print_move(int move)
{
    if (decode_move_promoted_piece(move))
        printf("%s%s%c", square_to_coordinates[decode_move_source_square(move)],
                           square_to_coordinates[decode_move_target_square(move)],
                           promoted_pieces[decode_move_promoted_piece(move)]);
    else
        printf("%s%s", square_to_coordinates[decode_move_source_square(move)],
                           square_to_coordinates[decode_move_target_square(move)]);
}

void print_move_scores(move_list *_move_list)
{
    printf("     Move scores:\n\n");
        
    // loop over moves within a move list
    for (int count = 0; count <= _move_list->last; count++)
    {
        printf("     move: ");
        print_move(_move_list->arr[count]);
        printf(" score: %d\n", score_move(_move_list->arr[count]));
    }
}

int sorting;
int quiescense;

static inline int quiescence(int alpha, int beta) {
	// increment nodes count
	nodes++;

	// quiescence recursion escape conditions
	int evaluation = evaluate();

	// fail-hard beta cutoff
	if (evaluation >= beta)
	{
		// node (move) fails high
		return beta;
	}
	
	// found a better move
	if (evaluation > alpha)
	{
		// PV node (move)
		alpha = evaluation;
	}
	
	// create move list instance
    move_list _move_list[1];
    
    // generate moves
	generate_moves(_move_list);

	// mvv-lva sorting
	sort_moves(_move_list);
    
    // loop over moves within a movelist
    for (int count = 0; count < _move_list->last; count++)
    {
        // preserve board state
        save_board();
        
        // increment ply
        ply++;
        
        // make sure to make only legal moves
        if (make_move(_move_list->arr[count], only_captures) == 0)
        {
            // decrement ply
            ply--;
            
            // skip to next move
            continue;
        }

        // score current move
        int score = -quiescence(-beta, -alpha);
        
        // decrement ply
        ply--;

        // take move back
        restore_board();
        
        // fail-hard beta cutoff
        if (score >= beta)
        {
            // node (move) fails high
            return beta;
        }
        
        // found a better move
        if (score > alpha)
        {
            // PV node (move)
            alpha = score;
        }
    }

	// return the node that fails low
	return alpha;
}

/**
 * Negamax search with alpha-beta pruning. This is the main search function.
 * This searches the tree from the root to the leaf and return the best move.
 * @param alpha The lower bound of the search.
 * @param beta The upper bound of the search.
 * @param depth The depth of the search.
 * @return The best move.
 */
static inline int negamax(int alpha, int beta, int depth)
{
    // recurrsion escape condition
    if (depth == 0)
        // ru quiescence search
        return quiescence(alpha, beta);
    
    // increment nodes count
    nodes++;

	// check if king is in check
	int in_check = is_square_attacked(
		(side == white) 
			? lsb_index(bitboards[K]) 
			: lsb_index(bitboards[k]),
		side ^ 1
	);

	if (in_check) depth++;

	// legal moves counter
	int legal_moves = 0;
    
    // best move so far
    int best_sofar;
    
    // old value of alpha
    int old_alpha = alpha;
    
    // create move list instance
    move_list _move_list[1];
    
    // generate moves
	generate_moves(_move_list);

	// mvv-lva sorting
	sort_moves(_move_list);
    
    // loop over moves within a movelist
    for (int count = 0; count < _move_list->last; count++)
    {
        // preserve board state
        save_board();
        
        // increment ply
        ply++;
        
        // make sure to make only legal moves
        if (make_move(_move_list->arr[count], all_moves) == 0)
        {
            // decrement ply
            ply--;
            
            // skip to next move
            continue;
        }

		// increment legal moves
		legal_moves++;
        
        // score current move
        int score = -negamax(-beta, -alpha, depth - 1);
        
        // decrement ply
        ply--;

        // take move back
        restore_board();
        
        // fail-hard beta cutoff
        if (score >= beta)
        {
			// store killer moves
			killer_moves[1][ply] = killer_moves[0][ply];
			killer_moves[0][ply] = _move_list->arr[count];

            // node (move) fails high
            return beta;
        }
        
        // found a better move
        if (score > alpha)
        {
			// store history moves
			history_moves[decode_move_piece(_move_list->arr[count])][decode_move_target_square(_move_list->arr[count])] += depth;

            // PV node (move)
            alpha = score;
            
            // if root move
            if (ply == 0)
                // associate best move with the best score
                best_sofar = _move_list->arr[count];
        }
    }

	// there are no legal moves to make in this position
	if (legal_moves == 0) {
		// king is in check, return -infinity
		if (in_check)
			return -49000 + ply; // adding ply is necessary in order to avoid stalemate

		// king is NOT in check, return stalemate score, this is a draw
		else
			return 0;
	}
    
    // found better move
    if (old_alpha != alpha)
        // init best move
        best_move = best_sofar;
    
    // node (move) fails low
    return alpha;
}

/**
 * Searches the best move for the current position.
 * @param depth Maximum depth of the search.
 * @return The best move for the current position.
 */
void search_position(int depth) {
	// printf("Searching (depth = %d)...\n", depth);

	// find best move for a given position
	int score = negamax(-50000, 50000, depth);

	if (best_move) {
		printf("> Found best move!\n");
		// best move placeholder
		//printf(/*"info score cp */"%d depth %d nodes %ld\n\n", score, depth, nodes);
		printf("> Bestmove:\t\t");
		print_uci_move(best_move);
		printf("\n");
	}
}

#pragma endregion

#pragma region UCI

/**
 * Parses the given move from a string. (e.g. "e7e8q")
 * @param move_str The string containing the move.
 * @return Whether the move is legal or not
 */
int parse_move(char* move_str) {
	move_list _move_list[1];

	// generate_moves_cmk(_move_list);
	generate_moves(_move_list);

	// parse source and target squares
	int source_square = (move_str[0] - 'a') + ((8 - (move_str[1] - '0')) * 8);
	int target_square = (move_str[2] - 'a') + ((8 - (move_str[3] - '0')) * 8);

	// define parse promoted piece
	int promoted_piece = 0;

	// loop over the moves within the move list
	for (int i = 0; i < _move_list->arr[i]; i++) {
		// init move
		int move = _move_list->arr[i];

		// make sure source and target squares are available within the generated move
		if (decode_move_source_square(move) == source_square && decode_move_target_square(move) == target_square) {
			// initialize promoted piece, if that's the case
			promoted_piece = decode_move_promoted_piece(move);

			// only check this if promoted piece is available
			if (promoted_piece) {
				// promotion to queen
				if ((promoted_piece == Q || promoted_piece == q) && move_str[4] == 'q')
					return move;

				// promotion to rook
				if ((promoted_piece == R || promoted_piece == r) && move_str[4] == 'r')
					return move;

				// promotion to bishop
				if ((promoted_piece == B || promoted_piece == b) && move_str[4] == 'b')
					return move;

				// promotion to knight
				if ((promoted_piece == N || promoted_piece == n) && move_str[4] == 'n')
					return move;

				// continue in case of bad promotion input
				continue;
			}

			// if the move is not a promotion, return it
			return move;
		}
	}

	// return illegal move by default
	return 0;
}

/**
 * Parse UCI "position" command from a given input string.
 * @param input_str The input string.
 */
void parse_position_command(char* input_str) {
	// shift pointer to the right where next token begins
	input_str += 9;

	// initialize the pointer to the current character in the command string
	char* current_char = input_str;

	// parse UCI "startpos" command
	if (strncmp(input_str, "startpos", 8) == 0) {
		// initialize the board with the starting position
		parse_fen(fen_starting_position);
	} else {
		// make sure "fen" is available within command string
		current_char = strstr(input_str, "fen");

		// if no "fen" command is available, return
		if (current_char == NULL)
			parse_fen(fen_starting_position);
		else {
			// shift pointer to the right where next token begins
			current_char += 4;

			// initialize board position from given fen string
			parse_fen(current_char);
		}

		printf("%s\n", current_char);
	}

	// parse moves for position
	current_char = strstr(input_str, "moves");

	// if "moves" command is available
	if (current_char != NULL) {
		// shift pointer to the right where next token begins
		current_char += 6;

		// loop over all moves
		while (*current_char) {
			// parse move
			int move = parse_move(current_char);

			// if there are no moves left, break
			if (move == 0)
				break;

			// make move
			make_move(move, all_moves);

			// shift pointer to the right
			while (*current_char && *current_char != ' ') {
				current_char++;
			}

			// go to the next move
			current_char++;
		}
	}

	// prinf board
	print_board();
}

/**
 * Parse UCI "go" command from a given input string.
 * @param input_str The input string.
 */
void parse_go_command(char *command)
{
    // init depth
    int depth = -1;
    
    // init character pointer to the current depth argument
    char *current_depth = NULL;
    
    // handle fixed depth search
    if (current_depth == strstr(command, "depth"))
        //convert string to integer and assign the result value to depth
        depth = atoi(current_depth + 6);
    
    // different time controls placeholder
    else
        depth = 6;
    
	printf("depth: %d\n", depth);
    // search position
    search_position(depth);
}

/* 
	GUI 	-> isready
	Engine 	-> readyok
	GUI		-> ucinewgame
*/

/**
 * Main UCI loop
 */
void uci_loop() {
	// reset stdin & stdout buffers
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	// define a user / GUI
	char input[2000];

	// print engine info
	printf("id name BBChess\n");
	printf("id author DaniGMX\n");
	printf("uciok\n");

	// main UCI loop
	while (1) {
		// reset GUI input
		memset(input, 0, sizeof(input));

		// make sure output reaches GUI
		fflush(stdout);

		// get user input
		if (!fgets(input, sizeof(input), stdin))
			continue;

		// make sure input is available
		else if (input[0] == '\n')
			continue;

		// parse UCI "isready" command
		else if (strncmp(input, "isready", 7) == 0) {
			printf("readyok\n");
			continue;
		}

		// parse UCI "position" command
		else if (strncmp(input, "position", 8) == 0)
			parse_position_command(input);

		// parse UCI "ucinewgame" command

		else if (strncmp(input, "ucinewgame", 10) == 0)
			parse_position_command("position startpos");

		// parse UCI "go" command
		else if (strncmp(input, "go", 2) == 0)
			parse_go_command(input);

		// parse UCI "quit" command
		else if (strncmp(input, "quit", 4) == 0)
			break;

		// parse UCI "uci" command
		else if (strncmp(input, "uci", 3) == 0) {
			// print engine info
			printf("id name BBChess\n");
			printf("id author DaniGMX\n");
			printf("uciok\n");
		}
	}
}

#pragma endregion

// main function
int main() {
	// initialize all
	init_all();

	// debug mode variable
	int debug = 1;

	if (debug) {
		parse_fen(fen_starting_position	);
		print_board();
		sorting = 1;
		quiescense = 1;
		int depth = 5;

		printf("--- SEARCH ---\n");
		printf("> Quiescense search:\t%s\n", quiescense ? "yes" : "no");
		printf("> MVV-LVA sorting:\t%s\n", sorting ? "yes" : "no");
		printf("> Depth:\t\t%d\n", depth);
		printf("> Begin search!\n");
		printf("> Seraching...\n");
		int start = get_time_millis();
		search_position(depth);
		int end = get_time_millis();
		printf("> Total num of nodes:\t%ld\n", nodes);
		printf("> Total search time:\t%d ms\n", end - start);
	}
	else {
		// conmnect with GUI
		uci_loop();
	}

	return 0;
} 

