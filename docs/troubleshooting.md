# 踩坑记录 · Troubleshooting

按遇到的时间顺序追加。每条固定四段：**现象 → 根因 → 解决 → 验证**，最后补一条「注意」防止复发。

环境基线：Windows + CLion + STM32F103C8T6 + ST-Link V2 + OpenOCD 0.12.0 + `arm-none-eabi-gcc`。

---

## 001 · CLion 调试连不上 ST-LINK GDB Server（改用 OpenOCD）

**日期**：2026-11-05　**涉及**：CLion 调试配置 / OpenOCD / ST-Link

### 现象

- 点 Debug 直接报 `could not connect (error 138)`；
- 调试配置下拉框里显示 **"ST-LINK 已损坏"**；
- 之后在 GDB 里敲 `monitor reset` / `continue`，全部报 `You can't do that when your target is 'exec'`；
- 但 **Upload（烧录）是成功的**，程序能正常跑。

### 根因

问题有两层，第二层才是坑的地方：

1. CLion 的 `ST-LINK_gdbserver` 调试配置是旧版 *Debug Servers* 迁移留下的产物，里面 gdbserver 可执行文件的路径没生效。结果服务器进程压根没启动，GDB 连不上端口，于是报 138。
2. **错误码 138 是 Windows 的 `ERROR_JOIN_TO_JOIN`（系统级错误），和调试没有任何关系**，属于彻头彻尾的误导信息。顺着 138 去搜只会越查越偏。

判断方向的关键依据：**能烧录就说明硬件链路没问题**——USB、SWD 接线、目标板供电、芯片识别全都是好的。所以故障一定在「调试服务器 + 配置」这一层，不用去动杜邦线，也不用怀疑芯片锁死。

同理，"ST-LINK 已损坏" 也只是 CLion 解析固件版本失败时的误报，不是真的坏了。

### 解决

放弃 `ST-LINK_gdbserver`，改用 **OpenOCD** 做 GDB Server。配置按两个维度拼：

| 维度 | 选择依据 | 本例取值 |
| --- | --- | --- |
| interface（调试器） | 按手里的调试器选 | ST-Link → `interface/stlink.cfg` |
| target（芯片系列） | 按芯片所属系列选，不是具体型号 | STM32F103 → `target/stm32f1x.cfg` |

`interface/stlink.cfg` 用的是 `adapter driver hla` + `hla_layout stlink`，VID/PID 表里同时列了 V1 / V2 / V2-1 / V3，所以一根通用配置能兼容整条 ST-Link 产品线。

两者合并成一个文件，仓库里是 `DevEnv/stm32f1_stlink.cfg`：

```tcl
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]
reset_config none
```

启动命令（`-s` 指定 OpenOCD 的 scripts 目录，cfg 里的 `[find ...]` 靠它定位；`-f` 指定这份板级配置）：

```bash
openocd.exe -s <openocd目录>/share/openocd/scripts \
  -f D:\stm32\FreeRTOS\DevEnv\stm32f1_stlink.cfg
```

CLion 里新建 **OpenOCD** 类型的调试配置，对应填：

- GDB Server 可执行文件：`DevEnv/openocd-v0.12.0-i686-w64-mingw32/bin/openocd.exe`
- 参数：`-s <...>/share/openocd/scripts -f D:\stm32\FreeRTOS\DevEnv\stm32f1_stlink.cfg`
- 调试器（GDB）：`arm-none-eabi-gdb`

### 验证

OpenOCD 日志里同时出现下面几行，就说明目标真正连上了：

```text
Info : STLINK V2J47S7 (API v2) VID:PID 0483:3748
Info : device id = 0x410
Info : flash size = 64 KiB
** Programming Finished **
```

`0x410` 是 STM32F103 中容量器件的 device id，`64 KiB` 对应 C8T6 的 Flash——这两个值对得上，说明芯片选型没错。

### 注意

- **调试必须保留 `gdb_port`**（默认 3333）。`gdb_port disabled` 是纯烧录用参数，加上它 OpenOCD 就不再监听 GDB 端口，Debug 必然失败。
- 烧录和调试是两条独立链路：upload 走 `program ... verify reset exit`，debug 走 gdbserver + GDB 会话。一条通不代表另一条通，反之亦然——所以「能烧不能调」要直接怀疑 gdbserver，而不是接线。

---

<!-- 新条目从这里往下追加，编号顺延 -->
