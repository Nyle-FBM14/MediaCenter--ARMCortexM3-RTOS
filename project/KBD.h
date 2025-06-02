#include <stdint.h>
#include <stdio.h>
#ifndef __KBD_H
#define __KBD_H

#define KBD_SELECT	0x01
#define KBD_UP			0x08
#define KBD_RIGHT		0x10
#define KBD_DOWN		0x20
#define KBD_LEFT		0x40
#define KBD_MASK		0x79

extern uint32_t KBD_val;

extern void KBD_Init(void);
extern uint32_t KBD_get(void);
extern uint32_t get_button(void);

#endif