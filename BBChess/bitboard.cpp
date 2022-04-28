#include "globals.h"

Bitboard bit_pawncaptures[2][64];
Bitboard bit_pawndefends[2][64];
Bitboard bit_left[2][64];
Bitboard bit_right[2][64];

Bitboard bit_pawnmoves[2][64];
Bitboard bit_knightmoves[64];
Bitboard bit_bishopmoves[64];
Bitboard bit_rookmoves[64];
Bitboard bit_queenmoves[64];
Bitboard bit_kingmoves[64];

Bitboard bit_pieces[2][7];
Bitboard bit_units[2];
Bitboard bit_all;

Bitboard bit_between[64][64];
Bitboard vectorbits[64][64];
Bitboard mask_vectors[64][8];

Bitboard mask_passed[2][64];
Bitboard mask_path[2][64];

Bitboard mask[64];
Bitboard not_mask[64];
Bitboard mask_cols[64];
Bitboard mask_isolated[64];

Bitboard mask_kingside;
Bitboard mask_queenside;
Bitboard not_a_file;
Bitboard not_h_file;

int pawnplus[2][64];
int pawndouble[2][64];
int pawnleft[2][64];
int pawnright[2][64];

void SetRanks();
void SetRowCol();

void SetBetweenVector();
int GetEdge(int sq, int plus);
void SetBit(Bitboard& bb, int square);
void SetBitFalse(Bitboard& bb, int square);
int NextBit(Bitboard bb);
void PrintBitboard(Bitboard bb);



void SetBit(Bitboard& bb, int square)
{
	bb |= (1ui64 << square);
}

void SetBitFalse(Bitboard& bb, int square) 
{
	bb &= ~mask[square];
}

void SetBits()
{
	SetRanks();
	SetRowCol();
	memset(bit_pawncaptures, 0, sizeof(bit_pawncaptures));
	memset(bit_pawnmoves, 0, sizeof(bit_pawnmoves));
	memset(bit_knightmoves, 0, sizeof(bit_knightmoves));
	memset(bit_bishopmoves, 0, sizeof(bit_bishopmoves));
	memset(bit_rookmoves, 0, sizeof(bit_rookmoves));
	memset(bit_queenmoves, 0, sizeof(bit_queenmoves));
	memset(bit_kingmoves, 0, sizeof(bit_kingmoves));
}

