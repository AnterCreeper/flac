#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "portme.h"

#define BS_PAGE_SIZE	512

struct bitstream {
	int bytepos; //heap position
	int bitlen;	 //bit length
	unsigned char* heap;
	unsigned char* buffer;
};

typedef struct bitstream bitstream_t;

void bitstream_init(bitstream_t* bs);
void bitstream_close(bitstream_t* bs);
void bitstream_prepare(bitstream_t* bs);
void bitstream_swapin(bitstream_t* bs);
unsigned int bitstream_readbits(int len, bitstream_t* bs);
void bitstream_align(struct bitstream* inp);
void bitstream_alignread(unsigned char* dst, struct bitstream* inp);

#endif
