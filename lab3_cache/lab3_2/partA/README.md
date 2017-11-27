***************README****************
This instance of simulator is designed for single-layer cache simulation.
To build the simulator, simply type
	make
The simulator takes 4 numbers as specs for the cache: c, a, b and w;
	Cache Size:			2^c*(32KB);
	Associativity:		a-way;
	Block (Line) Size:	b bytes;
	Write Policy:		w==0 ? (Write-back && Write-allocate)
							: (Write-through && Non-write-allocate)
