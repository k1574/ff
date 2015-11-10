/* See LICENSE file for copyright and license details. */
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#include "arg.h"

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage:%s\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	png_structp png_struct_p;
	png_infop png_info_p;
	png_bytepp png_row_p;
	int depth, color, interlace;
	uint32_t width, height, png_row_len, tmp32, r, i;
	uint16_t tmp16;

	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	/* load png */
	png_struct_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
	                                      NULL, NULL);
	png_info_p = png_create_info_struct(png_struct_p);

	if (!png_struct_p || !png_info_p || setjmp(png_jmpbuf(png_struct_p))) {
		fprintf(stderr, "failed to initialize libpng\n");
		return 1;
	}
	png_init_io(png_struct_p, stdin);
	png_set_add_alpha(png_struct_p, 255, PNG_FILLER_AFTER);
	png_set_gray_to_rgb(png_struct_p);
	png_read_png(png_struct_p, png_info_p, PNG_TRANSFORM_STRIP_16 |
	             PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
	png_get_IHDR(png_struct_p, png_info_p, &width, &height, &depth,
	             &color, &interlace, NULL, NULL);
	png_row_len = png_get_rowbytes(png_struct_p, png_info_p);
	png_row_p = png_get_rows(png_struct_p, png_info_p);

	/* write header */
	fprintf(stdout, "farbfeld");
	tmp32 = htobe32(width);
	fwrite(&tmp32, sizeof(uint32_t), 1, stdout);
	tmp32 = htobe32(height);
	fwrite(&tmp32, sizeof(uint32_t), 1, stdout);

	/* write data */
	/* TODO: allow 16 bit PNGs to be converted losslessly */
	for (r = 0; r < height; ++r) {
		for (i = 0; i < png_row_len; i++) {
			tmp16 = htobe16((uint16_t)png_row_p[r][i]);
			fwrite(&tmp16, sizeof(uint16_t), 1, stdout);
		}
	}

	/* cleanup */
	png_destroy_read_struct(&png_struct_p, &png_info_p, NULL);
	return 0;
}