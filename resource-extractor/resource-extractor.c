/*
 *  resource-extractor.c
 *
 *  This silly little programs attempts to extract resources (ICON and MICN)
 *  from Hobbit BeOS binaries. Successfully extract ICON and MICN's are stored
 *  in PNG format.
 *  Quick'n'Dirty hack.
 *
 *  Tested on Mac OS X 10.3.5 and OpenBSD/x86 3.5
 *
 *  Created by Steve White on Fri Sep 24 2004.
 *  Copyright (c) 2004 Steve White. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <png.h>
#include <malloc/malloc.h>

#include "palette.h"

typedef struct ResourceHeader {
	long type;
	long unknown1;
	long startsAt;
	long size;
	long unknown3;
} ResourceHeader;

int extractImage(char *buffer, char *tofile, int width, int height);
void usage(char *progname);
void debug_printf(char *fmt, ...);

char error[255];
int debug=0,allFiles=0,convert=0;

int main (int argc, char **argv) {
	int result,fd,i,total,outfd;
	struct stat filestat;
	long buf, start;
	ResourceHeader* hdr;
	char* buffer;
	char* filename;

	if (argc == 1) {
		usage(argv[0]);
	}

	while ((result = getopt(argc, argv, "adhc")) != -1) {
		switch (result) {
			case 'c':
				convert = 1;
				break;
			case 'a':
				allFiles = 1;
				break;
			case 'd':
				debug = 1;
				debug_printf("Debug mode enabled.\n");
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
	}

	if (argv[optind] == NULL) {
		usage(argv[0]);
		return -1;
	}

	fd = open(argv[argc-1], O_RDONLY);
	if (fd == -1) {
		sprintf(error, "%s line %d: open %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		return -1;
	}

	result = stat(argv[argc-1], &filestat);
	if (result == -1) {
		sprintf(error, "%s line %d: stat %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		goto done;
	}

	if (lseek(fd, filestat.st_size-8, SEEK_SET) != filestat.st_size-8) {
		sprintf(error, "%s line %d: seek %s to 0x%08x", __FILE__, __LINE__,
			argv[argc-1], (int)filestat.st_size-8);
		perror(error);
		goto done;
	}

	if (read(fd, &buf, sizeof(buf)) != sizeof(buf)) {
		sprintf(error, "%s line %d: read %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		goto done;
	}

	if (buf != 0x52535243) {
		printf("Did not find valid RSRC in %s.\n", argv[argc-1]);
		goto done;
	}

	if (read(fd, &start, sizeof(start)) != sizeof(start)) {
		sprintf(error, "%s line %d: read %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		goto done;
	}

	debug_printf("RSRC header starts at 0x%08x\n", start);

	if (lseek(fd, start, SEEK_SET) != start) {
		sprintf(error, "%s line %d: seek %s to 0x%08x", __FILE__, __LINE__,
			argv[argc-1],(int)start);
		perror(error);
		goto done;
	}

	if (read(fd, &buf, sizeof(buf)) != sizeof(buf)) {
		sprintf(error, "%s line %d: read %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		goto done;
	}

	printf("%li RSRC records in this file\n", buf);
	total = buf;
	hdr = malloc(sizeof(ResourceHeader)*total);

	for (i=0; i<total; i++) {
		if (read(fd, &hdr[i], sizeof(ResourceHeader)) != sizeof(ResourceHeader)) {
			sprintf(error, "%s line %d: read %s",__FILE__,__LINE__,argv[argc-1]);
			perror(error);
			goto done;
		}
		printf("Resource #%i: Type %c%c%c%c\n",i,(char)(hdr[i].type>>24&0xff),
			(char)(hdr[i].type>>16&0xff),(char)(hdr[i].type>>8&0xff),
			(char)(hdr[i].type&0xff));
		debug_printf("unknown1=0x%08x\n",hdr[i].unknown1);
		debug_printf("startsAt=0x%08x\n",start+(sizeof(ResourceHeader)*total)+
			hdr[i].startsAt+4); // +4 to skip past the number of resource headers
		debug_printf("size=0x%08x\n",hdr[i].size);
		debug_printf("unknown3=0x%08x\n",hdr[i].unknown3);
	}

	buffer = (char *)malloc(256);
	if (!buffer || malloc_size(buffer) != 256) {
		sprintf(error, "%s line %d: malloc", __FILE__, __LINE__);
		perror(error);
		goto done;
	}

	for (i=0; i<total; i++) {
		if (malloc_size(buffer) < hdr[i].size) {
			buffer = realloc(buffer, hdr[i].size);
			if (!buffer) {
				sprintf(error, "%s line %d: relloc", __FILE__, __LINE__);
				perror(error);
				goto done;
			}
		}

		if (read(fd, buffer, hdr[i].size) != hdr[i].size) {
			sprintf(error, "%s line %d: read %s",__FILE__,__LINE__,argv[argc-1]);
			perror(error);
			goto done;
		} else {
			switch (hdr[i].type) {
				case 0x49434f4e:
					if (convert && hdr[i].size == 1024) {
						filename = (char *)malloc( strlen(argv[argc-1]) + 11 );
						sprintf(filename, "%s-ICON-%i.png", argv[argc-1], i);
						if (extractImage(buffer, filename, 32, 32) == -1) {
							goto done;
						}
						printf("Extracted ICON RSRC to file %s\n", filename);
						break;
					} else if (convert) {
						printf("ICON is not 1024 bytes. Can not convert to PNG\n");
					}
				case 0x4d49434e:
					if (convert && hdr[i].size == 256) {
						filename = (char *)malloc(strlen(argc[argv-1])+11);
						sprintf(filename, "%s-MICN-%i.png", argv[argc-1], i);
						if (extractImage(buffer, filename, 16, 16) == -1) {
							goto done;
						}
						printf("Extracted MICN RSRC to file %s\n", filename);
						break;
					} else if (convert) {
						printf("MICN is not 256 bytes. Can not convert to PNG\n");
					}
				default:
					if (allFiles) {
						filename = (char *)malloc(strlen(argv[argc-1]) + 12);
						sprintf(filename, "%s-%c%c%c%c-%i.rsrc", argv[argc-1],
							(char)(hdr[i].type>>24&0xff),(char)(hdr[i].type>>16&0xff),
							(char)(hdr[i].type>>8&0xff),(char)(hdr[i].type&0xff), i);
						outfd = open(filename, O_WRONLY|O_TRUNC|O_CREAT, 0644);
						if (outfd == -1) {
							sprintf(error, "%s line %d: open %s", __FILE__, __LINE__,
								filename);
							perror(error);
							goto done;
						}
						write(outfd, buffer, hdr[i].size);
						close(outfd);
						printf("Extracted %c%c%c%c RSRC to file %s\n",
							(char)(hdr[i].type>>24&0xff), (char)(hdr[i].type>>16&0xff),
							(char)(hdr[i].type>>8&0xff),(char)(hdr[i].type&0xff),
							filename);
					}
					break;
			}
		}
	}

done:
	close(fd);
	return 0;
}

int extractImage(char *buffer, char *tofile, int width, int height) {
	png_structp png_ptr;
	png_infop png_info;
	png_byte** image;
	unsigned i, x, y;
	FILE *fp;

	fp = fopen(tofile, "wb");
	if (!fp) {
		sprintf(error, "%s line %d: open %s", __FILE__, __LINE__, tofile);
		perror(error);
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);

	png_set_IHDR (png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(png_ptr, png_info, palette, 256);

	image = malloc (sizeof (png_bytep) * height);
	for (i=0; i<height; i++) {
		image[i] = malloc (sizeof (png_byte) * width);
	}
	i = 0;

	for (y=0; y<height; y++) {
		for (x=0; x<width; x++) {
			image[y][x] = buffer[i];
			i++;
		}
	}

	png_write_info(png_ptr, png_info);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, png_info);
	png_destroy_write_struct(&png_ptr, &png_info);
	fclose(fp);

   return 0;
}

void usage(char *progname) {
   printf("Usage: %s [options] <hobbit binary>\n"
      "options:\n"
      "  -h this help message\n"
      "  -d turn on debug messages\n"
      "  -c convert ICON and MICN resources to PNG files\n"
      "  -a extract all resources (not just ICON and MICN)\n", progname);
   exit(1);
}

void debug_printf(char *fmt, ...) {
   va_list ap;
   if (!debug)
      return;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}
