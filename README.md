# STM32 PLC Runtime Engine and Hardware Design

An STM32F407-based PLC runtime engine developed in Embedded C, together with a custom PLC controller PCB and a Renode-based simulation environment.

The project focuses on designing a **reusable PLC runtime platform** rather than a single-purpose STM32 application. The architecture separates the permanent runtime, hardware abstraction, process image, communication, and user control logic.

---

## Project Goal

The main objective is to develop an embedded PLC platform capable of providing the core functions of a programmable industrial controller:

- PLC operating modes
- Cyclic and periodic program execution
- Process-image based I/O
- Digital and analog I/O management
- Retentive PLC variables
- Industrial communication
- Runtime monitoring
- Real-time task management
- Custom PLC hardware
- Firmware simulation and testing

The long-term architecture is based on separating the **permanent PLC runtime** from the **user program**, providing the foundation for a configurable and programmable embedded PLC.

## Runtime Execution and Validation

The following logs demonstrate PLC operating-mode transitions,
power recovery handling, and runtime execution.

<img width="800" height="643" alt="image" src="https://github.com/user-attachments/assets/07540d29-cd8a-40ad-a0b9-ed906f1b3bc7" />


## I/O Mapping Architecture

The following diagrams illustrate the input reading and output
writing processes, including hardware-to-process-image mapping.

<p align="center">
  <img src="images/<img width="800" height="643" alt="image" src="https://github.com/user-attachments/assets/47ae3f9d-df16-4146-83c8-283ea7eb489a" />
" width="90%">
</p>

---

## Project Goal
