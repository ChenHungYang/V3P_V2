// deslib.cpp : Defines the entry point for the DLL application.
//

// #include "stdafx.h"
#include "deslib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
*/

#define EN0	0	/* MODE == encrypt */
#define DE1	1	/* MODE == decrypt */

static void scrunch(unsigned char *, unsigned long *);
static void unscrun(unsigned long *, unsigned char *);
static void desfunc(unsigned long *, unsigned long *);
static void cookey(unsigned long *);
static void usekey(unsigned long *);
static void deskey(unsigned char *, short);
static void des(unsigned char *, unsigned char *);

static unsigned long KnL[32] = { 0L };

/* Use the key schedule specified in the Standard (ANSI X3.92-1981). */
static unsigned short bytebit[8]	= {
	0200, 0100, 040, 020, 010, 04, 02, 01 };

static unsigned long bigbyte[24] = {
	0x800000L,	0x400000L,	0x200000L, 	0x100000L,
	0x80000L,	0x40000L,	0x20000L,	0x10000L,
	0x8000L,	0x4000L,	0x2000L,	0x1000L,
	0x800L,		0x400L,		0x200L,		0x100L,
	0x80L,		0x40L,		0x20L,		0x10L,
	0x8L,		0x4L,		0x2L,		0x1L	};

static unsigned char pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,	 0, 57, 49, 41, 33, 25, 17,
	 9,  1, 58, 50, 42, 34, 26,	18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,	 6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28,	20, 12,  4, 27, 19, 11,  3 };

static unsigned char totrot[16] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28 };

static unsigned char pc2[48] = {
	13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31 };

static unsigned long SP1[64] = {
	0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
	0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
	0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
	0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
	0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
	0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
	0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
	0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
	0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
	0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
	0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
	0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
	0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
	0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L };

static unsigned long SP2[64] = {
	0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
	0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
	0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
	0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
	0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
	0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
	0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
	0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
	0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
	0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
	0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
	0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
	0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
	0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
	0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
	0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L };

static unsigned long SP3[64] = {
	0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
	0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
	0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
	0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
	0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
	0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
	0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
	0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
	0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
	0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
	0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
	0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
	0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
	0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
	0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
	0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L };

static unsigned long SP4[64] = {
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
	0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
	0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
	0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
	0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
	0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
	0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
	0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
	0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L };

static unsigned long SP5[64] = {
	0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
	0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
	0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
	0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
	0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
	0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
	0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
	0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
	0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
	0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
	0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
	0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
	0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
	0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
	0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
	0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L };

static unsigned long SP6[64] = {
	0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
	0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
	0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
	0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
	0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
	0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
	0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
	0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
	0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
	0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
	0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
	0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
	0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
	0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L };

static unsigned long SP7[64] = {
	0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L };

static unsigned long SP8[64] = {
	0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L };

void deskey(unsigned char *key, short edf)
{
	register int i, j, l, m, n;
	unsigned char pc1m[56], pcr[56];
	unsigned long kn[32];

	for ( j = 0; j < 56; j++ ) {
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
		}
	for( i = 0; i < 16; i++ ) {
		if( edf == DE1 ) m = (15 - i) << 1;
		else m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for( j = 0; j < 28; j++ ) {
			l = j + totrot[i];
			if( l < 28 ) pcr[j] = pc1m[l];
			else pcr[j] = pc1m[l - 28];
			}
		for( j = 28; j < 56; j++ ) {
		    l = j + totrot[i];
		    if( l < 56 ) pcr[j] = pc1m[l];
		    else pcr[j] = pc1m[l - 28];
		    }
		for( j = 0; j < 24; j++ ) {
			if( pcr[pc2[j]] ) kn[m] |= bigbyte[j];
			if( pcr[pc2[j+24]] ) kn[n] |= bigbyte[j];
			}
		}
	cookey(kn);
	return;
}

