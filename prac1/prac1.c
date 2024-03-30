WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         mds
 */

#include "contiki.h"
#include <stdio.h> /* For printf() */
#include <string.h>
#include "dev/leds.h"
#include "dev/serial-line.h"
#include "dev/cc26xx-uart.h"
#include "ieee-addr.h"
#include "buzzer.h"

#define TOGGLE_OFF 0
#define TOGGLE_ON 1 // Used to toggle ALL led, toggle buzzer on etc.
#define TOGGLE_GREEN 2
#define TOGGLE_RED 3

/* Process Declaration*/
PROCESS(procKeys, "Key-Press Handler Process");
PROCESS(procLEDs, "LED Toggle Handler Process");
AUTOSTART_PROCESSES(&procKeys, &procLEDs);

/* Global Variables */
// LED Variables
static struct ctimer led_ctimer;
int led_state = TOGGLE_OFF;

// Buzzer Variables
static struct ctimer buz_ctimer;
int buz_state = TOGGLE_OFF;
int buz_freq = 1000;
int buz_count = 0;

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
		buz_freq += 50;
		buzzer_start(buz_freq);
		buz_count++;
		ctimer_reset(&buz_ctimer);
	} 
}

void dec_buz_freq() {
	if (buz_count < 5 && buz_state == TOGGLE_ON) {
		buz_freq -= 50;
		buzzer_start(buz_freq);
		buz_count++;
		ctimer_reset(&buz_ctimer);

	} 
}

/* Processes Code*/
// Process to handle key-presses
PROCESS_THREAD(procKeys, ev, data) {
	PROCESS_BEGIN();

	cc26xx_uart_set_input(serial_line_input_byte);	//Initalise UART in serial driver
	
   	while (1) {
     	PROCESS_YIELD();	//Let other threads run

		if (ev == serial_line_event_message) {
       		printf("received line: %s\n\r", (char *)data);

       		if (strcmp(data, "r") == 0) {
       			if (led_state == TOGGLE_RED) 	led_state = TOGGLE_OFF;
       			else 						 	led_state = TOGGLE_RED;

       		} else if (strcmp(data, "g") == 0) {
       			if (led_state == TOGGLE_GREEN)  led_state = TOGGLE_OFF;
       			else 							led_state = TOGGLE_GREEN;

       		} else if (strcmp(data, "a") == 0) {
       			if (led_state == TOGGLE_ON) 	led_state = TOGGLE_OFF;
       			else 						 	led_state = TOGGLE_ON;

       		} else if (strcmp(data, "b") == 0) {
       			if (buz_state == TOGGLE_OFF) {
       				buz_state = TOGGLE_ON;
       				buzzer_start(buz_freq);

       			} else {
       				buz_state = TOGGLE_OFF;
       				buzzer_stop();

       			}

       		} else if (strcmp(data, "i") == 0) {
       			buz_count = 0;
    		  	ctimer_set(&buz_ctimer, CLOCK_SECOND, inc_buz_freq, NULL);

       		} else if (strcmp(data, "d") == 0) {
       			buz_count = 0;
    			 ctimer_set(&buz_ctimer, CLOCK_SECOND, dec_buz_freq, NULL);

       		} else if (strcmp(data, "n") == 0) {
       			uint8_t addr[8];
       			ieee_addr_cpy_to(addr, 8);
       			printf("IEEE address: ");
       			for (int i = 0; i < 8; i++) {
       				printf("%x.", addr[i]);
       			}
       			printf("\r\n");

       		}
     	}
   	}

   	PROCESS_END();
}

// Process to handle LED toggle
PROCESS_THREAD(procLEDs, ev, data) {
  PROCESS_BEGIN();

  while(1) {
    ctimer_set(&led_ctimer, CLOCK_SECOND, toggle_led, NULL);
    PROCESS_YIELD();
  }

  PROCESS_END();
}