/*
 *  ofs-extractor.c
 *
 *  This silly little programs attempts to read the ofs filesystem used in early
 *  BeBoxes, particularly the Hobbit-based BeBoxes, and "extracts" the files from
 *  it's filesystem into the local filesystem (preserving directory structure).
 *  Quick'n'Dirty hack.
 *
 *  Tested on Mac OS X 10.3.5 and OpenBSD/x86 3.5
 *
 *  Created by Steve White on Thu Sep 09 2004.
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
#include <string.h>
#include <sys/time.h>

#include "ofs-extractor.h"

#define countof(__x__) (sizeof(__x__) / sizeof(*__x__))

int fd,showFileType=0,recurse=0,writeToDisk=0,debug=0,wuss=1;
char error[255];

int main (int argc, char **argv) {
	TableOfContents toc;
	int result;
	uint16_t ofsMajor, ofsMinor;

	if (argc == 1) {
		usage(argv[0]);
	}

	while ((result = getopt(argc, argv, "fswdh")) != -1) {
		switch (result) {
			case 'f':
				writeToDisk = 1;
				break;
			case 'd':
				debug = 1;
				debug_printf("Debug mode enabled!");
				break;
			case 's':
				showFileType=1;
				break;
			case 'w':
				wuss = 0;
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
	}

	if (argv[optind] == NULL) {
		usage(argv[0]);
	}

	fd = open(argv[argc-1], O_RDONLY);
	if (fd == -1) {
		sprintf(error, "%s line %d: %s", __FILE__, __LINE__, argv[argc-1]);
		perror(error);
		goto done;
	}
	if (read(fd, &toc, sizeof(toc)) != sizeof(toc)) {
		printf("read(toc) != %li\n", sizeof(toc));
		goto done;
	}

	ofsMajor = htonl(toc.VersionNumber)>>16&0xffff;
	ofsMinor = htonl(toc.VersionNumber)&0xfff;

	printf("volume name: %s\n", toc.VolumeName);
	printf("ofs version: %u.%u\n", ofsMajor, ofsMinor);
	if ((ofsMajor != 1 && ofsMajor != 2 && ofsMajor != 3) && wuss) {
		printf("Only ofs versions 1.0, 2.0 and 3.0 are supported. Sorry.\n");
		goto done;
	}

	if (writeToDisk) {
		result = mkdir(toc.VolumeName, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		if (result == -1) {
			sprintf(error, "%s line %d: %s", __FILE__, __LINE__, toc.VolumeName);
			perror(error);
		}
		result = chdir(toc.VolumeName);
		if (result == -1) {
			sprintf(error, "%s line %d: %s", __FILE__, __LINE__, toc.VolumeName);
			perror(error);
		}
	}

	readDir(htonl(toc.FirstDirSector), ofsMajor);
done:
	close(fd);
	return 0;
}

void skipBytes(uint32_t bytes) {
	uint32_t offset;
	
	offset = lseek(fd, 0, SEEK_CUR);
	offset += bytes;
	if (lseek(fd, offset, SEEK_SET) != offset) {
		perror(error);
		exit(-1);
	}
}

void readBytes(void *dst, uint32_t numBytes) {
	if (read(fd, dst, numBytes) != numBytes) {
		perror(error);
		exit(-1);
	}
}

void readDir(int sector, uint16_t ofsMajor) {
	uint32_t tmpoff, offset;
	int outfd,i,j;
	static struct timeval tv[2];

	int fileNum;
	FileAttributes attrs;
	char fileName[65];
	uint32_t nextDirectoryBlock;
	uint8_t fileNameLength = (ofsMajor > 1 ? 64 : 32);

	do {
		offset = sector*512;
		debug_printf("looping! sector=%i,offset=%i\n",sector,offset);
		if (lseek(fd, offset, SEEK_SET) != offset) {
			sprintf(error, "%s line %d: lseek", __FILE__, __LINE__);
			perror(error);
			return;
		}
		
		readBytes(&nextDirectoryBlock, sizeof(nextDirectoryBlock));
		skipBytes(60);
		
		for (fileNum=0; fileNum<63; fileNum++) {
			memset(&fileName, 0, countof(fileName));
			readBytes(&fileName, fileNameLength);
			readBytes(&attrs, sizeof(attrs));
			if (ofsMajor > 1) {
				skipBytes(32);
			}

			if (fileName[0] == '\0' && fileName[1] != '\0') { // && showHiddenFiles) {
					if (htonl(attrs.LogicalSize) != 0) {
						fileName[0] = '*';
					}
			}
			if (fileName[0] != '\0') {
				for (j=0; j<recurse; j++) {
					printf("\t");
				}
				printf("%s", fileName);

				if (htonl(attrs.FileType) == -1) {
					printf("/");
					debug_printf("\tDIR\t%i", htonl(attrs.FirstAllocList));
					debug_printf("\t%i", htonl(attrs.LastAllocList));
				} else {
					if (!(htonl(attrs.FirstAllocList) & 0x80000000)) {
						debug_printf("\tYES FAT\t%i", htonl(attrs.FirstAllocList));
					} else {
						debug_printf("\tNO FAT\t%i", htonl(attrs.FirstAllocList)&0x7fffffff);
					}
					debug_printf("\t%i", htonl(attrs.LastAllocList));
					printf("\t%u", htonl(attrs.LogicalSize));
					debug_printf("\t%i", htonl(attrs.PhysicalSize));
				}

				debug_printf("\t%i", htonl(attrs.CreationDate));
				debug_printf("\t%i", htonl(attrs.ModificationDate));

				if (showFileType && htonl(attrs.FileType) != -1) {
					if (attrs.FileType != 0x00000000) {
						tmpoff = htonl(attrs.FileType);
						printf("\t%c%c%c%c", (char)(tmpoff>>24&0xff), (char)(tmpoff>>16&0xff),
											(char)(tmpoff>>8&0xff), (char)(tmpoff&0xff));
					}
					if (attrs.Creator != 0x00000000) {
						tmpoff = htonl(attrs.Creator);
						printf("\t%c%c%c%c", (char)(tmpoff>>24&0xff), (char)(tmpoff>>16&0xff),
											(char)(tmpoff>>8&0xff), (char)(tmpoff&0xff));
					}
				}
				printf("\n");

				offset = lseek(fd, 0, SEEK_CUR);				
				if (writeToDisk) {
					if (htonl(attrs.FileType) == -1) {
						recurse++;
						j = mkdir(fileName, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
						if (j != -1) {
							chdir(fileName);
							readDir(htonl(attrs.FirstAllocList), ofsMajor);
							chdir("..");
						} else {
							sprintf(error, "%s line %d: %s", __FILE__, __LINE__, fileName);
							perror(error);
						}
						recurse--;
					} else {	// it's a file, not a directory
						tmpoff = lseek(fd, 0, SEEK_CUR);
						outfd = open(fileName, O_WRONLY|O_TRUNC|O_CREAT, 0644);
						if (outfd == -1) {
							sprintf(error, "%s line %d: %s", __FILE__, __LINE__, fileName);
							perror(error);
						} else {
							if (!(htonl(attrs.FirstAllocList) & 0x80000000)) {
								extractFATFile(outfd, attrs);
							} else {
								extractNormalFile(outfd, attrs);
							}
						}
						close(outfd);
						lseek(fd, tmpoff, SEEK_SET);
					} // file|directory
					tv[0].tv_sec = htonl(attrs.CreationDate);
					tv[0].tv_usec = 0;
					tv[1].tv_sec = htonl(attrs.ModificationDate);
					tv[1].tv_usec = 0;
					j = utimes(fileName, tv);
				} else { // writeToDisk
					if (htonl(attrs.FileType) == -1) {
						recurse++;
						readDir(htonl(attrs.FirstAllocList), ofsMajor);
						recurse--;
					}
				}

				lseek(fd, offset, SEEK_SET);
			} // filename isn't \0
		} // for loop (1...63)
		
		skipBytes(64);
		
		sector = htonl(nextDirectoryBlock);
	} while (sector != 0);
} 

void extractFATFile(int outfd, FileAttributes e) {
	struct fat fats[64];
	char buffer[512];
	int i=1,count,current,length;

	length = htonl(e.LogicalSize);
	lseek(fd, htonl(e.FirstAllocList)*512, SEEK_SET);
	if (read(fd, fats, 512) != 512) {
		printf("read != 512\n");
		return;
	}

	if (htonl(fats[0].offset)) {
		printf("Not sure how to handle multiple FAT entries!\n");
		return;
	}

	while (htonl(fats[i].offset) != 0xffffffff) {
		debug_printf("should be reading offset 0x%08x %i times...\n",
			htonl(fats[i].offset)*512, htonl(fats[i].count));
		lseek(fd, htonl(fats[i].offset)*512, SEEK_SET);
		count = 0;
		while (count < htonl(fats[i].count) && length > 0) {
			current = read(fd, buffer, length<512?length:512);
			write(outfd, buffer, (unsigned int)current);
			length -= current;
			count++;
		}
		debug_printf("count=%i,fats[%i].count=%i\n",count,i,
			htonl(fats[i].count));
		i++;
	}
	debug_printf("i=%i\n",i-1);
}

void extractNormalFile(int outfd, FileAttributes e) {
	uint32_t length,current;
	char buffer[512];

	lseek(fd, (htonl(e.FirstAllocList)&0x7fffffff)*512, SEEK_SET);
	length = htonl(e.LogicalSize);
	while (length > 0) {
		current = read(fd, buffer, length<512?length:512);
		write(outfd, buffer, (unsigned int)current);
		length -= current;
	}
}

void usage(char *progname) {
	printf("Usage: %s [options] <hobbit image>\n"
		"options:\n"
		"	-h	this help message\n"
		"	-d	turn on debug messages\n"
		"	-s	show filetype and application creator (if available)\n"
		"	-f	write files (and make directories)\n", progname);
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
