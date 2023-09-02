/* Test FLD macro
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <encdec.h>

int
main(int argc, char *argv[])
{
	unsigned int i = 0xf8a1f8f1; /* 11111000 10100001 11111000 11110001 */
	unsigned int a; /* mask 0x00000000 is  0 */
	unsigned int b; /* mask 0x00000001 is  1 */
	unsigned int c; /* mask 0x0000000e is  0 */
	unsigned int d; /* mask 0x00000780 is  1 */
	unsigned int e; /* mask 0x003f0000 is 33 */
	unsigned int f; /* mask 0x00FFFF00 is 41464 */
	unsigned int g; /* mask 0xFF000000 is 248 */
	unsigned int h; /* mask 0x03f10000 is 161 */
	unsigned int j; /* mask 0xFFFFFFFF is i */

	a = FLD(i, 0x00000000);
	b = FLD(i, 0x00000001);
	c = FLD(i, 0x0000000e);
	d = FLD(i, 0x00000780);
	e = FLD(i, 0x003f0000);
	f = FLD(i, 0x00FFFF00);
	g = FLD(i, 0xFF000000);
	h = FLD(i, 0x03f10000);
	j = FLD(i, 0xFFFFFFFF);

	fprintf(stderr, "a=%u,b=%u,c=%u,d=%u,e=%u,f=%u,g=%u,h=%u,j=%u\n",
			a, b, c, d, e, f, g, h, j);
	fprintf(stderr, "a=0,b=1,c=0,d=1,e=33,f=41464,g=248,h=161,j=%u\n", i);

	return EXIT_SUCCESS;
}

