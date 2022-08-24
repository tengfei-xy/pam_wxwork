
#include "pam.h"
int get_pam_item(pam_handle_t *pamh, char *target, char *line)
{
    struct utsname u;
    const void *str = NULL;
    int k = 0;
    for (char *i = target; *i != '\0'; i++)
    {

        if (*i != 37)
        {
            line[k++] = *i;
            continue;
        }
        i++;
        if (*i < 96 || *i > 111)
            continue;

        switch (*i)
        {
        case 'a':
            pam_get_item(pamh, PAM_SERVICE, &str);
            break;

        case 'b':
            pam_get_item(pamh, PAM_USER, &str);
            break;

        case 'c':
            pam_get_item(pamh, PAM_TTY, &str);
            break;
        case 'd':
            pam_get_item(pamh, PAM_SERVICE, &str);
            if (strcmp(str, "login") == 0)
            {
                pam_syslog(pamh, LOG_INFO, "When the service is LOGIN, the variable cannot be \%d");
                str = "";
                break;
            }
            pam_get_item(pamh, PAM_RHOST, &str);

            break;
        case 'e':
            if (uname(&u) < 0)
                str = "";
            else
                str = u.nodename;
            break;
        default:
            pam_syslog(pamh, LOG_INFO, "Unsupported variables:\%%c", *i);
            break;
        }
        strncat(line, str, strlen(str));
        k = strlen(line);
    }

    return PAM_SUCCESS;
}
int get_pam_target(pam_handle_t *pamh,const char **argv,const char **file){
    const char * g;
    if (!strncmp(*argv, "file=", 5))
        *file = (5 + *argv);
    argv++;
    if (!strncmp(*argv, "target=", 7))
        g = (7 + *argv);

    if (strstr(g,TARGET_WATCH_QR)!=NULL){
        return 0;
    }else if (strstr(g,TARGET_SCAN_QR)!=NULL){
        return 1;
    }else if (strstr(g,TARGET_ADRAGON)!=NULL){
        return 2;
    }
    
}