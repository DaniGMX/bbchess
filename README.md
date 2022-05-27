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
- White pieces mask -     - Black pieces mask -      - Both colors mask -     
   0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1           1 1 1 1 1 1 1 1         
   0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1           1 1 1 1 1 1 1 1         
   0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
   0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
   0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0           0 0 0 0 0 0 0 0         
   1 1 1 1 1 1 1 1           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1         
   1 1 1 1 1 1 1 1           0 0 0 0 0 0 0 0           1 1 1 1 1 1 1 1         
```

This will come handy for move and capture generation, which will help fasten this process significantly.

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
    * [x] Pawn moves.
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
  * [ ] Encoding moves as integers.
  * [ ] Copy/make approach for making moves.
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
