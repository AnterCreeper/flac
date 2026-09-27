#ifndef __FLAC_H__
#define __FLAC_H__

#include "portme.h"
#include "bitstream.h"

/* flac_decode_frame() return values */
#define FLAC_SUCCESS            0
#define FLAC_BAD_SYNC           -1  //frame sync lost or input not parsable
#define FLAC_BLKSIZE_NOSUPPORT  -2  //reserved blocksize code, or blocksize > FLAC_CONV_BUFSIZE
#define FLAC_SR_NOSUPPORT       -3  //reserved samplerate code
#define FLAC_CHAN_NOSUPPORT     -4  //not a stereo stream
#define FLAC_BITS_NOSUPPORT     -5  //bit depth deferred to STREAMINFO or reserved
#define FLAC_RESCODE_RSV        -6  //reserved residual coding method
#define FLAC_RESCODE_NOSUPPORT  -7
#define FLAC_BAD_PARTITION      -8  //blocksize not divisible by rice partitions
#define FLAC_BAD_SUBFRAME       -9  //reserved subframe type

/* maximum supported blocksize; the workspace must hold this many int32_t samples */
#define FLAC_CONV_BUFSIZE       4608

/* parameters of the most recently decoded frame, read-only for callers */
struct flac_info {
	int sample_rate;	//Hz, or 0 when the frame defers to STREAMINFO
	int channels;		//currently always 2
	int samplebits;		//8, 12, 16, 20 or 24
	int blocksize;		//samples per channel
};
typedef struct flac_info flac_info_t;

/* internal ring buffer state, do not touch */
struct vector {
	int32_t* begin;
	int32_t* end;
	int32_t* data;
};
typedef struct vector vector_t;

struct flac_stream {
	flac_info_t info;

	/* internal state, do not touch */
	int mode;
	int wasted;
	int channum;
	vector_t buffer;
};
typedef struct flac_stream flac_stream_t;

/* one-time setup; buffer must hold FLAC_CONV_BUFSIZE int32_t */
void flac_stream_init(flac_stream_t* stream, int32_t* buffer);

/* decode one frame; returns FLAC_SUCCESS and updates stream->info, or a negative error code */
int flac_decode_frame(flac_stream_t* stream, bitstream_t* bs);

#endif
