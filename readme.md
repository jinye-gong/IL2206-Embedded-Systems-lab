# IL2206 Embedded Systems — Lab Projects

This repository contains my solutions and notes for the KTH course  
**IL2206 Embedded Systems** laboratory assignments:

- **Lab 1A – Concurrent Software Development in Ada**
- **Lab 1B – Real-Time Scheduling**
- **Lab 2 – Introduction to Real-Time Operating Systems (RTOS)**   

Each lab has its own folder and (mostly) its own README with more detail.

---

## Labs Overview

### Lab 1A – Concurrent Software Development in Ada

Focus: **concurrency and communication in Ada**. :contentReference[oaicite:1]{index=1}  

Topics:

- Ada tasks, **protected objects**, and rendezvous
- Implementing a **counting semaphore** as a protected object
- Multiple solutions to the **Producer–Consumer** problem:
  - using a protected object
  - using rendezvous (server task)
  - using semaphores + shared circular buffer

### Lab 1B – Real-Time Scheduling

Focus: **fixed-priority real-time scheduling theory in practice**. :contentReference[oaicite:2]{index=2}  

Topics:

- Periodic tasks and **worst-case response time (WCRT)** measurement
- Implementing **Rate-Monotonic Scheduling (RMS)**
- Comparing theoretical schedules with actual execution traces
- Adding **watchdog + helper task** for overload detection
- **Mixed scheduling**: high-priority real-time tasks + low-priority round-robin background tasks
- Experiments on **single-core vs multi-core** execution

### Lab 2 – Introduction to RTOS (FreeRTOS on ES Lab-Kit)

Focus: **task-level synchronization and communication in an RTOS** using FreeRTOS on the ES Lab-Kit board.   

Topics:

- Simple traffic light controller **without** RTOS (bare-metal)
- LED handshake using **two FreeRTOS tasks**
- Integer request/response using **shared memory** between tasks
- Multi-task **cruise control** application (Button / Control Law / Vehicle / Display tasks)
- Using **message queues** and task periods/priority (rate-monotonic inspired design)
- **Watchdog + overload detection** and overload experiments
- One **conditional/bonus task** (timer-based watchdog / gear box / accelerometer)

## Academic integrity

All files in this repository are intended **only** for study and course examination at KTH. 

Please do **not** use this repository for plagiarism, and do **not** redistribute the solutions in ways that violate KTH’s rules on collaboration and academic honesty.

---



## 中文

本仓库包含 KTH 课程 **IL2206 Embedded Systems** 的三个实验的代码：

- **Lab 1A：Ada 并发程序设计（Concurrency in Ada）**
- **Lab 1B：实时调度（Real-Time Scheduling）**
- **Lab 2：实时操作系统入门（RTOS / FreeRTOS）**   

### Lab 1A

- 学习 Ada 的并发特性：`task`、**protected object**、rendezvous  
- 自己实现一个 **计数信号量（semaphore）**  
- 用多生产者–多消费者模型练习同步与互斥（多种解法）

### Lab 1B

- 练习 **固定优先级实时调度** 与 **最坏响应时间（WCRT）**  
- 实现并分析 **Rate-Monotonic Scheduling (RMS)**  
- 设计并实现 **watchdog + overload detection**  
- 探索 **混合调度** 以及 **单核 vs 多核** 下的调度行为

### Lab 2

- 在 ES Lab-Kit 上使用 **FreeRTOS**  

- 裸机交通灯控制（不使用 RTOS）  

- 基于 FreeRTOS 任务的 LED 握手与共享内存通信  

- 实现多任务 **巡航控制（Cruise Control）** 应用  

- 使用消息队列、定时任务和 **watchdog** 实现过载检测及扩展任务

  

## 使用说明与学术诚信

- 本仓库仅用于 IL2206 课程学习与作业展示。
- 请遵守 KTH 的学术诚信政策，不要将本仓库内容用于任何形式的抄袭或违规共享。
