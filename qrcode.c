#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <syslog.h>
#include <unistd.h>
#include <security/pam_ext.h>
#include "qrcode.h"

// from https://github.com/google/google-authenticator-libpam/blob/master/src/google-authenticator.c
// thank qrencode, thank Google!
extern int displayQRCode(pam_handle_t *pamh,const char *url)
{

    void *qrencode = dlopen("libqrencode.so.2", RTLD_NOW | RTLD_LOCAL);
    if (!qrencode)
    {
        qrencode = dlopen("libqrencode.so.3", RTLD_NOW | RTLD_LOCAL);
    }
    if (!qrencode)
    {
        qrencode = dlopen("libqrencode.so.4", RTLD_NOW | RTLD_LOCAL);
    }
    if (!qrencode)
    {
        qrencode = dlopen("libqrencode.3.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    if (!qrencode)
    {
        qrencode = dlopen("libqrencode.4.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    if (!qrencode)
    {
        pam_syslog(pamh, LOG_ERR, "打开libqrencode库失败");
        return -1;
    }
        pam_syslog(pamh, LOG_INFO, "到这里");

    // QRcode Struct Reference
    // https://fukuchi.org/works/qrencode/manual/structQRcode.html#ae6f826314cfb99f4926c4c5734997c35
    typedef struct
    {
        int version;
        int width;
        unsigned char *data;
    } QRcode;

    QRcode *(*QRcode_encodeString8bit)(const char *, int, int) =(QRcode * (*)(const char *, int, int)) dlsym(qrencode, "QRcode_encodeString8bit");
    void (*QRcode_free)(QRcode * qrcode) = (void (*)(QRcode *))dlsym(qrencode, "QRcode_free");
    if (!QRcode_encodeString8bit || !QRcode_free)
    {
        dlclose(qrencode);
        pam_syslog(pamh, LOG_ERR, "QRcode_encodeString8bit或QRcode_free调用失败");
        return -1;
    }

  int ttyfd = open("/dev/console",(O_RDWR), 0644);  
     dup2(ttyfd,STDOUT_FILENO);  

    // 二维码版本
    int qrcode_version=5;
    // 纠错等级
    int qrcdoe_level=0;
    QRcode *qrcode = QRcode_encodeString8bit(url, qrcode_version, qrcdoe_level);

    const char *ptr = (char *)qrcode->data;
    for (int y = 0; y < qrcode->width; ++y) {
      printf(ANSI_BLACKONGREY"    ");
      int isBlack = 0;
      for (int x = 0; x < qrcode->width; ++x) {
        if (*ptr++ & 1) {
          if (!isBlack) {
            printf(ANSI_BLACK);
          }
          isBlack = 1;
        } else {
          if (isBlack) {
            printf(ANSI_WHITE);
          }
          isBlack = 0;
        }
        printf("  ");
      }
      if (isBlack) {
        printf(ANSI_WHITE);
      }
      puts("    "ANSI_RESET);
    }
    for (int i = 0; i < 2; ++i) {
      printf(ANSI_BLACKONGREY);
      for (int x = 0; x < qrcode->width + 4; ++x) printf("  ");
      puts(ANSI_RESET);
    }
    
    QRcode_free(qrcode);
    dlclose(qrencode);
    return 0;
}