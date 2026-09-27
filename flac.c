#include "flac.h"

void vector_init(int32_t* buffer, vector_t* vec) {
	vec->begin = buffer;
	vec->end = buffer;
	vec->data = buffer;
}

static void vector_prev(vector_t* vec) {
	if (vec->data == vec->begin) vec->data = vec->end;
	vec->data--;
	return;
}

static void vector_next(vector_t* vec) {
	vec->data++;
	if (vec->data == vec->end) vec->data = vec->begin;
	return;
}

static void vector_push(int32_t data, vector_t* vec) {
	*vec->end = data;
	vec->end++;
	return;
}

/* sign-extend a bits-wide two's complement value (bits <= 31) */
static int32_t flac_sext(int32_t value, int bits) {
	int32_t m = (int32_t)1 << (bits - 1);
	return (value ^ m) - m;
}

/* read a signed sample of up to 17 bits (16-bit samples plus side-channel bit) */
static int32_t flac_readsample(int bits, bitstream_t* bs) {
	int32_t value;
	if (bits > 16) {
		value = ((int32_t)bitstream_readbits(bits - 16, bs) << 16)
		      | bitstream_readbits(16, bs);
	} else {
		value = bitstream_readbits(bits, bs);
	}
	return flac_sext(value, bits);
}

static void flac_stream_echo(int32_t data, flac_stream_t* stream) {
	data <<= stream->wasted;
	if (stream->channum == 0) vector_push(data, &stream->buffer);
	else {
		int32_t left = *stream->buffer.data;
		int32_t right = data;
		switch (stream->mode) {
		case 0:
			break;
		case 1:
			right = left - right;
			break;
		case 2:
			left = left + right;
			break;
		case 3: {
			//RFC 9639: mid = (left + right) >> 1, side = left - right
			int32_t mid = (left << 1) | (right & 1);
			int32_t side = right;
			left = (mid + side) >> 1;
			right = (mid - side) >> 1;
		}
		}
		portme_stream((int16_t)left, (int16_t)right, stream->samplebits);
		vector_next(&stream->buffer);
	}
}

static int32_t flac_readunary(bitstream_t* bs) {
	int32_t val = 0;
	if (bs->heap[bs->bytepos] == 0) {
		val = bs->bitlen;
		bitstream_prepare(bs);
		val += flac_readunary(bs);
	} else {
		val = __BUILTIN_CLZ32(bs->heap[bs->bytepos]) - 24;
		int x = val + 1;
		bs->heap[bs->bytepos] <<= x;
		bs->bitlen -= x;
	}
	return val;
}

static int32_t flac_readrice(int len, bitstream_t* bs) {
	int32_t msb = flac_readunary(bs);
    if (len == 0) {
        return (msb >> 1) ^ -(msb & 1);
	} else {
        int32_t result = (msb << (len - 1)) | bitstream_readbits(len - 1, bs);
        result ^= -(int32_t)bitstream_readbits(1, bs);
        return result;
	}
}

static void flac_conv(int order, int shift, int32_t residual, const int* coefs, vector_t* warmup, flac_stream_t* stream) {
	int32_t sum = 0;
	for(int k = 0; k < order; k++) {
		vector_prev(warmup);
		sum += *warmup->data * coefs[k];
	}
	int32_t result = residual + (sum >> shift);
	*warmup->data = result;
	vector_next(warmup);
	flac_stream_echo(result, stream);
	return;
}

static int flac_decode_residuals(int lpcorder, int blocksize, int shift, const int* coefs, vector_t* warmup, flac_stream_t* stream, bitstream_t* bs) {
    int method = bitstream_readbits(2, bs);
    if(method >= 2) {
        return FLAC_RESCODE_RSV;
    }

    int partitionorder = bitstream_readbits(4, bs);
    int numpartitions = 1 << partitionorder;
    if (blocksize % numpartitions != 0) {
        //block size not divisible by number of rice partitions
        return FLAC_BAD_PARTITION;
    }

    const int parambits =   method == 0 ? 4 : 5;
    const int escapeparam = method == 0 ? 0xf : 0x1f;

    int count = blocksize >> partitionorder;
    for(int i = 0; i < numpartitions; i++) {
        int n = i == 0 ? count - lpcorder : count;
        int param = bitstream_readbits(parambits, bs);
        if (param < escapeparam) {
            for (int j = 0; j < n; j++) flac_conv(lpcorder, shift, flac_readrice(param, bs), coefs, warmup, stream);
        } else {
            int numbits = bitstream_readbits(5, bs);
            for (int j = 0; j < n; j++) flac_conv(lpcorder, shift, flac_readrice(numbits, bs), coefs, warmup, stream);
        }
    }
    return FLAC_SUCCESS;
}

static const int fixed_coefs[5][4] = {{}, {1}, {2, -1}, {3, -3, 1}, {4, -6, 4, -1}};

