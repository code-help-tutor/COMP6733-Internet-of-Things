WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
//**
// * \file
// *         A TCP socket echo server. Listens and replies on port 8080
// * \author
// *         mds
// */

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
int buz_state = TOGGLE_OFF;
int buz_freq = 0;

// Humidity Sensor Variables
static struct ctimer hdc_ctimer;
int hdc_state = TOGGLE_OFF;
int hdc_count = 0;
int hdc_samples = 10;
int hdc_sampling_freq = 2;
char hdc_message[1000];

// Pressure Sensor Variables
static struct ctimer bmp_ctimer;
int bmp_state = TOGGLE_OFF;
int bmp_count = 0;
int bmp_samples = 10;
int bmp_sampling_freq = 2;
char bmp_message[1000];


//* Error Function */
void http_error_request(char *msg) {

	char html[200];

	/* Header */
	sprintf(html,
			"HTTP/1.1 400 Bad Request\n"
			"\n"
			);
	tcp_socket_send_str(&socket, html);

	/* Content */
	char html2[500];
	sprintf(html2,
			"	<h1>Bad Request</h1>"
			"	<p>%s</p>",
			msg
		);
	tcp_socket_send_str(&socket, html2);
	tcp_socket_close(&socket);

	printf("Invalid Request\n");
}

// Success Function
void http_success_header(void) {

	char html[200];
	sprintf(html,
		"HTTP/1.1 200 OK\n"
		"\n"
		);
	tcp_socket_send_str(&socket, html);
}

void http_success_led(void) {

	char red[10];
	char green[10];

	switch (led_state) {
		case TOGGLE_OFF:
			strcpy(red, "off");
			strcpy(green, "off");
			break;
		case TOGGLE_GREEN:
			strcpy(red, "off");
			strcpy(green, "on");
			break;
		case TOGGLE_RED:
			strcpy(red, "on");
			strcpy(green, "off");
			break;
		default:
			strcpy(red, "on");
			strcpy(green, "on");
			break;
	}

	char html[500];
	sprintf(html,
		"	<h1>LED Status</h1>"
		"	<p>Red:   %s</p>"
		"	<p>Green: %s</p>",
		red,
		green
		);
	tcp_socket_send_str(&socket, html);

}

void http_buzzer(void) {
	char html[500];
	if (buz_state == TOGGLE_OFF) {
		sprintf(html,
			"	<h1>Buzzer</h1>"
			"	<p>Status: off</p>"
			);
	} else {
		sprintf(html,
			"	<h1>Buzzer</h1>"
			"	<p>Status: off</p>"
			"	<p>Freq: %d</p>",
			buz_freq
			);

	}
	tcp_socket_send_str(&socket, html);

}

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

void humidity_sample() {
	if (hdc_count < hdc_samples && hdc_state == TOGGLE_ON) {

		int humidity_temp_val;
		int humidity_val;
		humidity_temp_val = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_TEMP);		//Read Humidity Temp value
		humidity_val = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_HUMIDITY);		//Read Humidity value

		printf("Humidity=%d.%02d %%RH, Temp:%d.%02d C\n\r", humidity_val/100, humidity_val%100, humidity_temp_val/100, humidity_temp_val%100);

		int n;
		char humidity_message[100];
		n = sprintf(humidity_message, "Humidity=%d.%02d %%RH, Temp:%d.%02d C\n\r", humidity_val/100, humidity_val%100, humidity_temp_val/100, humidity_temp_val%100);
		strcat(hdc_message, humidity_message);
		// tcp_socket_send_str(&socket, humidity_message);

		hdc_count++;
		SENSORS_ACTIVATE(hdc_1000_sensor);
		ctimer_reset(&hdc_ctimer);

	} else {
		hdc_state = TOGGLE_OFF;
		ctimer_stop(&hdc_ctimer);

		tcp_socket_send_str(&socket, hdc_message);

		/* Close */
		tcp_socket_close(&socket);

	}
}

void pressure_sample() {
	if (bmp_count < bmp_samples && bmp_state == TOGGLE_ON) {

		int pressure_temp_val;
		int pressure_val;

		pressure_temp_val = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_TEMP);		//Read Pressure Temp value
		pressure_val = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_PRESS);			//Read Pressure value

		printf("Pressure=%d.%02d %%RH, Temp:%d.%02d C\n\r", pressure_val/100, pressure_val%100, pressure_temp_val/100, pressure_temp_val%100);

		int n;
		char pressure_message[100];
		n = sprintf(pressure_message, "Pressure=%d.%02d %%RH, Temp:%d.%02d C\n\r", pressure_val/100, pressure_val%100, pressure_temp_val/100, pressure_temp_val%100);
		strcat(bmp_message, pressure_message);
		// tcp_socket_send_str(&socket, pressure_message);

		bmp_count++;
		SENSORS_ACTIVATE(bmp_280_sensor);
		ctimer_reset(&bmp_ctimer);

	} else {
		bmp_state = TOGGLE_OFF;
		ctimer_stop(&bmp_ctimer);

		tcp_socket_send_str(&socket, bmp_message);

		/* Close */
		tcp_socket_close(&socket);


	}
}