static void cookey(register unsigned long *raw1)
{
	register unsigned long *cook, *raw0;
	unsigned long dough[32];
	register int i;

	cook = dough;
	for( i = 0; i < 16; i++, raw1++ ) {
		raw0 = raw1++;
		*cook	 = (*raw0 & 0x00fc0000L) << 6;
		*cook	|= (*raw0 & 0x00000fc0L) << 10;
		*cook	|= (*raw1 & 0x00fc0000L) >> 10;
		*cook++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cook	 = (*raw0 & 0x0003f000L) << 12;
		*cook	|= (*raw0 & 0x0000003fL) << 16;
		*cook	|= (*raw1 & 0x0003f000L) >> 4;
		*cook++ |= (*raw1 & 0x0000003fL);
		}
	usekey(dough);
	return;
}

void usekey(register unsigned long *from)
{
	register unsigned long *to, *endp;

	to = KnL, endp = &KnL[32];
	while( to < endp ) *to++ = *from++;
	return;
}

static void scrunch(register unsigned char *outof, register unsigned long *into)
{
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into++ |= (*outof++ & 0xffL);
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into	|= (*outof   & 0xffL);
	return;
}

static void unscrun(register unsigned long *outof, register unsigned char *into)
{
	*into++ = (*outof >> 24) & 0xffL;
	*into++ = (*outof >> 16) & 0xffL;
	*into++ = (*outof >>  8) & 0xffL;
	*into++ =  *outof++	 & 0xffL;
	*into++ = (*outof >> 24) & 0xffL;
	*into++ = (*outof >> 16) & 0xffL;
	*into++ = (*outof >>  8) & 0xffL;
	*into	=  *outof	 & 0xffL;
	return;
}

static void desfunc(register unsigned long *block, register unsigned long *keys)
{
	register unsigned long fval, work, right, leftt;
	register int round;

	leftt = block[0];
	right = block[1];
	work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
	right ^= work;
	leftt ^= (work << 4);
	work = ((leftt >> 16) ^ right) & 0x0000ffffL;
	right ^= work;
	leftt ^= (work << 16);
	work = ((right >> 2) ^ leftt) & 0x33333333L;
	leftt ^= work;
	right ^= (work << 2);
	work = ((right >> 8) ^ leftt) & 0x00ff00ffL;
	leftt ^= work;
	right ^= (work << 8);
	right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;

	for( round = 0; round < 8; round++ ) {
		work  = (right << 28) | (right >> 4);
		work ^= *keys++;
		fval  = SP7[ work		 & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = right ^ *keys++;
		fval |= SP8[ work		 & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		leftt ^= fval;
		work  = (leftt << 28) | (leftt >> 4);
		work ^= *keys++;
		fval  = SP7[ work		 & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = leftt ^ *keys++;
		fval |= SP8[ work		 & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		right ^= fval;
	}

	right = (right << 31) | (right >> 1);
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt = (leftt << 31) | (leftt >> 1);
	work = ((leftt >> 8) ^ right) & 0x00ff00ffL;
	right ^= work;
	leftt ^= (work << 8);
	work = ((leftt >> 2) ^ right) & 0x33333333L;
	right ^= work;
	leftt ^= (work << 2);
	work = ((right >> 16) ^ leftt) & 0x0000ffffL;
	leftt ^= work;
	right ^= (work << 16);
	work = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
	leftt ^= work;
	right ^= (work << 4);
	*block++ = right;
	*block = leftt;
	return;
}

void des(unsigned char *inblock, unsigned char *outblock)
{
	unsigned long work[2];

	scrunch(inblock, work);
	desfunc(work, KnL);
	unscrun(work, outblock);
	return;
}

static void xor(unsigned char *b1, unsigned char *b2, int size)
{
	int i;
	
	for (i=0; i<size; i++) {
		b1[i] = b1[i] ^ b2[i];
	}
}
// 目前沒在用
//static char hex2char(char *x)
//{
//  char c;
//
//  /* note : (x & 0xdf) makes x upper case */
//  c = (x[0] >= 'A' ? ((x[0] & 0xdf) - 'A') + 10: (x[0] - '0'));
//  c *=16;
//  c +=(x[1] >= 'A' ? ((x[1] & 0xdf) - 'A') + 10: (x[1] - '0'));
//
//  return c;
//}


void DES_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	unsigned char tmpinblock[DES_BLOCK_SIZE];
	memcpy(tmpinblock, inblock, DES_BLOCK_SIZE);
	deskey(key, EN0);
	des(tmpinblock, outblock);
}

void DES_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	unsigned char tmpinblock[DES_BLOCK_SIZE];
	memcpy(tmpinblock, inblock, DES_BLOCK_SIZE);
	deskey(key, DE1);
	des(tmpinblock, outblock);
}

static void _tdes_encrypt(unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	deskey(key1, EN0);
	des(inblock, outblock);
	deskey(key2, DE1);
	des(outblock, inblock);
	deskey(key3, EN0);
	des(inblock, outblock);
}

static void _tdes_decrypt(unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	deskey(key1, DE1);
	des(inblock, outblock);
	deskey(key2, EN0);
	des(outblock, inblock);
	deskey(key3, DE1);
	des(inblock, outblock);
}


static unsigned int _tdes_cbc_encrypt(
				unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				unsigned char *inblock, unsigned int inblksize, unsigned char *outblock)
{
	unsigned char prev[8];
	unsigned char curr[8];
	int i, numblk;

	numblk = inblksize / 8;
	memset(prev,0,8);
	for (i=0; i<numblk; i++) {
		memcpy(curr, &inblock[i*8], 8);
		xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
		memcpy(&outblock[i*8], prev, 8);
	}
	if (inblksize % 8 != 0) {
		i = numblk * 8;
		numblk++;	// count in padded block
		memset(curr, 0, 8);
		memcpy(curr, &inblock[i], inblksize % 8);
		xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
		memcpy(&outblock[i], prev, 8);
	}

	return (numblk*8);
}


//27072004 AY To Pass in the first 8 bytes of the GISKE header as Initial Vector in CBC Encrypt
static unsigned int tdes_IVcbc_encrypt(
				unsigned char *key1, unsigned char* key2, unsigned char* key3, unsigned char*iniVector, 
				unsigned char *inblock, unsigned int inblksize, unsigned char *outblock)
{
	unsigned char prev[8];
	unsigned char curr[8];
	int i, numblk;

	numblk = inblksize / 8;
	memset(prev,0,8);
	memcpy(prev,iniVector,8); // AY Pass in the IV
	for (i=0; i<numblk; i++) {
		memcpy(curr, &inblock[i*8], 8);
		xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
		memcpy(&outblock[i*8], prev, 8);
	}
	if (inblksize % 8 != 0) {
		i = numblk * 8;
		numblk++;	// count in padded block
		memset(curr, 0, 8);
		memcpy(curr, &inblock[i], inblksize % 8);
		xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
		memcpy(&outblock[i], prev, 8);
	}

	return (numblk*8);
}



static unsigned int _tdes_cbc_decrypt(
				unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				unsigned char *inblock, unsigned int inblksize, unsigned char *outblock)
{
	unsigned char prev[8];
	unsigned char curr[8];
	unsigned char *out;
	int i, numblk;

	numblk = inblksize / 8;
	memset(prev,0,8);
	for (i=0; i<numblk; i++) {
		memcpy(curr, &inblock[i*8], 8);
		_tdes_decrypt(key1, key2, key3, curr, (out = &outblock[i*8]));
		xor(out, prev, 8);
		memcpy(prev, &inblock[i*8], 8);
	}

	return (numblk*8);
}

// 22112004 NKK - Added for visa wave encryption
static unsigned int _tdes_ecb_encrypt(
				unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				unsigned char *inblock, unsigned int inblksize, unsigned char *outblock)
{
	unsigned char prev[8];
	unsigned char curr[8];
	int i, numblk;

//	22112004 NKK - added new array for ecb
	unsigned char szVector[8];
// end NKK

	memset(szVector, 0x00, sizeof(szVector)); // 22112004 NKK

	numblk = inblksize / 8;
	memset(prev,0,8);
	for (i=0; i<numblk; i++) {
		memcpy(curr, &inblock[i*8], 8);
		//xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
// 22112004 NKK		
		if (i==0)
			memcpy(szVector, prev, 8);
// end NKK			
		memcpy(&outblock[i*8], prev, 8);
// 22112004 NKK
		memcpy(prev, szVector, 8);
// end NKK
	}
	if (inblksize % 8 != 0) {
		i = numblk * 8;
		numblk++;	// count in padded block
		memset(curr, 0, 8);
		memcpy(curr, &inblock[i], inblksize % 8);
		xor(curr, prev, 8);
		_tdes_encrypt(key1, key2, key3, curr, prev);
		memcpy(&outblock[i], prev, 8);
	}

	return (numblk*8);
}

static unsigned int _tdes_ecb_decrypt(
				unsigned char *key1, unsigned char* key2, unsigned char* key3, 
				unsigned char *inblock, unsigned int inblksize, unsigned char *outblock)
{
	unsigned char prev[8];
	unsigned char curr[8];
	unsigned char *out;
	int i, numblk;

//	22112004 NKK - added new array for ecb
	unsigned char szVector[8];
// end NKK
	memset(szVector, 0x00, sizeof(szVector)); // 22112004 NKK
	numblk = inblksize / 8;
	memset(prev,0,8);
	for (i=0; i<numblk; i++) {
		memcpy(curr, &inblock[i*8], 8);
		_tdes_decrypt(key1, key2, key3, curr, (out = &outblock[i*8]));
		// xor(out, prev, 8);
		// 22112004 NKK	
		if (i==0)
		// end NKK	
		memcpy(prev, &inblock[i*8], 8);
	}

	return (numblk*8);
}
// end NKK

void TDES_3LenKey_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	_tdes_encrypt(key, &key[8], &key[16], inblock, outblock);
}


unsigned int TDES_3LenKey_CBC_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_cbc_encrypt(key, &key[8], &key[16], inblock, inblksize, outblock));
}

