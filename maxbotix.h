#ifndef MAXBOTIX_H
#define MAXBOTIX_H

#include "Arduino.h"

#define triggerPin D8 // for esp8266
#define MAXBOTIX_MOD MB7389
#define MAXBOTIX_MAX 4999
#define MAXBOTIX_MIN 300

void setup_maxbotix();

// uint16_t read_sensor_using_modes(unsigned int sensorMode, unsigned int sensor_sampling_rate, unsigned int sensor_numberOfReadings);
// uint16_t sensor_singleread(void);

char *maxbotix_readSerialOnce(char *);
char *maxbotix_readSerialContinuous(char *);

#endif
