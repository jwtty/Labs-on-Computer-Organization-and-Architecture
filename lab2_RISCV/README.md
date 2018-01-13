#RISCV64-simulator
An instruction level RISCV64-simulator written for practice

Anthor:
	Wantong Jiang <1400012901@pku.edu.cn>
	Pin Xu <pile@pku.edu.cn>

Instruction:
1. Type "make" to build a normal mode executive file named RV_SIM, then you can use this simulator to execute RISCV RV64I programs.

2. Other make options include:
	make debug_memory: Print memory content you want after the execution of each 						instruction. You need to input the address.
	make debug_register: Print register information.
	make debug_content: Print register information and the memory content you want.
	make debug: Single-step debugging mode, you can also add breakpoints. Follow the 				information printed on the screen.
	make clean: Remove all intermediate files.

3. There are some test case under directory "test".
