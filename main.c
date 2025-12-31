#include "flac.h"
#include "portme.h"

FILE* din, *dout;

void portme_fread(void* dest, size_t len) {
	fread(dest, 1, len, din);
    return;
}

void portme_stream(int32_t left, int32_t right, int samplebits) {
    if (samplebits == 16) {
        fwrite(&left,  1, 2, dout);
        fwrite(&right, 1, 2, dout);
    } else
        assert("Other Samplebits Not Supported!");
    return;
}

int main() {
    din  = fopen("stream.flac", "rb");
    dout = fopen("result.bin", "wb");
    bitstream_t bs;
    bitstream_init(&bs);
    int32_t* buffer = malloc(4 * FLAC_CONV_BUFSIZE);
	while (!feof(din)) {
        flac_stream_t stream;
        vector_init(buffer, &stream.buffer);
        int errno;
		if (errno = flac_decode_frame(&stream, &bs)) {
            printf("errno %d\n", errno);
            break;
        }
	}
	bitstream_close(&bs);
    fclose(din);
    fclose(dout);
	return 0;
}
