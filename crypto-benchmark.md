# AES/SHA 性能与 AES-NI/SHA-NI 基线对比

## 本机实测（OpenSSL 3.5.4，`-evp`，8192 字节块）

| 算法 | 吞吐 | 换算 |
|---|---|---|
| AES-128-CBC | 95586k B/s | 95.6 MB/s |
| AES-256-CBC | 68180k B/s | 68.2 MB/s |
| AES-128-GCM | 59500k B/s | 59.5 MB/s |
| AES-256-GCM | 47725k B/s | 47.7 MB/s |
| SHA1 | 369424k B/s | 369 MB/s |
| SHA256 | 138524k B/s | 138.5 MB/s |
| SHA512 | 267366k B/s | 267 MB/s |
| SHA3-256 | 158218k B/s | 158 MB/s |

原始输出见 `openssl_speed.txt`。输出中 `CPUINFO: N/A`，OpenSSL 未检测到 AES/SHA 硬件加速能力。

## AES-NI / SHA-NI 基线对比

基线来源：calomel.org（AES-NI 开/关对照）、tinycomputers（i5-2435M vs Cortex-A76）、CSDN/J4125（AES-NI 开关对照）、公开 SHA-NI 基准。

| 算法 | 本机 2K3000L（纯软件） | 软件基线 | AES-NI/SHA-NI 硬件基线 | 硬件/本机 |
|---|---|---|---|---|
| AES-128-CBC | 95.6 MB/s | 85–297 MB/s | 576–1075 MB/s | 6–11x |
| AES-256-CBC | 68.2 MB/s | — | ~792 MB/s | ~11.6x |
| AES-128-GCM | 59.5 MB/s | 72–212 MB/s | 580–1357 MB/s | 10–23x |
| AES-256-GCM | 47.7 MB/s | — | 988–3603 MB/s | 20–75x |
| SHA256 | 138.5 MB/s | 110–251 MB/s | 1429–2050 MB/s | 10–15x |
| SHA512 | 267 MB/s | 361–376 MB/s | 无加速，同档 | ~1.3x |

## 结论

- AES 硬件加速差距最大（6–23x）；SHA256 次之（10–15x）。
- SHA512 是公平对照：x86/ARM/龙芯都无硬件加速，本机 267 MB/s 与 i5 的 361 MB/s、A76 的 376 MB/s 同数量级，说明 2K3000L 通用整数运算能力并不弱，差距完全来自缺失加密专用指令。
- 实际影响集中在大量 TLS/磁盘加密场景（HTTPS 服务器、LUKS），软件加密会吃满 CPU。
