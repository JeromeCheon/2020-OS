cmd_/home/jerome/2020-OS/openhook/openhook.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/jerome/2020-OS/openhook/openhook.ko /home/jerome/2020-OS/openhook/openhook.o /home/jerome/2020-OS/openhook/openhook.mod.o ;  true