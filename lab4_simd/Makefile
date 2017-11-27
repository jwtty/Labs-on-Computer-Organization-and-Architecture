simd: main.o NonSimd.o base.o
	g++ -o main main.o NonSimd.o base.o
main.o: main.cpp yuv.h rgb.h
	g++ -c main.cpp
NonSimd.o: NonSimd.cpp yuv.h rgb.h
	g++ -c NonSimd.cpp
base.o: base.cpp yuv.h rgb.h
	g++ -c base.cpp


.PHONY: clean
clean:
	rm *.o main

.PHONY: mmx
mmx:
	g++ -c -D MMX main.cpp
	g++ -c Mmx.cpp
	g++ -c base.cpp
	g++ -o main main.o Mmx.o base.o

.PHONY: sse
sse:
	g++ -c -D SSE main.cpp
	g++ -c Sse.cpp
	g++ -c base.cpp
	g++ -o main main.o Sse.o base.o

.PHONY: avx
avx:
	g++ -c -D AVX main.cpp
	g++ -c Avx.cpp -mavx2
	g++ -c base.cpp
	g++ -o main main.o Avx.o base.o
