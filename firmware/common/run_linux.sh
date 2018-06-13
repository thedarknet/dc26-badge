echo "making messages"
../tools/bin/linux/flatc --cpp -o ../DevBoard/darknet-7/Src/darknet/messaging common.fbs
../tools/bin/linux/flatc --cpp -o ../DevBoard/darknet-7/Src/darknet/messaging stm_to_esp.fbs
../tools/bin/linux/flatc --cpp -o ../DevBoard/darknet-7/Src/darknet/messaging esp_to_stm.fbs
