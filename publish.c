#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_config.h"

#include "publish.h"

#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

static const int MAX_RETRIES = 25;

static void disconnectCallbackHandler(void) {
	fprintf(stderr,"MQTT Disconnect\n");
	IoT_Error_t rc = NONE_ERROR;
	if(aws_iot_is_autoreconnect_enabled()){
		fprintf(stdout,"Auto Reconnect is enabled, Reconnecting attempt will start now\n");
	}else{
		fprintf(stderr,"Auto Reconnect not enabled. Starting manual reconnect...\n");
		rc = aws_iot_mqtt_attempt_reconnect();
		if(RECONNECT_SUCCESSFUL == rc){
			fprintf(stderr,"Manual Reconnect Successful\n");
		}else{
			fprintf(stderr,"Manual Reconnect Failed - %d\n", rc);
		}
	}
}

void publishConfigDefault(publishConfig_t *config){

    memset(config, 0, sizeof(*config));
    config->port = AWS_IOT_MQTT_PORT;

}

bool publishInit(const publishConfig_t *config){

    IoT_Error_t rc = NONE_ERROR;


    fprintf(stdout,"\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    MQTTConnectParams connectParams = MQTTConnectParamsDefault;

    connectParams.KeepAliveInterval_sec = 10;
    connectParams.isCleansession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = "raspberrypi";
    connectParams.pHostURL = (char *)config->hostAddress;
    connectParams.port = config->port;
    connectParams.isWillMsgPresent = false;
    connectParams.pRootCALocation = (char *)config->caFilename;
    connectParams.pDeviceCertLocation = (char *)config->clientCertFilename;
    connectParams.pDevicePrivateKeyLocation = (char *)config->clientKeyFilename;
    connectParams.mqttCommandTimeout_ms = 5000;
    connectParams.tlsHandshakeTimeout_ms = 5000;
    connectParams.isSSLHostnameVerify = true; // ensure this is set to true for production
    connectParams.disconnectHandler = disconnectCallbackHandler;

    fprintf(stdout,"Connecting...");
    rc = aws_iot_mqtt_connect(&connectParams);
    if (NONE_ERROR != rc) {
        fprintf(stderr, "Error(%d) connecting to %s:%d", rc, connectParams.pHostURL, connectParams.port);
        return false;
    }

    rc = aws_iot_mqtt_autoreconnect_set_status(true);
    if (NONE_ERROR != rc) {
        fprintf(stderr, "Unable to set Auto Reconnect to true - %d", rc);
        return false;
    }

    return true;
}

void publishProcess(int timeout){
    aws_iot_mqtt_yield(timeout);
}

static const char *getTimestampString(char *buf, size_t buflen){
    time_t now;
    time(&now);
    strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return buf;
}

bool publishReflections(unsigned int count){

    char timestrbuf[32];
    IoT_Error_t rc = NONE_ERROR;
    MQTTMessageParams msg = MQTTMessageParamsDefault;
    msg.qos = QOS_1;
    char payload[64];
    snprintf(payload, sizeof(payload), "{ \"reflections\": %u}", count);
    msg.pPayload = (void *) payload;

    MQTTPublishParams params = MQTTPublishParamsDefault;
    params.pTopic = "iot/reflections";
    msg.PayloadLen = strlen(payload) + 1;
    params.MessageParams = msg;
    int retryCounter = 0;
    getTimestampString(timestrbuf, sizeof(timestrbuf));
    printf("publishing at %s (%s)\n", timestrbuf, payload);

    do {
        rc = aws_iot_mqtt_publish(&params);
        if (rc != 0){
            fprintf(stderr, "have to retry...\n");
            rc = aws_iot_mqtt_attempt_reconnect();
            if(RECONNECT_SUCCESSFUL == rc){
                fprintf(stderr,"Manual Reconnect Successful\n");
            }else{
                fprintf(stderr,"Manual Reconnect Failed - %d\n", rc);
            }
            ++retryCounter;
            sleep(5);
        }
        aws_iot_mqtt_yield(100);
    } while (rc != NONE_ERROR && retryCounter < MAX_RETRIES);

    if (rc != NONE_ERROR){
        fprintf(stderr, "%s Could not publish message (rc=%d)\n", timestrbuf, rc);
        return false;
    }
    printf("message published (%s) !\n", payload);

    return true;
}

void publishDestroy(void){


}
