#include <stdio.h>
#include <string.h>

char *systemGetUniqueDeviceId(void){

    char buf[32];
    char *retval = NULL;
    FILE *fp = popen("grep Serial /proc/cpuinfo | cut -f 2 -d : | tr -d \" \n\"", "r");

    if (fp == NULL){
        return retval;
    }

    if (fgets(buf, sizeof(buf), fp) != NULL){
        retval  = strdup(buf);
    }
    
    pclose(fp);

    return retval;

}
