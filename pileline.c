#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

/**************************************/

struct Control
{
	unsigned char RegDst;
	unsigned char Jump;
	unsigned char Branch;
	unsigned char MemRead;
	unsigned char MemtoReg;
	unsigned char ALUOp;
	unsigned char MemWrite;
	unsigned char ALUSrc;
	unsigned char RegWrite;
};

struct Reg_Read
{
	unsigned int Read_data_1;
	unsigned int Read_data_2;
};

struct ALU
{
	unsigned char zero; // 1: enable, 0: disable
	unsigned int ALU_result;
};

struct Control control;
struct Reg_Read reg_read;
struct ALU alu;
unsigned int mem[64] = { 0 };
unsigned int reg[32] = { 0 };

/**************************************/

unsigned int Inst_Fetch(unsigned int read_addr);
void Register_Read(unsigned int read_reg_1, unsigned int read_reg_2);
void Control_Signal(unsigned int opcode);
unsigned char ALU_Control_Signal(unsigned char signal);
void ALU_func(unsigned char ALU_control, unsigned int a, unsigned int b);
unsigned int Memory_Access(unsigned char MemWrite, unsigned char MemRead, unsigned int addr, unsigned int write_data);
void Register_Write(unsigned char RegWrite, unsigned int Write_reg, unsigned int Write_data);
unsigned int Sign_Extend(unsigned int inst_16);
unsigned int Shift_Left_2(unsigned int inst);
unsigned int Add(unsigned int a, unsigned int b);
unsigned int Mux(char signal, unsigned int a_0, unsigned int b_1);
void print_reg_mem(void);

/**************************************/

