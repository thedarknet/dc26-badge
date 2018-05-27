#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := dc26esp
CPPFLAGS += "-I $HOME/opt/xtensa-esp32-elf/xtensa-esp32-elf/include/c++/5.2.0"
CXXFLAGS += "-D FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE"

include $(IDF_PATH)/make/project.mk

flatBuffers: main/stm_to_esp_generated.h main/esp_to_stm_generated.h main/common_generated.h

main/stm_to_esp_generated.h: ../../common/stm_to_esp.fbs
	#../../tools/bin/linux/flatc --cpp -o main -M ../../common/stm_to_esp.fbs
	../../tools/bin/linux/flatc --cpp -o main ../../common/stm_to_esp.fbs

main/esp_to_stm_generated.h: ../../common/esp_to_stm.fbs
	../../tools/bin/linux/flatc --cpp -o main ../../common/esp_to_stm.fbs

main/common_generated.h: ../../common/common.fbs
	../../tools/bin/linux/flatc --cpp -o main ../../common/common.fbs