static int flac_decode_subframe(int blocksize, int samplebits, flac_stream_t* stream, bitstream_t* bs) {
	int header = bitstream_readbits(8, bs);

	int type = header >> 1;
	int wasted = header & 1;
    if (wasted) {
        //assume that there are not a lot of wasted bits.
		while(bitstream_readbits(1, bs) == 0) wasted++;
	}
	samplebits = samplebits - wasted;
	stream->wasted = wasted;

	if (type == 0) {
		int32_t data = flac_readsample(samplebits, bs);
		for (int i = 0; i < blocksize; i++) {
			flac_stream_echo(data, stream);
		}
	} else
	if (type == 1) {
		for (int i = 0; i < blocksize; i++) {
			int32_t data = flac_readsample(samplebits, bs);
			flac_stream_echo(data, stream);
		}
	} else
	if (type >= 8 && type <= 12) {
        int lpcorder = type & 0x7;
		vector_t warmup;
		vector_init(__builtin_alloca(4 * lpcorder), &warmup);
		for(int i = 0; i < lpcorder; i++) {
			int32_t data = flac_readsample(samplebits, bs);
			vector_push(data, &warmup);
			flac_stream_echo(data, stream);
		}
		int err = flac_decode_residuals(lpcorder, blocksize, 0, fixed_coefs[lpcorder], &warmup, stream, bs);
		if (err) return err;
	} else
	if (type >= 32 && type <= 63) {
		int lpcorder = (type & 0x1f) + 1;
		vector_t warmup;
		vector_init(__builtin_alloca(4 * lpcorder), &warmup);
		for(int i = 0; i < lpcorder; i++) {
			int32_t data = flac_readsample(samplebits, bs);
			vector_push(data, &warmup);
			flac_stream_echo(data, stream);
		}
		int precision = bitstream_readbits(4, bs);
		int shift = bitstream_readbits(5, bs);
        int coefs[32];
        for(int i = 0; i < lpcorder; i++) {
			coefs[i] = flac_sext(bitstream_readbits(precision + 1, bs), precision + 1);
		}
		int err = flac_decode_residuals(lpcorder, blocksize, shift, coefs, &warmup, stream, bs);
		if (err) return err;
	} else {
        return FLAC_BAD_SUBFRAME;
	}
	return FLAC_SUCCESS;
}

int flac_decode_frame(flac_stream_t* stream, bitstream_t* bs) {
	unsigned char header[4];

    //get header
	bitstream_alignread(&header[0], bs);
	bitstream_alignread(&header[1], bs);
	bitstream_alignread(&header[2], bs);
	bitstream_alignread(&header[3], bs);
	if(header[0] != 0xff || (header[1] & 0xfc) != 0xf8) {
		return FLAC_BAD_SYNC;
	}

	//bypass frame utf8 string
	unsigned char utf8;
	bitstream_alignread(&utf8, bs);
	while(utf8 >= 0xc0) {
		bitstream_alignread(NULL, bs);
		utf8 = (utf8 << 1) & 0xff;
	}

	//get codes
	int blocksizecode  	= header[2] >> 4;
	int sampleratecode	= header[2] & 0xf;
	int chanasgn 		= header[3] >> 4;
	int samplesizecode 	= header[3] & 0xf;

    //parse blocksize
	int blocksize;
	if (blocksizecode == 1) {
		blocksize = 192;
	} else
	if (blocksizecode >= 2 && blocksizecode <= 5) {
		blocksize = 576 << (blocksizecode - 2);
	} else
	if (blocksizecode >= 8 && blocksizecode <= 15) {
		blocksize = 256 << (blocksizecode - 8);
	} else
	if (blocksizecode == 6) {
		unsigned char x;
		bitstream_alignread(&x, bs);
		blocksize = x + 1;
	} else
	if (blocksizecode == 7) {
		unsigned char x;
		bitstream_alignread(&x, bs);
		blocksize = x;
		bitstream_alignread(&x, bs);
		blocksize = (blocksize << 8) + x + 1;
	} else {
		return FLAC_BLKSIZE_NOSUPPORT;
	}

	//bypass header CRC
	bitstream_alignread(NULL, bs);

    //test sampleratecode
	if(sampleratecode != 0xa) {
		return FLAC_SR_NOSUPPORT;   //48kHz only
	}

	//test chanasgn
	int mode;
	if (chanasgn == 1) mode = 0;
	else if (chanasgn == 8) mode = 1;
	else if (chanasgn == 9) mode = 2;
	else if (chanasgn == 10) mode = 3;
	else {
        return FLAC_CHAN_NOSUPPORT; //stereo only
	}
	stream->mode = mode;

	//test samplesizecode
	int samplebits = 16;
	if (samplesizecode != 0x8) {
        return FLAC_BITS_NOSUPPORT; //16-bit only
	}
	stream->samplebits = samplebits;

	//decode subframe
	int err;
	stream->channum = 0;
    if ((err = flac_decode_subframe(blocksize, samplebits + ((mode == 2) ? 1 : 0), stream, bs)))
        return err;
    stream->channum = 1;
    if ((err = flac_decode_subframe(blocksize, samplebits + ((mode == 2) || (mode == 0) ? 0 : 1), stream, bs)))
        return err;

    //align to byte
    bitstream_align(bs);

    //bypass frame CRC
    bitstream_alignread(NULL, bs);
    bitstream_alignread(NULL, bs);
	return FLAC_SUCCESS;
}
