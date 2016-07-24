# Design of Trantor OS

## Device Support

1. Graphics Card - Shared buffer
1. Keyboard - Interrupt
1. IDE (HD/CD-ROM) - Shared buffer
1. RTC - Interrupt
1. Timer - Interrupt
1. Mouse - Interrupt
1. Soundcard - Shared buffer

## Kernel Functions

- Create / terminate process
- Create / terminate thread
- Timer
- IPC

## Boot Process

The multiboot loader loads the kernel image (kinit). The kinit process (pid=0)
does the following things:

1. Initialize the memory manager.
1. Initialize the exception handlers.
1. Initialize the IRQ handlers.
1. Initialize the timer.
1. There are two multiboot modules loaded - vfs and initrd image. Start
   the vfs process (pid=1).
1. Mount the initrd content at /.
1. Start /sbin/relauncher (pid=2).
1. relauncher will read all commands in /etc/relauncher.conf and start driver processes.
   - There will be some mechanism to run /sbin/drivers/sysfs,
     /sbin/drivers/devfs, and /sbin/hwscan first and only on completion load all
     other drivers.
   - The first two will setup the sysfs mount at /sys and devfs mount at /dev.
1. relauncher sends a message to kinit on successful driver loading.
1. kinit then switches the root mount to the root device specified in the
   multiboot parameters. switch-mount will also remount /sys and /dev.
1. kinit then sends a message to relauncher of kinit_complete and exits.
1. relauncher launches /sbin/init on receiving kinit_complete. This starts the
   user level initialization.
