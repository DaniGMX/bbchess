# BB Chess
Bitboard based Chess Engine following the Code Monkey tutorials on YouTube. This project is for learning purposes on chess programming, and will serve as a starting point for developing a non-monolothic chess engine project.

I will  be following all the videos and tutorials step by step and creating C-stylized documentation for each and every function and/or variable so everything is more clear to me and hopefully to anyone who reads and takes a look at my repo.

# Bitboard representation
Bitboards are just unsigned long integers (64 bit integers), this way we can represent a whole chess board from it, each bit representing a square in the chess board. The trick comes when we can represent pieces, tables of attacks of possible moves with these representations.

The main idea is to have several Bitboards and arrays (tables) of bitboards representing different information about the chess board, such as pawn, knight or any piece mask. Here is an example of what this representations look like:

```
- White rooks mask -     - White knights mask -    - White bishops mask -     
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  1 0 0 0 0 0 0 1           0 1 0 0 0 0 1 0           0 0 1 0 0 1 0 0         
         
- White queen mask-      - White king mask -       - White pawns mask-        
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1         
  0 0 0 1 0 0 0 0           0 0 0 0 1 0 0 0           0 0 0 0 0 0 0 0         
         
- Black rooks mask -     - Black knights mask-     - Black bishops mask -   
  1 0 0 0 0 0 0 1           0 1 0 0 0 0 1 0           0 0 1 0 0 1 0 0       
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0       
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0       
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0       
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0       
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0       
  0 0 0 0 0 0 0 0           0 0 0 1 0 0 0 0           0 0 0 0 0 0 0 0       

- Black queen mask-       - Black king mask -       - Black pawns mask-
  0 0 0 1 0 0 0 0           0 0 0 0 1 0 0 0           0 0 0 0 0 0 0 0   
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1  
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0   
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0   
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0   
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0   
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0   
```

The above represents the bitboards for all pieces on the starting position of a chess game. This representation allows us to be able to generate the occupancy bitboards for both white and black pieces, as well as an occupancy bitboard for both sides by simply aplying a logic/bitwise OR operation, and the result would look something like this:

```
- White pieces mask -    - Black pieces mask -      - Both colors mask -     
  0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1           1 1 1 1 1 1 1 1         
  0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1           1 1 1 1 1 1 1 1         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
  1 1 1 1 1 1 1 1           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1         
  1 1 1 1 1 1 1 1           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1         
```

This will come handy for move and capture generation, which will help fasten this process significantly.

# Move encoding
The thing about encoding moves into integers instead of a structure holding the data is (from my view) quite tricky, but performance-wise, I think it is really cohesive taking into account the fact that we are making a bitboard-based chess engine. Not only that, but memory wise, it will perform much better due to memoty management.

How do we encode a move into a 32-bit integer then? Well, we just need to make use of the integer's bits and occupy the minimum number of bits we need to represent each field of the move. We basically only need 6 nibbles total (24 bits) in order to store all the information we need. The occupancy of the bits is shown below:

```
0000 0000 0000 0000 0000 0000 0011 1111 	source square			0x0000003F
0000 0000 0000 0000 0000 1111 1100 0000 	target square			0x00000FC0
0000 0000 0000 0000 1111 0000 0000 0000 	piece					0x0000F000
0000 0000 0000 1111 0000 0000 0000 0000 	promoted piece			0x000F0000
0000 0000 0001 0000 0000 0000 0000 0000 	capture flag			0x00100000
0000 0000 0010 0000 0000 0000 0000 0000 	double pawn push flag	0x00200000
0000 0000 0100 0000 0000 0000 0000 0000 	enpassant flag			0x00400000
0000 0000 1000 0000 0000 0000 0000 0000 	castling flag			0x00800000
```

We only need 6 bits for both source and target squares because there are 64 squares in the chessboard, indexed from 0 (a1) to 63 (h8), and therefore, the maximum number (63) is represented by a minimum of 6 bits. For the piece, as there are 12 pieces, we need a minimum of 4 bits to represent every piece, 4 bits for the promoted piece, in case the move is a promotion, and 1 bit per flag (move is a capture, a double pawn push, an en enpassant move or a clastling move).

# Features
Here is a list with all the features the engine should support when finished:
  * [x] Bitboard representation.
  * [x] Pre-calculated attacks for leaping pieces (pawns, knights and kings).
  * [x] Pre-calculated attacks for sliding pieces (bishops, rooks and queens).
    * [x] On-the-go calculations for sliding pieces.
    * [x] Pseudo-random number generation.
    * [x] Magic bitboards for sliding pieces.
    * [x] Attacks for bishop.
    * [x] Attacks for rook.
    * [x] Attacks fot queen.
  * [x] Board representation in ASCII/Unicode characters with all game information.
  * [x] FEN parsing.
  * [x] Move generation.
    * [x] Pawn promotions.
    * [x] Pawn pushes.
    * [x] Double pawn pushes.
    * [x] Pawn captures.
    * [x] Pawn en-passant captures.
    * [x] Castling moves.
    * [x] Knight moves.
    * [x] Bishop moves.
    * [x] Rook moves.
    * [x] Queen moves.
    * [x] King moves.
  * [x] Encoding moves as integers.
  * [x] Copy/make approach for making moves.
  * [ ] Negamax search with alpha-beta pruning.
  * [ ] PV/killer/history move ordering.
  * [ ] Iterative deepening.
  * [ ] Transposition tables.
  * [ ] Material/position/pawns/mobility/king safety evaluation.
  * [ ] UCI protocol.

# Run the engine
In order to run the program, just run this command on the root folder of the project:

```
make debug && ./bbchess
```

# Sources
* [The playlist][1] from Code Monkey in Chess Programming series on YouTube.
* Bill Jordan. _How to Write a Bitboard Chess Engine: How Chess Programs Work_, Kindle Edition, Jan 20th 2020.



[1]: https://youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs
