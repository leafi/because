rm -rf tmp
mkdir tmp

echo compiling kernel bits

echo kernel.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/kernel.o -c kernel.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo pci.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/pci.o -c pci.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ps2.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/ps2.o -c ps2.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo apic.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/apic.o -c apic.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo pic.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/pic.o -c pic.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo serial.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/serial.o -c serial.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ata_pio.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/ata_pio.o -c ata_pio.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo bga.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/bga.o -c bga.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding

/usr/local/bin/nasm -f elf64 -o tmp/bits.o bits.asm

echo linking flat 64bit kernel binary
# we use /usr/bin/local/ld b/c i compile on a mac and the default ld is inadequate.
# /usr/bin/local/ld can be installed by doing 'brew install binutils' if you have homebrew.
# if you already have gnu ld, you can just use that instead.
#
$HOME/opt/cross/bin/x86_64-elf-ld -T linker64.ld -o tmp/kernel64.sys tmp/kernel.o tmp/bits.o tmp/pci.o tmp/ps2.o tmp/apic.o tmp/pic.o tmp/serial.o tmp/ata_pio.o tmp/bga.o || exit

echo composing disk image
rm -rf disk.img
cp pure64/Pure64.img disk.img
echo attaching >boring.log
hdiutil attach disk.img >>boring.log
cp tmp/kernel64.sys /Volumes/BECAUSE/kernel64.sys
echo detaching >>boring.log
hdiutil detach /Volumes/BECAUSE >>boring.log

echo creating image for virtualbox
rm -rf disk.vdi
echo converting image using vboxmanage >>boring.log
VBoxManage convertfromraw disk.img disk.vdi --uuid '{4c2c12cf-ee32-4a37-afe9-8a4bd88abbca}' >>boring.log

echo done. starting vm

VBoxManage controlvm because poweroff >>boring.log

sleep 1
echo starting vm >>boring.log
VBoxManage startvm because >>boring.log

sleep 1
socat unix-connect:/tmp/foo stdout


