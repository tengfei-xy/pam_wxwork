#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

extern int strindex_x(char *str, char *sub, int start)
{
    if (str == NULL || sub == NULL)
    {
        return -1;
    }
    int str_len = strlen(str);
    int sub_len = strlen(sub);
    int i, j;
    for (i = start; i < str_len - sub_len; i++)
    {
        for (j = 0; j < sub_len; ++j)
            if (str[i + j] != sub[j])
                break;
        if (j == sub_len)
            return i;
    }
    return -1;
}

extern int strlen_x(const char *format, ...)
{
    va_list arg;
    int c = 1;
    va_start(arg, format);

    while (*format)
    {
        char ret = *format;
        if (ret == '%')
        {
            switch (*++format)
            {
            case 'c':
                va_arg(arg, int);
                c++;
                break;

            case 's':
            {
                char *pc = va_arg(arg, char *);
                while (*pc)
                {
                    pc++;
                    c++;
                }
                break;
            }
            case 'd':
            {
                int pc = va_arg(arg, int);
                while (pc > 10)
                {
                    // n = n/10
                    pc /= 10;
                    c++;
                }
                break;
            }
            case 'l':
            {
                if (*(format + 1) == 'd')
                {
                    long pc = va_arg(arg, long);

                    while (pc > 10)
                    {
                        pc /= 10;
                        c++;
                    }
                }
                format++;
            }

            default:
                break;
            }
        }
        else
            c++;
        format++;
    }
    va_end(arg);
    return c;
}

extern char * json_extract_key(char *json,char * target)
{
    int s = strindex_x(json,target,0);
    if (s==-1){
        return NULL;
    }
    s = s+2+strlen(target);
    int e = strindex_x(json,"\"",s);
    char * value = (char * )malloc(e-s);
    for(int i=0;i<e-s;i++){
        value[i]=json[s+i];
    }
    value[e-s] = '\0';
    return value;
}