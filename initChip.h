  
#ifndef INITCHIP_H
#define	INITCHIP_H
#include <xc.h> // include processor files - each processor file is guarded. 

void initWDT(void);
void initCPU(void);
void initPORTA(void);
void initPORTC(void);
void initChip(void);

#endif	/* INITCHIP_H */

