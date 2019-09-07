/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mod_devicetable.h>

#include <linux/mtd/cfi.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of_platform.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <net/sock.h>  
#include <net/netlink.h> 

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_WRSR		0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Used for SST flashes only. */
#define	OPCODE_BP		0x02	/* Byte program */
#define	OPCODE_WRDI		0x04	/* Write disable */
#define	OPCODE_AAI_WP		0xad	/* Auto address increment word program */

/* Used for Macronix flashes only. */
#define	OPCODE_EN4B		0xb7	/* Enter 4-byte mode */
#define	OPCODE_EX4B		0xe9	/* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define	OPCODE_BRWR		0x17	/* Bank register write */

/* Used for some Micron flashes only. */
#define	OPCODE_RDFSR		0x70	/* Read flags status register */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Flags Status Register bits (Micron specific) */
#define	FSR_WCRDY			0x80	/* Write (erase/program) controller ready */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(40 * HZ)	/* M25P16 specs 40s max chip erase */
#define	MAX_CMD_SIZE		5

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#define JEDEC_MFR(_jedec_id)	((_jedec_id) >> 16)

/****************************************************************************/

struct m25p {
	struct spi_device	*spi;
	struct mutex		lock;
	struct mtd_info		mtd;
	unsigned		flag_status_register:1;
	u32			jedec_id;
	u16			page_size;
	u16			addr_width;
	u8			erase_opcode;
	u8			*command;
};

static inline struct m25p *mtd_to_m25p(struct mtd_info *mtd)
{
	return container_of(mtd, struct m25p, mtd);
}

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	u8 val;

	retval = spi_write_then_read(flash->spi, &code, 1, &val, 1);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static int write_sr(struct m25p *flash, u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return spi_write(flash->spi, flash->command, 2);
}

/*
 * Read the Flag Status Register (required for some Micron chips).
 * Return the Flag Status Register value.
 * Returns negative if error occurred.
 */
