#!/bin/bash
gcc -fPIC -lcurl -lpam -ldl -std=c99 -c qrcode.c -c strings.c -c pam_wxwork.c  -c conf.c -c wxwork.c -c url.c -c file.c -c pam.c|| { echo "生成中间文件失败!" && exit 2;}
gcc -lpam -lcurl -ldl -shared -o pam_wxwork.so  pam_wxwork.o  qrcode.o  strings.o  wxwork.o conf.o url.o file.o pam.o || { echo "生成PAM模块失败" && exit 2;}
sudo rm /usr/lib64/security/pam_wxwork.so
sudo cp  pam_wxwork.so /usr/lib64/security