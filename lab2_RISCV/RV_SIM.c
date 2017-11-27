#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdbool.h>
#define memory_size			1 << 29
#define OPCODE_MASK			127
#define RD_MASK				(0b11111 << 7)
#define F3_MASK				(0b111 << 12)
#define RS1_MASK			(0b11111 << 15)
#define RS2_MASK			(0b11111 << 20)
#define IMM10_5_MASK		(0b111111 << 25)
#define F7_MASK				(0b1111111 << 25)
#define MASK_10_1s			(0b1111111111)
#define IMM11_MASK			(0b111111111111 << 20)
#define IMM31_MASK			~(0b111111111111)
#define IMM19_MASK			(0b11111111 << 12)
#define SYS_close 57
#define SYS_lseek 62
#define SYS_brk 214
#define SYS_fstat 80
typedef long Address;
bool single_step = 0;
Address breakpoints[100] = {0};
int breakpoint_cnt = 0;
struct Elf64_Ehdr	//64bytes
{
	unsigned char e_ident[16];
	short e_type;
	short e_machine;
	int e_version;
	long e_entry;
	long e_phoff;
	long e_shoff;
	int e_flags;
	short e_ehsize;
	short e_phentsize;
	short e_phnum;
	short e_shentsize;
	short e_shnum;
	short e_shtrndx;
} *p_ehdr;
struct Elf64_Phdr	//56bytes
{
	int p_type;
	int p_flags;
	long p_offset;
	long p_vaddr;
	long p_paddr;
	long p_filesz;
	long p_memsz;
	long p_align;
} *p_phdr;
struct Elf64_Shdr	//64bytes
{
	int sh_name;
	int sh_type;
	long sh_flags;
	long sh_addr;
	long sh_offset;
	long sh_size;
	int sh_link;
	int sh_info;
	long sh_addralign;
	long sh_entsize;
} *p_shdr;
struct Memory 
{
	char mem[memory_size];
	Address base;
	Address pc;
	Address sp;
	Address heap;
} *p_mem;
struct Register
{
	long reg[32];
	float sfreg[32];
	double dfreg[32];
	int fcsr;
} *p_reg;
FILE *fp = NULL;
int endofexec = 0;
unsigned int instruction;
struct Instruction
{
	unsigned char opcode;
	unsigned char rs1;
	unsigned char rs2;
	unsigned char rd;
	int imm;
	unsigned char func3;
	unsigned char func7;
} p_inst;
void prep_I()
{
	p_inst.func3 = (instruction & F3_MASK) >> 12;
	p_inst.rs1 = (instruction & RS1_MASK) >> 15;
	p_inst.rd = (instruction&RD_MASK) >> 7;
	int midimm = instruction & IMM11_MASK;
	p_inst.imm = midimm >> 20;
}
void prep_R()
{
	p_inst.func3 = (instruction & F3_MASK) >> 12;
	p_inst.rs1 = (instruction & RS1_MASK) >> 15;
	p_inst.rd = (instruction & RD_MASK) >> 7;
	p_inst.func7 = (instruction & F7_MASK) >> 25;
	p_inst.rs2 = (instruction & RS2_MASK) >> 20;
}
void prep_U()
{
	p_inst.rd = (instruction & RD_MASK) >> 7;
	p_inst.imm = ((int)instruction) & IMM31_MASK;
}
void prep_S()
{
	p_inst.func3 = (instruction & F3_MASK) >> 12;
	p_inst.rs1 = (instruction & RS1_MASK) >> 15;
	int imm2 = (instruction & RD_MASK) >> 7;
	int imm1 = (((int)instruction) & F7_MASK) >> 20;
	p_inst.rs2 = (instruction & RS2_MASK) >> 20;
	p_inst.imm = imm2 | imm1;
}
void prep_SB()
{
	p_inst.func3 = (instruction & F3_MASK) >> 12;
	p_inst.rs1 = (instruction & RS1_MASK) >> 15;
	p_inst.rs2 = (instruction & RS2_MASK) >> 20;
	unsigned int imm2 = (instruction & RD_MASK) >> 7;
	unsigned int topbit = (instruction & (1 << 31)) >> 19;
	unsigned int b10_5 = (instruction & (IMM10_5_MASK)) >> 20;
	unsigned int b4_1 = imm2 & (~1);
	unsigned int b11 = (imm2 & 1) << 11;
	unsigned u_imm = topbit | b11 | b10_5 | b4_1;
	p_inst.imm = u_imm << 19;
	p_inst.imm >>= 19;
}
void prep_UJ()
{
	p_inst.rd = (instruction & RD_MASK) >> 7;
	unsigned int topbit = (instruction >> 11) & (1 << 20);
	unsigned int b10_1 = (instruction & (MASK_10_1s << 21)) >> 20;
	unsigned int b11 = (instruction & (1 << 20)) >> 9;
	unsigned int b19_12 = (instruction & IMM19_MASK);
	unsigned int res = topbit | b10_1 | b11 | b19_12;
	p_inst.imm = res << 11;
	p_inst.imm >>= 11;
}
void *mmu(Address address)
{
	int offset = (int)(address - p_mem->base);
	return &(p_mem->mem[offset]);
}
void exec_LOAD()
{
	prep_I();
	Address dest = p_inst.imm + p_reg->reg[p_inst.rs1];
	void* d = mmu(dest);
	switch (p_inst.func3)
	{
	case 0:
		p_reg->reg[p_inst.rd] = (long)*(char*)d;	break;
	case 1:
		p_reg->reg[p_inst.rd] = (long)*(short*)d;	break;
	case 2:
		p_reg->reg[p_inst.rd] = (long)*(int*)d;		break;
	case 3:
		p_reg->reg[p_inst.rd] = *(long*)d;			break;
	case 4:
		p_reg->reg[p_inst.rd] = (long)*(unsigned char*)d;	break;
	case 5:
		p_reg->reg[p_inst.rd] = (long)*(unsigned short*)d;	break;
	case 6:
		p_reg->reg[p_inst.rd] = (long)*(unsigned int*)d;	break;
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}	
	p_mem->pc += 4;
}
void exec_STORE()
{
	prep_S();
	Address dest = p_inst.imm + p_reg->reg[p_inst.rs1];
	void* d = mmu(dest);
	switch (p_inst.func3)
	{
	case 0:
		*(char*)d = (char)p_reg->reg[p_inst.rs2];	break;
	case 1:
		*(short*)d = (short)p_reg->reg[p_inst.rs2];	break;
	case 2:
		*(int*)d = (int)p_reg->reg[p_inst.rs2];		break;
	case 3:
		*(long*)d = p_reg->reg[p_inst.rs2];			break;
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_OP_IMM()
{
	prep_I();
	switch (p_inst.func3)
	{
	case 0:
		p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] + p_inst.imm;	break;
	case 1:
		p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] << p_inst.imm;	break;
	case 2:
		if (p_reg->reg[p_inst.rs1] < p_inst.imm)
			p_reg->reg[p_inst.rd] = 1;
		else
			p_reg->reg[p_inst.rd] = 0;
		break;
	case 3:
		if ((unsigned long)p_reg->reg[p_inst.rs1] < (unsigned long)p_inst.imm)
			p_reg->reg[p_inst.rd] = 1;
		else
			p_reg->reg[p_inst.rd] = 0;
		break;
	case 4:
		p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] ^ (long)p_inst.imm;	break;
	case 5:
	{
		int shamt = p_inst.imm & 63;
		unsigned int stypr = (p_inst.imm >> 11) & 1;
		if (stypr == 0)
			p_reg->reg[p_inst.rd] = ((unsigned long)p_reg->reg[p_inst.rs1]) >> shamt;
		else
			p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] >> shamt;
		break;
	}
	case 6:
		p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] | (long)p_inst.imm;	break;
	case 7:
		p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] & (long)p_inst.imm;	break;
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_OP_IMM_32()
{
	prep_I();
	switch (p_inst.func3)
	{
	case 0:
	{
		long sum = p_inst.imm + p_reg->reg[p_inst.rs1];
		p_reg->reg[p_inst.rd] = (int)sum;
		break;
	}
	case 1:
	{
		int shamt = p_inst.imm & 31;
		int res = ((int)p_reg->reg[p_inst.rs1]) << shamt;
		p_reg->reg[p_inst.rd] = res;
		break;
	}
	case 5:
	{
		int shamt = p_inst.imm & 31;
		int stypr = (p_inst.imm >> 11) & 1;
		if (stypr == 0)
		{
			unsigned int res = ((unsigned int)p_reg->reg[p_inst.rs1]) >> shamt;
			p_reg->reg[p_inst.rd] = res;
		}
		else
		{
			int res = ((int)p_reg->reg[p_inst.rs1]) >> shamt;
			p_reg->reg[p_inst.rd] = res;
		}
		break;
	}
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_AUIPC()
{
	prep_U();
	long imm = p_inst.imm;
	p_reg->reg[p_inst.rd] = p_mem->pc + imm;
	p_mem->pc += 4;
}
void exec_LUI()
{
	prep_U();
	p_reg->reg[p_inst.rd] = p_inst.imm;
	p_mem->pc += 4;
}
void exec_OP()
{
	prep_R();
	int neg_op = (p_inst.func7 >> 5) & 1;
	int is_mul = p_inst.func7 & 1;
	if (!is_mul)
	{
		switch (p_inst.func3)
		{
		case 0:
		{
			if (neg_op)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] - p_reg->reg[p_inst.rs2];
			else
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs2] + p_reg->reg[p_inst.rs1];
			break;
		}
		case 1:
		{
			unsigned int shamt = p_reg->reg[p_inst.rs2] & 63;
			p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] << shamt;
			break;
		}
		case 2:
		{
			if (p_reg->reg[p_inst.rs1] < p_reg->reg[p_inst.rs2])
				p_reg->reg[p_inst.rd] = 1;
			else
				p_reg->reg[p_inst.rd] = 0;
			break;
		}
		case 3:
		{
			if ((unsigned long)p_reg->reg[p_inst.rs1] < (unsigned long)p_reg->reg[p_inst.rs2])
				p_reg->reg[p_inst.rd] = 1;
			else
				p_reg->reg[p_inst.rd] = 0;
			break;
		}
		case 4:
			p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] ^ p_reg->reg[p_inst.rs2];	break;
		case 5:
		{
			int shamt = p_reg->reg[p_inst.rs2] & 63;
			if (neg_op)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] >> shamt;
			else
				p_reg->reg[p_inst.rd] = ((unsigned long)p_reg->reg[p_inst.rs1]) >> shamt;
			break;
		}
		case 6:
			p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] | p_reg->reg[p_inst.rs2];	break;
		case 7:
			p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] & p_reg->reg[p_inst.rs2];	break;
		default:
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		}
	}
	else
	{
		switch (p_inst.func3)
		{
		case 0: 
		{
			__int128 res = p_reg->reg[p_inst.rs1] * p_reg->reg[p_inst.rs2];
			p_reg->reg[p_inst.rd] = res;
			break;
		}
		case 1:
		{
			__int128 res = p_reg->reg[p_inst.rs1] * p_reg->reg[p_inst.rs2];
			p_reg->reg[p_inst.rd] = res >> 64;
			break;
		}
		case 2:
		{
			__uint128_t res = p_reg->reg[p_inst.rs1] * (unsigned long)p_reg->reg[p_inst.rs2];
			p_reg->reg[p_inst.rd] = res >> 64;
			break;
		}
		case 3:
		{
			__uint128_t res = (unsigned long)p_reg->reg[p_inst.rs1] * (unsigned long)p_reg->reg[p_inst.rs2];
			p_reg->reg[p_inst.rd] = res >> 64;
			break;
		}
		case 4:
		{
			if (p_reg->reg[p_inst.rs2] == 0)
				p_reg->reg[p_inst.rd] = -1;
			else
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] / p_reg->reg[p_inst.rs2];
			break;
		}
		case 5:
		{
			if (p_reg->reg[p_inst.rs2] == 0)
				p_reg->reg[p_inst.rd] = -1;
			else
				p_reg->reg[p_inst.rd] = ((unsigned long)p_reg->reg[p_inst.rs1] / (unsigned long)p_reg->reg[p_inst.rs2]);
			break;
		}
		case 6:
		{
			if (p_reg->reg[p_inst.rs2] == 0)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1];
			else
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1] % p_reg->reg[p_inst.rs2];
			break;
		}
		case 7:
		{
			if (p_reg->reg[p_inst.rs2] == 0)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1];
			else
				p_reg->reg[p_inst.rd] = ((unsigned long)p_reg->reg[p_inst.rs1] % (unsigned long)p_reg->reg[p_inst.rs2]);
			break;
		}
		default:
		{
			printf("Error! Undefined instruction!");
			exit(0);
		}
		}
	}
	p_mem->pc += 4;
}
void exec_OP_32()
{
	prep_R();
	int neg_op = (p_inst.func7 >> 5) & 1;
	int is_mul = p_inst.func7 & 1;
	if (!is_mul)
	{
		switch (p_inst.func3)
		{
		case 0:
		{
			if (neg_op)
			{
				int res = (int)p_reg->reg[p_inst.rs1] - (int)p_reg->reg[p_inst.rs2];
				p_reg->reg[p_inst.rd] = res;
			}
			else
			{
				int res = (int)p_reg->reg[p_inst.rs1] + (int)p_reg->reg[p_inst.rs2];
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		case 1:
		{
			unsigned int shamt = p_reg->reg[p_inst.rs2] & 31;
			unsigned int res = ((unsigned int)p_reg->reg[p_inst.rs1]) << shamt;
			p_reg->reg[p_inst.rd] = res;
			break;
		}
		case 5:
		{
			int shamt = p_reg->reg[p_inst.rs2] & 31;
			if (neg_op)
			{
				int res = ((int)p_reg->reg[p_inst.rs1]) >> shamt;
				p_reg->reg[p_inst.rd] = res;
			}
			else
			{
				unsigned int res = ((unsigned int)p_reg->reg[p_inst.rs1]) >> shamt;
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		default:
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		}
	}
	else
	{
		int nrs1 = p_reg->reg[p_inst.rs1];
		int nrs2 = p_reg->reg[p_inst.rs1];
		unsigned int urs1 = p_reg->reg[p_inst.rs1];
		unsigned int urs2 = p_reg->reg[p_inst.rs1];
		switch (p_inst.func3)
		{
		case 0:
		{
			int res = nrs1*nrs2;
			p_reg->reg[p_inst.rd] = res;
			break;
		}
		case 4:
		{
			if (nrs2 == 0)
				p_reg->reg[p_inst.rd] = -1;
			else
			{
				int res = nrs1 / nrs2;
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		case 5:
		{
			if (urs2 == 0)
				p_reg->reg[p_inst.rd] = -1;
			else
			{
				unsigned int res = urs1 / urs2;
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		case 6:
		{
			if (nrs2 == 0)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1];
			else
			{
				int res = nrs1 % nrs2;
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		case 7:
		{
			if (urs2 == 0)
				p_reg->reg[p_inst.rd] = p_reg->reg[p_inst.rs1];
			else
			{
				unsigned int res = urs1 % urs2;
				p_reg->reg[p_inst.rd] = res;
			}
			break;
		}
		default:
		{
			printf("Error! undefined instruction!: %x", instruction);
			exit(0);
		}
		}
	}
	p_mem->pc += 4;
}
void exec_BRANCH()
{
	prep_SB();
	Address target = p_mem->pc + p_inst.imm;
	long nrs1 = p_reg->reg[p_inst.rs1];
	long nrs2 = p_reg->reg[p_inst.rs2];
	unsigned long urs1 = p_reg->reg[p_inst.rs1];
	unsigned long urs2 = p_reg->reg[p_inst.rs2];
	int flag=0;
	switch (p_inst.func3)
	{
	case 0:
		if (nrs1 == nrs2)	flag=1;
		break;
	case 1:
		if (nrs1 != nrs2)	flag=1;
		break;
	case 4:
		if (nrs1 < nrs2)	flag=1;
		break;
	case 5:
		if (nrs1 >= nrs2)	flag=1;
		break;
	case 6:
		if (urs1 < urs2)	flag=1;
		break;
	case 7:
		if (urs1 >= urs2)	flag=1;
		break;
	default:
		printf("Error! Undefined instruction!");
		exit(0);
		break;
	}
	if(flag)
		p_mem->pc=target;
	else
		p_mem->pc+=4;
}
void exec_JALR()
{
	prep_I();
	p_reg->reg[p_inst.rd] = p_mem->pc + 4;
	p_mem->pc = (p_reg->reg[p_inst.rs1] + p_inst.imm) & (~1);
}
void exec_JAL()
{
	prep_UJ();
	p_reg->reg[p_inst.rd] = p_mem->pc + 4;
	p_mem->pc += p_inst.imm;
}
void exec_LOAD_FP()
{
	prep_I();
	Address dest = p_inst.imm + p_reg->reg[p_inst.rs1];
	void* d = mmu(dest);
	switch (p_inst.func3)
	{
	case 2:
		p_reg->sfreg[p_inst.rd] = *(float*)d;
		break;
	case 3:
		p_reg->dfreg[p_inst.rd] = *(double*)d;
		break;
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_STORE_FP()
{
	prep_S();
	Address dest = p_inst.imm + p_reg->reg[p_inst.rs1];
	void* d = mmu(dest);
	switch (p_inst.func3)
	{
	case 2:
		*(float*)d = p_reg->sfreg[p_inst.rs2];
		break;
	case 3:
		*(double*)d = p_reg->dfreg[p_inst.rs2];
		break;
	default:
	{
		printf("Error! undefined instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_OP_FP()
{
	prep_R();
	switch (p_inst.func7)
	{
	case 0x0:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] + p_reg->sfreg[p_inst.rs2]; break;
	case 0x1:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] + p_reg->dfreg[p_inst.rs2]; break;
	case 0x4:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] - p_reg->sfreg[p_inst.rs2]; break;
	case 0x5:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] - p_reg->dfreg[p_inst.rs2]; break;
	case 0x8:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] * p_reg->sfreg[p_inst.rs2]; break;
	case 0x9:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] * p_reg->dfreg[p_inst.rs2]; break;
	case 0xc:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] / p_reg->sfreg[p_inst.rs2]; break;
	case 0xd:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] / p_reg->dfreg[p_inst.rs2]; break;
	case 0x10:
	{
		if (p_inst.func3 == 0)
			p_reg->sfreg[p_inst.rd] = (float)((*(unsigned*)(&p_reg->sfreg[p_inst.rs1]) & 0x7fffffff) | (*(unsigned*)(&p_reg->sfreg[p_inst.rs2]) & 0x80000000));
		else if (p_inst.func3 == 1)
			p_reg->sfreg[p_inst.rd] = (float)((*(unsigned*)(&p_reg->sfreg[p_inst.rs1]) & 0x7fffffff) | ((~*(unsigned*)(&p_reg->sfreg[p_inst.rs2])) & 0x80000000));
		else if (p_inst.func3 == 2)
		{
			unsigned tmp = *(unsigned*)(&p_reg->dfreg[p_inst.rs1]) ^ *(unsigned*)(&p_reg->dfreg[p_inst.rs2]);
			p_reg->sfreg[p_inst.rd] = (float)((*(unsigned*)(&p_reg->dfreg[p_inst.rs1]) & 0x7fffffff) | (tmp & 0x80000000));
		}
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x11:
	{
		if (p_inst.func3 == 0)
			p_reg->dfreg[p_inst.rd] = (double)((*(long*)(&p_reg->dfreg[p_inst.rs1]) & 0x7fffffffffffffff) | (*(long*)(&p_reg->dfreg[p_inst.rs2]) & 0x8000000000000000));
		else if (p_inst.func3 == 1)
			p_reg->dfreg[p_inst.rd] = (double)((*(long*)(&p_reg->dfreg[p_inst.rs1]) & 0x7fffffffffffffff) | ((~*(long*)(&p_reg->dfreg[p_inst.rs2])) & 0x8000000000000000));
		else if (p_inst.func3 == 2)
		{
			long tmp = *(long*)(&p_reg->dfreg[p_inst.rs1]) ^ *(long*)(&p_reg->dfreg[p_inst.rs2]);
			p_reg->dfreg[p_inst.rd] = (double)((*(long*)(&p_reg->dfreg[p_inst.rs1]) & 0x7fffffffffffffff) | (tmp & 0x8000000000000000));
		}
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x14:
	{
		if (p_inst.func3 == 0)
			p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] < p_reg->sfreg[p_inst.rs2] ? p_reg->sfreg[p_inst.rs1] : p_reg->sfreg[p_inst.rs2];
		else if (p_inst.func3 == 1)
			p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] > p_reg->sfreg[p_inst.rs2] ? p_reg->sfreg[p_inst.rs1] : p_reg->sfreg[p_inst.rs2];
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x20:
		p_reg->sfreg[p_inst.rd] = (float)p_reg->dfreg[p_inst.rs1]; break;
	case 0x21:
		p_reg->dfreg[p_inst.rd] = (double)p_reg->sfreg[p_inst.rs1]; 
		break;
	case 0x2c:
		p_reg->sfreg[p_inst.rd] = (float)sqrt((double)p_reg->sfreg[p_inst.rs1]); break;
	case 0x50:
	{
		if (p_inst.func3 == 2)
			p_reg->reg[p_inst.rd] = !(p_reg->sfreg[p_inst.rs1] < p_reg->sfreg[p_inst.rs2] || p_reg->sfreg[p_inst.rs1] > p_reg->sfreg[p_inst.rs2]);
		else if (p_inst.func3 == 1)
			p_reg->reg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] < p_reg->sfreg[p_inst.rs2];
		else if (p_inst.func3 == 0)
			p_reg->reg[p_inst.rd] = !(p_reg->sfreg[p_inst.rs1] > p_reg->sfreg[p_inst.rs2]);
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x51:
	{
		if (p_inst.func3 == 2)
			p_reg->reg[p_inst.rd] = !(p_reg->dfreg[p_inst.rs1] < p_reg->dfreg[p_inst.rs2] || p_reg->dfreg[p_inst.rs1] > p_reg->dfreg[p_inst.rs2]);
		else if (p_inst.func3 == 1)
			p_reg->reg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] < p_reg->dfreg[p_inst.rs2];
		else if (p_inst.func3 == 0)
			p_reg->reg[p_inst.rd] = !(p_reg->dfreg[p_inst.rs1] > p_reg->dfreg[p_inst.rs2]);
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x60:
	{
		if (p_inst.rs2 == 0)
			p_reg->reg[p_inst.rd] = (int)p_reg->sfreg[p_inst.rs1];
		else if (p_inst.rs2 == 1)
			p_reg->reg[p_inst.rd] = (unsigned)p_reg->sfreg[p_inst.rs1];
		else if (p_inst.rs2 == 2)
			p_reg->reg[p_inst.rd] = (long)p_reg->sfreg[p_inst.rs1];
		else if (p_inst.rs2 == 3)
			p_reg->reg[p_inst.rd] = (unsigned long)p_reg->sfreg[p_inst.rs1];
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x61:
	{
		if (p_inst.rs2 == 0)
			p_reg->reg[p_inst.rd] = (int)p_reg->dfreg[p_inst.rs1];
		else if (p_inst.rs2 == 1)
			p_reg->reg[p_inst.rd] = (unsigned int)p_reg->dfreg[p_inst.rs1];
		else if (p_inst.rs2 == 2)
			p_reg->reg[p_inst.rd] = (long)p_reg->dfreg[p_inst.rs1];
		else if (p_inst.rs2 == 3)
			p_reg->reg[p_inst.rd] = (unsigned long)p_reg->dfreg[p_inst.rs1];
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x68:
	{
		if (p_inst.rs2 == 0)
			p_reg->sfreg[p_inst.rd] = (float)((int)p_reg->reg[p_inst.rs1]);
		else if (p_inst.rs2 == 1)
			p_reg->sfreg[p_inst.rd] = (float)((unsigned int)p_reg->reg[p_inst.rs1]);
		else if (p_inst.rs2 == 2)
			p_reg->sfreg[p_inst.rd] = (float)p_reg->reg[p_inst.rs1];
		else if (p_inst.rs2 == 3)
			p_reg->sfreg[p_inst.rd] = (float)((unsigned long)p_reg->reg[p_inst.rs1]);
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;

	}
	case 0x69:
	{
		if (p_inst.rs2 == 0)
			p_reg->dfreg[p_inst.rd] = (double)(int)p_reg->reg[p_inst.rs1];
		else if (p_inst.rs2 == 1)
			p_reg->dfreg[p_inst.rd] = (double)(unsigned)p_reg->reg[p_inst.rs1];
		else if (p_inst.rs2 == 2)
			p_reg->dfreg[p_inst.rd] = (double)p_reg->reg[p_inst.rs1];
		else if (p_inst.rs2 == 3)
			p_reg->dfreg[p_inst.rd] = (double)(unsigned long)p_reg->reg[p_inst.rs1];
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x70:
	{
		if (p_inst.func3 == 0)
		{
			float *a = &p_reg->sfreg[p_inst.rs1];
			p_reg->reg[p_inst.rd] = *(int*)a;
		}
		else if (p_inst.func3 == 1)
		{
			unsigned tmp = *(unsigned*)&p_reg->sfreg[p_inst.rs1];
			unsigned mask_sign = (tmp & 0x80000000) >> 31;
			unsigned mask_exp = tmp & 0x7f800000;
			unsigned mask_frac = tmp & 0x00700000;
			if (mask_sign == 1 && mask_exp == 0x7f800000 && mask_frac == 0)
				p_reg->reg[p_inst.rd] = 1 << 8;
			else if (mask_sign == 1 && mask_exp == 0x7f800000 && mask_frac != 0)
				p_reg->reg[p_inst.rd] = 1;
			else if (mask_sign == 1 && mask_exp != 0)
				p_reg->reg[p_inst.rd] = 2;
			else if (mask_sign == 1 && mask_exp == 0 && mask_frac != 0)
				p_reg->reg[p_inst.rd] = 1 << 2;
			else if (mask_sign == 1 && mask_exp == 0 && mask_frac == 0)
				p_reg->reg[p_inst.rd] = 1 << 3;
			else if (mask_sign == 0 && mask_exp == 0x7f800000 && mask_frac == 0)
				p_reg->reg[p_inst.rd] = 1 << 9;
			else if (mask_sign == 0 && mask_exp == 0x7f800000 && mask_frac != 0)
				p_reg->reg[p_inst.rd] = 1 << 7;
			else if (mask_sign == 0 && mask_exp != 0)
				p_reg->reg[p_inst.rd] = 1 << 6;
			else if (mask_sign == 0 && mask_exp == 0 && mask_frac != 0)
				p_reg->reg[p_inst.rd] = 1 << 5;
			else if (mask_sign == 0 && mask_exp == 0 && mask_frac == 0)
				p_reg->reg[p_inst.rd] = 1 << 4;
		}
		else
		{
			printf("Error! undefined instruction!");
			exit(0);
		}
		break;
	}
	case 0x71:
	{
		double *a = &p_reg->dfreg[p_inst.rs1];
		p_reg->reg[p_inst.rd] = *(long*)a;
		break;
	}
	case 0x78:
	{
		unsigned *a = &p_reg->reg[p_inst.rs1];
		p_reg->sfreg[p_inst.rd] = *(float*)a;
		break;
	}
	case 0x79:
	{
		
		long *a = &p_reg->reg[p_inst.rs1];
		p_reg->dfreg[p_inst.rd] = *(double*)a;
		break;
	}
	}
	p_mem->pc += 4;
}
void exec_MADD()
{
	prep_R();
	switch (p_inst.func7 & 0x3)
	{
	case 0:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] * p_reg->sfreg[p_inst.rs2] + p_reg->sfreg[(p_inst.func7 >> 2)];
		break;
	case 1:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] * p_reg->dfreg[p_inst.rs2] + p_reg->dfreg[(p_inst.func7 >> 2)];
		break;
	default:
	{
		printf("Error! Undefined custom instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_MSUB()
{
	prep_R();
	switch (p_inst.func7 & 0x3)
	{
	case 0:
		p_reg->sfreg[p_inst.rd] = p_reg->sfreg[p_inst.rs1] * p_reg->sfreg[p_inst.rs2] - p_reg->sfreg[(p_inst.func7 >> 2)];
		break;
	case 1:
		p_reg->dfreg[p_inst.rd] = p_reg->dfreg[p_inst.rs1] * p_reg->dfreg[p_inst.rs2] - p_reg->dfreg[(p_inst.func7 >> 2)];
		break;
	default:
	{
		printf("Error! Undefined custom instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_NMADD()
{
	prep_R();
	switch (p_inst.func7 & 0x3)
	{
	case 0:
		p_reg->sfreg[p_inst.rd] = 0 - (p_reg->sfreg[p_inst.rs1] * p_reg->sfreg[p_inst.rs2] + p_reg->sfreg[(p_inst.func7 >> 2)]);
		break;
	case 1:
		p_reg->dfreg[p_inst.rd] = 0 - (p_reg->dfreg[p_inst.rs1] * p_reg->dfreg[p_inst.rs2] + p_reg->dfreg[(p_inst.func7 >> 2)]);
		break;
	default:
	{
		printf("Error! Undefined custom instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void exec_NMSUB()
{
	prep_R();
	switch (p_inst.func7 & 0x3)
	{
	case 0:
		p_reg->sfreg[p_inst.rd] = 0 - (p_reg->sfreg[p_inst.rs1] * p_reg->sfreg[p_inst.rs2] - p_reg->sfreg[(p_inst.func7 >> 2)]);
		break;
	case 1:
		p_reg->dfreg[p_inst.rd] = 0 - (p_reg->dfreg[p_inst.rs1] * p_reg->dfreg[p_inst.rs2] + p_reg->dfreg[(p_inst.func7 >> 2)]);
		break;
	default:
	{
		printf("Error! Undefined custom instruction!");
		exit(0);
	}
	}
	p_mem->pc += 4;
}
void s_close()
{
	close((int)p_reg->reg[10]);
	p_reg->reg[10] = 0;
}
void s_write()
{
	Address cur = p_reg->reg[11];
	Address lim = p_reg->reg[11] + p_reg->reg[12];
	for (Address i = cur;i < lim;i++)
	{
		printf("%c", p_mem->mem[i]);
		fflush(stdin);
	}
	p_reg->reg[10] = p_reg->reg[12];
}
void s_read()
{
	long len = -1;
	char temp_str[1024];
	scanf("%s", temp_str);
	len = strlen(temp_str);
	for (Address i = 0;i <= len;i++)
	{
		p_mem->mem[p_reg->reg[11] + i] = temp_str[i];
	}
	p_reg->reg[10] = len + 1;
}
void s_fstat()
{
	p_reg->reg[10] = fstat((int)p_reg->reg[10], (struct stat*)p_reg->reg[11]);
}
void s_gettimeofday()
{
	struct timeval t;
	if (gettimeofday(&t, NULL) == 0)
	{
		long s = t.tv_sec;
		long u = t.tv_usec;
		Address loc = p_reg->reg[10];
		*(long*)(&p_mem->mem[loc]) = s;
		*(long*)(&p_mem->mem[loc + 8]) = u;
	}
	else
	{
		printf("Call to gettimeofday failed!\n");
	}
}
int exec_SYS()
{
	prep_I();
	if (p_inst.imm != 0)
		exit(0);
	else if (p_reg->reg[17] == 93)
	{
		return 0;
	}
	else
	{
		switch (p_reg->reg[17])
		{
		case 57:
			s_close(); break;
		case 64:
			s_write(); break;
		case 63:
			s_read(); break;
		case 80:
			s_fstat(); break;
		case 169:
			s_gettimeofday(); break;
		default:
			break;
		}
	}
	p_mem->pc += 4;
	return 1;
}
void print_reg()
{
	printf("interger registers:\n");
	for (int j = 0; j < 32; j += 4)
	{
		printf("0x%lx\t0x%lx\t0x%lx\t0x%lx\n", p_reg->reg[j], p_reg->reg[j + 1], p_reg->reg[j + 1], p_reg->reg[j + 1]);
	}
	printf("float registers:\n");
	for (int j = 0; j < 32; j += 4)
	{
		printf("%f\t%f\t%f\t%f\n", p_reg->sfreg[j], p_reg->sfreg[j + 1], p_reg->sfreg[j + 1], p_reg->sfreg[j + 1]);
	}
	printf("double registers:\n");
	for (int j = 0; j < 32; j += 4)
	{
		printf("%lf\t%lf\t%lf\t%lf\n", p_reg->dfreg[j], p_reg->dfreg[j + 1], p_reg->dfreg[j + 1], p_reg->dfreg[j + 1]);
	}
	printf("\n");
}
void print_mem()
{
	long check_mem = 0;
	printf("Please input the memory address you want to check (Press zero to exit):\n");
	while(1)
	{
		scanf("%lx", &check_mem);
		if(check_mem == 0)
		{
			printf("\n");
			break;
		}
		printf("Addr: 0x%lx:  Content: 0x%02hhx\n", check_mem, p_mem->mem[check_mem]);
	}
}
void debug(Address pc)
{
	bool break_flag = false;
	char token[2];
	for (int j = 0; j < breakpoint_cnt; ++j)
	{
		if(breakpoints[j] == pc)
		{
			break_flag = true;
			single_step = true;
			printf("Breakpoint reached!\n");
		}
	}
	if(single_step || break_flag)
	{
		printf("Please input an action. Input \'h\' for help.\n");
		while(1)
		{
			scanf("%s", token);
			//printf("%s\n", token);
			if(strcmp(token, "h") == 0)
			{
				printf("Help:\n");
				printf("p: print reg&mem information\n");
				printf("b: set a breakpoint\n");
				printf("c: continue\n");
				printf("r: run until next breakpoint or the end of the program\n");
				printf("e: exit the program\n");
				continue;
			}
			else if(strcmp(token, "p") == 0)
			{
				print_reg();
				print_mem();
				continue;
			}
			else if(strcmp(token, "e") == 0)
				exit(0);
			else if(strcmp(token, "r") == 0)
			{
				single_step = false;
				fflush(stdin);
				return;
			}
			else if(strcmp(token, "b") == 0)
			{
				Address break_addr = 0;
				scanf("%lx", &break_addr);
				breakpoints[breakpoint_cnt++] = break_addr;
				printf("Breakpoint added!\n");
				fflush(stdin);
				continue;
			}
			else if(strcmp(token, "c") == 0)
				return;
			else
				printf("Wrong action. Input \'h\' for help.\n");
		}
	}
}
void execute()
{
	//printf("%lx\n", p_mem->pc);
	debug(p_mem->pc);
	p_reg->reg[0]=0;
	p_inst.imm = p_inst.func3 = p_inst.func7 = p_inst.opcode = p_inst.rd = p_inst.rs1 = p_inst.rs2 = 0;
	instruction = *(unsigned int *)mmu(p_mem->pc);
	p_inst.opcode = instruction & 127;
	unsigned int optop = (p_inst.opcode & 96) >> 5;			//opcode[6:5]
	unsigned int opbottom = (p_inst.opcode & 28) >> 2;		//opcode[4:2]
	switch (optop)
	{
	case 0:
		switch (opbottom)
		{
		case 0:	exec_LOAD();	break;
		case 1:	exec_LOAD_FP();	break;
		case 4:	exec_OP_IMM();	break;
		case 5:	exec_AUIPC();	break;
		case 6:	exec_OP_IMM_32(); break;
		default:
		{
			printf("Error! Undefined custom instruction!");
			exit(0);
		}
		}
		break;
	case 1:
		switch (opbottom)
		{
		case 0:	exec_STORE();		break;
		case 1:	exec_STORE_FP();	break;
		case 4:	exec_OP();			break;
		case 5:	exec_LUI();			break;
		case 6:	exec_OP_32();		break;
		default:
		{
			printf("Error! Undefined custom instruction!");
			exit(0);
		}
		}
		break;
	case 2:
		switch (opbottom)
		{
		case 0: exec_MADD();	break;
		case 1:	exec_MSUB();	break;
		case 2:	exec_NMSUB();	break;
		case 3:	exec_NMADD();	break;
		case 4:	exec_OP_FP();	break;
		default:
		{
			printf("Error! Undefined custom instruction!");
			exit(0);
		}
		}
		break;
	case 3:
		switch (opbottom)
		{
		case 0:	exec_BRANCH();	break;
		case 1:	exec_JALR();	break;
		case 3:	exec_JAL();		break;
		case 4:	
		{
			int num=exec_SYS();	
			if (num == 0) exit(0);
			break;
		}
		default:
		{
			printf("Error! Undefined custom instruction!");
			exit(0);
		}
		}
		break;
	default:
	{
		printf("Error! Wrong Instruction!\n");
		exit(0);
	}
	}
	#ifdef DEBUG_REGISTER
		print_reg();
	#endif
	#ifdef DEBUG_MEMORY
		print_mem();
	#endif
	#ifdef DEBUG_CONTENT
		print_reg();
		print_mem();
	#endif
}
void Mem(FILE *file)
{
	p_mem = malloc(sizeof(struct Memory));
	memset(p_mem->mem, 0, sizeof(p_mem->mem));
	p_mem->pc = p_ehdr->e_entry;
	p_mem->sp = memory_size - 1;
	p_mem->base = 0;
	char hdr_buf[70];
	char *pro_buf;
	int flag = 0;
	long total = 0;
	for (long i = 0; i < p_ehdr->e_phnum; ++i)
	{
		fseek(file, p_ehdr->e_phoff + i * p_ehdr->e_phentsize, SEEK_SET);
		fread(hdr_buf, 1, 56, file);
		p_phdr = (struct Elf64_Phdr*)hdr_buf;
		if (p_phdr->p_type == 1)
		{
			pro_buf = malloc(p_phdr->p_filesz);
			fseek(file, p_phdr->p_offset, SEEK_SET);
			fread(pro_buf, 1, p_phdr->p_filesz, file);
			memcpy((p_phdr->p_vaddr - p_mem->base) + p_mem->mem, pro_buf, p_phdr->p_filesz);
			total = p_phdr->p_vaddr - p_mem->base + p_phdr->p_memsz;
			free(pro_buf);
		}
	}
	p_mem->heap = total;
}
void Reg()
{
	p_reg = malloc(sizeof(struct Register));
	for(int i=0; i<32; i++)
	{
		p_reg->reg[i]=0;
		p_reg->sfreg[i]=0.0;
		p_reg->dfreg[i]=0.0;
	}
	p_reg->reg[2]=0x2000000;
}
Address load(char *file)
{
	if (!(fp = fopen(file, "rb")))
	{
		printf("Load: Error, can not find the file!\n");
		return 0;
	}
	char buff[70];
	fread(buff, 1, 64, fp);
	p_ehdr = (struct Elf64_Ehdr*)buff;
	if (!(p_ehdr->e_ident[0] == 0x7f && p_ehdr->e_ident[1] == 0x45 && p_ehdr->e_ident[2] == 0x4c && p_ehdr->e_ident[3] == 0x46))
	{
		printf("Load: Error, not an ELF file!\n");
		fclose(fp);
		return 0;
	}
	Mem(fp);
	Reg();
	fclose(fp);
	return p_mem->pc;
}
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Parameter missing!\n");
		return 0;
	}
	if (!load(argv[1]))
		exit(0);
	#ifdef DEBUG
		single_step = true;
	#endif
	while(p_mem->pc>=0x10000)execute();
	if(p_mem->pc<0x10000)printf("Ouch! pc is wrong!\n");
	return 0;
}
