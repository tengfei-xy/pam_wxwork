# pam-wxwork

安装依赖

```bash
yum install -y gcc qrencode-devel curl-devel pam-devel
```

编译

```
gcc -fPIC -c pam_wxwork.c -o pam_wxwork.o
gcc -shared -o pam_wxwork.so pam_wxwork.o -lpam
cp pam_wxwork.so /usr/lib64/security
```



## 企业微信二维码验证过程

登录链接，手动构造于[官方参考文档](https://developer.work.weixin.qq.com/document/path/91019)

```bash
# 参数获取说明
# appid: 由企业微信管理员提供的参数之一，企业微信应用ID
# agentid: 企业微信管理员提供的参数
# redirect_uri: 企业微信的应用后台管理界面的企业微信授权登录中设置
#               仅手动设置授权回调域
# state: 用户自定义
https://open.work.weixin.qq.com/wwopen/sso/qrConnect?appid=ww49cfd580625083c9&agentid=1000014&redirect_uri=http://zhiweiadserver.xunyang.site:16823/wxauth&state=666
```



二维码指向的链接保存于登录链接的img标签下，如下方地址所示
```
//open.work.weixin.qq.com/wwopen/sso/qrImg?key=0cf6bfe24b3be4a9
```



在浏览器中打开登录链接后，其实每间隔20秒发送下方地址
```bash
# 参数获取说明
# key: 从二维码指向的链接中获取
# redirect_uri: 重定向的链接，但不包括参数
# appid: 企业微信的应用ID
# _: 时间戳
https://open.work.weixin.qq.com/wwopen/sso/l/qrConnect?callback=jsonpCallback&key=76966ecba2d8f88d&redirect_uri=http%3A%2F%2Fzhiweiadserver.xunyang.site%3A16823%2Fwxauth&appid=ww49cfd580625083c9&_=1660187708226
```



直到手机扫描二维码后，上方地址将获取到json
```
jsonpCallback({"status":"QRCODE_SCAN_SUCC","auth_code":"-v70iodQUS3MqxP-dOejzob4PW3CGOuxW9yiSdsvmA8"})
```



获取到auth_code字段后，即可生成新的重定向链接，如下方地址所示
```bash
# 参数获取说明
# code: 上文中提取
# state: 用户自定义的子弹
# appid: 企业微信的应用ID
http://zhiweiadserver.xunyang.site:16823/wxauth?code=_RCW8EiyhEBK7OyXismdLbRBJp-Stsrmzn6HCHFLOV8&state=666&appid=ww49cfd580625083c9
```

