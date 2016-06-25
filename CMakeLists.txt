cmake_minimum_required (VERSION 2.6)

file(GLOB AWS_IOT_SOURCES 
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/*.c"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/common/*.c"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/openssl/*.c"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/shadow/*.c"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/utils/*.c")

set(AWS_IOT_INCLUDES 
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt" 
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/common"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/openssl"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/shadow"
    "aws-iot-device-sdk-embedded-C/aws_iot_src/utils")

file(GLOB_RECURSE MQTT_SOURCES
    "aws-iot-device-sdk-embedded-C/aws_mqtt_embedded_client_lib/*.c")

set(MQTT_INCLUDES 
    "aws-iot-device-sdk-embedded-C/aws_mqtt_embedded_client_lib/MQTTClient-C/src"
    "aws-iot-device-sdk-embedded-C/aws_mqtt_embedded_client_lib/MQTTPacket/src")

#For some reason there is hard dependency from MQTTClient.h towards the aws_iot_config.h.
#The latter is only included in the samples, so we have an awful dependency from MQTT on the samples.. :-/
set(EXTRA_INCLUDES
    "aws-iot-device-sdk-embedded-C/sample_apps/shadow_sample")
    

file(GLOB IOT_PHIDGET_SOURCES "*.c")




set ( PROJECT_LINK_LIBS libphidget21.so libdl.so libssl.so libcrypto.so )
set(CMAKE_C_FLAGS "-g -Wall -Wextra -std=c11")
project (iot-phidget)
file(MAKE_DIRECTORY build)
include_directories(${AWS_IOT_INCLUDES} ${MQTT_INCLUDES} ${EXTRA_INCLUDES})
add_definitions(-D_GNU_SOURCE -DIOT_ERROR -DIOT_WARN)
add_executable(build/iot-phidget ${IOT_PHIDGET_SOURCES} ${AWS_IOT_SOURCES} ${MQTT_SOURCES})
target_link_libraries(build/iot-phidget ${PROJECT_LINK_LIBS})
