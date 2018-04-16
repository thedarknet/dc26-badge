#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "rom/cache.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"

//DRAM_ATTR char sendbuf[129]="";
//DRAM_ATTR char recvbuf[129]="";
//Main application



/**
 * TEST CODE BRIEF
 *
 * This example will show you how to use I2C module by running two tasks on i2c bus:
 *
 * - read external i2c sensor, here we use a BH1750 light sensor(GY-30 module) for instance.
 * - Use one I2C port(master mode) to read or write the other I2C port(slave mode) on one ESP32 chip.
 *
 * Pin assignment:
 *
 * - slave :
 *    GPIO25 is assigned as the data signal of i2c slave port
 *    GPIO26 is assigned as the clock signal of i2c slave port
 *
 */

#define DATA_LENGTH                        512              /*!<Data buffer length for test buffer*/
#define RW_TEST_LENGTH                     129              /*!<Data length for r/w test, any value from 0-DATA_LENGTH*/
#define DELAY_TIME_BETWEEN_ITEMS_MS        1234             /*!< delay time between different test items */

#define I2C_SLAVE_SCL_IO           26               /*!<gpio number for i2c slave clock  */
#define I2C_SLAVE_SDA_IO           25               /*!<gpio number for i2c slave data */
#define I2C_SLAVE_NUM              I2C_NUM_0        /*!<I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN       (2*DATA_LENGTH)  /*!<I2C slave rx buffer size */

#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */

#define ESP_SLAVE_ADDR                     1             /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#define I2C_EXAMPLE_MASTER_SCL_IO          19               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          18               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */
SemaphoreHandle_t print_mux = NULL;

static void i2c_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}
/**
 * @brief i2c slave initialization
 */
static void i2c_slave_init()
{
    int i2c_slave_port = I2C_SLAVE_NUM;
    i2c_config_t conf_slave;
    conf_slave.sda_io_num = I2C_SLAVE_SDA_IO;
    conf_slave.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.scl_io_num = I2C_SLAVE_SCL_IO;
    conf_slave.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.mode = I2C_MODE_SLAVE;
    conf_slave.slave.addr_10bit_en = 0;
    conf_slave.slave.slave_addr = ESP_SLAVE_ADDR;
    if(ESP_OK!=i2c_param_config(i2c_slave_port, &conf_slave)) {
		printf("param failed\n");
	 } else {
		printf("param successfull\n");
	 }
    if(ESP_OK!=i2c_driver_install(i2c_slave_port, conf_slave.mode,
                       I2C_SLAVE_RX_BUF_LEN,
                       I2C_SLAVE_TX_BUF_LEN, 0)) {
		printf("driver failed\n");
	 } else {
		printf("driver installed\n");
	 }
}

/**
 * @brief test function to show buffer
 */
static void disp_buf(uint8_t* buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if (( i + 1 ) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static void i2c_test_task(void* arg)
{
    uint32_t task_idx = (uint32_t) arg;
    uint8_t* data = (uint8_t*) malloc(DATA_LENGTH);
    int cnt = 0;
    while (1) {
        //printf("test cnt: %d\n", cnt++);
		  cnt++;
		  memset(data,0,DATA_LENGTH);
        xSemaphoreTake(print_mux, portMAX_DELAY);
		  size_t receiveSize = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, RW_TEST_LENGTH, 1000/portTICK_RATE_MS);
		  if (ESP_FAIL == receiveSize) {
				printf("failed to read\n");
		  } else if (receiveSize >0) {
				//echo back
				printf("received %u bytes\n",receiveSize);
				disp_buf(data, receiveSize);
        		size_t tx_size = i2c_slave_write_buffer(I2C_SLAVE_NUM, data, RW_TEST_LENGTH, 1000 / portTICK_RATE_MS);
		  		if (ESP_FAIL == tx_size) {
					printf("failed to write\n");
		  		} else  {
					printf("send %u bytes\n",tx_size);
				}
		  } else if (receiveSize ==0) {
				printf("received %u bytes\n",receiveSize);
		  }
        xSemaphoreGive(print_mux);
        vTaskDelay(( DELAY_TIME_BETWEEN_ITEMS_MS * ( task_idx + 1 ) ) / portTICK_RATE_MS);
    }
}

void app_main()
{
	 /////////////////////////////////////////////////////////
	 printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, \n", chip_info.revision);
    fflush(stdout);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    fflush(stdout);

    print_mux = xSemaphoreCreateMutex();
    //i2c_master_init();
    i2c_slave_init();
	 printf("slave inited\n");
    fflush(stdout);
    xTaskCreate(i2c_test_task, "i2c_test_task_0", 1024 * 2, (void* ) 0, 10, NULL);
}
