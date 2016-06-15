#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_config.h"

#include "publish.h"

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

static void disconnectCallbackHandler(void) {
	WARN("MQTT Disconnect");
	IoT_Error_t rc = NONE_ERROR;
	if(aws_iot_is_autoreconnect_enabled()){
		INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	}else{
		WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect();
		if(RECONNECT_SUCCESSFUL == rc){
			WARN("Manual Reconnect Successful");
		}else{
			WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}

void publishConfigDefault(publishConfig_t *config){

    memset(config, 0, sizeof(*config));
    config->port = AWS_IOT_MQTT_PORT;

}

bool publishInit(const publishConfig_t *config){

    IoT_Error_t rc = NONE_ERROR;


    INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    MQTTConnectParams connectParams = MQTTConnectParamsDefault;

    connectParams.KeepAliveInterval_sec = 10;
    connectParams.isCleansession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    connectParams.pClientID = "CSDK-test-device";
    connectParams.pHostURL = (char *)config->hostAddress;
    connectParams.port = config->port;
    connectParams.isWillMsgPresent = false;
    connectParams.pRootCALocation = (char *)config->caFilename;
    connectParams.pDeviceCertLocation = (char *)config->clientCertFilename;
    connectParams.pDevicePrivateKeyLocation = (char *)config->clientKeyFilename;
    connectParams.mqttCommandTimeout_ms = 15000;
    connectParams.tlsHandshakeTimeout_ms = 5000;
    connectParams.isSSLHostnameVerify = true; // ensure this is set to true for production
    connectParams.disconnectHandler = disconnectCallbackHandler;

    INFO("Connecting...");
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

bool publishSingleReflection(void){

    printf("%d publishing\n", time(NULL));
    IoT_Error_t rc = NONE_ERROR;
    static uint32_t counter;

    MQTTMessageParams msg = MQTTMessageParamsDefault;
    msg.qos = QOS_0;
    char payload[64];
    snprintf(payload, sizeof(payload), "{ counter: %"PRIu32"\n}", counter++);
    msg.pPayload = (void *) payload;

    MQTTPublishParams params = MQTTPublishParamsDefault;
    params.pTopic = "iot/reflections";
    msg.PayloadLen = strlen(payload) + 1;
    params.MessageParams = msg;
    rc = aws_iot_mqtt_publish(&params);
    if (rc != NONE_ERROR){
        fprintf(stderr, "%d Could not publish message (rc=%d)\n", time(NULL), rc);
        return false;
    }
    printf("message published (%s) !\n", payload);
    aws_iot_mqtt_yield(100);

    return true;
}

void publishDestroy(void){


}
