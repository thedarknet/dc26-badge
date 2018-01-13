deps_config := \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/app_trace/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/aws_iot/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/bt/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/esp32/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/ethernet/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/fatfs/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/freertos/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/heap/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/libsodium/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/log/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/lwip/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/mbedtls/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/openssl/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/pthread/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/spi_flash/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/spiffs/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/tcpip_adapter/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/wear_levelling/Kconfig \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/cmdc0de/dev/darknet/26/dc26-badge/firmware/esp-wroom-32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
