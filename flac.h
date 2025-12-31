#ifndef __FLAC_H__
#define __FLAC_H__

#include "portme.h"
#include "bitstream.h"

#define FLAC_SUCCESS            0
#define FLAC_BAD_SYNC           -1
#define FLAC_BLKSIZE_NOSUPPORT  -2
#define FLAC_SR_NOSUPPORT       -3
#define FLAC_CHAN_NOSUPPORT     -4
#define FLAC_BITS_NOSUPPORT     -5
#define FLAC_RESCODE_RSV        -6
#define FLAC_RESCODE_NOSUPPORT  -7
#define FLAC_BAD_PARTITION      -8
#define FLAC_BAD_SUBFRAME       -9

#define FLAC_CONV_BUFSIZE       4608

struct vector {
	int32_t* begin;
	int32_t* end;
	int32_t* data;
};
typedef struct vector vector_t;

void vector_init(int32_t* buffer, vector_t* vec);

struct flac_stream {
	int mode;
	int wasted;
	int channum;
    int samplebits;
	vector_t buffer;
};
typedef struct flac_stream flac_stream_t;

int flac_decode_frame(flac_stream_t* stream, bitstream_t* bs);

#endif
