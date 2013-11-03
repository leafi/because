rm -rf tmp
mkdir tmp

echo compiling kernel bits

echo kernel.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/kernel.o -c kernel.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo pci.c
$HOME/opt/cross/bin/x86_64-elf-gcc -m64 -o tmp/pci.o -c pci.c -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding


/usr/local/bin/nasm -f elf64 -o tmp/bits.o bits.asm

echo linking flat 64bit kernel binary
# we use /usr/bin/local/ld b/c i compile on a mac and the default ld is inadequate.
# /usr/bin/local/ld can be installed by doing 'brew install binutils' if you have homebrew.
# if you already have gnu ld, you can just use that instead.
#
$HOME/opt/cross/bin/x86_64-elf-ld -T linker64.ld -o tmp/kernel64.sys tmp/kernel.o tmp/bits.o tmp/pci.o

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

sleep 1
echo starting vm >>boring.log
VBoxManage startvm because >>boring.log

