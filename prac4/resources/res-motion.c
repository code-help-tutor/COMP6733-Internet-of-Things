WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com

// #include "contiki.h"
// #include "contiki-net.h"
// #include "sys/cc.h"
// #include "dev/serial-line.h"
// #include "dev/cc26xx-uart.h"
// #include "board-peripherals.h"

// #include <limits.h>
// #include <stdlib.h>
// #include <string.h>
// #include "rest-engine.h"

// static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

// RESOURCE(res_motion,
//          "title=\"Motion\";rt=\"Temperature\";obs",
//          res_get_handler,
//          NULL,
//          NULL,
//          NULL);

// static struct ctimer gyro_ctimer;
// int gyro_count = 0;
// int gyro_samples = 5;
// int gyro_sampling_freq = 1;

// static void 
// read_gyro()
// {
// 	gyro_count++;

// 	if (gyro_count >= gyro_samples) {
// 		gyro_count = 0;
// 		ctimer_stop(&gyro_ctimer);
// 		return;
// 	}

// 	int x = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
// 	printf("Gyroscrope value for X is: %d\r\n", x);
// 	SENSORS_ACTIVATE(mpu_9250_sensor);
// 	ctimer_reset(&gyro_ctimer);
// }

// static void
// res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
// {
// 	printf("Starting process for Motion Sensor\r\n");
// 	SENSORS_ACTIVATE(mpu_9250_sensor);
// 	ctimer_set(&gyro_ctimer, CLOCK_SECOND / gyro_sampling_freq, read_gyro, NULL);
// }

