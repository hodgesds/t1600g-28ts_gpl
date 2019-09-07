/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

extern int sector_num;
extern int sector_size;

struct winbond_spi_flash_params {
	uint16_t	id;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= 0x3013,
		.nr_blocks		= 8,
		.name			= "W25X40",
	},
	{
		.id			= 0x3015,
		.nr_blocks		= 32,
		.name			= "W25X16",
	},
	{
		.id			= 0x3016,
		.nr_blocks		= 64,
		.name			= "W25X32",
	},
	{
		.id			= 0x3017,
		.nr_blocks		= 128,
		.name			= "W25X64",
	},
	{
		.id			= 0x4014,
		.nr_blocks		= 16,
		.name			= "W25Q80BL",
	},
	{
		.id			= 0x4015,
		.nr_blocks		= 32,
		.name			= "W25Q16",
	},
	{
		.id			= 0x4016,
		.nr_blocks		= 64,
		.name			= "W25Q32",
	},
	{
		.id			= 0x4017,
		.nr_blocks		= 128,
		.name			= "W25Q64",
	},
	{
		.id			= 0x4018,
		.nr_blocks		= 256,
		.name			= "W25Q128",
	},
	{
		.id			= 0x4019,
		.nr_blocks		= 512,
		.name			= "W25Q256",
	},
	{
		.id			= 0x5014,
		.nr_blocks		= 128,
		.name			= "W25Q80",
	},
};
#if 0   /* ²âÊÔ _spi_flash_set_ea_register() */
#define FLASH_WAIT_DONE(cmd0, exp, retryMax, errarg...)     retry = retryMax;\
    do{\
	    spi_flash_cmd(flash->spi, cmd0, data, 1);\
	    if (exp) break;\
        printf(errarg);\
    }while (--retry != 0);
#define DBG(arg...) printf(arg)
#else
#define FLASH_WAIT_DONE(cmd0, exp, retryMax, errarg...)
#define DBG(arg...)
#endif
#define spi_flash_set_ea_register(a,b) if (_spi_flash_set_ea_register(a,b,__LINE__))\
                                       {printf("SF: unable to set ea.\n");return -1;}
static inline int _spi_flash_set_ea_register(struct spi_flash *flash,  u8 value, int line)
{
	struct spi_slave *spi = flash->spi;
    int ret = 0;
	u8 cmd[5] = {0};
	u8 data[5] ={0xff,0xff,0xff,0xff,0xff};
    int retry = 10;
    DBG("Line%d\r\n", line);

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		printf("SF: unable to claim SPI bus\n");
		return ret;
	}

    /* write enable reg */
    ret = spi_flash_cmd_write_enable(flash);
	if (ret) {
		printf("SF: unable to set wel\n");
		return ret;
	}

    /* write EA reg */
	cmd[0] = 0xC5;
	data[0] = value;
	ret = spi_flash_cmd_write(spi, cmd, 1, data, 1);
	if (ret) {
		printf("SF: unable to write ea to flash\n");
		return ret;
	}

    /* write disable reg */
	ret = spi_flash_cmd_write_disable(flash);
	if (ret) {
		printf("SF: unable to clear wel to flash\n");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}


#define FLASH_HALF 0x1000000


static  int spi_flash_cmd_read_tp(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	
	int ret; 

	if (offset >> 24)
	{

		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_read_fast(flash, offset, len, data);

		spi_flash_set_ea_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_read_fast(flash, offset, FLASH_HALF - offset, data);

		/**/
		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_read_fast(flash, FLASH_HALF, len + offset - FLASH_HALF, (void*)((char*)data + FLASH_HALF - offset));

		spi_flash_set_ea_register(flash, 0);

	}
	else
	{
		ret = spi_flash_cmd_read_fast(flash, offset, len, data);
	}


	return ret;

}


static int spi_flash_cmd_write_multi_tp(struct spi_flash *flash, u32 offset,
size_t len, const void *buf)
{
	    		
	int ret; 

	if (offset >> 24)
	{

		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_write_multi(flash, offset, len, buf);

		spi_flash_set_ea_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_write_multi(flash, offset, FLASH_HALF - offset, buf);

		/**/
		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_write_multi(flash, FLASH_HALF, len + offset - FLASH_HALF,  (void*)((char*)buf + FLASH_HALF - offset));

		spi_flash_set_ea_register(flash, 0);

	}
	else
	{
		ret = spi_flash_cmd_write_multi(flash, offset, len, buf);
	}

	return ret;
}

static  int spi_flash_cmd_erase_tp(struct spi_flash *flash, u32 offset, size_t len)
{
	int ret;


	if (offset >> 24)
	{

		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_erase(flash, offset, len);

		spi_flash_set_ea_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_erase(flash, offset, FLASH_HALF - offset);

		/**/
		spi_flash_set_ea_register(flash, 1);

		ret = spi_flash_cmd_erase(flash, FLASH_HALF, len + offset - FLASH_HALF);

		spi_flash_set_ea_register(flash, 0);

	}
	else
	{
		ret = spi_flash_cmd_erase(flash, offset, len);
	}

	return ret;
}

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode)
{
	const struct winbond_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		debug("SF: Unsupported Winbond ID %02x%02x\n",
				idcode[1], idcode[2]);
		return NULL;
	}

	flash = malloc(sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	flash->spi = spi;
	flash->name = params->name;

	flash->write = spi_flash_cmd_write_multi_tp;
	flash->erase = spi_flash_cmd_erase_tp;
	flash->read = spi_flash_cmd_read_tp;
	flash->page_size = 256;
	flash->sector_size = 4096 * 16;
	flash->size = flash->sector_size * params->nr_blocks;

	sector_size = flash->sector_size;  /* 64k */
	sector_num = params->nr_blocks;    /* 512 */

	return flash;
}
