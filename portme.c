#include "portme.h"

void __assert(const char* str, int line) {
#ifdef HAVE_STDIO
    printf("assert fault at line %d, reason %s\r\n", line, str);
#endif
    while(1);
    __builtin_unreachable();
    return;
}

/**
 * @brief target specific non-OS fread implementation
 * @note This function will wait for the device to become idle, then submit new read command and return immediately without blocking.
 * @param dest the pointer where data be loaded
 * @param len the size of data
 * @return the number of bytes actually read, < len means end of stream
 */
__attribute((weak)) unsigned int portme_fread(void* dest, unsigned int len) {
    (void)dest; (void)len;
    assert("Unimplemented portme_fread(void*, unsigned int)!");
    return 0;
}

/**
 * @brief target specific non-OS stream implementation
 * @note This function will stream the data to device.
 * @param left the pcm data of left channel
 * @param right the pcm data of right channel
 * @param samplebits the sample bits of data
 * @return
 */
__attribute((weak)) void portme_stream(int32_t left, int32_t right, int samplebits) {
    (void)left; (void)right; (void)samplebits;
    assert("Unimplemented portme_stream(int32_t, int32_t, int)!");
    return;
}
