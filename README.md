# wasm_os

**Short:** Hosting programs compiled to WebAssembly in a user-space virtual operating system.

**Stage:** Prototype / Proof of concept


## A Virtual Operative System

The dream: Imagine your operating system being freed from the hardware it runs on. It it not running on either your phone or on your desktop or in the cloud, it runs on all of them at the same time. You always have access to all your files and applications, no matter where you are. You no longer have to think in terms of 'which' computer has these files and these apps. Computing tasks can be moved between systems seamlessly depending on the needs at any given moment.

A virtual operating system is not a widely used term. But I define it as this: An operative system instance that runs independently of the underlaying hardware and, like a virtual machine, can be moved between platforms without having to be changed.

I imagine this could be implemented as a high-level kernel, that runs on top of an existing operative system, like Linux or Windows or Android, in user-space. Instead of having hardware drivers, it has software driver. Its a High-level abstraction layer instead of a low-level one. One driver for instance provides access to the OpenGL API, another provides socket access, the file system is abstracted through the standard libc file system API. The files can be served directly from the disk or from some cloud storage provider.

Obviously this would come with the caveat that not all applications are not suited to be running everywhere, for example some games needs to run on the desktop with direct access to GPU hardware. But that is really a question of hardware capabilities which changes constantly. There will be a time in the future, where phones can run todays AAA games. Abstracting away the operative system will ensure that the software we enjoy today can be used many years into the future.

Web assembly is a bytecode language that is becoming increasingly popular and that many compilers( for example LLVM) support as a compilation target.  Many existing applications can be compiled and executed directly on this system, all it needs is that the APIs of those applications are implemented in the WebAssembly runtime library.

I have previously implemented a WebAssembly interpreter(https://github.com/rolfrm/awsm).

My plan is to extend it and build an application around it that serves as an 'operative system'.

I will add support for commonly used APIs so that it will become possible to compile common programs for it.

## High Level Goals

- Implement a simple working shell interface
- Compile and run some relative large existing application(for example https://github.com/id-Software/DOOM/)
- Support moving running applications between devices.
- Abstract the file system between devices and allow using other devices' files seamlessly
- Support Linux, Windows and Android.