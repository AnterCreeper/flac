#include "bitstream.h"

#ifdef STATIC_MEM
unsigned char bs_buffer0[BS_PAGE_SIZE];
unsigned char bs_buffer1[BS_PAGE_SIZE];
#endif

void bitstream_init(bitstream_t* bs) {
	bs->bytepos = 0;
	bs->bitlen  = 8;
	bs->fetched = 0;
	bs->pages   = 0;
	bs->eof     = 0;
#ifdef STATIC_MEM
	bs->heap = bs_buffer0;
	bs->buffer = bs_buffer1;
#else
	bs->heap = (unsigned char*)malloc(BS_PAGE_SIZE);
	bs->buffer = (unsigned char*)malloc(BS_PAGE_SIZE);
#endif
	unsigned int n;
	n = portme_fread(bs->heap, BS_PAGE_SIZE);
	bs->fetched += n;
	if (n < BS_PAGE_SIZE) bs->eof = 1;
	n = portme_fread(bs->buffer, BS_PAGE_SIZE);
	bs->fetched += n;
	if (n < BS_PAGE_SIZE) bs->eof = 1;
	return;
}

void bitstream_close(bitstream_t* bs) {
#ifndef STATIC_MEM
	free(bs->heap);
	free(bs->buffer);
#endif
}

void bitstream_swapin(bitstream_t* bs) {
	unsigned char* tmp;
	tmp = bs->buffer;
	bs->buffer = bs->heap;
	bs->heap = tmp; //swap buffer to heap
	bs->pages++; //old heap page fully consumed
	unsigned int n = portme_fread(bs->buffer, BS_PAGE_SIZE); //issue a async read 1 page into buffer
	bs->fetched += n;
	if (n < BS_PAGE_SIZE) bs->eof = 1;
	return;
}

int bitstream_end(bitstream_t* bs) {
	return bs->eof && (bs->pages * BS_PAGE_SIZE + bs->bytepos) >= bs->fetched;
}

void bitstream_prepare(bitstream_t* bs) {
	if(bs->bytepos == BS_PAGE_SIZE - 1) {
		bitstream_swapin(bs);
		bs->bytepos = 0;
	} else ++bs->bytepos;
	bs->bitlen = 8;
	return;
}

unsigned int bitstream_readbits(int len, bitstream_t* bs) {
	unsigned int dst;
	if (len == 0) return 0;
	if (len > 16) assert("bitstream_readbits: len > 16");
	if (len > 16) return 0;
	if (bs->bitlen < len) {
		dst = bs->heap[bs->bytepos] << 8;
		dst = dst >> (16 - len);
		len = len - bs->bitlen;
		bitstream_prepare(bs);
		dst = dst | bitstream_readbits(len, bs);
	} else {
		dst = bs->heap[bs->bytepos] >> (8 - len);
		bs->heap[bs->bytepos] = bs->heap[bs->bytepos] << len;
		bs->bitlen = bs->bitlen - len;
	}
	return dst;
}

void bitstream_align(bitstream_t* inp) {
	if(inp->bitlen != 8) bitstream_prepare(inp);
	return;
}

void bitstream_alignread(unsigned char* dst, bitstream_t* inp) {
	if (dst) *dst = inp->heap[inp->bytepos];
	bitstream_prepare(inp);
	return;
}
