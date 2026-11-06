# 踩坑记录

## 001 · CLion 连不上 ST-LINK（error 138）

**错误**

1. Debug 报 `could not connect (error 138)`
2. 调试配置里显示 "ST-LINK 已损坏"
3. 烧录正常 —— ST-Link 和板子没坏，是 CLion 的调试服务器配置失效

**解决**

1. 右上角编译按钮旁边 → **编辑配置**
2. 左上角 **+** → **OpenOCD Download and Run**
3. **可执行文件** → 选 `openocd.exe`（在 `DevEnv/` 里）
4. **Board config file** → 选 `D:\stm32\FreeRTOS\DevEnv\stm32f1_stlink.cfg`
5. 点 **下载**，程序烧进单片机

成功标志：日志出现 `device id = 0x410`、`Programming Finished`

<!-- 新条目往下追加，编号顺延 -->
