# IL2206 Embedded Systems — Lab 2

This repository contains code and supporting files for **Lab 2: Introduction to Real-Time Operating Systems (RTOS)** in the KTH course **IL2206 Embedded Systems**.

It contains:

- Work with **FreeRTOS** on the ES Lab-Kit board
- Implement basic traffic light control without an RTOS
- Create simple inter-task communication via shared memory and message queues
- Build a multi-task **cruise control** application (Button, Control Law, Vehicle, Display tasks)
- Apply **rate-monotonic scheduling** and study timing behavior
- Add **watchdog and overload detection** plus one conditional extension task (timer-based watchdog, gear box, or accelerometer)

------

本仓库包含 KTH 课程 **IL2206 Embedded Systems** 中实验 **Lab 2: Introduction to Real-Time Operating Systems (RTOS)** 的代码与相关文件。

本实验在 ES Lab-Kit 开发板上使用 **FreeRTOS**，练习实时操作系统中的多任务同步与通信，包括：

- 了解 RTOS 中的任务、调度以及对象（信号量、消息队列等）
- 用 **裸机程序** 实现简易交通灯控制（不使用 FreeRTOS）
- 使用 FreeRTOS 任务与延时实现 **LED 握手（Handshake）**
- 通过共享内存实现两个任务之间的整数请求/应答通信
- 基于多个周期任务（Button / Control Law / Vehicle / Display）实现 **巡航控制（Cruise Control）** 应用
- 使用 **队列（message queues）** 连接任务，并按 **Rate-Monotonic** 原则设定周期与优先级
- 添加 **看门狗与过载检测（watchdog & overload detection）**，分析系统利用率
- 完成一个可选扩展任务。
