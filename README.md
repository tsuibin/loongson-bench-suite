# Loongson 2K3000L 完整性能评测套件

本机平台：**Loongson-2K3000L / Debian 6.6.52-loong64 / OpenSSL 3.5.4**

| 项目 | 值 |
|---|---|
| CPU | Loongson-2K3000L，8 核 @ 2.2GHz |
| 缓存 | L1d 512KB×8，L1i 512KB×8，L2 6MB 共享，无 L3 |
| 内存 | 5.8GB，DRAM 带宽 ~10 GB/s，延迟 ~100ns |
| CPU Flags | `cpucfg lam ual fpu lsx crc32 lbt_x86 lbt_arm lbt_mips` |
| 缺失特性 | LASX、CRYPTO（AES-NI/SHA-NI 等效）、SE 加解密引擎 |

## 测试报告

| 文件 | 内容 | 核心结论 |
|---|---|---|
| `lbt-result.md` | LBT 二进制翻译效率 | 翻译后为原生 38%–65%，但比 QEMU TCG 快 1.4–6.1x |
| `crypto-benchmark.md` | AES/SHA 性能与 AES-NI/SHA-NI 基线 | 无硬件加速，比 AES-NI 平台低一个数量级 |
| `vaes-verification.md` | OpenSSL LSX vAES 有效性 | vAES 生效但在本机是负优化，关闭反而快 20% |
| `memory-bench.md` | 内存带宽与延迟 | Triad ~10 GB/s，DRAM 延迟 ~100ns |
| `openssl_speed.txt` | OpenSSL 原始测试数据 | — |

## 测试工具源码

- `stream_bench.c` — STREAM 风格内存带宽基准（Copy/Scale/Add/Triad）

```bash
gcc -O2 -o stream_bench stream_bench.c
./stream_bench
```
