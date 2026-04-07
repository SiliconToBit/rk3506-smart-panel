#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#ifdef __cplusplus
extern "C" {
#endif


uint32_t clock_ms(void);
uint64_t clock_us(void);
void timestamp(char *fmt, int index, int start);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

