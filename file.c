#include "file.h"
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return (size_t)fwrite(ptr, size, nmemb, stream);
}
int write_url(char * key,char *url,char *appid){
    FILE *f = fopen("/tmp/pam_wxwork", "w");
    if (f==NULL){
        return -1;
    }
    fputs(key,f);
    fputc('\n',f);
    fputs(url,f);
    fputc('\n',f);

    fputs(appid,f);
    fputc('\n',f);

    fclose(f);
    return 0;
}
int read_url(char * key,char *url,char *appid){
    FILE *f = fopen("/tmp/pam_wxwork", "r");
    if (f==NULL){
        return -1;
    }
    fgets(key,ORD_LEN,f);
    key[strlen(key)-1]='\0';

    fgets(url,URL_LEN,f);
    url[strlen(url)-1]='\0';

    fgets(appid,ORD_LEN,f);
    appid[strlen(appid)-1]='\0';


    fclose(f);
    return 0;
}