int main(void)
{
	unsigned int pc = 0;
	FILE *fp;
	unsigned int inst = 0;
	unsigned int inst_31_26 = 0;
	unsigned int inst_25_21 = 0;
	unsigned int inst_20_16 = 0;
	unsigned int inst_15_11 = 0;
	unsigned int inst_15_0 = 0;
	unsigned int inst_ext_32 = 0;
	unsigned int inst_ext_shift = 0;
	unsigned int pc_add_4 = 0;
	unsigned int pc_add_inst = 0;
	unsigned int mux_result = 0;
	unsigned char ALU_control = 0;
	unsigned int inst_25_0 = 0;
	unsigned int jump_addr = 0;
	unsigned int mem_result = 0;
	int total_cycle = 0;

	// register initialization
	/**************************************/
	reg[8] = 41621;
	reg[9] = 41621;
	reg[16] = 40;
	/**************************************/

	// memory initialization
	/**************************************/
	mem[40] = 3578;

	/*
	if ( !(fp = fopen("input.txt", "r")) )
	{
		printf("error: file open fail !!\n");
		exit(1);
	}

	while (feof(fp) == false)
	{
		fscanf(fp, "%x", &inst);
		mem[pc] = inst;
		pc = pc + 4;
	}
	*/



	while (1)
	{
		scanf("%x", &inst);
		if (inst == 0)
			break;
		mem[pc] = inst;
		pc = pc + 4;
	}




	/**************************************/

	// control initialization
	/**************************************/
	control.RegDst = 0;
	control.Jump = 0;
	control.Branch = 0;
	control.MemRead = 0;
	control.ALUOp = 0;
	control.MemWrite = 0;
	control.ALUSrc = 0;
	control.RegWrite = 0;
	/**************************************/

	print_reg_mem();

	printf("\n ***** Processor START !!! ***** \n");

	pc = 0;

	while (pc < 64)
	{
		// pc +4
		pc_add_4 = Add(pc, 4);

		// instruction fetch
		inst = Inst_Fetch(pc);
		printf("Instruction = %08x \n", inst);


		// instruction decode
		inst_31_26 = inst >> 26;
		inst_25_21 = (inst & 0x03e00000) >> 21;
		inst_20_16 = (inst & 0x001f0000) >> 16;
		inst_15_11 = (inst & 0x0000f800) >> 11;
		inst_15_0 = inst & 0x0000ffff;
		inst_25_0 = inst & 0x03ffffff;

		//printf("%x, %x, %x, %x, %x, %x", inst_31_26, inst_25_21, inst_20_16, inst_15_11, inst_15_0, inst_25_0);

		if (inst_31_26 == 2)
		{
			printf(">> JUMP\n");
			control.Jump = 1;
			inst_ext_shift = Shift_Left_2(inst_25_0);
			if (control.Jump == 1)
				pc = inst_ext_shift;
		}
		else
		{
			// register read
			Register_Read(reg[inst_25_21], reg[inst_20_16]);
			// create control signal
			Control_Signal(inst_31_26);
			// create ALU control signal
			ALU_control = ALU_Control_Signal(control.ALUOp);
			// ALU
			inst_ext_32 = Sign_Extend(inst_15_0);
			inst_ext_shift = Shift_Left_2(inst_ext_32);//beq때문에
			mux_result = Mux(control.ALUSrc, reg[inst_20_16], inst_ext_32);
			ALU_func(ALU_control, reg[inst_25_21], mux_result);
			// memory access
			mem_result = Memory_Access(control.MemWrite, control.MemRead, alu.ALU_result, reg_read.Read_data_2);
			// register write
			mux_result = Mux(control.RegDst, inst_20_16, inst_15_11);
			if (control.MemtoReg == 0)//R-add if문이 먹스역할
				Register_Write(control.RegWrite, mux_result, alu.ALU_result);
			else if (control.MemtoReg == 1)//I-lw
				Register_Write(control.RegWrite, mux_result, mem_result);

			if (inst_31_26 == 4)//BEQ
			{
				printf(">> BEQ\n");
				ALU_func(0010, pc_add_4, inst_ext_shift);
				if (alu.zero == 1)
				{
					mux_result = Mux(control.Branch, pc_add_4, alu.ALU_result);
					pc = mux_result;
				}
				else
					pc = pc_add_4;
			}
			else
				pc = pc_add_4;
		}

		if (inst_31_26 == 0)
			printf(">> ADD\n");
		else if (inst_31_26 == 35)
			printf(">> Load\n");
		else if (inst_31_26 == 43)
			printf(">> Store\n");

		total_cycle++;

		// result
		/********************************/
		printf("PC : %d \n", pc);
		printf("Total cycle : %d \n", total_cycle);
		print_reg_mem();
		/********************************/

		//system("pause");
	}

	printf("\n ***** Processor END !!! ***** \n");



	return 0;
}

unsigned int Inst_Fetch(unsigned int read_addr)
{
	return mem[read_addr];
}

void Register_Read(unsigned int read_reg_1, unsigned int read_reg_2)
{
	reg_read.Read_data_1 = read_reg_1;
	reg_read.Read_data_2 = read_reg_2;
}

void Control_Signal(unsigned int opcode)
{
	if (opcode == 0)
	{
		control.RegDst = 1;
		control.Jump = 0;
		control.Branch = 0;
		control.MemRead = 0;
		control.MemtoReg = 0;
		control.ALUOp = 10;
		control.MemWrite = 0;
		control.ALUSrc = 0;
		control.RegWrite = 1;
	}
	else if (opcode == 35)
	{
		control.RegDst = 0;
		control.Jump = 0;
		control.Branch = 0;
		control.MemRead = 1;
		control.MemtoReg = 1;
		control.ALUOp = 00;
		control.MemWrite = 0;
		control.ALUSrc = 1;
		control.RegWrite = 1;
	}
	else if (opcode == 43)
	{
		control.RegDst = '\0';
		control.Jump = 0;
		control.Branch = 0;
		control.MemRead = 0;
		control.MemtoReg = '\0';
		control.ALUOp = 00;
		control.MemWrite = 1;
		control.ALUSrc = 1;
		control.RegWrite = 0;
	}
	else if (opcode == 4)
	{
		control.RegDst = '\0';
		control.Jump = 0;
		control.Branch = 1;
		control.MemRead = 0;
		control.MemtoReg = '\0';
		control.ALUOp = 01;
		control.MemWrite = 0;
		control.ALUSrc = 0;
		control.RegWrite = 0;
	}
}

