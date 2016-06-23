#ifndef PUBLISH_H_
#define PUBLISH_H_
#include <stdbool.h>
typedef struct {
        const char *hostAddress;
        unsigned short port;
	const char *caFilename;
	const char *clientCertFilename;
	const char *clientKeyFilename;
} publishConfig_t;


void publishConfigDefault(publishConfig_t *config);

bool publishInit(const publishConfig_t *config);

bool publishReflections(unsigned int count);

void publishProcess(void);

void publishDestroy(void);
#endif
