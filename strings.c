#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
unsigned char hexchars[] = "0123456789ABCDEF";

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



/**
 * @brief url_encode : encode the base64 string "str"
 *
 * @param str:  the base64 encoded string
 * @param strsz:  the str length (exclude the last \0)
 * @param result:  the result buffer
 * @param resultsz: the result buffer size(exclude the last \0)
 *
 * @return: >=0 represent the encoded result length
 *              <0 encode failure
 *
 * Note:
 * 1) to ensure the result buffer has enough space to contain the encoded string, we'd better
 *     to set resultsz to 3*strsz
 *
 * 2) we don't check whether str has really been base64 encoded
 */

extern int url_encode(const char *str, const int strsz, char *result, const int resultsz)
{
    int i,j;
	char ch;

    if(strsz < 0 || resultsz < 0)
		return -1;

	for(i = 0,j = 0;i<strsz && j < resultsz;i++)
	{
		ch = *(str + i);
		if((ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= '0' && ch <= '9') ||
			ch == '.' || ch == '-' || ch == '*' || ch == '_')
			result[j++] = ch;
		else if(ch == ' ')
			result[j++] = '+';
		else{
			if(j + 3 <= resultsz)
			{
			    result[j++] = '%';
				result[j++] = hexchars[(unsigned char)ch >> 4];
				result[j++] = hexchars[(unsigned char)ch & 0xF];
			}
			else{
				return -2;
			}
		}
	}

    if(i == 0)
		return 0;
	else if(i == strsz)
		return j;
	return -2;
}


/**
 * @brief url_decode : decode the urlencoded str to base64 encoded string
 *
 * @param str:  the urlencoded string
 * @param strsz:  the str length (exclude the last \0)
 * @param result:  the result buffer
 * @param resultsz: the result buffer size(exclude the last \0)
 *
 * @return: >=0 represent the decoded result length
 *              <0 encode failure
 *
 * Note:
 * 1) to ensure the result buffer has enough space to contain the decoded string, we'd better
 *     to set resultsz to strsz
 *
 */
extern int url_decode(const char *str, const int strsz, char *result, const int resultsz, const char **last_pos)
{
    int i,j;
	char ch;
	char a;

    *last_pos = str;
    if(strsz < 0 || resultsz < 0)
		return -1;

    for(i = 0,j = 0;i<strsz && j<resultsz;j++)
    {
        ch = *(str + i);

		if(ch == '+')
		{
			result[j] = ' ';
			i += 1;
		}
		else if(ch == '%')
		{
		    if(i+3 <= strsz)
		    {
		        ch = *(str + i + 1);

				if(ch >= 'A' && ch <= 'F')
				{
				    a = (ch - 'A')+10;
				}
				else if(ch >= '0' && ch <= '9')
				{
				    a = ch - '0';
				}
				else if(ch >= 'a' && ch <= 'f')
				{
				   a = (ch - 'a') + 10;
				}
				else{
					return -2;
				}

			    a <<= 4;

				ch = *(str + i + 2);
				if(ch >= 'A' && ch <= 'F')
				{
				    a |= (ch - 'A') + 10;
				}
				else if(ch >= '0' && ch <= '9')
				{
				    a |= (ch - '0');
				}
				else if(ch >= 'a' && ch <= 'f')
				{
				     a |= (ch - 'a') + 10;
				}
				else{
					return -2;
				}

			    result[j] = a;

                i += 3;
		    }
			else
				break;
		}
		else if((ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= '0' && ch <= '9') ||
			ch == '.' || ch == '-' || ch == '*' || ch == '_'){

			result[j] = ch;
			i+=1;
		}
		else{
			return -2;
		}

    }

    *last_pos =  str + i;
    return j;

}
