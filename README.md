Bot for HACK*MATCH minigame of [EXAPUNKS](http://www.zachtronics.com/exapunks)

[557,400 points](https://www.youtube.com/watch?v=vauEdAkAXSE)

### Build

```
clang++ -O3 -Wall -Werror -Wextra -std=c++17 board.cpp x11_handling.cpp solver.cpp main.cpp -lX11 -lXtst
```

### Prereq

```
sudo apt install libx11-dev
sudo apt install libxtst-dev
```
Make EXAPUNKS window 1600x900, run the binary, free cheeve
