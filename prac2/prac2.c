WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
/**
 * \file
 *         A TCP socket echo server. Listens and replies on port 8080
 * \author
 *         mds
 */

#include "contiki.h"
#include "contiki-net.h"
#include "sys/cc.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include "dev/cc26xx-uart.h"
#include "ieee-addr.h"
#include "buzzer.h"
#include "board-peripherals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 8080

static struct tcp_socket socket;

#define INPUTBUFSIZE 400
static uint8_t inputbuf[INPUTBUFSIZE];

#define OUTPUTBUFSIZE 400
static uint8_t outputbuf[OUTPUTBUFSIZE];

PROCESS(tcp_server_process, "TCP echo process");
AUTOSTART_PROCESSES(&tcp_server_process);
static uint8_t get_received;
static int bytes_to_send;


/* LED and Buzzer Definitions */
#define TOGGLE_OFF 0
#define TOGGLE_ON 1 // In LED case, turns on all LED.
#define TOGGLE_GREEN 2
#define TOGGLE_RED 3

/* Global Variables */
// LED Variables
static struct ctimer led_ctimer;
int led_state = TOGGLE_OFF;
int led_sampling_freq = 2;

// Buzzer Variables
static struct ctimer buz_ctimer;
int buz_state = TOGGLE_OFF;
int buz_freq = 800;
int buz_count = 0;

// Humidity Sensor Variables
static struct ctimer hdc_ctimer;
int hdc_state = TOGGLE_OFF;
int hdc_count = 0;
int hdc_sampling_freq = 1;

// Pressure Sensor Variables
static struct ctimer bmp_ctimer;
int bmp_state = TOGGLE_OFF;
int bmp_count = 0;
int bmp_sampling_freq = 1;

/* Helper Functions */
void toggle_led() {
	ctimer_reset(&led_ctimer);

	if (led_state == TOGGLE_OFF) {
		leds_off(LEDS_ALL);

	} else if (led_state == TOGGLE_RED) {
		leds_off(LEDS_GREEN);
		leds_toggle(LEDS_RED);

	} else if (led_state == TOGGLE_GREEN) {
		leds_off(LEDS_RED);
		leds_toggle(LEDS_GREEN);

	} else if (led_state == TOGGLE_ON) {
		leds_toggle(LEDS_ALL);

	}
}

void inc_buz_freq() {
	if (buz_count < 5 && buz_state == TOGGLE_ON) {
		buz_freq += 10;
		buzzer_start(buz_freq);
		buz_count++;
		ctimer_reset(&buz_ctimer);
	} 
}

void dec_buz_freq() {
	if (buz_count < 5 && buz_state == TOGGLE_ON) {
		buz_freq -= 10;
		buzzer_start(buz_freq);
		buz_count++;
		ctimer_reset(&buz_ctimer);

	} 
}

void humidity_sample() {
	if (hdc_count < 10 && hdc_state == TOGGLE_ON) {

		int humidity_temp_val;
		int humidity_val;
		humidity_temp_val = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_TEMP);		//Read Humidity Temp value
		humidity_val = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_HUMIDITY);		//Read Humidity value

		printf("Humidity=%d.%02d %%RH, Temp:%d.%02d C\n\r", humidity_val/100, humidity_val%100, humidity_temp_val/100, humidity_temp_val%100);

		int n;
		char humidity_message[100];
		n = sprintf(humidity_message, "Humidity=%d.%02d %%RH, Temp:%d.%02d C\n\r", humidity_val/100, humidity_val%100, humidity_temp_val/100, humidity_temp_val%100);
		tcp_socket_send_str(&socket, humidity_message);

		hdc_count++;
		SENSORS_ACTIVATE(hdc_1000_sensor);
		ctimer_reset(&hdc_ctimer);

	} else {
		hdc_state = TOGGLE_OFF;
		ctimer_stop(&hdc_ctimer);

	}
}

