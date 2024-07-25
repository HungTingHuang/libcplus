#ifndef __CPLUS_SYS_H__
#define __CPLUS_SYS_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * cplus_sys_skip_file_path(const char * filepath);
void cplus_sys_print_stack(void);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_SYS_H__
