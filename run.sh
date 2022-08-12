#!/bin/bash
#!/bin/bash
gcc -fPIC -lcurl -lpam -ldl -std=c99 -c qrcode.c -c wxwork.c -c network.c -c strings.c -c pam_wxwork.c || { echo "生成中间文件失败!" && exit 2;}
gcc -lpam -lcurl -ldl -shared -o pam_wxwork.so network.o  pam_wxwork.o  qrcode.o  strings.o  wxwork.o  || { echo "生成PAM模块失败" && exit 2;}
sudo rm /usr/lib64/security/pam_wxwork.so
sudo cp  pam_wxwork.so /usr/lib64/security
sudo cp  pam_wxwork.so /etc/pam.d

# 如果要调试需要添加 -g 参数
#gcc -l curl -std=c99 -ldl main.c qrcode.c wxwork.c network.c strings.c -o main