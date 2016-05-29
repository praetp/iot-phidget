#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_
#include <stdbool.h>

typedef void (*reflectionDetected_t)(void);

typedef struct {
    unsigned int reflectionMeterPort; /* [0-7] */
    unsigned int reflectionMeterThreshold; /* [0-1000] below this value we consider reflection */
    reflectionDetected_t onReflection; /* edge triggered, runs in phidget thread */
    
} measurementConfig_t;

bool measurementInit(const measurementConfig_t *config);

void measurementConfigDefault(measurementConfig_t *config);

void measurementDestroy(void);
#endif
