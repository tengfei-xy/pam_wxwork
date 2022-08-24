#include <security/pam_ext.h>
#include <security/pam_modules.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/utsname.h>
#include <string.h>
#include "strings.h"


#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
#define CONF_LINE_MAX 100

#define TARGET_WATCH_QR "watch_qr"
#define TARGET_SCAN_QR "scan_qr"
#define TARGET_ADRAGON "adragon"
int get_pam_item(pam_handle_t *pamh, char *target, char *line);
int get_pam_target(pam_handle_t *pamh,const char **argv,const char **file);