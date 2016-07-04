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
static unsigned int reflections;
static pthread_mutex_t lock;
static const unsigned int PUBLISH_INTERVAL_YIELD_S = 1;
static const unsigned int DEFAULT_INTERVAL_S = 15 * 60;  

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

static bool parseOptions(int argc, 
                  char **argv, 
                  measurementConfig_t *measurementConfig, 
                  publishConfig_t *publishConfig, 
                  unsigned int *interval,
                  bool *daemon, 
                  bool *fake){
    int opt;

    while (-1 != (opt = getopt(argc, argv, "fdh:p:a:k:c:t:i"))) {
        switch (opt) {
            case 'i':{
                int tmp = atoi(optarg);
                if (tmp > 10){
                    *interval = tmp;
                } else {
                    fprintf(stderr, "Invalid interval\n");
                }
                break;
            }
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

    unsigned int lastCount = 0;
    bool daemonize = false;
    bool fake = false;
    unsigned int interval = DEFAULT_INTERVAL_S;
    struct sigaction sa = { .sa_handler = sigHandler };
    time_t lastPublish = (time_t)0;

    setvbuf(stdout, NULL, _IONBF, 0);
    
    publishConfig_t publishConfig;
    publishConfigDefault(&publishConfig);

    measurementConfig_t measurementConfig;
    measurementConfigDefault(&measurementConfig);
    measurementConfig.onReflection = onReflection;

    if (parseOptions(argc, argv, &measurementConfig, &publishConfig, &interval, &daemonize, &fake) == false){
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

    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (running){
        time_t now;
        pthread_mutex_lock(&lock);
        unsigned int count = reflections;
        pthread_mutex_unlock(&lock);
        
        if (difftime(time(&now), lastPublish) > (double)interval){
            lastPublish = now;
            if (publishReflections(count - lastCount) == false){
                fprintf(stderr, "Could not publish single reflection\r\n");
            }
            lastCount = count;
        }
        if (fake == true){
            onReflection();
        }
        publishProcess(PUBLISH_INTERVAL_YIELD_S);
    }
    
    measurementDestroy();
    publishDestroy();

    return 0;

}
