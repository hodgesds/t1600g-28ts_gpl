/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 *
 * Author: Mingkai Hu (Mingkai.hu@freescale.com)
 * Based on stmicro.c by Wolfgang Denk (wd@denx.de),
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com),
 * and  Jason McMullan (mcmullan@netapp.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"


#define FLASH_HALF 0x1000000

extern int sector_num;
extern int sector_size;

struct spansion_spi_flash_params {
	u16 idcode1;
	u16 idcode2;
	u16 pages_per_sector;
	u16 nr_sectors;
	const char *name;
};

static const struct spansion_spi_flash_params spansion_spi_flash_table[] = {
	{
		.idcode1 = 0x0213,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 16,
		.name = "S25FL008A",
	},
	{
		.idcode1 = 0x0214,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 32,
		.name = "S25FL016A",
	},
	{
		.idcode1 = 0x0215,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032A",
	},
	{
		.idcode1 = 0x0216,
		.idcode2 = 0,
		.pages_per_sector = 256,
		.nr_sectors = 128,
		.name = "S25FL064A",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x0301,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128P_64K",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x0300,
		.pages_per_sector = 1024,
		.nr_sectors = 64,
		.name = "S25FL128P_256K",
	},
	{
		.idcode1 = 0x0215,
		.idcode2 = 0x4d00,
		.pages_per_sector = 256,
		.nr_sectors = 64,
		.name = "S25FL032P",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128S_64K",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL128S_64K",
	},
	{
		.idcode1 = 0x0219,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 512,
		.name = "S25FL256S_64K",
	},
	{
		.idcode1 = 0x2018,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 256,
		.name = "S25FL129P_64K",
	},
	{
		.idcode1 = 0x2019,
		.idcode2 = 0x4d01,
		.pages_per_sector = 256,
		.nr_sectors = 512,
		.name = "S25FL256S",
	},
};


static int spi_flash_set_bank_register(struct spi_flash *flash,  u8 value)
{
	struct spi_slave *spi = flash->spi;
	u8 data[5] ={0xff,0xff,0xff,0xff,0xff} ;
	u8 cmd[5] = {0};
	
//	printf("offset = %0x, set the BA24 field\n", offset);
	spi_claim_bus(spi);

	/*read the Bank Register, opcode 0x16h */
	cmd[0] = 0x16;
//	spi_flash_read_common(flash, cmd, 1, data,1);
	spi_flash_cmd(spi, 0x16, data, 1);
//	printf("Bank register data[0]=%0x  data[1]=%0x  data[2]=%0x\n",data[0], data[1], data[2]);

	/*write the Bank Register, set the field of BA24 ,opcode 0x17h */
	cmd[0] = 0x17;
	cmd[1] = value;
	spi_flash_cmd_write(spi, cmd, 2, NULL, 0);

//     printf("[%s] %d\n", __FUNCTION__, __LINE__);
	/*read the Bank Register, opcode 0x16h */
	cmd[0] = 0x16;
//	spi_flash_read_common(flash, cmd, 1, data, 1);
	spi_flash_cmd(spi, 0x16, data, 1);
//	printf("Bank register data[0]=%0x  data[1]=%0x  data[2]=%0x\n",data[0], data[1], data[2]);
	spi_release_bus(spi);

	return 0;
}



static  int spi_flash_cmd_read_fast_tp(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	
	int ret; 

	if (offset >> 24)
	{

		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_read_fast(flash, offset, len, data);

		spi_flash_set_bank_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_read_fast(flash, offset, FLASH_HALF - offset, data);

		/**/
		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_read_fast(flash, FLASH_HALF, len + offset - FLASH_HALF, (void*)((char*)data + FLASH_HALF - offset));

		spi_flash_set_bank_register(flash, 0);

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

		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_write_multi(flash, offset, len, buf);

		spi_flash_set_bank_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_write_multi(flash, offset, FLASH_HALF - offset, buf);

		/**/
		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_write_multi(flash, FLASH_HALF, len + offset - FLASH_HALF,  (void*)((char*)buf + FLASH_HALF - offset));

		spi_flash_set_bank_register(flash, 0);

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

		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_erase(flash, offset, len);

		spi_flash_set_bank_register(flash, 0);
	}
	else if((offset + len - 1) >> 24)
	{
		/**/
		ret = spi_flash_cmd_erase(flash, offset, FLASH_HALF - offset);

		/**/
		spi_flash_set_bank_register(flash, 1);

		ret = spi_flash_cmd_erase(flash, FLASH_HALF, len + offset - FLASH_HALF);

		spi_flash_set_bank_register(flash, 0);

	}
	else
	{
		ret = spi_flash_cmd_erase(flash, offset, len);
	}

	return ret;
}

struct spi_flash *spi_flash_probe_spansion(struct spi_slave *spi, u8 *idcode)
{
	const struct spansion_spi_flash_params *params;
	struct  spi_flash *flash;
	unsigned int i;
	unsigned short jedec, ext_jedec;

	jedec = idcode[1] << 8 | idcode[2];
	ext_jedec = idcode[3] << 8 | idcode[4];

    for(i = 0; i < ARRAY_SIZE(spansion_spi_flash_table); i++) {
		params = &spansion_spi_flash_table[i];
		if(params->idcode1 == jedec) {
			if (params->idcode2 == ext_jedec)
				
			break;
		}
	}

	if(i == ARRAY_SIZE(spansion_spi_flash_table)) {
		debug("SF: Unsupported SPANSION ID %04x %04x\n", jedec, ext_jedec);
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
	flash->read = spi_flash_cmd_read_fast_tp;
	flash->page_size = 256;
	flash->sector_size = 256 * params->pages_per_sector;
	flash->size = flash->sector_size * params->nr_sectors;

	sector_size = flash->sector_size;
	sector_num = params->nr_sectors;

	return flash;
}

