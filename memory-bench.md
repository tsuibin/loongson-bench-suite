# 内存性能实测（Loongson-2K3000L）

测试平台：Loongson-2K3000L @ 2.2GHz，8 核，L1d 512KB×8、共享 L2 6MB，无 L3。
测试时可用内存约 1.3–1.7GB。测试工具：自写 STREAM 风格基准（`stream_bench.c`）与指针追逐延迟程序。

## 1. 带宽（STREAM，128MB×3 数组，远大于 6MB L2，DRAM 主导）

| 操作 | 带宽 |
|---|---|
| Copy（读1写1） | 7.1–8.5 GB/s |
| Scale（读1写1+乘） | 9.5–10.1 GB/s |
| Add（读2写1） | 10.0–11.0 GB/s |
| Triad（读2写1+乘加） | 10.1–11.0 GB/s |

STREAM Triad 约 10–11 GB/s，按 2.2GHz 折算约 4.6–5.0 bytes/cycle。

## 2. 延迟（随机指针追逐，Fisher-Yates 洗牌打败硬件预取器）

| 工作集 | 延迟 | 命中层级 |
|---|---|---|
| 16–64 KB | ~3.7 ns | L1d |
| 128–256 KB | 9–12 ns | L1/L2 边界 |
| 512KB–2MB | 13–18 ns | L2（6MB 共享） |
| 4–8 MB | 40–50 ns | L2 末端/DRAM 过渡 |
| 16–128 MB | 95–110 ns | DRAM |

## 解读

- 缓存层级清晰：L1 ~3.7ns、L2 ~13–18ns、DRAM ~100ns。
- 带宽 10 GB/s 级，低于现代 x86 桌面（单通道 DDR4-3200 约 20+ GB/s），推测为单通道或低频内存控制器。
- DRAM 延迟 ~100ns 比现代 x86（60–80ns）慢。
- 无 L3，L2 仅 6MB 共享，4MB 后延迟陡增，大工作集应用不友好。

## 复现

```bash
gcc -O2 -o stream_bench stream_bench.c
./stream_bench              # 默认 16M doubles (128 MB/array)
./stream_bench 8388608      # 8M doubles (64 MB/array)
```
