#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "measurement.h"

static bool running = true;

static void onReflection(void){

    if (publishSingleReflection() == false){
        fprintf(stderr, "Could not publish single reflection\r\n");
    }

}

static void sigHandler(int signum){
	switch (signum){
		case SIGINT:
		case SIGTERM:
			running = 0;
			fprintf(stderr, "signal %d results in running = %d\r\n", signum, running);
			break;
		case SIGHUP:
			fprintf(stderr, "signal SIGHUP does nothing\r\n");
			break;
		default:
			fprintf(stderr, "signal %d not handled\r\n", signum);
	}
}

int main(int argc, char **argv){

    
    if (publishInit() == false){
        fprintf(stderr, "Could not init publish\r\n");
        return EXIT_FAILURE;
    }

    const measurementConfig_t measurementConfig = {
        .reflectionMeterPort = 0,
        .reflectionMeterThreshold = 200,
        .onReflection = onReflection 
    };

    if (measurementInit(&measurementConfig) == false){
        fprintf(stderr, "Could not init measure\r\n");
        return EXIT_FAILURE;

    } 

    struct sigaction sa = { .sa_handler = sigHandler };
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (running){
        sleep(1);
    }
    
    measurementDestroy();



};
