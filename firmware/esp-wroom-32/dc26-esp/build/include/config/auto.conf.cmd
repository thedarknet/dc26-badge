deps_config := \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/app_trace/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/aws_iot/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/bt/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/esp32/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/ethernet/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/fatfs/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/freertos/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/heap/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/libsodium/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/log/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/lwip/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/mbedtls/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/openssl/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/pthread/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/spi_flash/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/spiffs/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/tcpip_adapter/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/wear_levelling/Kconfig \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/bootloader/Kconfig.projbuild \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/components/partition_table/Kconfig.projbuild \
	/d/MyStuff/dev/defcon/defcon26/dc26-badge/firmware/esp-wroom-32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
