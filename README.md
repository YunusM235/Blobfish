
<div align="center">

<h3>Blobfish</h3>

UCI chess engine written in C++

<br>

[![lichess-rapid](https://lichess-shield.vercel.app/api?username=Blobfish-Bot&format=bullet)](https://lichess.org/@/Blobfish-Bot/perf/bullet)
[![lichess-rapid](https://lichess-shield.vercel.app/api?username=Blobfish-Bot&format=blitz)](https://lichess.org/@/Blobfish-Bot/perf/blitz)
[![lichess-rapid](https://lichess-shield.vercel.app/api?username=Blobfish-Bot&format=rapid)](https://lichess.org/@/Blobfish-Bot/perf/rapid)

<br>

</div>


## Features
- NNUE evaluation
    - basic 768 input features
    - Accumulator of size 256
    - Trained on 210M positions from self-play
    - Trained using [bullet](https://github.com/jw1912/bullet)
    - Inference uses SIMD instructions
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

The engine uses BMI2 and AVX2 instructions. So if your CPU is really old the engine might not compile/run.

---

To compile the engine just run
```bash
make
```

---

### Some useful resources about chess programming I used

https://www.chessprogramming.org

https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs

https://official-stockfish.github.io/docs/nnue-pytorch-wiki/docs/nnue.html

https://github.com/jw1912/bullet/blob/main/docs/1-basics.md

https://talkchess.com/viewforum.php?f=7
