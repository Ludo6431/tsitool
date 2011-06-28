#ifndef _TOOLS_H
#define _TOOLS_H

#define be2cpu16(u) (((((u)>>0)&0xFF) << 8) | ((((u)>>8)&0xFF) << 0))
#define be2cpu32(u) (((((u)>>0)&0xFF) << 24) | ((((u)>>8)&0xFF) << 16) | ((((u)>>16)&0xFF) << 8) | ((((u)>>24)&0xFF) << 0))
#define MAKEID(a, b, c, d) be2cpu32((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((c)&0xFF)<<8) | (((d)&0xFF)<<0))

#endif

