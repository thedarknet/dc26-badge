echo "making messages"
..\tools\bin\win\flatc --cpp -o ..\DevBoard\darknet-7\Src\darknet\messaging common.fbs
..\tools\bin\win\flatc --cpp -o ..\DevBoard\darknet-7\Src\darknet\messaging stm_to_esp.fbs
..\tools\bin\win\flatc --cpp -o ..\DevBoard\darknet-7\Src\darknet\messaging esp_to_stm.fbs
