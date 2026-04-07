#ifndef __SOC_H__
#define __SOC_H__

#ifdef __cplusplus
extern "C" {
#endif

char *get_compatible_name(void);
char *get_soc_name(char *buf);
char *get_kernel_version(char *buf);
char *get_system_version(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

