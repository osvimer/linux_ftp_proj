#ifndef __INDENTIFY__
#define __INDENTIFY__
#include <pwd.h>
#include <sys/utsname.h>
#include "head.h"
void get_salt(char *, char *);
void client_identify(int);
#endif
