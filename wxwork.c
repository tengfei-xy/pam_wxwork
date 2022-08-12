#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <security/pam_ext.h>
#include <syslog.h>
#include "wxwork.h"
#include "strings.h"
#include "network.h"

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return (size_t)fwrite(ptr, size, nmemb, stream);
}

extern char *get_wxwork_key(pam_handle_t *pamh, char *url)
{

    CURL *curl;
    CURLcode res;
    FILE *curl_tmp_file = fopen("/tmp/pam_wxwork", "rw+");
    struct curl_slist *http_header = NULL;

    curl = curl_easy_init();
    if (!curl)
    {
        pam_syslog(pamh, LOG_ERR,"%s", "curl init failed");
        return NULL;
    }
    char * errbuf = (char * )malloc(10200);
    curl_easy_setopt(curl, CURLOPT_URL, url);          // url地址
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);        //打印调试信息
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_tmp_file);
    // http_header = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //设置为非0,响应头信息location
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); //接收数据时超时设置，如果10秒内数据未

    res = curl_easy_perform(curl);

    switch (res)
    {
    case CURLE_OK:
        break;
    case CURLE_UNSUPPORTED_PROTOCOL:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误:%s", url, "不支持的协议,由URL的头部指定");
        return NULL;

    case CURLE_COULDNT_CONNECT:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误:%s", url, "不能连接到远程主机或者代理");
        return NULL;

    case CURLE_HTTP_RETURNED_ERROR:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误:%s", url, "http返回错误");
        return NULL;

    case CURLE_READ_ERROR:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误:%s", url, "发送读错误");
        return NULL;

    case CURLE_WRITE_ERROR:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误:%s", url, "发送写错误");
        return NULL;

    default:
        pam_syslog(pamh, LOG_ERR, "请求url:%s 错误返回值:%d", url, res);
        pam_syslog(pamh, LOG_ERR, "具体错误:%s",errbuf);
        free (errbuf);
        return NULL;

    }
   
    long buflen = ftell(curl_tmp_file);
    char *body = (char *)malloc(buflen);
    // pam_syslog(pamh, LOG_INFO, "文件长度:%lld",buflen);
    fseek(curl_tmp_file, 0, SEEK_SET);
    fread(body, buflen, 1, curl_tmp_file);
    // pam_syslog(pamh, LOG_INFO, "文件内容:%s",body);

    char *start = body;
    char *label_img_url = "open.work.weixin.qq.com/wwopen/sso/qrImg?key=";
    int s = strindex_x(body, label_img_url, strlen(label_img_url)) + strlen(label_img_url);
    if (s == -1)
        return NULL;
    int e = strindex_x(body, "\"", s);

    char *key = (char *)malloc(e - s);

    for (int i = 0; i < e - s; i++)
        key[i] = body[s + i];
    key[e - s] = '\0';

    // 释放
    free(body);
    curl_easy_cleanup(curl);
    fclose(curl_tmp_file);
    return key;
}

extern char *get_wxwork_qrcode(char *key)
{
    char *qrcode_base_url = "https://open.work.weixin.qq.com/wwopen/sso/confirm2?k=";
    char *qrcode_url = (char *)malloc(strlen(qrcode_base_url) + strlen(key));
    strcpy(qrcode_url, qrcode_base_url);
    strcat(qrcode_url, key);
    return qrcode_url;
}

