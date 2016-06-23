#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "measurement.h"
#include "publish.h"

static bool running = true;
static int reflections;
static pthread_mutex_t lock;

static void onReflection(void){

    fprintf(stdout, "Reflection detected\n");
    pthread_mutex_lock(&lock);
    ++reflections;
    pthread_mutex_unlock(&lock);

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

bool parseOptions(int argc, char **argv, measurementConfig_t *measurementConfig, publishConfig_t *publishConfig, bool *daemon, bool *fake){
    int opt;

    while (-1 != (opt = getopt(argc, argv, "fdh:p:a:k:c:t:i"))) {
        switch (opt) {
            case 'd':
                *daemon = true;
                break;
            case 'f':
                *fake = true;
                break;
            case 'h':
                publishConfig->hostAddress = optarg;
                break;
            case 'p':
                publishConfig->port = atoi(optarg);
                break;
            case 'a':
                publishConfig->caFilename = optarg;
                break;
            case 'k':
                publishConfig->clientKeyFilename = optarg;
                break;
            case 'c':
                publishConfig->clientCertFilename = optarg;
                break;
            case 't':
                measurementConfig->reflectionMeterThreshold = atoi(optarg);
                measurementConfig->reflectionMeterPort = atoi(optarg);
                break;
            case '?':
                if (isprint(optopt)) {
                    fprintf(stderr,"Unknown option `-%c'.", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.", optopt);
                }
                return false;
            default:
                fprintf(stderr, "Error in command line argument parsing");
                return false;
        }
    }

    return true;
}

int main(int argc, char **argv){

    setvbuf(stdout, NULL, _IONBF, 0);
    
    publishConfig_t publishConfig;
    publishConfigDefault(&publishConfig);

    measurementConfig_t measurementConfig;
    measurementConfigDefault(&measurementConfig);
    measurementConfig.onReflection = onReflection;
    bool daemonize = false;
    bool fake = false;

    if (parseOptions(argc, argv, &measurementConfig, &publishConfig, &daemonize, &fake) == false){
        fprintf(stderr, "Could not parse options\r\n");
        return EXIT_FAILURE;
    }

    if (daemonize == true){
        if (daemon(1, 1) < 0){
            fprintf(stderr, "Could not daemonize (%s)\r\n", strerror(errno));
        }

    }

    if (publishInit(&publishConfig) == false){
        fprintf(stderr, "Could not init publish\r\n");
        return EXIT_FAILURE;
    }

    if (measurementInit(&measurementConfig) == false){
        fprintf(stderr, "Could not init measure\r\n");
        if (fake == false){
            return EXIT_FAILURE;
        }
    } 

    struct sigaction sa = { .sa_handler = sigHandler };
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (running){
        pthread_mutex_lock(&lock);
        int tosend = reflections;
        reflections = 0;
        pthread_mutex_unlock(&lock);
        
        for (int i = 0; i < tosend; ++i){
            fprintf(stdout, "Publishing (%d/%d)\n", i, tosend);
            if (publishSingleReflection() == false){
                fprintf(stderr, "Could not publish single reflection\r\n");
            }
        }
        if (fake == true){
            onReflection();
        }
        publishProcess();
    }
    
    measurementDestroy();
    publishDestroy();

    return 0;

}
