
# Blobfish

UCI chess engine written in C++

---

https://lichess.org/@/Blobfish-Bot

## Features
- Evaluation
    - Material score
    - Piece square tables
    - Pawn structure (passed pawns, doubled pawns)
    - Bishop pair bonus
    - Tapered evaluation
- Search
    - Alpha-beta pruning
    - Quiescence search
    - MVV-LVA move ordering
    - Transposition table
    - Iterative deepening
    - Null move pruning
    - Reverse futility pruning
    - Killer heuristic
    - History heuristic
    - Late move reduction
    - Principal variation search
    - Aspiration windows
    - Delta pruning
    - Check extensions
- Board class
    - Bitboards
    - Staged move generation (captures, non-captures)
    - Zobrist hashing

New features are tested using SPRT with cutechess-cli.

## Building

You need g++ and make.

The engine uses BMI2 instructions. So if your CPU is really old the engine might not compile.

---

To compile the engine just run
```bash
make
```

---

### Some useful resources about chess programming I used

https://www.chessprogramming.org

https://talkchess.com/viewforum.php?f=7

https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs
