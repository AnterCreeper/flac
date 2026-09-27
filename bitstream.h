#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "portme.h"

#define BS_PAGE_SIZE	512

struct bitstream {
	int bytepos; //heap position
	int bitlen;	 //bit length
	unsigned char* heap;
	unsigned char* buffer;
	unsigned long fetched;	//total bytes delivered by portme_fread
	unsigned long pages;	//heap pages fully consumed
	int eof;	//a short read has occurred
};

typedef struct bitstream bitstream_t;

void bitstream_init(bitstream_t* bs);
void bitstream_close(bitstream_t* bs);
int bitstream_end(bitstream_t* bs);
void bitstream_prepare(bitstream_t* bs);
void bitstream_swapin(bitstream_t* bs);
/* read at most 16 bits per call, so the result always fits a 16-bit unsigned int */
unsigned int bitstream_readbits(int len, bitstream_t* bs);
void bitstream_align(bitstream_t* bs);
void bitstream_alignread(unsigned char* dst, bitstream_t* bs);

#endif
