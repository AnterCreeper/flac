#include "flac.h"
#include "portme.h"

static FILE* din, *dout;

unsigned int portme_fread(void* dest, unsigned int len) {
	return fread(dest, 1, len, din);
}

void portme_stream(int16_t left, int16_t right, int samplebits) {
    if (samplebits == 16) {
        fwrite(&left,  1, 2, dout);
        fwrite(&right, 1, 2, dout);
    } else
        assert("Other Samplebits Not Supported!");
    return;
}

/* skip "fLaC" marker and all metadata blocks; raw frame streams are left alone */
static void skip_metadata(FILE* f) {
    unsigned char hdr[4];
    if (fread(hdr, 1, 4, f) != 4) return;
    if (hdr[0] != 'f' || hdr[1] != 'L' || hdr[2] != 'a' || hdr[3] != 'C') {
        fseek(f, 0, SEEK_SET);
        return;
    }
    for (;;) {
        unsigned char blk[4];
        if (fread(blk, 1, 4, f) != 4) return;
        unsigned long len = ((unsigned long)blk[1] << 16) | ((unsigned long)blk[2] << 8) | blk[3];
        fseek(f, (long)len, SEEK_CUR);
        if (blk[0] & 0x80) break; //last metadata block
    }
    return;
}

int main(int argc, char** argv) {
    din  = fopen(argc > 1 ? argv[1] : "stream.flac", "rb");
    dout = fopen(argc > 2 ? argv[2] : "result.bin", "wb");
    if (!din || !dout) {
        printf("cannot open file\n");
        return 1;
    }
    skip_metadata(din);

    bitstream_t bs;
    bitstream_init(&bs);
    static int32_t buffer[FLAC_CONV_BUFSIZE];

    flac_stream_t stream;
	while (!bitstream_end(&bs)) {
        vector_init(buffer, &stream.buffer);
        int err = flac_decode_frame(&stream, &bs);
		if (err) {
            printf("errno %d\n", err);
            break;
        }
	}
	bitstream_close(&bs);
    fclose(din);
    fclose(dout);
	return 0;
}
