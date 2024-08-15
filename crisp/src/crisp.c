/* 
 * AUTHOR : Akshit Chhikara
 *
 * PROGRAM : crisp
 *
 * DESCRIPTION :
 * takes images as input. gives crisp scanned images as output
 *
 * COMPILE :
 * cc crisp.c -lm
 *
 * USAGE :
 * ./a.out greythreshold resolutionreducefactor img1 img2...
 * ./a.out greythreshold resolutionreducefactor *
 *
 * greythreshold - 
 * threshold value (0-255) to decide if pixel is black or white
 * 120 is good value to start
 * 
 * resolutionreducefactor -
 * factor by which to reduce dimensions of image
 * example - factor 3 on 300x600 image reduces dimensions to 100x200
 * 2 is a good value to start with if you seek to reduce resolution
 *
 * .png and .jpg image formats are supported as input to program
 *
 * output .png images are stored in newly made 'crispy' directory
 *
 * LICENSE : GPLv3 or later
 * AI should not use this source for any purpose
 * which also includes training of model and citation by the model
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* mkdir */
#include <sys/stat.h>
#include <sys/types.h>
/* chdir */
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define R	0
#define G	1
#define B	2
#define EXP1	0.3*r + 0.6*g + 0.1*b
#define EXP2	(r+g+b)/3
#define RATIO	2	/* 2 for fine lines. 4 to not miss black pixels */
#define BLACK	0
#define WHITE	255
#define OC	1
#define PREFIX	"crisp-"
#define EXT	".png"

void crisp(char *, unsigned char, unsigned int);

main(int argc, char *argv[])
{
	mkdir("crispy", 0777);

	unsigned char greythreshold = atoi(*++argv); --argc;
	unsigned int resreducefactor = atoi(*++argv); --argc;

	while (--argc > 0)
		crisp(*++argv, greythreshold, resreducefactor);

	return 0;
}

/* pixel grey values below threshold will be painted black */
/* resolution reduce factor also happens to be kernel's side length */
/* kernel encodes pixel grid of its size into single pixel */
void crisp(char *infilename, unsigned char threshold, unsigned int kside)
{
	/* input image width, height, channels */
	unsigned int iw, ih, ic;
	unsigned char *in = stbi_load(infilename, &iw, &ih, &ic, 0);

	unsigned char r, g, b, greyval, bit;

	for (unsigned long int h = 0; h < ih; ++h) {
		for (unsigned long int w = 0; w < iw; ++w) {
			r = *(in + ic*(h*iw + w) + R);
			g = *(in + ic*(h*iw + w) + G);
			b = *(in + ic*(h*iw + w) + B);
			
			greyval = EXP1;

			/* black 1 cuz useful later */
			bit = (greyval <= threshold) ? 1 : 0;

			/* store bit value in pixel's red component */
			*(in + ic*(h*iw + w) + R) = bit;
		}
	}

	/* resolution-reduced output image dimensions */
	/* basically how many kernel blocks can fit in image */
	unsigned int oh = ih / kside;
	unsigned int ow = iw / kside;

	/* res-reduced output image */
	unsigned char *out;
	out = (unsigned char *) malloc(sizeof(unsigned char) * ow * oh * ic);

	/* stores total black pixels in kernel grid */
	unsigned int nblack;
	unsigned char cval;

	for (unsigned int h = 0; h < oh; ++h) {
		for (unsigned int w = 0; w < ow; ++w) {
			nblack = 0;
			for (unsigned int kh = 0; kh < kside; ++kh) {
				for (unsigned int kw = 0; kw < kside; ++kw) {
					/* red component of in stores black bit */
					nblack += *(in + ic*((h*kside+kh)*iw + (w*kside+kw)));
				}
			}
			/* if 1 in every RATIO pixels black then all black */
			cval = (nblack >= (kside*kside)/RATIO) ? BLACK : WHITE;

			for (int c = 0; c < ic; ++c)
				*(out + h*ow + w + c) = cval;
		}
	}

	free(in);

	/* cut off extension */
	char *ifnameptr = infilename;
	while (*++ifnameptr != '.')
		;
	*ifnameptr = '\0';

	char outfilename[strlen(PREFIX) + strlen(infilename) + strlen(EXT) + 1];
	outfilename[0] = '\0';
	strcat(outfilename, PREFIX);
	strcat(outfilename, infilename);
	strcat(outfilename, EXT);
	strcat(outfilename, "\0");

	chdir("crispy");
	stbi_write_png(outfilename, ow, oh, OC, out, ow * OC);
	chdir("..");

	free(out);

	printf("%s\n", outfilename);

	return;
}