void TDES_3LenKey_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	_tdes_decrypt(key, &key[8], &key[16], inblock, outblock);
}

unsigned int TDES_3LenKey_CBC_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_cbc_decrypt(key, &key[8], &key[16], inblock, inblksize, outblock));
}


void TDES_2LenKey_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	_tdes_encrypt(key, &key[8], key, inblock, outblock);
}

unsigned int TDES_2LenKey_CBC_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_cbc_encrypt(key, &key[8], key, inblock, inblksize, outblock));
}

//22072004 AY 
unsigned int TDES_2LenKey_IVCBC_Encrypt(unsigned char *key, unsigned char*initVector,
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(tdes_IVcbc_encrypt(key, &key[8], &key[16], initVector,inblock, inblksize, outblock));
}

void TDES_2LenKey_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock)
{
	_tdes_decrypt(key, &key[8], key, inblock, outblock);
}

unsigned int TDES_2LenKey_CBC_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_cbc_decrypt(key, &key[8], key, inblock, inblksize, outblock));
}


void TDES_2LenKey_CBC_MAC(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	unsigned char *pblk;
	int numblk;

	if (inblksize % 8==0) {
		numblk = (inblksize/8);
	} else {
		numblk = (inblksize/8) + 1;
	}
	pblk = (unsigned char *) malloc (numblk*8*sizeof(unsigned char));
	memset(&pblk[(numblk-1)*8], 0, 8);  // zero-ize last block as padding
	_tdes_cbc_encrypt(key, &key[8], key, inblock, inblksize, pblk);
	memcpy(outblock, &pblk[(numblk-1)*8], 8);  // last block is the MAC
	/*
	memset(pblk,0,numblk*8);
	memcpy(pblk,inblock,inblksize);

	memset(prev,0,8);
	for (i=0; i<numblk; i++) {
		memcpy(curr,&pblk[i*8],8);
		xor(curr,prev,8);
		TDES_2LenKey_Encrypt(key,curr,prev);
	}

	memcpy(outblock,prev,8);
	*/
	free(pblk);
}

// 22112004 NKK - Added for Visa Wave encryption
unsigned int TDES_2LenKey_ECB_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_ecb_encrypt(key, &key[8], key, inblock, inblksize, outblock));
}


unsigned int TDES_2LenKey_ECB_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock)
{
	return(_tdes_ecb_decrypt(key, &key[8], key, inblock, inblksize, outblock));
}
// end NKK
