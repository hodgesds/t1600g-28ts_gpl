/*
 * Copyright (C) 2013, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/device.h>
#include <mach/io_map.h>
#include <mach/reg_utils.h>
#include <plat/swreset_rec.h>

#define MAX_SWRESET_RECORD_COUNT        (4)

struct swreset_record {
    int position;
};

struct swreset_record records[MAX_SWRESET_RECORD_COUNT];
static unsigned int record_count;
static unsigned int record_width;
static DEFINE_SPINLOCK(swrr_lock); 

unsigned int
swreset_record_get_record_width(void)
{
    return record_width;
}

unsigned int
swreset_record_get_record_count(void)
{
    return record_count;
}

SWRR_HANDLE
swreset_record_register(const char *name)
{
    int i;
    int j;
    
    if (record_count == 0) {
        return NULL;
    }
    
    /* Use the name as a hash value to try to avoid race condition */
    j = 0;
    while(*name != 0) {
        j += *name;
        if (j > 0xff)
            j = (j & 0xff) + 1;
        name++;
    }

    for(i=0; i<record_count; i++, j++) {
        j = j % record_count;
        if (records[j].position == -1) {
            records[j].position = j;
            return (SWRR_HANDLE)&records[j];
        }
    }

    return NULL;
}

void
swreset_record_unregister(SWRR_HANDLE handle)
{
    struct swreset_record *swrr = (struct swreset_record *)handle;
    if (handle == NULL)
        return;
    if (swrr->position < 0 || swrr->position >= record_count)
        return;
    swrr->position = -1;
}

int
swreset_record_set(SWRR_HANDLE handle, int value)
{
    struct swreset_record *swrr = (struct swreset_record *)handle;
    void __iomem *reg;
    unsigned long rval;
    unsigned long flags;
    
    if (handle == NULL)
        return -1;
    if (swrr->position < 0 || swrr->position >= record_count)
        return -1;
    if (value & ~((1 << record_width) - 1))
        return -1;
        
    /* This requires atomic operation */
    spin_lock_irqsave(&swrr_lock, flags);
    
    /* CoStar specific; could require modification for newer chips */
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_CLKSET_KEY_OFFSET);
    writel(0xea68, reg); /* magic number */
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_GENPLL_CONTROL5_OFFSET);
    writel(readl(reg) | (1 << IPROC_DMU_GENPLL_CONTROL5__SEL_SW_SETTING), reg);
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_GENPLL_CONTROL7_OFFSET);
    rval = readl(reg) & ~(1 << (IPROC_DMU_GENPLL_CONTROL7__SW_RESET_REC + swrr->position));
    rval |= value << (IPROC_DMU_GENPLL_CONTROL7__SW_RESET_REC + swrr->position);
    writel(rval, reg);
    
    spin_unlock_irqrestore(&swrr_lock, flags);
    return 0;
}

int
swreset_record_get(SWRR_HANDLE handle, int *value)
{
    struct swreset_record *swrr = (struct swreset_record *)handle;
    void __iomem *reg;
    unsigned long flags;
    
    if (handle == NULL)
        return -1;
    if (swrr->position < 0 || swrr->position >= record_count)
        return -1;
    if (value == NULL)
        return 0;

    /* This requires atomic operation */
    spin_lock_irqsave(&swrr_lock, flags);
    
    /* CoStar specific; could require modification for newer chips */
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_CLKSET_KEY_OFFSET);
    writel(0xea68, reg); /* magic number */
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_GENPLL_CONTROL5_OFFSET);
    writel(readl(reg) | (1 << IPROC_DMU_GENPLL_CONTROL5__SEL_SW_SETTING), reg);
    reg = IOMEM(IPROC_DMU_BASE_VA + IPROC_DMU_GENPLL_CONTROL7_OFFSET);
    *value = readl(reg) >> IPROC_DMU_GENPLL_CONTROL7__SW_RESET_REC;
    *value = (*value >> swrr->position) & 1;
    
    spin_unlock_irqrestore(&swrr_lock, flags);
    return 0;
}

int __init
init_swreset_records(void)
{
    int i;
    for(i=0; i<MAX_SWRESET_RECORD_COUNT; i++) {
        records[i].position = -1;
    }

    /* Currently it's only supported on CoStar */
    if ((readl(IOMEM(IPROC_CCA_CORE_REG_VA)) & 0xffff) == 0xcf1a) {
        record_count = IPROC_DMU_GENPLL_CONTROL7__SW_RESET_REC_WIDTH;
        record_width = 1;   /* 1 bit per record */
    } else {
        record_count = 0;
        record_width = 0;
    }
    return 0;
}

early_initcall(init_swreset_records);
EXPORT_SYMBOL(swreset_record_get_record_count);
EXPORT_SYMBOL(swreset_record_get_record_width);
EXPORT_SYMBOL(swreset_record_register);
EXPORT_SYMBOL(swreset_record_unregister);
EXPORT_SYMBOL(swreset_record_set);
EXPORT_SYMBOL(swreset_record_get);
