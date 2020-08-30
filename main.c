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

void sample(registers_t* registers, uint8_t* instructionBuffer, uint8_t* command, FILE* fileStream)
{
	for (uint8_t i = 0; i < 5; i++) instructionBuffer[i] = 0;

	uint8_t instructionLength = 0;
	fread(instructionBuffer, 1, 1, fileStream);
	
	*command = instructionBuffer[0];

	switch (*command)
	{
		case mov:
			;instructionLength = 4;
			registers->ip += instructionLength + 1;
			fread(instructionBuffer + 1, 1, instructionLength, fileStream);	
			break;
		case end:
			;fread(instructionBuffer, 1, 1, fileStream);
			break;
		default:
			break;
	}
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

void e_mov(uint8_t* instructionBuffer, registers_t* registers, uint8_t* memory)
{
	uint8_t registerByte = instructionBuffer[1];
	uint16_t value = 0;
	
	if (instructionBuffer[2] != 0) value = (((uint16_t) instructionBuffer[3]) << 8) | ((uint16_t) instructionBuffer[4]); // Converting two consecutive bytes to a two-byte number

	switch (instructionBuffer[2])
	{
		case 0:
			registerWrite(registers, registerByte, registerRead(registers, instructionBuffer[4]));
			break;
		case 1:
			registerWrite(registers, registerByte, value);
			break;
		case 2:
			registerWrite(registers, registerByte, (uint16_t) *(memory + value));
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
		case end:
			break;
		default:
			;printf("Command %x doesn't exist.\n", command);
			exit(EXIT_FAILURE);
	}	
}

char main(uint8_t argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE* program = fopen(argv[1], "rb");

	if (program == (void*) 0)
	{
		printf("Can't open '%s' file\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	uint8_t* memory = malloc(8 * 1024);
	registers_t* registers = malloc(sizeof(registers_t));		
	uint8_t* instructionBuffer = malloc(5);

	memory[255] = 17;
	
	registers->ip = 0;
	registers->ax = 0;
	registers->bx = 0;
	registers->cx = 0;
	
	uint8_t command = 0;
	while (1) // Instruction cycle
	{
		if (command == 4) break;
		sample(registers, instructionBuffer, &command, program);
		execute(command, instructionBuffer, registers, memory);
	}

	printf("REGISTERS INFO\n");
	printf("ip: %d\n", registers->ip);
	printf("ax: %d\n", registers->ax);
	printf("bx: %d\n", registers->bx);
	printf("cx: %d\n", registers->cx);

	free(memory);
	free(registers);
	free(instructionBuffer);
		
	fclose(program);

	exit(EXIT_SUCCESS);
}