static int read_fsr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDFSR;
	u8 val;

	retval = spi_write_then_read(flash->spi, &code, 1, &val, 1);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading FSR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct m25p *flash)
{
	u8	code = OPCODE_WREN;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

/*
 * Send write disble instruction to the chip.
 */
static inline int write_disable(struct m25p *flash)
{
	u8	code = OPCODE_WRDI;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

/*
 * Enable/disable 4-byte addressing mode.
 */
static inline int set_4byte(struct m25p *flash, int enable)
{
	int ret;

	switch (JEDEC_MFR(flash->jedec_id)) {
	case CFI_MFR_MACRONIX:
		flash->command[0] = enable ? OPCODE_EN4B : OPCODE_EX4B;
		return spi_write(flash->spi, flash->command, 1);
	case CFI_MFR_ST:
		flash->command[0] = enable ? OPCODE_EN4B : OPCODE_EX4B;
		ret = write_enable(flash);
		if (!ret) ret = spi_write(flash->spi, flash->command, 1);
		if (!ret) ret = write_disable(flash);
		return ret;
	default:
		/* Spansion style */
		flash->command[0] = OPCODE_BRWR;
		flash->command[1] = enable << 7;
		return spi_write(flash->spi, flash->command, 2);
	}
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct m25p *flash)
{
	unsigned long deadline;
	int sr;

	deadline = jiffies + MAX_READY_WAIT_JIFFIES;

	do {
		if ((sr = read_sr(flash)) < 0) {
			break;
		} else if (!(sr & SR_WIP)) {
			if (flash->flag_status_register) {
				/* For some Micron flashes, polling for FSR is required */
				if ((sr = read_fsr(flash)) < 0) {
					break;
				} else if (sr & FSR_WCRDY) {
					return 0;
				}
			} else {
				return 0;
			}
		}

		cond_resched();

	} while (!time_after_eq(jiffies, deadline));

	return 1;
}

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(struct m25p *flash)
{
	pr_debug("%s: %s %lldKiB\n", dev_name(&flash->spi->dev), __func__,
			(long long)(flash->mtd.size >> 10));

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;

	spi_write(flash->spi, flash->command, 1);

	return 0;
}

static void m25p_addr2cmd(struct m25p *flash, unsigned int addr, unsigned int len, u8 *cmd)
{
   u16 addr_width = flash->addr_width;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (addr + len > 0x1000000) {
		addr_width = 4;
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	/* opcode is in cmd[0] */
	cmd[1] = addr >> (addr_width * 8 -  8);
	cmd[2] = addr >> (addr_width * 8 - 16);
	cmd[3] = addr >> (addr_width * 8 - 24);
	cmd[4] = addr >> (addr_width * 8 - 32);
}

static int m25p_cmdsz(struct m25p *flash, unsigned int addr)
{
#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (addr >= 0x1000000) {
		return 1 + 4;
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	return 1 + flash->addr_width;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct m25p *flash, u32 offset)
{
	pr_debug("%s: %s %dKiB at 0x%08x\n", dev_name(&flash->spi->dev),
			__func__, flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (offset >= 0x1000000) {
		set_4byte(flash, 1);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	m25p_addr2cmd(flash, offset, 1, flash->command);

	spi_write(flash->spi, flash->command, m25p_cmdsz(flash, offset));

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int m25p80_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 addr,len;
	uint32_t rem;

	pr_debug("%s: %s at 0x%llx, len %lld\n", dev_name(&flash->spi->dev),
			__func__, (long long)instr->addr,
			(long long)instr->len);

	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* whole-chip erase? */
	if (len == flash->mtd.size) {
		if (erase_chip(flash)) {
			instr->state = MTD_ERASE_FAILED;
			mutex_unlock(&flash->lock);
			return -EIO;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			if (erase_sector(flash, addr)) {
				instr->state = MTD_ERASE_FAILED;
				mutex_unlock(&flash->lock);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Reset to 3-byte mode if it was set to 4-byte mode */
	if (addr > 0x1000000) {
		wait_till_ready(flash);
		set_4byte(flash, 0);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int m25p80_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	struct spi_transfer t[2];
	struct spi_message m;

	pr_debug("%s: %s from 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)from, len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash, from + len - 1) + FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&flash->lock);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		return 1;
	}

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (from + len > 0x1000000) {
		set_4byte(flash, 1);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	m25p_addr2cmd(flash, from, len, flash->command);

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - m25p_cmdsz(flash, from + len -1) - FAST_READ_DUMMY_BYTE;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Reset to 3-byte mode if it was set to 4-byte mode */
	if (from + len > 0x1000000) {
		set_4byte(flash, 0);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */
	
	mutex_unlock(&flash->lock);

	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int m25p80_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;

	pr_debug("%s: %s to 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)to, len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash, to);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		mutex_unlock(&flash->lock);
		return 1;
	}

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (to >= 0x1000000) {
		set_4byte(flash, 1);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	m25p_addr2cmd(flash, to, 1, flash->command);

	page_offset = to & (flash->page_size - 1);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= flash->page_size) {
		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = len;

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash, to);
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = flash->page_size - page_offset;

		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = page_size;
		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash, to);

		/* write everything in flash->page_size chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > flash->page_size)
				page_size = flash->page_size;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
			/* Set to 4-byte mode if this is the first time > 16MB */
			if (to + i - page_size < 0x1000000 && to + i >= 0x1000000) {
				set_4byte(flash, 1);
			}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

			/* write the next page to flash */
			m25p_addr2cmd(flash, to + i, 1, flash->command);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			*retlen += m.actual_length - m25p_cmdsz(flash,to);
		}
	}

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Reset to 3-byte mode if it was set to 4-byte mode */
	if (to + len > 0x1000000) {
		wait_till_ready(flash);
		set_4byte(flash, 0);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	mutex_unlock(&flash->lock);

	return 0;
}

/* winbon W25Q256FV��4-byte��ַģʽʱ����ȡ�쳣
 * ʹ��3-byte��ַģʽ������extended address�Ĵ��������ʸ�16M�ռ� */
#define FLASH_HALF 0x1000000    /* 16M */
#if 0   /* ���ڵ���w25q256fv_setea()�ӿ� */
#define FLASH_WAIT_DONE(cmd0, exp, retryMax, errarg...)     retry = retryMax;\
    do{\
        cmd[0] = cmd0;\
	    spi_write_then_read(flash->spi, cmd, 1, data, 1);\
	    if (exp) break;\
        printk(errarg);\
    }while (--retry != 0);
#define dbg_prk(arg...) printk(arg)
#else
#define FLASH_WAIT_DONE(cmd0, exp, retryMax, errarg...)
#define dbg_prk(arg...)
#endif
#define FLASH_WAIT_READY(flash)  \
    if (wait_till_ready(flash))\
    {\
        dbg_prk("Flash is not ready in setting extended address.\r\n");\
        return -1;\
    }
static inline int w25q256fv_setea(struct m25p *flash, int enable)
{
    int ret = 0;
    int retry = 0;
    u8 cmd[5] ={0xff,0xff,0xff,0xff,0xff};
    u8 data[5] ={0xff,0xff,0xff,0xff,0xff};
	FLASH_WAIT_READY(flash);
    /* wel enable */
    ret = write_enable(flash);
    FLASH_WAIT_DONE(0x5,(data[0] & 0x2),10,"ERROR: Data[0]=%d, expect set bit2, retry=%d\r\n", data[0], retry);
    
	FLASH_WAIT_READY(flash);
    /* write EA reg */
    flash->command[0] = 0xC5;
    flash->command[1] = (enable ? 1:0);
    if (!ret) spi_write(flash->spi, flash->command, 2);
    FLASH_WAIT_DONE(0xc8,(data[0] == enable),10,"ERROR: Data[0]=%d, value=%d, retry=%d\r\n", data[0], enable, retry);

	FLASH_WAIT_READY(flash);
    /* wel disable */
    if (!ret) ret = write_disable(flash);
    FLASH_WAIT_DONE(0x5,(0 == (data[0] & 0x2)),10,"ERROR: Data[0]=%d, expect clr bit2, retry=%d\r\n", data[0], retry);
    
    return ret;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector_tp(struct m25p *flash, u32 offset)
{
    int eraseOffset = offset;
	pr_debug("%s: %s %dKiB at 0x%08x\n", dev_name(&flash->spi->dev),
			__func__, flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	if (offset >= FLASH_HALF) {
        eraseOffset = eraseOffset - FLASH_HALF;
		w25q256fv_setea(flash, 1);
	}

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	m25p_addr2cmd(flash, eraseOffset, 1, flash->command);

	spi_write(flash->spi, flash->command, m25p_cmdsz(flash, eraseOffset));

	if (offset >= FLASH_HALF) {
        wait_till_ready(flash);
		w25q256fv_setea(flash, 0);
	}

	return 0;
}

/* for winbon w25q256fv flash only */
static int w25q256fv_erase_tp(struct mtd_info *mtd, struct erase_info *instr)
{
    struct m25p *flash = mtd_to_m25p(mtd);
    u32 addr,len;
    uint32_t rem;

    dbg_prk("Erase:%s: %s at 0x%llx, len %lld\n", dev_name(&flash->spi->dev),
            __func__, (long long)instr->addr,
            (long long)instr->len);

    div_u64_rem(instr->len, mtd->erasesize, &rem);
    if (rem)
        return -EINVAL;

    addr = instr->addr;
    len = instr->len;

    mutex_lock(&flash->lock);

    /* whole-chip erase? */
    if (len == flash->mtd.size) {
        if (erase_chip(flash)) {
            instr->state = MTD_ERASE_FAILED;
            mutex_unlock(&flash->lock);
            return -EIO;
        }

    /* REVISIT in some cases we could speed up erasing large regions
     * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
     * to use "small sector erase", but that's not always optimal.
     */

    /* "sector"-at-a-time erase */
    } else {
        while (len) {
            if (erase_sector_tp(flash, addr)) {
                instr->state = MTD_ERASE_FAILED;
                mutex_unlock(&flash->lock);
                return -EIO;
            }

            addr += mtd->erasesize;
            len -= mtd->erasesize;
        }
    }

    mutex_unlock(&flash->lock);

    instr->state = MTD_ERASE_DONE;
    mtd_erase_callback(instr);

    return 0;
}

/* winbon w25q256fv fast read */
static int m25p80_read_fast_tp(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	struct spi_transfer t[2];
	struct spi_message m;

	dbg_prk("%s: %s from 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)from, len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash, from + len - 1) + FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		return 1;
	}

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	m25p_addr2cmd(flash, from, len, flash->command);

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - m25p_cmdsz(flash, from + len -1) - FAST_READ_DUMMY_BYTE;

	return 0;
}

/* for winbon w25q256fv flash only */
static int w25q256fv_read_tp(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
    int tmp1, tmp2, retlenTmp, ret;

	dbg_prk("Read:%s: %s from 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)from, len);

	mutex_lock(&flash->lock);
    /* ȫ���Ǹ�16M */
    if (from >= FLASH_HALF)
    {
        w25q256fv_setea(flash, 1);
        ret = m25p80_read_fast_tp(mtd, from-FLASH_HALF, len, retlen, buf);
        w25q256fv_setea(flash, 0);
    }
    /* ��Խ16M�ֽ��� */
    else if ((from + len) > FLASH_HALF)
    {
        tmp1 = FLASH_HALF - from;
        tmp2 = len - FLASH_HALF;
        *retlen = 0;

        m25p80_read_fast_tp(mtd, from, tmp1, &retlenTmp, buf);
        *retlen += retlenTmp;

        w25q256fv_setea(flash, 1);
        ret = m25p80_read_fast_tp(mtd, 0, tmp2, &retlenTmp, buf+tmp1);
        w25q256fv_setea(flash, 0);

        *retlen += retlenTmp;
    }
    /* ��16M */
    else
    {
        ret = m25p80_read_fast_tp(mtd, from, len, retlen, buf);
    }
    mutex_unlock(&flash->lock);
    return ret;
}

static int m25p80_write_tp(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;

	pr_debug("%s: %s to 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)to, len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash, to);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		return 1;
	}

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	m25p_addr2cmd(flash, to, 1, flash->command);

	page_offset = to & (flash->page_size - 1);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= flash->page_size) {
		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = len;

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash, to);
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = flash->page_size - page_offset;

		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = page_size;
		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash, to);

		/* write everything in flash->page_size chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > flash->page_size)
				page_size = flash->page_size;

			/* write the next page to flash */
			m25p_addr2cmd(flash, to + i, 1, flash->command);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			*retlen += m.actual_length - m25p_cmdsz(flash,to);
		}
	}

	return 0;
}

/* for winbon w25q256fv flash only */
static int w25q256fv_write_tp(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
    struct m25p *flash = mtd_to_m25p(mtd);
    int tmp1, tmp2, retlenTmp, ret;
	dbg_prk("Write:%s: %s to 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)to, len);

	mutex_lock(&flash->lock);
    /* ȫ���Ǹ�16M */
    if (to >= FLASH_HALF)
    {
        w25q256fv_setea(flash, 1);
        ret = m25p80_write_tp(mtd, to-FLASH_HALF, len, retlen, buf);
        w25q256fv_setea(flash, 0);
    }
    /* ��Խ16M�ֽ��� */
    else if ((to + len) > FLASH_HALF)
    {
        tmp1 = FLASH_HALF - to;
        tmp2 = len - FLASH_HALF;
        *retlen = 0;

        m25p80_write_tp(mtd, to, tmp1, &retlenTmp, buf);
        *retlen += retlenTmp;
        w25q256fv_setea(flash, 1);
        ret = m25p80_write_tp(mtd, 0, tmp2, &retlenTmp, buf+tmp1);
        w25q256fv_setea(flash, 0);

        *retlen += retlenTmp;
    }
    /* ��16M */
    else
    {
        ret = m25p80_write_tp(mtd, to, len, retlen, buf);
    }
    mutex_unlock(&flash->lock);
    return ret;
}

static int sst_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	struct spi_transfer t[2];
	struct spi_message m;
	size_t actual;
	int cmd_sz, ret;
#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	int addr_4byte = 0;
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	pr_debug("%s: %s to 0x%08x, len %zd\n", dev_name(&flash->spi->dev),
			__func__, (u32)to, len);

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash, to);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	ret = wait_till_ready(flash);
	if (ret)
		goto time_out;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (to >= 0x1000000) {
		set_4byte(flash, 1);
		addr_4byte = 1;
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	write_enable(flash);

	actual = to % 2;
	/* Start write from odd address. */
	if (actual) {
		flash->command[0] = OPCODE_BP;
		m25p_addr2cmd(flash, to, 1, flash->command);

		/* write one byte. */
		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = 1;
		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - m25p_cmdsz(flash, to);
	}
	to += actual;

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Use 4-byte mode only if the address is > 16MB */
	if (to >= 0x1000000 && !addr_4byte) {
		set_4byte(flash, 1);
		addr_4byte = 1;
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

	flash->command[0] = OPCODE_AAI_WP;
	m25p_addr2cmd(flash, to, 1, flash->command);

	/* Write out most of the data here. */
	cmd_sz = m25p_cmdsz(flash, to);
	for (; actual < len - 1; actual += 2) {
		t[0].len = cmd_sz;
		/* write two bytes. */
		t[1].len = 2;
		t[1].tx_buf = buf + actual;

		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - cmd_sz;
		cmd_sz = 1;
		to += 2;
	}
	write_disable(flash);
	ret = wait_till_ready(flash);
	if (ret)
		goto time_out;

	/* Write out trailing byte if it exists. */
	if (actual != len) {
    
#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
		/* Use 4-byte mode only if the address is > 16MB */
		if (to >= 0x1000000 && !addr_4byte) {
			set_4byte(flash, 1);
			addr_4byte = 1;
		}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */
    
		write_enable(flash);
		flash->command[0] = OPCODE_BP;
		m25p_addr2cmd(flash, to, 1, flash->command);
		t[0].len = m25p_cmdsz(flash, to);
		t[1].len = 1;
		t[1].tx_buf = buf + actual;

		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - m25p_cmdsz(flash, to);
		write_disable(flash);
	}

#ifdef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
	/* Reset to 3-byte mode if it was set to 4-byte mode */
	if (addr_4byte) {
		set_4byte(flash, 0);
	}
#endif /* CONFIG_M25PXX_STAY_IN_3BYTE_MODE */

time_out:
	mutex_unlock(&flash->lock);
	return ret;
}

/****************************************************************************/

/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;
	u16             ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16		n_sectors;

	u16		page_size;
	u16		addr_width;

	u16		flags;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
#define	M25P_NO_ERASE	0x02		/* No erase command needed */
#define	M25P_FSR_POLL	0x04		/* Need to poll for Flag Status Register */
};

#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
	((kernel_ulong_t)&(struct flash_info) {				\
		.jedec_id = (_jedec_id),				\
		.ext_id = (_ext_id),					\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),					\
	})

#define CAT25_INFO(_sector_size, _n_sectors, _page_size, _addr_width)	\
	((kernel_ulong_t)&(struct flash_info) {				\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = (_page_size),				\
		.addr_width = (_addr_width),				\
		.flags = M25P_NO_ERASE,					\
	})

#define JEDEC_ID_W25Q256FV (0xef4019)
/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static const struct spi_device_id m25p_ids[] = {
	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  INFO(0x1f6601, 0, 32 * 1024,   4, SECT_4K) },
	{ "at25fs040",  INFO(0x1f6604, 0, 64 * 1024,   8, SECT_4K) },

	{ "at25df041a", INFO(0x1f4401, 0, 64 * 1024,   8, SECT_4K) },
	{ "at25df321a", INFO(0x1f4701, 0, 64 * 1024,  64, SECT_4K) },
	{ "at25df641",  INFO(0x1f4800, 0, 64 * 1024, 128, SECT_4K) },

	{ "at26f004",   INFO(0x1f0400, 0, 64 * 1024,  8, SECT_4K) },
	{ "at26df081a", INFO(0x1f4501, 0, 64 * 1024, 16, SECT_4K) },
	{ "at26df161a", INFO(0x1f4601, 0, 64 * 1024, 32, SECT_4K) },
	{ "at26df321",  INFO(0x1f4700, 0, 64 * 1024, 64, SECT_4K) },

	/* EON -- en25xxx */
	{ "en25f32", INFO(0x1c3116, 0, 64 * 1024,  64, SECT_4K) },
	{ "en25p32", INFO(0x1c2016, 0, 64 * 1024,  64, 0) },
	{ "en25q32b", INFO(0x1c3016, 0, 64 * 1024,  64, 0) },
	{ "en25p64", INFO(0x1c2017, 0, 64 * 1024, 128, 0) },

	/* Everspin */
	{ "mr25h256", CAT25_INFO(  32 * 1024, 1, 256, 2) },

	/* Intel/Numonyx -- xxxs33b */
	{ "160s33b",  INFO(0x898911, 0, 64 * 1024,  32, 0) },
	{ "320s33b",  INFO(0x898912, 0, 64 * 1024,  64, 0) },
	{ "640s33b",  INFO(0x898913, 0, 64 * 1024, 128, 0) },

	/* Macronix */
	{ "mx25l2005a",  INFO(0xc22012, 0, 64 * 1024,   4, SECT_4K) },
	{ "mx25l4005a",  INFO(0xc22013, 0, 64 * 1024,   8, SECT_4K) },
	{ "mx25l8005",   INFO(0xc22014, 0, 64 * 1024,  16, 0) },
	{ "mx25l1606e",  INFO(0xc22015, 0, 64 * 1024,  32, SECT_4K) },
	{ "mx25l3205d",  INFO(0xc22016, 0, 64 * 1024,  64, 0) },
	{ "mx25l6405d",  INFO(0xc22017, 0, 64 * 1024, 128, 0) },
	{ "mx25l12805d", INFO(0xc22018, 0, 64 * 1024, 256, 0) },
	{ "mx25l12855e", INFO(0xc22618, 0, 64 * 1024, 256, 0) },
	{ "mx25l25635e", INFO(0xc22019, 0, 64 * 1024, 512, 0) },
	{ "mx25l25655e", INFO(0xc22619, 0, 64 * 1024, 512, 0) },
	{ "mx66l51235f", INFO(0xc2201a, 0, 64 * 1024, 1024, 0) },

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a",  INFO(0x010212,      0,  64 * 1024,   8, 0) },
	{ "s25sl008a",  INFO(0x010213,      0,  64 * 1024,  16, 0) },
	{ "s25sl016a",  INFO(0x010214,      0,  64 * 1024,  32, 0) },
	{ "s25sl032a",  INFO(0x010215,      0,  64 * 1024,  64, 0) },
	{ "s25sl032p",  INFO(0x010215, 0x4d00,  64 * 1024,  64, SECT_4K) },
	{ "s25sl064a",  INFO(0x010216,      0,  64 * 1024, 128, 0) },
	{ "s25fl256s0", INFO(0x010219, 0x4d00, 256 * 1024, 128, 0) },
	{ "s25fl256s1", INFO(0x010219, 0x4d01,  64 * 1024, 512, 0) },
	{ "s25fl512s",  INFO(0x010220, 0x4d00, 256 * 1024, 256, 0) },
	{ "s70fl01gs",  INFO(0x010221, 0x4d00, 256 * 1024, 256, 0) },
	{ "s25sl12800", INFO(0x012018, 0x0300, 256 * 1024,  64, 0) },
	{ "s25sl12801", INFO(0x012018, 0x0301,  64 * 1024, 256, 0) },
	{ "s25fl129p0", INFO(0x012018, 0x4d00, 256 * 1024,  64, 0) },
	{ "s25fl129p1", INFO(0x012018, 0x4d01,  64 * 1024, 256, 0) },
	{ "s25fl016k",  INFO(0xef4015,      0,  64 * 1024,  32, SECT_4K) },
	{ "s25fl064k",  INFO(0xef4017,      0,  64 * 1024, 128, SECT_4K) },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", INFO(0xbf258d, 0, 64 * 1024,  8, SECT_4K) },
	{ "sst25vf080b", INFO(0xbf258e, 0, 64 * 1024, 16, SECT_4K) },
	{ "sst25vf016b", INFO(0xbf2541, 0, 64 * 1024, 32, SECT_4K) },
	{ "sst25vf032b", INFO(0xbf254a, 0, 64 * 1024, 64, SECT_4K) },
	{ "sst25wf512",  INFO(0xbf2501, 0, 64 * 1024,  1, SECT_4K) },
	{ "sst25wf010",  INFO(0xbf2502, 0, 64 * 1024,  2, SECT_4K) },
	{ "sst25wf020",  INFO(0xbf2503, 0, 64 * 1024,  4, SECT_4K) },
	{ "sst25wf040",  INFO(0xbf2504, 0, 64 * 1024,  8, SECT_4K) },
	{ "sst26vf016",  INFO(0xbf2601, 0, 64 * 1024, 32, SECT_4K) },
	{ "sst26vf032",  INFO(0xbf2602, 0, 64 * 1024, 64, SECT_4K) },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  INFO(0x202010,  0,  32 * 1024,   2, 0) },
	{ "m25p10",  INFO(0x202011,  0,  32 * 1024,   4, 0) },
	{ "m25p20",  INFO(0x202012,  0,  64 * 1024,   4, 0) },
	{ "m25p40",  INFO(0x202013,  0,  64 * 1024,   8, 0) },
	{ "m25p80",  INFO(0x202014,  0,  64 * 1024,  16, 0) },
	{ "m25p16",  INFO(0x202015,  0,  64 * 1024,  32, 0) },
	{ "m25p32",  INFO(0x202016,  0,  64 * 1024,  64, 0) },
	{ "m25p64",  INFO(0x202017,  0,  64 * 1024, 128, 0) },
	{ "m25p128", INFO(0x202018,  0, 256 * 1024,  64, 0) },

	{ "m25p05-nonjedec",  INFO(0, 0,  32 * 1024,   2, 0) },
	{ "m25p10-nonjedec",  INFO(0, 0,  32 * 1024,   4, 0) },
	{ "m25p20-nonjedec",  INFO(0, 0,  64 * 1024,   4, 0) },
	{ "m25p40-nonjedec",  INFO(0, 0,  64 * 1024,   8, 0) },
	{ "m25p80-nonjedec",  INFO(0, 0,  64 * 1024,  16, 0) },
	{ "m25p16-nonjedec",  INFO(0, 0,  64 * 1024,  32, 0) },
	{ "m25p32-nonjedec",  INFO(0, 0,  64 * 1024,  64, 0) },
	{ "m25p64-nonjedec",  INFO(0, 0,  64 * 1024, 128, 0) },
	{ "m25p128-nonjedec", INFO(0, 0, 256 * 1024,  64, 0) },

	{ "m45pe10", INFO(0x204011,  0, 64 * 1024,    2, 0) },
	{ "m45pe80", INFO(0x204014,  0, 64 * 1024,   16, 0) },
	{ "m45pe16", INFO(0x204015,  0, 64 * 1024,   32, 0) },

	{ "m25pe80", INFO(0x208014,  0, 64 * 1024, 16,       0) },
	{ "m25pe16", INFO(0x208015,  0, 64 * 1024, 32, SECT_4K) },

	{ "m25px32",    INFO(0x207116,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px32-s0", INFO(0x207316,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px32-s1", INFO(0x206316,  0, 64 * 1024, 64, SECT_4K) },
	{ "m25px64",    INFO(0x207117,  0, 64 * 1024, 128, 0) },

	/* Micron */
	{ "n25q032", INFO(0x20bb16, 0, 64 * 1024, 64, SECT_4K) },
	{ "n25q256", INFO(0x20ba19, 0, 64 * 1024, 512, 0) },
	{ "n25q512", INFO(0x20ba20, 0, 64 * 1024, 1024, M25P_FSR_POLL) },
	{ "n25q00",  INFO(0x20ba21, 0, 64 * 1024, 2048, M25P_FSR_POLL) },

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", INFO(0xef3011, 0, 64 * 1024,  2,  SECT_4K) },
	{ "w25x20", INFO(0xef3012, 0, 64 * 1024,  4,  SECT_4K) },
	{ "w25x40", INFO(0xef3013, 0, 64 * 1024,  8,  SECT_4K) },
	{ "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K) },
	{ "w25x16", INFO(0xef3015, 0, 64 * 1024,  32, SECT_4K) },
	{ "w25x32", INFO(0xef3016, 0, 64 * 1024,  64, SECT_4K) },
	{ "w25q32", INFO(0xef4016, 0, 64 * 1024,  64, SECT_4K) },
	{ "w25x64", INFO(0xef3017, 0, 64 * 1024, 128, SECT_4K) },
	{ "w25q64", INFO(0xef4017, 0, 64 * 1024, 128, SECT_4K) },
	{ "w25q80", INFO(0xef5014, 0, 64 * 1024,  16, SECT_4K) },
	{ "w25q256", INFO(0xef4019, 0, 64 * 1024, 512, 0) },

	/* Catalyst / On Semiconductor -- non-JEDEC */
	{ "cat25c11", CAT25_INFO(  16, 8, 16, 1) },
	{ "cat25c03", CAT25_INFO(  32, 8, 16, 2) },
	{ "cat25c09", CAT25_INFO( 128, 8, 32, 2) },
	{ "cat25c17", CAT25_INFO( 256, 8, 32, 2) },
	{ "cat25128", CAT25_INFO(2048, 8, 64, 2) },
	{ },
};
MODULE_DEVICE_TABLE(spi, m25p_ids);

static const struct spi_device_id *__devinit jedec_probe(struct spi_device *spi)
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[5];
	u32			jedec;
	u16                     ext_jedec;
	struct flash_info	*info;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	tmp = spi_write_then_read(spi, &code, 1, id, 5);
	if (tmp < 0) {
		pr_debug("%s: error %d reading JEDEC ID\n",
				dev_name(&spi->dev), tmp);
		return ERR_PTR(tmp);
	}
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	ext_jedec = id[3] << 8 | id[4];

	for (tmp = 0; tmp < ARRAY_SIZE(m25p_ids) - 1; tmp++) {
		info = (void *)m25p_ids[tmp].driver_data;
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return &m25p_ids[tmp];
		}
	}
	dev_err(&spi->dev, "unrecognized JEDEC id %06x\n", jedec);
	return ERR_PTR(-ENODEV);
}

struct m25p			*tpFlashdev = NULL;

static u8 flash_uid[8] = {0};

int read_uid(u8 *val)
{
	u8 code[5];

	code[0] = 0x4b;
	code[1] = 0;
	code[2] = 0;
	code[3] = 0;
	code[4] = 0;

	if (tpFlashdev == NULL)
	{
		return -1;
	}

	if(val == NULL)
		return -1;
	return spi_write_then_read(tpFlashdev->spi, code, 5, val, 8);
}

static struct sock *nl_sk = NULL;  

static void flash_api_send(int targetPid)
{
	int ret = 0;
    struct sk_buff *skb;  
    struct nlmsghdr *nlh;
	int len = NLMSG_SPACE(32);
	unsigned char code[8] = {0};
	char* data = NULL;
	/*printk(KERN_ERR "flash return msg\r\n");*/

	//read_uid(code);
	memcpy(code, flash_uid, sizeof(code));

	skb = alloc_skb(len, GFP_ATOMIC);	
	if(!skb)	
	{  
		printk(KERN_ERR "my_net_link:alloc_skb_1 error\n");  
	}  

	nlh = nlmsg_put(skb, 0,0,0, len, 0);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_len = NLMSG_LENGTH(len);
    nlh->nlmsg_pid = 0;
    nlh->nlmsg_type = 0;
	data = NLMSG_DATA(nlh);
	memset(data, 0, sizeof(code));
	memmove(data, code, sizeof(code));
	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).dst_group = 0;

	/*{
		int i;
		printk("[kernel]target pid %d UID \r\n", targetPid);
		for(i = 0; i < 8; i++)
		{
			printk("%x ", *(data+i));
		}
	}
	printk("\r\n");*/
	
	ret = netlink_unicast(nl_sk, skb, targetPid, MSG_DONTWAIT);

	if (ret < 0)
	{
		printk(KERN_ERR"kernel to user fail %d\r\n", ret);
	}
	
	return;
}

static void flash_api_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;   /* netlink header */
	int msgLen = 0;
	char *data;
	int pid;

    nlh = (struct nlmsghdr *)skb->data;
    msgLen = skb->len;
	pid = nlh->nlmsg_pid;
	
	/*printk(KERN_ERR "flash rec msg from proc %d\r\n", pid);*/

	while (NLMSG_OK(nlh, msgLen))
	{
		data = NLMSG_DATA(nlh);
		memmove(&pid, data, sizeof(pid));
		flash_api_send(pid);
		nlh = NLMSG_NEXT(nlh, msgLen);
	}
	
	return;
}

static int __devexit m25p_remove(struct spi_device *spi)
{
	struct m25p	*flash = dev_get_drvdata(&spi->dev);
	int		status;

	/* Clean up MTD stuff. */
	status = mtd_device_unregister(&flash->mtd);
	if (status == 0) {
		kfree(flash->command);
		kfree(flash);
	}
	return 0;
}

int tp_flash_init(void)
{
    struct netlink_kernel_cfg cfg = {
		.input	= flash_api_rcv,
        .groups = 0,
	};
	
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TP_FLASH_DEV, THIS_MODULE, &cfg);  
  
    if(!nl_sk){  
        printk(KERN_ERR "my_net_link: create netlink socket error.\n");  
        return 1;  
    } 
  
    printk(KERN_ERR"flash api: create netlink socket ok.\n");  
  
    return 0;
}

#include <generated/autoconf.h>
#ifdef CONFIG_TPLINK_MTD_RAM_DEVICE

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device(struct mtd_info *mtd);


static int mtdram_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	memset((char *)mtd->priv + instr->addr, 0xff, instr->len);
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);
	return 0;
}

