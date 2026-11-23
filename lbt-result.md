# 龙芯 LBT 翻译效率实测结果

测试日期：2026-11-23
主机：`n412-pc`，Loongson-2K3000L（8 核 2.2GHz），Debian 6.6.52-loong64

## 测试方法

用龙芯官方的 LATX 翻译器（LAT 1.6.6，基于 QEMU 深度优化、利用 LBT 硬件扩展）运行同一个 x86-64 静态二进制（Debian busybox-static 1.37），与以下两种模式做三方对照：

- **原生 LoongArch**：同版本 busybox 的 loong64 编译版（无翻译）
- **LATX（LBT 硬件）**：`/tmp/lat-tools/root/usr/bin/latx-x86_64`
- **QEMU TCG（纯软件）**：`/tmp/qemu-tools/root/usr/bin/qemu-x86_64`

每种负载多次运行取稳定值，计时用 `date +%s%N`。

## 结果

| 负载 | 原生 LoongArch | LATX（LBT 硬件） | QEMU（纯软件 TCG） | LBT vs 原生 | LBT 相对软件翻译提速 |
|---|---|---|---|---|---|
| SHA256 200MB（整数/位运算密集） | 1937 ms | 3886 ms | 5488 ms | 50%（2.0x 开销） | 1.41x |
| gzip -9 200MB（混合负载） | ~2800 ms | 4349 ms | 9183 ms | 64%（1.55x 开销） | 2.11x |
| awk 300 万次循环（解释型负载） | 4538 ms | 11860 ms | 72264 ms | 38%（2.6x 开销） | 6.1x |

## 解读

1. **LBT 硬件确实在工作**。LATX 二进制包含 `generate_eflag_by_lbt` 路径，并声明 "LAT needs LSX/LASX and LBT extension support"。能正确执行 x86 程序，前提是 `/proc/cpuinfo` 的 `lbt_x86` 标志 + 内核 `CONFIG_CPU_HAS_LBT=y`。

2. **翻译效率约为原生的 40%–65%**。计算密集型（SHA256）达到原生的一半；解释型负载（awk）最差约 38%——x86 每条指令的译码/标志位翻译开销被解释器的高频循环放大。

3. **相比纯软件翻译提升巨大**。这是 LBT 硬件的核心价值：同样跑 x86 代码，比 QEMU 纯 TCG 快 1.4x～6.1x，解释型负载提升最明显（6 倍）。没有 LBT，x86 程序在该机器上基本不可用；有了 LBT 可达到可用性能水平。

## 补充说明

- 用户态无法直接开关 LBT（CSR.EUEN 访问被 SIGILL 拦截），LATX 通过 ptrace/sigcontext 的 `NT_LOONGARCH_LBT` 上下文接口由内核受控启用，因此平时感知不到该硬件的存在。
- 本机 `features` 只有 `lsx` 无 `lasx`，LATX 自动回退到 128 位向量路径；换用支持 LASX 的龙芯 3A6000，SIMD 密集负载的翻译效率会更高。

## 相关路径

- LATX 翻译器：`/tmp/lat-tools/root/usr/bin/latx-{x86_64,i386}`
- QEMU 用户态：`/tmp/qemu-tools/root/usr/bin/qemu-x86_64`
- x86 busybox：`/tmp/x86bins/x/usr/bin/busybox`
- 原生 busybox：`/tmp/x86bins/native/usr/bin/busybox`
- awk 基准脚本：`/tmp/bench_awk.sh`
