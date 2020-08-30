#include <stdio.h>

char main(void)
{
	FILE* file = fopen("pon", "wb");
	const unsigned char buffer[] = {0x0, 0x2, 0x2, 0x0, 0xFF, 0x4};

	fwrite(buffer, 1, 6, file);
	fclose(file);

	return 0;
}