void pressure_sample() {
	if (bmp_count < 10 && bmp_state == TOGGLE_ON) {

		int pressure_temp_val;
		int pressure_val;

		pressure_temp_val = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_TEMP);		//Read Pressure Temp value
		pressure_val = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_PRESS);			//Read Pressure value

		printf("Pressure=%d.%02d %%RH, Temp:%d.%02d C\n\r", pressure_val/100, pressure_val%100, pressure_temp_val/100, pressure_temp_val%100);

		int n;
		char pressure_message[100];
		n = sprintf(pressure_message, "Pressure=%d.%02d %%RH, Temp:%d.%02d C\n\r", pressure_val/100, pressure_val%100, pressure_temp_val/100, pressure_temp_val%100);
		tcp_socket_send_str(&socket, pressure_message);

		bmp_count++;
		SENSORS_ACTIVATE(bmp_280_sensor);
		ctimer_reset(&bmp_ctimer);

	} else {
		bmp_state = TOGGLE_OFF;
		ctimer_stop(&bmp_ctimer);

	}
}

/*---------------------------------------------------------------------------*/
//Input data handler
static int input(struct tcp_socket *s, void *ptr, const uint8_t *inputptr, int inputdatalen) {

  	printf("input %d bytes '%s'\n\r", inputdatalen, inputptr);

  	char data = inputptr[0];

    ctimer_set(&led_ctimer, CLOCK_SECOND / led_sampling_freq , toggle_led, NULL);
  	if (data == 'r') {
		if (led_state == TOGGLE_RED) 	led_state = TOGGLE_OFF;
		else 						 	led_state = TOGGLE_RED;

	} else if (data == 'g') {
		if (led_state == TOGGLE_GREEN)  led_state = TOGGLE_OFF;
		else 							led_state = TOGGLE_GREEN;

	} else if (data == 'a') {
		if (led_state == TOGGLE_ON) 	led_state = TOGGLE_OFF;
		else 						 	led_state = TOGGLE_ON;

	} else if (data == 'b') {
		if (buz_state == TOGGLE_OFF) {
			buz_state = TOGGLE_ON;
			buzzer_start(buz_freq);

		} else {
			buz_state = TOGGLE_OFF;
			buzzer_stop();

		}

	} else if (data == 'i') {
		buz_count = 0;
		ctimer_set(&buz_ctimer, CLOCK_SECOND, inc_buz_freq, NULL);

	} else if (data == 'd') {
		buz_count = 0;
		ctimer_set(&buz_ctimer, CLOCK_SECOND, dec_buz_freq, NULL);

	} else if (data == 'n') {
		uint8_t addr[8];
		ieee_addr_cpy_to(addr, 8);
		printf("IEEE address: ");
		for (int i = 0; i < 8; i++) {
			printf("%x.", addr[i]);
		}
		printf("\r\n");

	} else if (data == 'h') {
		if (hdc_state == TOGGLE_ON) {
			hdc_state = TOGGLE_OFF;
			ctimer_stop(&hdc_ctimer);

		} else  {
			hdc_state = TOGGLE_ON; 
			hdc_count = 0;
			SENSORS_ACTIVATE(hdc_1000_sensor);
			ctimer_set(&hdc_ctimer, CLOCK_SECOND / hdc_sampling_freq, humidity_sample, NULL);

		}

	} else if (data == 'p') {
		if (bmp_state == TOGGLE_ON) {
			bmp_state = TOGGLE_OFF;
			ctimer_stop(&bmp_ctimer);

		} else  {
			bmp_state = TOGGLE_ON; 
			bmp_count = 0;
			SENSORS_ACTIVATE(bmp_280_sensor);
			ctimer_set(&bmp_ctimer, CLOCK_SECOND / bmp_sampling_freq, pressure_sample, NULL);

		}
	}

	tcp_socket_send_str(&socket, inputptr);	//Reflect byte

	//Clear buffer
	memset(inputptr, 0, inputdatalen);
    return 0; // all data consumed 
}

/*---------------------------------------------------------------------------*/
//Event handler
static void event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev) {
	
	printf("event %d\n", ev);
}

/*---------------------------------------------------------------------------*/
//TCP Server process
PROCESS_THREAD(tcp_server_process, ev, data) {

  	PROCESS_BEGIN();

	//Register TCP socket
  	tcp_socket_register(&socket, NULL,
               inputbuf, sizeof(inputbuf),
               outputbuf, sizeof(outputbuf),
               input, event);
  	tcp_socket_listen(&socket, SERVER_PORT);

	printf("Listening on %d\n", SERVER_PORT);
	
	while(1) {
	
		//Wait for event to occur
		PROCESS_PAUSE();
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
