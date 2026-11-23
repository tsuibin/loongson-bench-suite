# OpenSSL LSX vAES 有效性验证

## 背景

`libcrypto.so.3` 中包含字符串 "Vector Permutation AES for loongarch64/lsx, Mike Hamburg (Stanford University)"，且整个库含约 1200 条 LSX 向量指令（`vxor.v`/`vld`/`vst` 等）。本机 CPU 具备 `lsx`（128 位 SIMD，HWCAP bit 4），但不具备 `crypto`/`lasx`。

## 实验方法

通过 `LD_PRELOAD` 覆盖 `getauxval()`，清除 `AT_HWCAP` 的 LSX 位（bit 4），让 OpenSSL 认为 CPU 不支持 LSX，从而走纯标量 C 路径。已验证覆盖生效（`0x1c5f` → `0x1c4f`）。A/B 对比同一 `openssl speed -evp` 吞吐。

## 结果（8192 字节块，多次取稳定值）

| 算法 | LSX 启用（默认） | 屏蔽 LSX（纯标量） | 差异 |
|---|---|---|---|
| AES-128-CBC | 95.7 MB/s | 117.8 MB/s | LSX 慢 23% |
| AES-256-GCM | 47.5 MB/s | 57.1 MB/s | LSX 慢 20% |

## 结论

- **LSX vAES 确实生效**（屏蔽 hwcap 后性能变化显著，证明默认走的就是 vAES 路径）。
- **但在 2K3000L 上是负优化**：向量置换 AES 的大量洗牌/加载/异或操作在这颗 CPU 上比优化良好的标量 AES 更慢。
- 因此本机 AES 慢的根因是：既无 AES-NI（CRYPTO 扩展缺失），退而求其次的 LSX vAES 又是负优化。

## 实用建议

- 若跑 AES 密集负载，可通过 mask LSX hwcap 强制走标量路径，白得约 20% 提升。
- 长久方案：重新编译 OpenSSL 时禁用 LSX AES（`no-asm` 或关闭 loongarch vAES），强制标量实现。

## 验证用代码

```c
#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/auxv.h>
unsigned long getauxval(unsigned long type) {
    static unsigned long (*real)(unsigned long) = 0;
    if (!real) real = dlsym(RTLD_NEXT, "getauxval");
    unsigned long v = real(type);
    if (type == AT_HWCAP) v &= ~(1UL << 4);  /* clear LSX bit */
    return v;
}
```
编译：`gcc -shared -fPIC -o nolsx.so nolsx.c -ldl`
使用：`LD_PRELOAD=./nolsx.so openssl speed -evp aes-128-cbc`