extern char *wait_auth_wxwork_qrcode(char *key, char *login_url, char *appid)
{

    char *flag = "redirect_uri=";
    int s = strindex_x(login_url, flag, 0) + strlen(flag);
    int e = strindex_x(login_url, "?", s);
    char *redirect_uri = (char *)malloc(e - s);

    for (int i = 0; i < e - s; i++)
        redirect_uri[i] = login_url[s + i];
    // redirect_uri[e-s]='\0';
    // printf("redirect_uri=%s\n", redirect_uri);

    char *encode_redirect_uri = (char *)malloc(58);
    *encode_redirect_uri = '\0';
    url_encode(redirect_uri, strlen(redirect_uri), encode_redirect_uri, 58);
    struct timeb st;
    ftime(&st);
    time_t t = time(NULL);
    long timestamp = time(&t) * 1000 + st.millitm;
    char *url = (char *)malloc(strlen_x("https://open.work.weixin.qq.com/wwopen/sso/l/qrConnect?callback=jsonpCallback&key=%s&redirect_uri=%s&appid=%s&_=%ld", key, encode_redirect_uri, appid, timestamp));
    sprintf(url, "https://open.work.weixin.qq.com/wwopen/sso/l/qrConnect?callback=jsonpCallback&key=%s&redirect_uri=%s&appid=%s&_=%ld", key, encode_redirect_uri, appid, timestamp);

    // printf("key=%s\n", key);
    // printf("encode_redirect_uri=%s\n", encode_redirect_uri);
    // printf("appid=%s\n", appid);
    // printf("url=%s\n", url);
    int c = 0;
    char *value = NULL;

    while (c < 60)
    {

        CURL *curl;
        CURLcode res;
        FILE *curl_tmp_file = fopen("/tmp/pam_wxwork", "rw+");
        struct curl_slist *http_header = NULL;

        curl = curl_easy_init();
        if (!curl)
        {
            fprintf(stderr, "curl init failed\n");
            return NULL;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);          // url地址
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); //不检查ssl，可访问https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1); //不检查ssl，可访问https
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);        //打印调试信息
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_tmp_file);
        // http_header = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //设置为非0,响应头信息location
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20); //接收数据时超时设置，如果10秒内数据未

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            switch (res)
            {
            case CURLE_UNSUPPORTED_PROTOCOL:
                fprintf(stderr, "不支持的协议,由URL的头部指定\n");
            case CURLE_COULDNT_CONNECT:
                fprintf(stderr, "不能连接到remote主机或者代理\n");
            case CURLE_HTTP_RETURNED_ERROR:
                fprintf(stderr, "http返回错误\n");
            case CURLE_READ_ERROR:
                fprintf(stderr, "发送读错误\n");
            case CURLE_WRITE_ERROR:
                fprintf(stderr, "发送写错误\n");
            default:
                fprintf(stderr, "错误返回值:%d\n", res);
            }
            return NULL;
        }

        long buflen = ftell(curl_tmp_file);
        char *body = (char *)malloc(buflen);
        fseek(curl_tmp_file, 0, SEEK_SET);
        fread(body, buflen, 1, curl_tmp_file);
        // printf("文件内容:%s\n", body);

        if (strindex_x(body, "QRCODE_SCAN_SUCC", 0) != -1)
        {
            value = json_extract_key(body, "\"auth_code\"");
            if (value == NULL)
            {
                printf("查找值失败\n");
                break;
            }
            break;
        }
        else if (strindex_x(body, "QRCODE_SCAN_ING", 0) != -1)
        {
            printf("正在验证，请稍等\n");
        }
        else if (strindex_x(body, "QRCODE_SCAN_ERR", 0) != -1)
        {
            printf("回调失败,重新发起请求\n");
        }

        // 释放
        free(body);
        curl_easy_cleanup(curl);
        fclose(curl_tmp_file);
        sleep(3);
        c++;
    }
    free(url);
    free(redirect_uri);
    return value;
}
char *create_redirect_url(char *login_url, char *auth_code, char *appid)
{
    char *flag = "redirect_uri=";
    int s = strindex_x(login_url, flag, 0) + strlen(flag);
    int e = strlen(login_url);
    char *redirect_uri = (char *)malloc(e - s);

    for (int i = 0; i < e - s; i++)
        redirect_uri[i] = login_url[s + i];
    redirect_uri[e - s] = '\0';

    // printf("redirect_uri=%s\n", redirect_uri);
    // printf("appid=%s\n", appid);
    // printf("auth_code=%s\n", auth_code);

    char *url = (char *)malloc(strlen_x("%s&code=%s&appid=%s", redirect_uri, auth_code, appid));
    sprintf(url, "%s&code=%s&appid=%s", redirect_uri, auth_code, appid);
    free(redirect_uri);
    return url;
}

int req_auth_server(pam_handle_t *pamh, char *url)
{
    pam_info(pamh, "重定向访问验证服务器:%s", url);
    int ret = 1;
    CURL *curl;
    CURLcode res;
    FILE *curl_tmp_file = fopen("/tmp/pam_wxwork", "rw+");
    struct curl_slist *http_header = NULL;

    curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "curl init failed\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);          // url地址
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1); //不检查ssl，可访问https
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);        //打印调试信息
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_tmp_file);
    // http_header = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //设置为非0,响应头信息location
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20); //接收数据时超时设置，如果10秒内数据未

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        switch (res)
        {
        case CURLE_UNSUPPORTED_PROTOCOL:
            fprintf(stderr, "不支持的协议,由URL的头部指定\n");
        case CURLE_COULDNT_CONNECT:
            fprintf(stderr, "不能连接到remote主机或者代理\n");
        case CURLE_HTTP_RETURNED_ERROR:
            fprintf(stderr, "http返回错误\n");
        case CURLE_READ_ERROR:
            fprintf(stderr, "发送读错误\n");
        case CURLE_WRITE_ERROR:
            fprintf(stderr, "发送写错误\n");
        default:
            fprintf(stderr, "错误返回值:%d\n", res);
        }
        return -1;
    }

    long buflen = ftell(curl_tmp_file);
    char *body = (char *)malloc(buflen);
    fseek(curl_tmp_file, 0, SEEK_SET);
    fread(body, buflen, 1, curl_tmp_file);
    // printf("文件内容:%s\n", body);

    if (body == "success")
    {

        pam_info(pamh, "%s", "验证通过！！");
        ret = 0;
    }
    else
    {
        pam_info(pamh, "%s", "验证失败！！");
        ret = 1;
    }

    // 释放
    free(body);
    curl_easy_cleanup(curl);
    fclose(curl_tmp_file);
    return 0;
}