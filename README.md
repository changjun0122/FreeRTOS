# FreeRTOS 学习记录 · STM32F103C8T6

这是我学习 FreeRTOS 的代码仓库。每个 `Pxx_` 目录是一个可独立编译、烧录的最小例程，按「任务 → 队列 → 后续同步机制」的顺序逐步推进，用来把 RTOS 的概念在真实硬件上跑一遍。

芯片是 **STM32F103C8T6**（Cortex-M3，64 KB Flash / 20 KB RAM），RTOS 内核为 **FreeRTOS V10.3.1**，通过 STM32CubeMX 生成 **CMSIS-RTOS v2** 封装层，构建系统为 **CMake + Ninja + arm-none-eabi-gcc**。

---

## 目录结构

```text
FreeRTOS/
├── P1_FirstTask/                # 例程 1：任务创建与抢占式调度
│   ├── Core/
│   │   ├── Inc/                 # main.h / FreeRTOSConfig.h 等头文件
│   │   ├── Src/                 # main.c / freertos.c / stm32f1xx_it.c ...
│   │   └── Startup/             # 启动汇编 startup_stm32f103c8tx.s
│   ├── Drivers/                 # STM32F1 HAL 库 + CMSIS
│   ├── Middlewares/
│   │   └── Third_Party/FreeRTOS/ # FreeRTOS 内核源码 + CMSIS-RTOS v2
│   ├── cmake/                   # 交叉编译工具链文件、CubeMX 源文件清单
│   ├── CMakeLists.txt
│   ├── CMakePresets.json        # Debug / Release 预设
│   ├── P1_FirstTask.ioc         # CubeMX 工程配置（引脚、时钟、RTOS 参数）
│   ├── STM32F103XX_FLASH.ld     # 链接脚本
│   └── startup_stm32f103xb.s
│
├── P2_Queue/                    # 例程 2：消息队列（结构同上）
│   └── ...
│
├── docs/
│   └── troubleshooting.md       # 踩坑记录：现象 → 根因 → 解决 → 验证
│
├── DevEnv/                      # 本地工具链（arm-none-eabi-gcc + OpenOCD，不入库）
└── .gitignore
```

两个工程结构完全一致，`Drivers/` 与 `Middlewares/` 是 CubeMX 随工程一起拷贝的库代码，换芯片型号时由 CubeMX 重新生成。

## 例程一览

| 目录 | 主题 | 涉及的内核对象 | 现象 |
| --- | --- | --- | --- |
| `P1_FirstTask` | 多任务创建与优先级调度 | 2 个动态任务 | LED（PB0）以 1 tick 周期翻转；USART2（115200）每 200 ms 打印 `Hello World!` |
| `P2_Queue` | 任务间通信：消息队列 | `BtnQueue`（深度 16，元素 `uint32_t`）+ 2 个任务 | 按键（PB12）消抖计数并 `osMessageQueuePut` 入队；`DataTask` 阻塞在 `osMessageQueueGet`，取到后延时 1 s 打印「按键次数 / 数据次数」 |

`P1_FirstTask` 里两个任务的优先级故意拉开（Normal 24 / Low 8），用来观察高优先级任务长期占用 CPU 时低优先级任务被饿死、以及 `osDelay` 让出 CPU 的效果。

## 主要模块说明

| 文件 / 目录 | 作用 |
| --- | --- |
| `Core/Src/main.c` | 时钟与外设初始化、`osKernelInitialize()` / `osKernelStart()`、任务创建 |
| `Core/Src/freertos.c` | `MX_FREERTOS_Init()`：创建队列/任务；`StartXxxTask()` 任务函数体 |
| `Core/Inc/FreeRTOSConfig.h` | 内核裁剪配置：`configTICK_RATE_HZ = 1000`、`configMAX_PRIORITIES = 56`、`configTOTAL_HEAP_SIZE = 3072`（heap_4） |
| `Middlewares/Third_Party/FreeRTOS/Source` | 内核本体：`tasks.c`、`queue.c`、`list.c`、`timers.c`、`event_groups.c`，移植层在 `portable/GCC/ARM_CM3` |
| `cmake/stm32cubemx/` | CubeMX 自动收集的源文件列表，新增 `.c` 后需重新生成或手动加入 |
| `*.ioc` | 图形化配置的唯一真源，改动引脚/时钟/任务后重新生成代码 |

> 约定：CubeMX 生成的代码不要手改，自己的逻辑只写在 `/* USER CODE BEGIN */` 与 `/* USER CODE END */` 之间，这样重新生成时不会丢代码。

## 编译与烧录

仓库**不包含工具链**（`DevEnv/` 约 1.2 GB，已被 `.gitignore` 排除）。本地需要先装好 `arm-none-eabi-gcc` 并加入 `PATH`，另需 CMake ≥ 3.22 与 Ninja。

```bash
cd P2_Queue
cmake --preset Debug        # 生成 build/Debug
cmake --build --preset Debug
```

产物为 `build/Debug/P2_Queue.elf`，烧录用 OpenOCD + ST-Link：

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/P2_Queue.elf verify reset exit"
```

没有 ST-Link 时也可以用 STM32CubeProgrammer 直接下载 `.elf` / `.hex`。

## 调试（OpenOCD）

CLion 自带的 `ST-LINK_gdbserver` 在本机配置失效（现象与排查过程见 [docs/troubleshooting.md](docs/troubleshooting.md) 第 001 条），所以统一改用 OpenOCD 当 GDB Server。

`DevEnv/stm32f1_stlink.cfg` 已经把「调试器 + 芯片」两层配置拼在一起，内容是：

```tcl
source [find interface/stlink.cfg]   # ST-Link：hla 驱动，兼容 V1/V2/V2-1/V3
source [find target/stm32f1x.cfg]    # STM32F1 系列（Cortex-M3）
reset_config none
```

手动拉起服务器：

```bash
openocd.exe -s <openocd目录>/share/openocd/scripts \
  -f D:\stm32\FreeRTOS\DevEnv\stm32f1_stlink.cfg
```

日志里出现 `device id = 0x410` 和 `flash size = 64 KiB` 就说明连上了目标。注意调试时不能加 `gdb_port disabled`——那是纯烧录用的参数，一旦加上 OpenOCD 就不监听 GDB 端口。

## 版本控制约定

以下内容**不入库**，都能由源码或 `.ioc` 重新生成，避免仓库被几十 MB 的中间文件撑爆：

- 编译产物：`build/`、`*.o` / `*.d` / `*.elf` / `*.map` / `*.su` 等
- IDE 配置：`.idea/`、`.vscode/`、`.settings/`、`.cproject`、`.project`
- 工具生成的过程文件：`.mxproject`
- 本地工具链：`DevEnv/`、`DevEnv.zip`

## 学习进度

- [x] P1 任务的创建、优先级与 `osDelay` 让出 CPU
- [x] P2 消息队列：生产者/消费者模型、阻塞等待
- [ ] 二值信号量 / 计数信号量
- [ ] 互斥量（优先级继承）
- [ ] 事件标志组
- [ ] 软件定时器
