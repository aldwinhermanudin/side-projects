#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "lora/lora.h"
#include "task.h"

inline void delay(int ms){
	vTaskDelay(ms/portTICK_PERIOD_MS);
}

uint32_t counter = 0;
void loop(void *pvParameters){
	while(1) {
		taskYIELD();	  
    }
}

void onReceive(int packetSize) {
 
	// received a packet
	printf("Received packet '");


	for (int i = 0; i < packetSize; i++) {
		printf("%c",(char)lora_read());
	}
	
	// print RSSI of packet
	printf("' with RSSI ");
	printf("%d\n",lora_packet_rssi());
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    
    
    lora_set_pins(15, DEFAULT_RESET_PIN, DEFAULT_IRQ_PIN);
    if (!lora_begin(915E6)) {
		printf("Starting LoRa failed!");
		while (1);
	}
	
	lora_set_coding_rate_4(8);
	lora_set_spreading_factor(11);
	lora_set_signal_bandwidth(41.7E3);
	lora_set_tx_power_pa_boost(17);
	
	// register the receive callback
	lora_on_receive(onReceive);

	// put the radio into receive mode
	lora_receive(0);
   
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    xTaskCreate(loop, "loop", 1024, NULL, 2, NULL);
}
