# Software for the Darknet-7 badge

## DevBoard/Darknet-7: STM32 firmware
* git clone this repo
* download and install System Workbench for STM32
  * http://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32
* Open system work bench:
  * File -> Import -> Existing Projects info Workspace
  * Navigate to the directory you cloned this repo into then to DevBoard->Darknet-7
  * A project should show up in the projects text box.
  * push finish
    * DO NOT select copy projects into workspace!
* Project -> build all will the project
* To debug
  * Debug -> Debug Configurations -> highlight Ac6 STM32 Debugging
  * Click new launch configuration
  * Ensure main looks like this:
  ![alt text](https://github.com/thedarknet/dc26-badge/blob/master/firmware/main1.PNG "Main")

  * Ensure debugger looks like this.
  ![alt text](https://github.com/thedarknet/dc26-badge/blob/master/firmware/main2.PNG "Debug")

  * Then launch debug build.

## dc26-esp32
* if you haven't already clone this repo
* Follow these instructions to get https://esp-idf.readthedocs.io/en/latest/get-started/index.html
* cd in to the dc26-esp32 directory 
```
make flashfatfs flash monitor
```
* when you see the ...... when make flash is running you'll need to press the esp32 reset button, before letting go press the esp32 prog button then release the reset button, you should see the upload start
* you will need to press the reset button after upload.
* there is no ide set up for this env
