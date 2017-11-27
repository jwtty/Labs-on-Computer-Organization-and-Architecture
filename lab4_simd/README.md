Source File: dem1.yuv dem2.yuv
Destination File: result.yuv
Our demo result is saved as "ourdem.yuv"

To run part one without SIMD:
make clean
make
./main 1

To run part two without SIMD:
make clean
make
./main 2

To run part one with MMX:
make clean
make mmx
./main 1

To run part two with MMX:
make clean
make mmx
./main 2

To run part one with SSE:
make clean
make sse
./main 1

To run part two with SSE:
make clean
make sse
./main 2

To run part one with AVX:
make clean
make avx
./main 1

To run part two with AVX:
make clean
make avx
./main 2

Have a nice day~
