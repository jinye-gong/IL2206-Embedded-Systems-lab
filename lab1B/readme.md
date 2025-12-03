#  IL2206 Embedded Systems — Lab 1B

This repository contains code and materials for **Lab 1B: Real-Time Scheduling** in the KTH course **IL2206 Embedded Systems**.:contentReference[oaicite:1]{index=1}  

The lab focuses on **applying real-time scheduling theory in practice**, including:

- Running real-time programs using the **Ada Real-Time Annex** or **FreeRTOS on the Embedded Systems Lab-Kit board**  
- Implementing and analysing **periodic tasks** with fixed-priority scheduling and measuring worst-case response times (WCRT)  
- Designing and implementing a **Rate-Monotonic Scheduling (RMS)** task set and comparing theoretical and measured schedules  
- Adding a **watchdog timer** and helper task for **overload detection**  
- Implementing **mixed scheduling** with high-priority real-time tasks and low-priority round-robin background tasks  
- Studying the differences between **single-processor and multi-processor execution** for the same real-time workload  

------

本仓库包含 KTH 课程 **IL2206 Embedded Systems** 中实验 **Lab 1B: Real-Time Scheduling** 的代码与相关文件。

本实验主要练习 **实时调度理论在实际系统中的应用**，包括：

- 使用 **Ada Real-Time Annex** 或 **FreeRTOS + Lab-Kit 开发板** 运行实时任务  
- 分析与实现 **周期任务（periodic tasks）** 的优先级调度与最坏响应时间（WCRT）测量  
- 使用 **Rate-Monotonic Scheduling (RMS)** 设计和实现任务集，并比较理论调度与实际运行结果  
- 通过 **watchdog timer** 和辅助任务进行 **系统过载检测（overload detection）**  
- 实现 **混合调度（mixed scheduling）**：高优先级实时任务 + 低优先级轮转调度后台任务  
- 比较 **单核 vs 多核** 处理器上实时程序的执行差异  
