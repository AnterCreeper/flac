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

Currently supports: 48 kHz, 16-bit, stereo (independent / left-side / right-side / mid-side), constant / verbatim / fixed / LPC subframes.
当前支持: 48 kHz、16-bit、立体声（独立/左-边/右-边/中-边），constant / verbatim / fixed / LPC 子帧。

### Layout 结构

- `bitstream.[ch]`: paged bit reader with one-page prefetch 分页预取位流读取器
- `flac.[ch]`: frame/subframe decoder (RFC 9639) 帧与子帧解码器
- `portme.[ch]`: target-specific hooks 平台移植接口
  - `portme_fread(dest, len)` -> bytes actually read (`< len` at end of stream)
  - `portme_stream(left, right, samplebits)` -> decoded PCM output
- `main.c`: hosted demo (decodes a `.flac` file to interleaved s16le PCM)

### Portability 可移植性

All arithmetic is 16-bit-MCU friendly: samples are `int16_t`, and `int32_t`
(a `long` on 16-bit targets) is only used where the format requires it
(17-bit side channel, LPC accumulator). No 64-bit math, no 32-bit `int`
assumption. Define `STATIC_MEM` to avoid `malloc`, `PORTME_NO_STDINT` if
`<stdint.h>` is unavailable, `HAVE_STDIO` on hosted targets.

全部计算对 16 位 MCU 友好：采样点为 `int16_t`，仅在格式必需处使用
`int32_t`（在 16 位平台上即 `long`，用于 17 位边带与 LPC 累加器）。
无 64 位运算，不假设 32 位 `int`。

### Host Build 主机构建

```sh
make          # produces flacdemo
./flacdemo input.flac output.pcm
```

Verified bit-exact against ffmpeg-decoded PCM (all stereo modes, block
sizes 192/576/200/1000/4608, compression levels 0/5/12).
已与 ffmpeg 解码 PCM 逐字节比对验证。

### Reference
- Beurden, Martijn van, and Anew Weaver. 2024. Free Lossless Audio Codec (FLAC). Request for Comments RFC 9639. Internet Engineering Task Force. https://doi.org/10.17487/RFC9639.
