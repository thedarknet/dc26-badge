echo "make file setting"
..\tools\flatc\flatc --cpp -o src -M stm_to_esp.fbs
echo "running for realz"
..\tools\flatc\flatc --cpp -o src stm_to_esp.fbs