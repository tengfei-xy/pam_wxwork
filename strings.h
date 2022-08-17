extern int strindex_x(char *str, char *sub, int start);
extern int strlen_x(const char *format, ...);
extern char *json_extract_key(char *json,char * target);
extern int url_encode(const char *str, const int strsz, char *result, const int resultsz);
extern int url_decode(const char *str, const int strsz, char *result, const int resultsz, const char **last_pos);