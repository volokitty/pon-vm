#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum mnemonics
{
	mov,
	load,
	store,
	add,
	end
};

typedef struct
{
	uint16_t ip;
	uint16_t ax;
	uint16_t bx;
	uint16_t cx;
} registers_t;

uint16_t bytesToTwoByte(uint8_t* buffer, uint8_t index)
{
	return (((uint16_t) buffer[index]) << 8) | ((uint16_t) buffer[index + 1]);
}

void sample(registers_t* registers, uint8_t* instructionBuffer, uint8_t* command, uint8_t* programMemory)
{
	for (uint8_t i = 0; i < 5; i++) instructionBuffer[i] = 0;
	
	memcpy(instructionBuffer, programMemory + registers->ip, 5);
	*command = instructionBuffer[0];
	if (*command == 4) return;
	registers->ip += 5;	
}

void registerWrite(registers_t* dst, uint8_t registerByte, uint16_t src)
{
	switch (registerByte)
	{
		case 0:
			dst->ip = src;
			break;
		case 1:
			dst->ax = src;
			break;
		case 2:
			dst->bx = src;
			break;
		case 3:
			dst->cx = src;
			break;
		default:
			printf("Invalid register byte.\n");
			exit(EXIT_FAILURE);
	}
}

uint16_t registerRead(registers_t* src, uint8_t registerByte)
{
	uint16_t value = 0;
	
	switch (registerByte)
	{
		case 0:
			value = src->ip;
			break;
		case 1:
			value = src->ax;
			break;
		case 2:
			value = src->bx;
			break;
		case 3:
			value = src->cx;
			break;
		default:
			printf("Invalid register byte.\n");
			exit(EXIT_FAILURE);
	}
	
	return value;
}

void axAdd(registers_t* registers, uint8_t value)
{
	registers->ax += value;
}

void e_mov(uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	uint8_t registerByte = instructionBuffer[1];
	uint16_t value = 0;
	
	if (instructionBuffer[2] != 0) value = bytesToTwoByte(instructionBuffer, 3);

	switch (instructionBuffer[2])
	{
		case 0:
			registerWrite(registers, registerByte, registerRead(registers, instructionBuffer[4]));
			break;
		case 1:
			registerWrite(registers, registerByte, value);
			break;
		/*case 2:
			registerWrite(registers, registerByte, (uint16_t) *(memory + value));
			break;*/
		default:
			;printf("Invalid source byte.\n");
			exit(EXIT_FAILURE);	
	}
}

void e_load(uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	uint16_t adress;
	uint8_t registerByte = instructionBuffer[1];	
	
	if (instructionBuffer[2] == 1) adress = registerRead(registers, registerByte);
	else adress = bytesToTwoByte(instructionBuffer, 3);

	registerWrite(registers, registerByte, *(memory + adress));
}

void e_store(uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	uint8_t adress;
	uint8_t registerByte = instructionBuffer[3];

	if (instructionBuffer[1] == 1) adress = registerRead(registers, registerByte);
	else adress = bytesToTwoByte(instructionBuffer, 2);

	*(memory + adress) = (uint8_t) registerRead(registers, instructionBuffer[4]);
}

void e_add(uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	switch (instructionBuffer[1])
	{
		case 0:
			axAdd(registers, registerRead(registers, instructionBuffer[3]));
			break;
		case 1:
			axAdd(registers, instructionBuffer[3]);
			break;
		case 2:
			axAdd(registers, *(memory + bytesToTwoByte(instructionBuffer, 2)));
			break;
		default:
			;printf("Invalid source byte.\n");
			exit(EXIT_FAILURE);
	}
}

void execute(uint8_t command, uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	switch (command)
	{
		case mov:
			;e_mov(instructionBuffer, registers, memory);
			break;
		case load:
			;e_load(instructionBuffer, registers, memory);
			break;
		case store:
			;e_store(instructionBuffer, registers, memory);
			break;
		case add:
			;e_add(instructionBuffer, registers, memory);
			break;
		case end:
			break;
		default:
			;printf("Command 0x%x doesn't exist.\n", command);
			exit(EXIT_FAILURE);
	}	
}

void registersInfo(registers_t* registers)
{
	printf("\nREGISTERS INFO\n");
	printf("ip: 0x%x\n", registers->ip);
	printf("ax: 0x%x\n", registers->ax);
	printf("bx: 0x%x\n", registers->bx);
	printf("cx: 0x%x\n", registers->cx);	
}

void outputBuffer(uint8_t* memory)
{
	printf("\nOUTPUT BUFFER CONTENT\n");
	printf("%s\n", memory);
}

uint16_t getFileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
	uint16_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

char main(uint8_t argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE* program = fopen(argv[1], "rb");
	uint8_t* programMemory = malloc(56 * 1024);

	if (program == (void*) 0)
	{
		printf("Can't open '%s' file\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	fread(programMemory, 1, getFileSize(program), program);

	fclose(program);

	uint8_t* memory = malloc(8 * 1024);
	registers_t* registers = malloc(sizeof(registers_t));		
	uint8_t* instructionBuffer = malloc(5);

	memory[1023] = '\0'; 

	registers->ip = 0;
	registers->ax = 0;
	registers->bx = 0;
	registers->cx = 0;
	
	uint8_t command = programMemory[0];
	while (1) // Instruction cycle
	{
		if (command == 4) break;
		sample(registers, instructionBuffer, &command, programMemory);
		execute(command, instructionBuffer, registers, memory);
	}

	registersInfo(registers);
	outputBuffer(memory);

	free(memory);
	free(registers);
	free(instructionBuffer);	

	exit(EXIT_SUCCESS);
}
