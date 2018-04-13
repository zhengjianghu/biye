#ifndef __FLASH_H__
#define __FLASH_H__


#define PARA_ADDR		(0x80000)

u8 ICACHE_FLASH_ATTR flash_write(u32 WriteAddr,u8 *data,u32 datalen);
u8 ICACHE_FLASH_ATTR flash_readx(u32 ReadAddr,u8 *data,u32 datalen);

#endif