unsigned char ALU_Control_Signal(unsigned char signal)
{
	unsigned char ALUOp0 = 0, ALUOp1 = 0;
	ALUOp0 = signal % 10;
	signal = signal / 10;
	ALUOp1 = signal % 10;

	if (ALUOp1 == 0 && ALUOp0 == 0)
		return 0010;
	else if (ALUOp1 == 0 && ALUOp0 == 1)//Beq
		return 0110;
	else if (ALUOp1 == 1 && ALUOp0 == 0)//R-add
		return 0010;
}

void ALU_func(unsigned char ALU_control, unsigned int a, unsigned int b)
{
	if (control.ALUOp == 10 && ALU_control == 0010)//add
	{
		alu.ALU_result = a + b;
		alu.zero = 0;
	}
	else if (control.ALUOp == 00 && ALU_control == 0010)//lw
	{
		alu.ALU_result = a + b;
		alu.zero = 0;
	}
	else if (control.ALUOp == 01 && ALU_control == 0110)//beq
	{
		alu.ALU_result = a - b;
		if (alu.ALU_result == 0)
			alu.zero = 1;
	}
	else if (control.ALUOp == 01 && ALU_control == 0010)//beq add
	{
		alu.ALU_result = a + b;
	}
}

unsigned int Memory_Access(unsigned char MemWrite, unsigned char MemRead, unsigned int addr, unsigned int write_data)
{
	if (MemRead == 1)//I-lw
	{
		return mem[addr];
	}
	if (MemWrite == 1)//I-sw
	{
		mem[addr] = write_data;
		return;
	}
}

void Register_Write(unsigned char RegWrite, unsigned int Write_reg, unsigned int Write_data)
{
	if (RegWrite == 1)
		reg[Write_reg] = Write_data;
}

unsigned int Sign_Extend(unsigned int inst_16)
{
	unsigned int inst_32 = 0;
	if ((inst_16 & 0x00008000)) // minus
	{
		inst_32 = inst_16 | 0xffff0000;
	}
	else // plus
	{
		inst_32 = inst_16;
	}

	return inst_32;
}

unsigned int Shift_Left_2(unsigned int inst)
{
	return inst << 2;
}

unsigned int Mux(char signal, unsigned int a_0, unsigned int b_1)
{
	if (signal == 0)
		return a_0;
	else if (signal == 1)
		return b_1;
	else
		return -1;
}

unsigned int Add(unsigned int a, unsigned int b)
{
	return a + b;
}

void print_reg_mem(void)
{
	int reg_index = 0;
	int mem_index = 0;

	printf("\n===================== REGISTER =====================\n");

	for (reg_index = 0; reg_index < 8; reg_index++)
	{
		printf("reg[%02d] = %08d        reg[%02d] = %08d        reg[%02d] = %08d        reg[%02d] = %08d \n",
			reg_index, reg[reg_index], reg_index + 8, reg[reg_index + 8], reg_index + 16, reg[reg_index + 16], reg_index + 24, reg[reg_index + 24]);
	}

	printf("===================== REGISTER =====================\n");

	printf("\n===================== MEMORY =====================\n");

	for (mem_index = 0; mem_index < 32; mem_index = mem_index + 4)
	{
		printf("mem[%02d] = %012d        mem[%02d] = %012d \n",
			mem_index, mem[mem_index], mem_index + 32, mem[mem_index + 32]);
	}
	printf("===================== MEMORY =====================\n");
}
