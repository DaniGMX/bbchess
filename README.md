# BB Chess
Bitboard based Chess Engine following the Code Monkey tutorials on YouTube. This project is for learning purposes on chess programming, and will serve as a starting point for developing a non-monolothic chess engine project.

I will  be following all the videos and tutorials step by step and creating C-stylized documentation for each and every function and/or variable so everything is more clear to me and hopefully to anyone who reads and takes a look at my repo.

# Features
Here is a list with all the features the engine should support when finished:
  * [x] Bitboard representation.
  * [x] Pre-calculated attacks for leaping pieces (pawns, knights and kings)
  * [x] Pre-calculated attacks for sliding pieces (bishops, rooks and queens).
    * [x] On-the-go calculations for sliding pieces.
    * [x] Pseudo-random number generation.
    * [x] Magic bitboards for sliding pieces.
  * [ ] Board representation in ASCII/Unicode characters with all game information.
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
