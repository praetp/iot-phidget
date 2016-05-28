#include "measurement.h"
#include <phidget21.h>
#include <stdio.h>

static CPhidgetInterfaceKitHandle _ifKit;
static measurementConfig_t  _config;
static bool _currentReflectionState;
static const int HYSTERESIS = 100;

static void processReflectionSensor1102(int value){
    if (value < _config.reflectionMeterThreshold){
        /* we have reflection */
        if (_currentReflectionState == false){
            /* state change ! */
            _currentReflectionState = true;
            _config.onReflection();
        } else {
            if (_config.reflectionMeterThreshold + HYSTERESIS < value){
                _currentReflectionState = false;
            }
        }
        

    }
}


static int onSensorChanged(CPhidgetInterfaceKitHandle IFK, void *usrptr, int index, int value)

{
    if (index == _config.reflectionMeterPort){
        processReflectionSensor1102(value);
    }
    
    printf("Sensor: %d > Value: %d\n", index, value);

    return 0;

}

bool validateConfig(const measurementConfig_t *config){

    if (config == NULL){
        return false;
    }

    if (config->reflectionMeterPort < 0 || config->reflectionMeterPort > 7){
        return false;
    }

    if (config->reflectionMeterThreshold < 0 || config->reflectionMeterThreshold > 1000){
        return false;
    }

    if (config->onReflection == NULL){
        return false;
    }

    return true;

}


bool measurementInit(const measurementConfig_t *config){

    int result;
    if (validateConfig(config) == false){
        return false;
    }

    _config = *config;
    
    //create the InterfaceKit object
    CPhidgetInterfaceKit_create(&_ifKit);

    //open the interfacekit for device connections
    CPhidget_open((CPhidgetHandle)_ifKit, -1);

    //get the program to wait for an interface kit device to be attached
    if((result = CPhidget_waitForAttachment((CPhidgetHandle)_ifKit, 1000)))
    {
        const char *err;
        CPhidget_getErrorDescription(result, &err);
        fprintf(stderr, "Problem waiting for attachment: %s\n", err);
        return false;
    }
    CPhidgetInterfaceKit_setSensorChangeTrigger(_ifKit, config->reflectionMeterPort, config->reflectionMeterThreshold);
    CPhidgetInterfaceKit_setRatiometric(_ifKit, true);
    CPhidgetInterfaceKit_set_OnSensorChange_Handler (_ifKit, onSensorChanged, NULL);
    
    return true;

}

void measurementDestroy(void){

}