//*---------------------------------------------------------------------------*/
//Input data handler
static int input(struct tcp_socket *s, void *ptr, const uint8_t *inputptr, int inputdatalen) {

	printf("input %d bytes '%s'\n\r", inputdatalen, inputptr);

	// tcp_socket_send_str(&socket, inputptr);	//Reflect byte

	///*
	// * Start of My HTTP Response Code
	// */
	char *http_type;
	http_type = strtok(inputptr, " ");

	/* Handle HTTP GET request*/
	if (strcmp(http_type, "GET") == 0) {

		char *token;
		token = strtok(NULL, " ");

		// Got the URL info: e.g. /leds/r/1
		char url[100];
		memcpy(url, token, strlen(token) + 1);

		token = strtok(url, "/");
		if (token == NULL) { 
			http_error_request("Invalid functionality");
			memset(inputptr, 0, inputdatalen); 
		    return 0; 
		}

		// LEDs Handler
		if (strcmp(token, "leds") == 0) {
			printf("Handling LEDs now....\n");

			token = strtok(NULL, "/");
			if (token == NULL) {
				http_error_request("Missing LED colour");
				memset(inputptr, 0, inputdatalen); 
			    return 0; 
			}

			ctimer_set(&led_ctimer, CLOCK_SECOND / led_sampling_freq , toggle_led, NULL);
  			char led;

			if 		(strcmp(token, "r") == 0) 	{ led = 'r'; }
			else if (strcmp(token, "g") == 0) 	{ led = 'g'; }
			else if (strcmp(token, "a") == 0)	{ led = 'a'; }
			else 	{ 
				http_error_request("Invalid LED colour");
				memset(inputptr, 0, inputdatalen); 
			    return 0; 
			}

			token = strtok(NULL, "/");
			if (token == NULL) {
				http_error_request("Missing LED program: toggle on/off?");
				memset(inputptr, 0, inputdatalen); // Clear buffer
			    return 0; // all data consumed 
			}

			if 	(strcmp(token, "1") == 0) {
				switch (led) {
					case 'r': led_state = TOGGLE_RED	; break;
					case 'g': led_state = TOGGLE_GREEN	; break;
					default : led_state = TOGGLE_ON		; break;
				}

			} else if (strcmp(token, "0") == 0) {
				led_state = TOGGLE_OFF;

			} else { 
				http_error_request("Invalid LED program. 1/0 only."); 
				memset(inputptr, 0, inputdatalen);  
			    return 0; 

			}

			http_success_header();
			http_success_led();
			/* Close */
			tcp_socket_close(&socket);

		// Buzzer Handler
		} else if (strcmp(token, "buzzer") == 0) {
			printf("Handling Buzzer now.....\n");

			token = strtok(NULL, "/");
			if (token == NULL) {
				http_error_request("Missing buzzer frequency");
				memset(inputptr, 0, inputdatalen); // Clear buffer
			    return 0; // all data consumed 
			}

			int freq = atoi(token);
			if (freq == buz_freq) {
				buz_freq = 0;
				buz_state = TOGGLE_OFF;
				buzzer_stop();

				http_success_header();
				http_buzzer();

			} else if (freq == 0) {
				buz_freq = 0;
				buz_state = TOGGLE_OFF;
				buzzer_stop();

				http_success_header();
				http_buzzer();

			} else {
				buz_freq = freq;
				buz_state = TOGGLE_ON;
				buzzer_start(buz_freq);

				http_success_header();
				http_buzzer();

				/* Close */
				tcp_socket_close(&socket);

			}

		} else if (strcmp(token, "pressure") == 0) {
			printf("Handling Pressure now......\n");

			token = strtok(NULL, "/");
			if (token == NULL) {
				http_error_request("Missing Pressure sample count");
				memset(inputptr, 0, inputdatalen); // Clear buffer
			    return 0; // all data consumed 
			}

			bmp_samples = atoi(token);

			http_success_header();
			
			// Only overwrites if not yet started
			if (bmp_state == TOGGLE_OFF) {
				bmp_state = TOGGLE_ON;			
				bmp_count = 0;
				memset(bmp_message, '\0', sizeof(char) * 1000);

				SENSORS_ACTIVATE(bmp_280_sensor);
				ctimer_set(&bmp_ctimer, CLOCK_SECOND / bmp_sampling_freq, pressure_sample, NULL);
			}



		} else if (strcmp(token, "humidity") == 0) {
			printf("Handling Humidity now......\n");

			token = strtok(NULL, "/");
			if (token == NULL) {
				http_error_request("Missing Humidity sample count");
				memset(inputptr, 0, inputdatalen); // Clear buffer
			    return 0; // all data consumed 
			}

			hdc_samples = atoi(token);

			http_success_header();
			
			// Only overwrites if not yet started
			if (hdc_state == TOGGLE_OFF) {
				hdc_state = TOGGLE_ON;			
				hdc_count = 0;
				memset(hdc_message, '\0', sizeof(char) * 1000);

				SENSORS_ACTIVATE(hdc_1000_sensor);
				ctimer_set(&hdc_ctimer, CLOCK_SECOND / hdc_sampling_freq, humidity_sample, NULL);
			}

		}
	}

	memset(inputptr, 0, inputdatalen); // Clear buffer
    return 0; // all data consumed 
}

//*---------------------------------------------------------------------------*/
//Event handler
static void event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev) {
  printf("event %d\n", ev);
}

//*---------------------------------------------------------------------------*/
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
//*---------------------------------------------------------------------------*/
