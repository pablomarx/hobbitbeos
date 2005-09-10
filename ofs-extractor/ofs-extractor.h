/*
 *  ofs-extractor.h
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

typedef struct	TableOfContents {
	long	VersionNumber;
	long  unknown1[3];
	long	FirstDirSector;
	long  unknown2[6];
	char	VolumeName[32];
	long	unknown3[109];
} TableOfContents;

typedef struct	FileEntry {
	char	FileName[32];
	long	FirstAllocList;
	long	LastAllocList;
	long	FileType;
	long	CreationDate;
	long	ModificationDate;
	long	LogicalSize;
	long	PhysicalSize;
	long	Creator;
} FileEntry;

typedef struct	DirectoryBlock {
	long				NextDirectoryBlock;
	long				unknown1[15];
	FileEntry	  	Entries[63];
	long				unknown2[16];
} DirectoryBlock;

struct fat {
	long	offset;
	long	count;
} fat;

void usage(char *progname);
void extractNormalFile(int output, FileEntry e);
void extractFATFile(int output, FileEntry e);
void readDir(int sector);
void debug_printf(char *fmt, ...);
