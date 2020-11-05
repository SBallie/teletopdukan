
    _/_/_/_/_/                                _/                    _/_/      _/_/_/
       _/      _/  _/_/    _/_/_/  _/_/_/    _/_/_/    _/    _/  _/    _/  _/
      _/      _/_/      _/    _/  _/    _/  _/    _/  _/    _/  _/    _/    _/_/
     _/      _/        _/    _/  _/    _/  _/    _/  _/    _/  _/    _/        _/
    _/      _/          _/_/_/  _/    _/  _/_/_/      _/_/_/    _/_/    _/_/_/
                                                         _/
                                                    _/_/


Tranby OS
===========

Developing x86 simple operating system to learn the fundamentals of the boot process, device interaction, among other things ...


Try it out TranbyOS (tested on macOS, should work on any host where QEMU runs)

- http://www.qemu-project.org/download
- https://github.com/stevetranby/tranbyos/blob/master/tools/grub_disk_backup.img
- https://github.com/stevetranby/tranbyos/blob/master/tools/tranbyos-hd-32mb.img

## Used run commands during testing:
```
qemu-system-i386 -m 64 -rtc base=localtime,clock=host,driftfix=slew -hda tranbyos-hd-32mb.img -hdb tranbyos-hd-32mb.img -vga std -serial stdio -fda grub_disk_backup.img
```
Simpler Method that should still work: (try -hda if -hdb doesn't work)
```
qemu-system-i386  -m 64 -hdb tranbyos-hd-32mb.img -vga std -serial stdio -fda grub_disk_backup.img
```
Press a key until asks for [SPACE]

Should see debug logs in stdout (assume ru