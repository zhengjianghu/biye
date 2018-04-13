#include "esp_common.h"



u8 ICACHE_FLASH_ATTR flash_write(u32 WriteAddr,u8 *data,u32 datalen)
{
	u32 secpos = 0;
	u16 secoff = 0;
	u32 needsector = 0;
	u16 icnt = 0;
	
	secpos=WriteAddr/4096;//扇区地址
	secoff=WriteAddr%4096;//在扇区内的偏移

	if(secoff > 0)
	{
		//os_printf("flash_write:err0\n");
		return 0;
	}	
	needsector = datalen/4096;
	secoff = datalen%4096;

	if(secoff > 0)
	{
		needsector = needsector + 1;
	}		

	for(icnt = secpos; icnt < needsector;icnt++)
	{
		spi_flash_erase_sector(icnt);
	}	
	spi_flash_write(WriteAddr,(u32 *)data,datalen);

	return 1;
}

u8 ICACHE_FLASH_ATTR flash_readx(u32 ReadAddr,u8 *data,u32 datalen)
{
	if(ReadAddr%4 > 0)
	{
		//os_printf("flash_read:err0\n");
		return 0;
	}
	spi_flash_read(ReadAddr,(u32 *)data,datalen);
	return 1;
}


