# Flac Decode Library

This Project is under **Work-In-Progress** now  
**该项目还未完善**

If any questions, welcome for Issues & PRs   
如果有疑问，欢迎提交Issues或PR

**WARNING! This Project is not been fully long-term productive tested and without warranty of any kind, use at your own risk!**   
**警告！该项目未经充分的长期生产环境测试，作者不作任何保证，使用需要你自己衡量**

### License

This project is licensed under the LGPL-2.1-or-later license. **DO NOT** download or clone this project until you have read and agree the LICENSE.     
该项目采用 `LGPL-2.1 以及之后版本` 授权。当你下载或克隆项目时，默认已经阅读并同意该协定。

### Overview

This Project is used to demo `FLAC` audio decode, which is fully optimized and could be used at any target.
该项目用于 `FLAC` 音频展示, 经过优化可用于任何设备。

Currently supports: any sample rate, 8/12/16/20/24-bit, stereo (independent / left-side / right-side / mid-side), constant / verbatim / fixed / LPC subframes, blocksize up to 4608. Mono, multichannel and headerless sample-size streams are cleanly rejected.
当前支持: 任意采样率、8/12/16/20/24-bit、立体声（独立/左-边/右-边/中-边）、constant / verbatim / fixed / LPC 子帧，blocksize 最大 4608。单声道、多声道以及位深需查 STREAMINFO 的流会被干净地拒绝。

Verified bit-exact against ffmpeg on the official IETF CELLAR
[flac-test-files](https://github.com/ietf-wg-cellar/flac-test-files) suite:
`tests/run_official_suite.sh`
已通过 IETF CELLAR 官方测试集与 ffmpeg 逐字节比对验证。

### Layout 结构

- `bitstream.[ch]`: paged bit reader with one-page prefetch 分页预取位流读取器
- `flac.[ch]`: frame/subframe decoder (RFC 9639) 帧与子帧解码器
- `portme.[ch]`: target-specific hooks 平台移植接口
  - `portme_fread(dest, len)` -> bytes actually read (`< len` at end of stream)
  - `portme_stream(left, right, samplebits)` -> decoded PCM output
- `main.c`: hosted demo (decodes a `.flac` file to interleaved left-justified PCM)

### Portability 可移植性

The decoder is 16-bit-MCU friendly: all sample storage uses `int32_t`
(a `long` on 16-bit targets, holding up to 25-bit side-channel samples),
no 32-bit `int` assumption. The only 64-bit math is the LPC accumulator,
which the format requires for 20/24-bit content. Define `STATIC_MEM` to
avoid `malloc`, `PORTME_NO_STDINT` if `<stdint.h>` is unavailable,
`HAVE_STDIO` on hosted targets.

本解码器对 16 位 MCU 友好：采样存储统一使用 `int32_t`（在 16 位平台
上即 `long`，可容纳最高 25 位边带采样），不假设 32 位 `int`。唯一的
64 位运算是 LPC 累加器，这是 20/24-bit 内容的格式必需。

### API 调用方式

```c
static int32_t buffer[FLAC_CONV_BUFSIZE];
flac_stream_t stream;
flac_stream_init(&stream, buffer);   /* one-time setup 一次性初始化 */

bitstream_t bs;
bitstream_init(&bs);
while (!bitstream_end(&bs)) {
    int err = flac_decode_frame(&stream, &bs);
    if (err) break;                  /* negative FLAC_* error code */
    /* stream->info: sample_rate / channels / samplebits / blocksize */
}
```

Decoded sample pairs are delivered through `portme_stream()`.
解码后的立体声采样通过 `portme_stream()` 回调输出。

### Host Build 主机构建

```sh
make          # produces flacdemo
./flacdemo input.flac output.pcm
```

### Reference
- Beurden, Martijn van, and Anew Weaver. 2024. Free Lossless Audio Codec (FLAC). Request for Comments RFC 9639. Internet Engineering Task Force. https://doi.org/10.17487/RFC9639.
