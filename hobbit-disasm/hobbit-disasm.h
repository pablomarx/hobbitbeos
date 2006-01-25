/*
 *  hobbit-disasm.h
 *
 *  This is a basic disassembler for Hobbit (AT&T CRISP 92010) ELF binaries.
 *  Namely those from the prototype Hobbit based BeBoxes.
 *
 *  This requires libelf (v0.8.6).
 *
 *  Portions of this file were stolen from Plan 9 1st Edition.
 *  Thanks to jmk @ Bell Labs for sending along that code.
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
typedef struct Optable {
	unsigned short	opcode;
	unsigned short	mask;
	char*		proto;
} Optable;

struct Optable	optable[] = {
		/* one parcel */
	0x0000,	0xfc00,	"KCALL	$%r",
	0x0400,	0xfc00,	"CALL	%p",
	0x0800,	0xfc03,	"ENTER	R%Z",
	0x0801,	0xfc03,	"CATCH	R%z",
	0x0802,	0xfc03,	"RETURN	R%z",
	0x0803,	0xfc03,	"POPN	R%z",
	0x0c00,	0xfc00,	"JMP	%p",
	0x1000,	0xfc00,	"JMPFN	%p",
	0x1400,	0xfc00,	"JMPFY	%p",
	0x1800,	0xfc00,	"JMPTN	%p",
	0x1c00,	0xfc00,	"JMPTY	%p",
	0x2800,	0xfc00,	"MOV	$%I,%S",
	0x2c00,	0xffff,	"CPU",
	0x2c01,	0xffff,	"KRET",
	0x2c02,	0xffff,	"NOP",
	0x2c03,	0xffff,	"FLUSHI",
	0x2c04,	0xffff,	"FLUSHP",
	0x2c05,	0xffff,	"CRET",
	0x2c06,	0xffff,	"FLUSHD",
	0x2c08,	0xffff,	"TESTV",
	0x2c09,	0xffff,	"TESTC",
	0x2c0a,	0xffff,	"CLRE",
	0x3400,	0xfc00,	"ADD3	$%I,%S",
	0x3800,	0xfc00,	"AND3	$%i,%S",
	0x3c00,	0xfc00,	"AND	%s,%S",
	0x4000,	0xfc00,	"CMPEQ	$%i,%S",
	0x4400,	0xfc00,	"CMPGT	%s,%S",
	0x4800,	0xfc00,	"CMPGT	$%i,%S",
	0x4c00,	0xfc00,	"CMPEQ	%s,%S",
	0x5000,	0xfc00,	"ADD	$%i,%S",
	0x5400,	0xfc00,	"ADD3	$%i,%S",
	0x5800,	0xfc00,	"ADD	%s,%S",
	0x5c00,	0xfc00,	"ADD3	%s,%S",
	0x6000,	0xfc00,	"MOV	%s,%S",
	0x6400,	0xfc00,	"MOV	*%s,%S",
	0x6800,	0xfc00,	"MOV	%s,*%S",
	0x6c00,	0xfc00,	"MOV	*%s,*%S",
	0x7000,	0xfc00,	"MOV	$%i,%S",
	0x7400,	0xfc00,	"MOVA	%s,%S",
	0x7800,	0xfc00,	"SHL3	$%u,%S",
	0x7c00,	0xfc00,	"SHR3	$%u,%S",
		/* three parcel and five parcel */
	0x8000,	0xff0f,	"KCALL	%w",
	0x8001,	0xff0f,	"CALL	%c",
	0x8002,	0xff0f,	"RETURN	%m",
	0x8003,	0xff0f,	"JMP	%c",
	0x8004,	0xff0f,	"JMPFN	%C",
	0x8005,	0xff0f,	"JMPFY	%C",
	0x8006,	0xff0f,	"JMPTN	%C",
	0x8007,	0xff0f,	"JMPTY	%C",
	0x8008,	0xff0f,	"CATCH	%w",
	0x8009,	0xff0f,	"ENTER	%w",
	0x800a,	0xff0f,	"LDRAA	%c",
	0x800b,	0xff0f,	"FLUSHPTE %w",
	0x800c,	0xff0f,	"FLUSHPBE %w",
	0x800d,	0xff0f,	"FLUSHDCE %w",
	0x800f,	0xff0f,	"POPN	%m",
	0x8100,	0xbf00,	"ORI	%g,%G",
	0x8200,	0xbf00,	"ANDI	%g,%G",
	0x8300,	0xbf00,	"ADDI	%g,%G",
	0x8400,	0xbf00,	"MOVA	%W,%G",
	0x8500,	0xbf00,	"UREM	%g,%G",
	0x8600,	0xbf00,	"MOV	%g,%G",
	0x8700,	0xbf00,	"DQM	%g,%G",
	0x8800,	0xbf00,	"FNEXT	%f,%F",
	0x8900,	0xbf00,	"FSCALB	%g,%F",
	0x8b00,	0xbf00,	"FREM	%f,%F",
	0x8c00,	0xbf00,	"TADD	%g,%G",
	0x8d00,	0xbf00,	"TSUB	%g,%G",
	0x9000,	0xbf00,	"FSQRT	%f,%F",
	0x9100,	0xbf00,	"FMOV	%f,%F",
	0x9200,	0xbf00,	"FLOGB	%f,%F",
	0x9300,	0xbf00,	"FCLASS	%f,%G",
	0x9800,	0xbf00,	"FCMPGE	%f,%F",
	0x9900,	0xbf00,	"FCMPGT	%f,%F",
	0x9a00,	0xbf00,	"FCMPEQ	%f,%F",
	0x9b00,	0xbf00,	"FCMPEQN %f,%F",
	0x9c00,	0xbf00,	"FCMPN	%f,%F",
	0x9d00,	0xbf00,	"CMPGT	%g,%d",
	0x9e00,	0xbf00,	"CMPHI	%g,%d",
	0x9f00,	0xbf00,	"CMPEQ	%g,%d",
	0xa000,	0xbf00,	"SUB	%g,%G",
	0xa100,	0xbf00,	"OR	%g,%G",
	0xa200,	0xbf00,	"AND	%g,%G",
	0xa300,	0xbf00,	"ADD	%g,%G",
	0xa400,	0xbf00,	"XOR	%g,%G",
	0xa500,	0xbf00,	"REM	%g,%G",
	0xa600,	0xbf00,	"MUL	%g,%G",
	0xa700,	0xbf00,	"DIV	%g,%G",
	0xa800,	0xbf00,	"FSUB	%f,%F",
	0xa900,	0xbf00,	"FMUL	%f,%F",
	0xaa00,	0xbf00,	"FDIV	%f,%F",
	0xab00,	0xbf00,	"FADD	%f,%F",
	0xac00,	0xbf00,	"SHR	%g,%G",
	0xad00,	0xbf00,	"USHR	%g,%G",
	0xae00,	0xbf00,	"SHL	%g,%G",
	0xaf00,	0xbf00,	"UDIV	%g,%G",
	0xb000,	0xbf00,	"SUB3	%g,%G",
	0xb100,	0xbf00,	"OR3	%g,%G",
	0xb200,	0xbf00,	"AND3	%g,%G",
	0xb300,	0xbf00,	"ADD3	%g,%G",
	0xb400,	0xbf00,	"XOR3	%g,%G",
	0xb500,	0xbf00,	"REM3	%g,%G",
	0xb600,	0xbf00,	"MUL3	%g,%G",
	0xb700,	0xbf00,	"DIV3	%g,%G",
	0xb800,	0xbf00,	"FSUB3	%f,%F",
	0xb900,	0xbf00,	"FMUL3	%f,%F",
	0xba00,	0xbf00,	"FDIV3	%f,%F",
	0xbb00,	0xbf00,	"FADD3	%f,%F",
	0xbc00,	0xbf00,	"SHR3	%g,%G",
	0xbd00,	0xbf00,	"USHR3	%g,%G",
	0xbe00,	0xbf00,	"SHL3	%g,%G",
	0,	0,	0,
};
