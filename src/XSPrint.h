#ifndef __XS_PRINT_H__
#define __XS_PRINT_H__

#define XSInfo(fmt, args ...) printf(GREEN"[XSocket]%s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);
#define XSDbug(fmt, args ...) printf(YELLOW"[XSocket]%s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);
#define XSWarn(fmt, args ...) printf(RED"[XSocket]%s[%d]:"fmt, __FUNCTION__, __LINE__, ##args);


/** printf color scheme in C */
#define NONE          "\033[m"   
#define RED           "\033[0;32;31m"   
#define GREEN         "\033[0;32;32m"   
#define BLUE          "\033[0;32;34m"   
#define DARY_GRAY     "\033[1;30m"   
#define CYAN          "\033[0;36m"   
#define PURPLE        "\033[0;35m"     
#define BROWN         "\033[0;33m"   
#define YELLOW        "\033[1;33m"   
#define WHITE         "\033[1;37m"   


#endif
