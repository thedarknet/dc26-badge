#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := dc26esp
CPPFLAGES += "-I $HOME/opt/xtensa-esp32-elf/xtensa-esp32-elf/include/c++/5.2.0"

include $(IDF_PATH)/make/project.mk

fileImage:
	rm fat.fs
	dd if=/dev/zero of=fat.fs bs=1024 count=1024
	sudo mkfs.vfat fat.fs
	sudo mount -o loop  fat.fs /home/cmdc0de/dev/defcon/dc26-badge/firmware/esp-wroom-32/dc26-esp/mnt
	sudo chmod -R 777 ./mnt
	cp -R ./files/* ./mnt

main/stm_to_esp_generated.h: ../../common/stm_to_esp.fbs
	../../tools/bin/linux/flatc --cpp -o main -M ../../common/stm_to_esp.fbs