static int mtdram_point(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, void **virt, resource_size_t *phys)
{
	*virt = mtd->priv + from;
	*retlen = len;
	return 0;
}

static int mtdram_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
	return 0;
}

static unsigned long mtdram_get_unmapped_area(struct mtd_info *mtd,
					   unsigned long len,
					   unsigned long offset,
					   unsigned long flags)
{
	return (unsigned long) mtd->priv + offset;
}

static int mtdram_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	memcpy(buf, mtd->priv + from, len);
	*retlen = len;
	return 0;
}

static int mtdram_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	memcpy((char *)mtd->priv + to, buf, len);
	*retlen = len;
	return 0;
}


/*!
 *\fn		static int mtdram_setup( struct mtd_info* master)
 *\brief		
 *
 *\param[in]	N/A
 *\param[out]	N/A
 *
 *\return		
 *
 *\note 		added by zengjunjun
*/
static int mtdram_setup( struct mtd_info* master)
{
	struct mtd_info* mtd;
	extern int mtd_result;
	unsigned int size = 0;
	unsigned int mtd_ram_addr = 0;	

	if ( -1 == mtd_result )
	{
	    printk(KERN_ERR "%s: hasn't found %s on ram.\r\n", __func__,CONFIG_TPLINK_ROOFS_MTD_NAME);
	    return -1;
	}

	mtd_ram_addr = (unsigned long )__va(CONFIG_TPLINK_ROOFS_LOAD_ADDR);

	mtd = get_mtd_device_nm(CONFIG_TPLINK_ROOFS_MTD_NAME);

	if (NULL != mtd && ERR_PTR(-ENODEV) != mtd)
	{
		printk(KERN_NOTICE"%s: get mtd ok,mtdindex=%d\r\n", __func__,mtd->index);

		size = mtd->size;	
		printk(KERN_NOTICE "mtdsize=%d,reserved size=%d\n",size ,CONFIG_TPLINK_MTD_RESV_RAM_SIZE_KB*1024);
		if( size > (CONFIG_TPLINK_MTD_RESV_RAM_SIZE_KB*1024))
		{
			printk(KERN_ERR"%s: hasn't reserved enough memeory for %s\r\n", 
				__func__,CONFIG_TPLINK_ROOFS_MTD_NAME);
			return -ENOMEM;
		}
		
		put_mtd_device(mtd);
		del_mtd_device(mtd);
	}
	else
	{
		return -ENODEV;
	}

	printk(KERN_ERR "%s: malloc mtd\r\n", __func__);
	mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);

	memset(mtd, 0, sizeof(*mtd));

	mtd->name      = CONFIG_TPLINK_ROOFS_MTD_NAME;
	mtd->type      = MTD_RAM;
	mtd->flags     = MTD_CAP_NORFLASH;
	mtd->size      = size; 
	mtd->writesize = master->writesize;
	mtd->writebufsize = master->writebufsize; 
	mtd->erasesize = master->erasesize;
	mtd->priv      = (void*)mtd_ram_addr;
	mtd->owner = THIS_MODULE;
	mtd->_erase = mtdram_erase;
	mtd->_point = mtdram_point;
	mtd->_unpoint = mtdram_unpoint;
	mtd->_get_unmapped_area = mtdram_get_unmapped_area;
	mtd->_read = mtdram_read;
	mtd->_write = mtdram_write;

	if (add_mtd_device(mtd))
	{
	    printk(KERN_ERR"%s: add_mtd_device return error!\r\n", __func__);
	    return -EIO;
	}

	printk(KERN_NOTICE "%s: booting %s fs @%x/%x \r\n", 
	   __func__,CONFIG_TPLINK_ROOFS_MTD_NAME, mtd_ram_addr, size);

	return 0;
}

