/*
 *  hobbit-disasm.c
 *
 *  This is a basic disassembler for Hobbit (AT&T CRISP 92010) ELF binaries.
 *  Namely those from the prototype Hobbit based BeBoxes.
 *  This requires libelf (v0.8.6).
 *
 *  Tested on OpenBSD/macppc 3.7
 *
 *  Created by Steve White on Thu Jan 26 2006.
 *  Copyright (c) 2006 Steve White. All rights reserved.
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
#include "hobbit-disasm.h"

#include <libelf.h>
#include <stdio.h>
#include <fcntl.h>

void decode(unsigned short opcode) {

}

void disassemble(int fp, unsigned long offset, unsigned long size) {
	unsigned long amtread=0;

	unsigned short opcode,a;
	unsigned long b,c,d;

	Optable *op;


	if (lseek(fp, offset, SEEK_SET) != offset) {
		perror("lseek");
		return;
	}

	while (amtread < size) {
		read(fp, &opcode, sizeof(opcode));
		read(fp, &a, sizeof(a));
		read(fp, &b, sizeof(b));
		read(fp, &c, sizeof(c));
		read(fp, &d, sizeof(d));
		printf("%08x: %04x%04x %08x %08x %08x # ", amtread, opcode, a, b, c, d);

		switch(opcode&0xc000) {
			default:
			case 0x0000:
			case 0x4000:		/* One Parcel */
#if 0
				i->type = ONEPARCEL;
				i->opcode = p;
				i->s10 = p&0x3ff;
				i->s8  = (p>>2)&0xff;
				i->subcode = p&0x03;
				i->src = (p>>5)&0x1f;
				i->dst = p&0x1f;
#endif
				break;
			case 0x8000:		/* Three Parcel */
#if 0
				i->type = THREEPARCEL;
				i->opcode = p;
				i->subcode = p&0x0f;
				i->smode = (p>>4)&0x0f;;
				i->dmode = p&0x0f;
				i->operand = b;
				
#endif
				break;
			case 0xc000:		/* Five Parcel */
#if 0
				i->type = FIVEPARCEL;
				i->opcode = p;
				i->smode = (p>>4)&0x0f;
				i->dmode = p&0x0f;
				i->src = b;
				i->dest = c;
#endif
				break;
		}

		for (op = optable; op->proto; op++) {
			if ((opcode&op->mask) == op->opcode) {
				printf(" %s", op->proto);
			}
		}

		printf("\n");
		amtread += 16;
	}
}

void parse(int fp) {
	Elf *elf, *arf;
	Elf_Cmd cmd;
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	Elf_Scn *scn;
	Elf_Data *strtab;
	char *secname;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		printf("erm?\n");
		return;
	}

	cmd = ELF_C_READ;
	arf = elf_begin(fp, cmd, (Elf *)0);
	while ((elf = elf_begin(fp, cmd, arf)) != 0) {
		if (elf_kind(elf) != ELF_K_ELF) {
			printf("File not in ELF format\n");
			return;
		}

		if ((ehdr = elf32_getehdr(elf)) != 0) {
			scn = elf_getscn(elf, (size_t)ehdr->e_shstrndx);
			if ((shdr = elf32_getshdr(scn)) != 0) {
				if (shdr->sh_type = SHT_STRTAB) {
					if ((strtab = elf_getdata(scn, (Elf_Data *)0))==0 || strtab->d_size==0) {
						printf("No .shstrtab data!\n");
					}
				}
			}
		}

		scn = 0;
		printf("NAME\tSIZE\tOFFSET\n");
		while ((scn = elf_nextscn(elf, scn))) {
			shdr = elf32_getshdr(scn);
			if (shdr->sh_type == SHT_PROGBITS) {
				secname = &((char *)(strtab->d_buf))[shdr->sh_name];
				printf("%s\t0x%08x\t0x%08x\n", secname, shdr->sh_size, shdr->sh_offset);
				if (strcmp(".text", secname)==0 || strcmp(".ztext", secname)==0) {
					disassemble(fp, shdr->sh_offset, shdr->sh_size);
				}
			}
		}

		cmd = elf_next(elf);
		elf_end(elf);
	}
	elf_end(arf);
}

int main(int argc, char **argv) {
	int i, fp;

	if (argc < 2) {
		fprintf(stderr, "usage: %s [file ...]\n", argv[0]);
		return -1;
	}

	for (i=1; i<argc; i++) {
		if ((fp = open(argv[i], O_RDONLY)) == -1) {
			perror(argv[i]);
		} else {
			parse(fp);
			close(fp);
		}
	}
}

