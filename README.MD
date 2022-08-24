# PAM-WXWork v0.3

一种用于Linux验证登录的pam模块，通过企业微信的二维码验证登录。



## 运行方法

clone

```bash
git clone git@github.com:tengfei-xy/pam_wxwork.git
```

临时关闭selinux（或永久）

```bash
setenforce 0
```

安装依赖

```bash
sudo yum install -y gcc qrencode-devel curl-devel pam-devel
```

编译并复制

```bash
 cd pam_wxwork && ./build.sh
```

编辑并复制模块所需的配置文件到指定目录，

注：pam_wxwork.conf的配置方法可以参考下方企业微信二维码验证过程

```bash
cp pam_wxwork.conf pam_wxwork.conf.save
vi pam_wxwork.conf.save
cp pam_wxwork.conf.save /usr/local/pam_wxwork/pam_wxwork.conf
```

编辑/etc/pam.d/login，添加如下内容

> target支持的有效值：watch_qr（查看二维码）、scan_ar（扫描和验证二维码）、adragon（查看、扫描并验证二维码）

```
account    required     pam_wxwork.so file=/usr/local/pam_wxork/pam_wxwork.conf target=adragon
```



## 使用说明

1. 目前仅支持从控制台来（/etc/pam.d/login）和暂不支持从sshd（/etc/pam.d/sshd）通过命令行扫描二维码方式进行验证（除非利用web进行中转web）
2. 一旦将pam_wxwork.so复制到模块目录，将实时生效
3. 模块的日志参考于/var/log/secure
4. 需要关闭SELINUX
5. 验证服务器返回字符串success即可
6. 提供的长链接转短链接的api地址如下，当前为短链接时效为永久有效。

   ```
   short_url_api="https://link.xunyang.site/pam?get="
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
https://open.work.weixin.qq.com/wwopen/sso/qrConnect?appid=abcd&agentid=666&redirect_uri=http://xxx.simple.com:1234/wxauth&state=666
```



二维码指向的链接保存于登录链接的img标签下，如下方地址所示
```
https://open.work.weixin.qq.com/wwopen/sso/confirm2?k=0cf6bfe24b3be4a9
```



在浏览器中打开登录链接后，其实每间隔20秒发送下方地址
```shell
# 参数获取说明
# key: 从二维码指向的链接中获取
# redirect_uri: 重定向的链接，但不包括参数
# appid: 企业微信的应用ID
# _: 时间戳
https://open.work.weixin.qq.com/wwopen/sso/l/qrConnect?callback=jsonpCallback&key=76966ecba2d8f88d&redirect_uri=http%3A%2F%2Fxxx.simple.com%3A16823%2Fwxauth&appid=abcd&_=1660187708226
```



直到手机扫描二维码后，上方地址将获取到json
```
jsonpCallback({"status":"QRCODE_SCAN_SUCC","auth_code":"-v70iodQUS3MqxP-dOejzob4PW3CGOuxW9yiSdsvmA8"})
```



获取到auth_code字段后，即可生成新的重定向链接，如下方地址所示
```shell
# 参数获取说明
# code: 上文中提取
# state: 用户自定义的子弹
# appid: 企业微信的应用ID
http://xxx.simple.com:16823/wxauth?code=_RCW8EiyhEBK7OyXismdLbRBJp-Stsrmzn6HCHFLOV8&state=666&appid=abcd
```



## 更新说明

2022-08-24 v0.3

> 1. 查看二维码和验证二维码步骤分开（想测试从sshd中扫描二维码，但失败）
> 2. 支持更小的二维码

2022-08-17 v0.2 

> 1. 配置文件中的wxwork_auth_value支持变量

2022-08-15 v0.1

> 1. 企业微信二维码的验证的支持从**控制台**登录 