#endif /* defined(CONFIG_TPLINK_MTD_RAM_DEVICE) */


/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit m25p_probe(struct spi_device *spi)
{
	const struct spi_device_id	*id = spi_get_device_id(spi);
	struct flash_platform_data	*data;
	struct m25p			*flash;
	struct flash_info		*info;
	unsigned			i;
	struct mtd_part_parser_data	ppdata;

#ifdef CONFIG_MTD_OF_PARTS
	if (!of_device_is_available(spi->dev.of_node))
		return -ENODEV;
#endif

	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
	data = spi->dev.platform_data;
	if (data && data->type) {
		const struct spi_device_id *plat_id;

		for (i = 0; i < ARRAY_SIZE(m25p_ids) - 1; i++) {
			plat_id = &m25p_ids[i];
			if (strcmp(data->type, plat_id->name))
				continue;
			break;
		}

		if (i < ARRAY_SIZE(m25p_ids) - 1)
			id = plat_id;
		else
			dev_warn(&spi->dev, "unrecognized id %s\n", data->type);
	}

	info = (void *)id->driver_data;

	if (info->jedec_id) {
		const struct spi_device_id *jid;

		jid = jedec_probe(spi);
		if (IS_ERR(jid)) {
			return PTR_ERR(jid);
		} else if (jid != id) {
			/*
			 * JEDEC knows better, so overwrite platform ID. We
			 * can't trust partitions any longer, but we'll let
			 * mtd apply them anyway, since some partitions may be
			 * marked read-only, and we don't want to lose that
			 * information, even if it's not 100% accurate.
			 */
			dev_warn(&spi->dev, "found %s, expected %s\n",
				 jid->name, id->name);
			id = jid;
			info = (void *)jid->driver_data;
		}
	}

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return -ENOMEM;
	flash->command = kmalloc(MAX_CMD_SIZE + FAST_READ_DUMMY_BYTE, GFP_KERNEL);
	if (!flash->command) {
		kfree(flash);
		return -ENOMEM;
	}

	flash->spi = spi;
	mutex_init(&flash->lock);
	dev_set_drvdata(&spi->dev, flash);

	/*
	 * Atmel, SST and Intel/Numonyx serial flash tend to power
	 * up with the software protection bits set
	 */

	if (JEDEC_MFR(info->jedec_id) == CFI_MFR_ATMEL ||
	    JEDEC_MFR(info->jedec_id) == CFI_MFR_INTEL ||
	    JEDEC_MFR(info->jedec_id) == CFI_MFR_SST) {
		write_enable(flash);
		write_sr(flash, 0);
	}

	if (data && data->name)
		flash->mtd.name = data->name;
	else
		flash->mtd.name = dev_name(&spi->dev);

	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;
    if (info->jedec_id == JEDEC_ID_W25Q256FV)
    {
	    flash->mtd._erase = w25q256fv_erase_tp;
	    flash->mtd._read = w25q256fv_read_tp;
    }
    else
    {
	    flash->mtd._read = m25p80_read;
	    flash->mtd._erase = m25p80_erase;
    }

	/* sst flash chips use AAI word program */
	if (JEDEC_MFR(info->jedec_id) == CFI_MFR_SST)
		flash->mtd._write = sst_write;
    if (info->jedec_id == JEDEC_ID_W25Q256FV)
	    flash->mtd._write = w25q256fv_write_tp;
    else
	    flash->mtd._write = m25p80_write;

	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	if (info->flags & M25P_NO_ERASE)
		flash->mtd.flags |= MTD_NO_ERASE;

	/* Some Micron chips require polling for Flag Status Register for writing */
	flash->flag_status_register = 0;
	if (info->flags & M25P_FSR_POLL) {
		flash->flag_status_register = 1;
	}

	ppdata.of_node = spi->dev.of_node;
	flash->mtd.dev.parent = &spi->dev;
	flash->page_size = info->page_size;
	flash->mtd.writebufsize = flash->page_size;
	flash->jedec_id = info->jedec_id;

    if (info->jedec_id == JEDEC_ID_W25Q256FV)
    {
        info->addr_width = 3;
        flash->addr_width = 3;
    }else if (info->addr_width)
		flash->addr_width = info->addr_width;
	else {
#ifndef CONFIG_M25PXX_STAY_IN_3BYTE_MODE
		/* enable 4-byte addressing if the device exceeds 16MiB */
		if (flash->mtd.size > 0x1000000) {
			flash->addr_width = 4;
			set_4byte(flash, 1);
		} else
#endif /* !CONFIG_M25PXX_STAY_IN_3BYTE_MODE */
			flash->addr_width = 3;
	}

	dev_info(&spi->dev, "%s (%lld Kbytes)\n", id->name,
			(long long)flash->mtd.size >> 10);


	pr_debug("mtd .name = %s, .size = 0x%llx (%lldMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		(long long)flash->mtd.size, (long long)(flash->mtd.size >> 20),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			pr_debug("mtd.eraseregions[%d] = { .offset = 0x%llx, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (long long)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);

	tpFlashdev = flash;
	read_uid(flash_uid);
	tp_flash_init();
	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
#ifdef CONFIG_TPLINK_MTD_RAM_DEVICE 
	if(1)
	{
		int rv = mtd_device_parse_register(&flash->mtd, NULL, &ppdata,
			data ? data->parts : NULL,
			data ? data->nr_parts : 0);
		if( rv != 0 )
			return rv;
		
		printk(KERN_NOTICE "%s: call mtdram_setup\r\n", __func__);
	    	rv = mtdram_setup(&flash->mtd);
		return rv;
	}

#else
	return mtd_device_parse_register(&flash->mtd, NULL, &ppdata,
			data ? data->parts : NULL,
			data ? data->nr_parts : 0);
#endif /*defined(CONFIG_TPLINK_MTD_RAM_DEVICE)*/
}


static struct spi_driver m25p80_driver = {
	.driver = {
		.name	= "m25p80",
		.owner	= THIS_MODULE,
	},
	.id_table	= m25p_ids,
	.probe	= m25p_probe,
	.remove	= __devexit_p(m25p_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

module_spi_driver(m25p80_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("MTD SPI driver for ST M25Pxx flash chips");