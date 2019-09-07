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


#ifndef __SOCREGS_H
#define __SOCREGS_H

#define ChipcommonA_ChipID 0x18000000
#define ChipcommonB_PWMCTL 0x18031000
#define ChipcommonB_WDT_WDOGLOAD 0x18039000
#define ChipcommonB_GP_DATA_IN 0x18030000
#define ChipcommonB_GP_AUX_SEL_BASE 0x028
#define ChipcommonB_SMBus_Config 0x18038000
#define QSPI_mspi_SPCR0_LSB 0x18027200
#define QSPI_mspi_DISABLE_FLUSH_GEN 0x18027384
#define QSPI_bspi_registers_REVISION_ID 0x18027000
#define QSPI_bspi_registers_BSPI_PIO_DATA 0x1802704c
#define QSPI_raf_START_ADDR 0x18027100
#define QSPI_raf_interrupt_LR_fullness_reached 0x180273a0
#define QSPI_mspi_interrupt_MSPI_halt_set_transaction_done 0x180273b8
#define QSPI_IDM_IDM_IO_CONTROL_DIRECT 0x1811c408
#define QSPI_raf_CURR_ADDR 0x18027120
#define CRU_control 0x1803e000
#define GMAC0_DEVCONTROL 0x18022000
#define GMAC1_DEVCONTROL 0x18023000
#define ChipcommonA_GPIOEvent_BASE 0x078
#define ChipcommonA_GPIOInput_BASE 0x060
#define ChipcommonB_GP_INT_CLR_BASE 0x024
#define ChipcommonA_GPIOEventIntMask_BASE 0x07c
#define ChipcommonA_GPIOInput_BASE 0x060
#define ChipcommonB_GP_INT_MSK_BASE 0x018
#define ChipcommonA_GPIOIntMask_BASE 0x074
#define ChipcommonB_GP_INT_MSK_BASE 0x018
#define ChipcommonA_GPIOEventIntMask_BASE 0x07c
#define ChipcommonB_GP_INT_MSTAT_BASE 0x020
#define ChipcommonA_GPIOEventIntPolarity_BASE 0x084
#define ChipcommonA_IntStatus_BASE 0x020
#define ChipcommonA_GPIOIntPolarity_BASE 0x070
#define ChipcommonA_IntStatus_BASE 0x020
#define ChipcommonB_GP_INT_DE_BASE 0x010
#define ChipcommonB_GP_INT_EDGE_BASE 0x014
#define ChipcommonB_GP_INT_TYPE_BASE 0x00c
#define ChipcommonA_GPIOIntPolarity_BASE 0x070
#define ChipcommonB_GP_AUX_SEL_BASE 0x028
#define ChipcommonB_GP_PAD_RES_BASE 0x034
#define ChipcommonB_GP_RES_EN_BASE 0x038
#define ChipcommonA_ChipID 0x18000000
#define DMAC_pl330_DS 0x18020000
#define ChipcommonA_GPIOInput 0x18000060
#define ChipcommonB_GP_DATA_IN 0x18030000
#define PAXB_0_CLK_CONTROL 0x18012000
#define PAXB_0_CONFIG_IND_ADDR_BASE 0x120
#define ChipcommonB_MII_Management_Control 0x18032000
#define NAND_nand_flash_REVISION 0x18026000
#define NAND_direct_read_rd_miss 0x18026f00
#define NAND_IDM_IDM_IO_CONTROL_DIRECT 0x1811b408
#define ChipcommonB_PWM_PERIOD_COUNT0_BASE 0x004
#define ChipcommonB_PWM_PRESCALE_BASE 0x024
#define ChipcommonB_PWM_PERIOD_COUNT1_BASE 0x00c
#define ChipcommonB_PWM_PERIOD_COUNT2_BASE 0x014
#define ChipcommonB_PWM_PERIOD_COUNT3_BASE 0x01c
#define ChipcommonB_PWM_DUTYHI_COUNT0_BASE 0x008
#define ChipcommonB_PWM_DUTYHI_COUNT1_BASE 0x010
#define ChipcommonB_PWM_DUTYHI_COUNT2_BASE 0x018
#define ChipcommonB_PWM_DUTYHI_COUNT3_BASE 0x020
#define ChipcommonB_PWMCTL_BASE 0x000
#define ChipcommonB_rng_CTRL 0x18033000
#define USB2_IDM_IDM_IO_CONTROL_DIRECT 0x18115408
#define DMU_CRU_RESET_BASE 0x200
#define ChipcommonB_SMBus1_SMBus_Config 0x1803b000

#define DMU_PCU_IPROC_STRAPS_CAPTURED_BASE 0x028
#define DMU_PCU_IPROC_STRAPS_CAPTURED__strap_boot_dev_R 9
#define DMU_PCU_IPROC_STRAPS_CAPTURED__strap_nand_type_R 5
#define DMU_PCU_IPROC_STRAPS_CAPTURED__strap_nand_page_R 3
#define ChipcommonA_IntMask_BASE 0x024
#define DMU_PCU_CRU_RESET_REASON 0x1803f014
#define DMU_PCU_CRU_RESET_REASON__watchdog_reset 0
#define ChipcommonA_GPIOOut 0x18000064
#define ChipcommonA_GPIOOutEn 0x18000068
#define AMAC_IDM0_IO_CONTROL_DIRECT 0x18110408
#define AMAC_IDM0_IO_CONTROL_DIRECT__CLK_250_SEL 6
#define AMAC_IDM0_IO_CONTROL_DIRECT__DIRECT_GMII_MODE 5
#define AMAC_IDM0_IO_CONTROL_DIRECT__DEST_SYNC_MODE_EN 3
#define AMAC_IDM1_IO_CONTROL_DIRECT 0x18111408
#define AMAC_IDM1_IO_CONTROL_DIRECT__CLK_250_SEL 6
#define AMAC_IDM1_IO_CONTROL_DIRECT__DIRECT_GMII_MODE 5
#define AMAC_IDM1_IO_CONTROL_DIRECT__DEST_SYNC_MODE_EN 3
#define IPROC_WRAP_USBPHY_CTRL 0x1803fc20
#define IPROC_WRAP_MISC_CONTROL__UNICORE_SERDES_CTRL_SEL 1
#define IPROC_WRAP_MISC_CONTROL__IPROC_MDIO_SEL 3
#define IPROC_WRAP_MISC_CONTROL 0x1803fc24
#define IPROC_WRAP_MISC_CONTROL__UNICORE_SERDES_MDIO_SEL 2
#define IHOST_PROC_CLK_PLLARMA								0x19000c00
#define IHOST_PROC_CLK_PLLARMB								0x19000c04
#define IHOST_PROC_CLK_PLLARMA__pllarm_pdiv_R				24
#define IHOST_PROC_CLK_POLICY_FREQ							0x19000008
#define DMU_CRU_RESET										0x1803f200
#define ChipcommonA_CoreCtrl								0x18000008
#define ChipcommonA_CoreCtrl__UARTClkOvr					0
#define APBW_IDM_IDM_IO_CONTROL_DIRECT						0x18131408
#define APBW_IDM_IDM_IO_CONTROL_DIRECT__UARTClkSel			17
#define ChipcommonA_ClkDiv									0x180000a4
#define ChipcommonA_ClkDiv__UartClkDiv_R					0
#define ChipcommonA_ClkDiv__UartClkDiv_WIDTH				8
#define CRU_ihost_pwrdwn_en									0x1803e004
#define CRU_ihost_pwrdwn_en__logic_pwrdown_neon1			13
#define CRU_ihost_pwrdwn_en__logic_pwrdown_neon1_WIDTH		1
#define CRU_ihost_pwrdwn_en__logic_clamp_on_neon1			12
#define CRU_ihost_pwrdwn_en__logic_clamp_on_neon1_WIDTH		1
#define CRU_ihost_pwrdwn_en__logic_pwrdown_neon0			5
#define CRU_ihost_pwrdwn_en__logic_pwrdown_neon0_WIDTH		1
#define CRU_ihost_pwrdwn_en__logic_clamp_on_neon0			4
#define CRU_ihost_pwrdwn_en__logic_clamp_on_neon0_WIDTH		1
#define CRU_ihost_pwrdwn_en__logic_clamp_on_cpu1			8
#define CRU_ihost_pwrdwn_en__ram_clamp_on_cpu1				10
#define CRU_ihost_pwrdwn_status								0x1803e008
#define CRU_cpu0_powerdown									0x1803e00c
#define CRU_cpu0_powerdown__start_cpu0_powerdown_seq		31
#define CRU_ihost_pwrdwn_en__logic_pwrdown_cpu1				9
#define CRU_ihost_pwrdwn_en__ram_pwrdown_cpu1				11
#define IHOST_PROC_CLK_WR_ACCESS							0x19000000
#define IHOST_PROC_CLK_POLICY_FREQ							0x19000008
#define IHOST_PROC_CLK_POLICY_FREQ__priv_access_mode		31
#define IHOST_PROC_CLK_POLICY_FREQ__policy3_freq_R			24
#define IHOST_PROC_CLK_POLICY_FREQ__policy2_freq_R			16
#define IHOST_PROC_CLK_POLICY_FREQ__policy1_freq_R			8
#define IHOST_PROC_CLK_POLICY_FREQ__policy0_freq_R			0
#define IHOST_PROC_CLK_POLICY_CTL							0x1900000c
#define IHOST_PROC_CLK_PLLARMB__pllarm_ndiv_frac_R			0
#define IHOST_PROC_CLK_PLLARMB__pllarm_ndiv_frac_WIDTH		20
#define IHOST_PROC_CLK_PLLARMA__pllarm_ndiv_int_R			8
#define IHOST_PROC_CLK_PLLARMA__pllarm_soft_resetb			0
#define IHOST_PROC_CLK_CORE0_CLKGATE						0x19000200
#define IHOST_PROC_CLK_CORE1_CLKGATE						0x19000204
#define IHOST_PROC_CLK_ARM_SWITCH_CLKGATE					0x19000210
#define IHOST_PROC_CLK_ARM_PERIPH_CLKGATE					0x19000300
#define IHOST_PROC_CLK_APB0_CLKGATE							0x19000400
#define IHOST_PROC_CLK_POLICY_CTL__GO						0
#define IHOST_PROC_CLK_POLICY_CTL__GO_AC					1
#define IHOST_PROC_CLK_PLLARMA__pllarm_lock					28
#define IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb		1
#define DDR_BistStatuses									0x18010460
#define DDR_BistStatuses__BistFinished						0
#define DDR_BistErrorOccurred								0x1801046c
#define PAXB_0_CONFIG_IND_ADDR								0x18012120
#define PAXB_0_CONFIG_IND_DATA								0x18012124
#define DDR_PHY_CONTROL_REGS_STRAP_CONTROL					0x180108b0
#define DDR_PHY_CONTROL_REGS_STRAP_CONTROL2					0x180108b4
#define ROM_S0_IDM_IO_STATUS								0x1811a500
#define DDR_S1_IDM_IO_STATUS								0x18109500
#define DDR_S1_IDM_IO_CONTROL_DIRECT						0x18109408
#define DDR_PHY_CONTROL_REGS_PLL_DIVIDERS					0x1801081c
#define DDR_PHY_CONTROL_REGS_PLL_CONFIG						0x18010814
#define DDR_PHY_CONTROL_REGS_PLL_STATUS						0x18010810
#define DDR_DENALI_CTL_22									0x18010058
#define DDR_DENALI_CTL_00									0x18010000
#define DDR_DENALI_CTL_89									0x18010164
#define DDR_DENALI_CTL_213									0x18010354
#define DDR_DENALI_CTL_67									0x1801010c
#define DDR_DENALI_CTL_66									0x18010108
#define DDR_BistConfig										0x18010400
#define DDR_BistGeneralConfigurations						0x18010408
#define DDR_BistConfigurations								0x1801040c
#define DDR_BistConfigurations__ConsAddr8Banks				21
#define DDR_BistConfigurations__ReadWeight_R				8
#define DDR_BistConfigurations__WriteWeight_R				0
#define DDR_BistStartAddress								0x18010414
#define DDR_BistEndAddress									0x18010418
#define DDR_BistEndAddress__BistEndAddress_WIDTH			26
#define DDR_BistNumberOfActions								0x18010410
#define DDR_BistPatternWord0								0x1801043c
#define DDR_BistPatternWord1								0x18010438
#define DDR_BistPatternWord2								0x18010434
#define DDR_BistPatternWord3								0x18010430
#define DDR_BistPatternWord4								0x1801042c
#define DDR_BistPatternWord5								0x18010428
#define DDR_BistPatternWord6								0x18010424
#define DDR_BistPatternWord7								0x18010420
#define DDR_BistConfigurations__IndWrRdAddrMode				19
#define DDR_BistConfigurations__BistEn						25
#define DDR_S1_IDM_RESET_CONTROL							0x18109800
#define DDR_S2_IDM_RESET_CONTROL							0x1810a800
#define DDR_PHY_CONTROL_REGS_REVISION						0x18010800
#define DDR_BistConfig										0x18010400
#define DDR_BistConfig__axi_port_sel						1
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE_RD_EN			0x18010a00
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_RD_EN				0x18010e00
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_W				0x18010a04
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_W					0x18010e04
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_R_P			0x18010a08
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_R_P				0x18010e08
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_R_N			0x18010a0c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_R_N				0x18010e0c
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT0_W			0x18010a10
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_W				0x18010e10
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT1_W			0x18010a14
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_W				0x18010e14
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT2_W			0x18010a18
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_W				0x18010e18
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT3_W			0x18010a1c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_W				0x18010e1c
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_DM_W			0x18010a30
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_DM_W				0x18010e30
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT0_R_P		0x18010a34
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_R_P			0x18010e34
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT0_R_N		0x18010a38
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_R_N			0x18010e38
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT1_R_P		0x18010a3c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_R_P			0x18010e3c
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT1_R_N		0x18010a40
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_R_N			0x18010e40
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT2_R_P		0x18010a44
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_R_P			0x18010e44
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT2_R_N		0x18010a48
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_R_N			0x18010e48
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT3_R_P		0x18010a4c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_R_P			0x18010e4c
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT3_R_N		0x18010a50
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_R_N			0x18010e50
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT_RD_EN		0x18010a74
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT_RD_EN			0x18010e74
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_R_P		0x18010b28
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_R_P			0x18010f28
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_R_N		0x18010b2c
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_R_N			0x18010f2c
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_BIT_R_P	0x18010b30
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_BIT_R_P		0x18010f30
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_BIT_R_N	0x18010b34
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_BIT_R_N		0x18010f34
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_W			0x18010b38
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_W				0x18010f38
#define DDR_PHY_WORD_LANE_0_DYN_VDL_OVRIDE_BYTE0_BIT_W		0x18010b3c
#define DDR_PHY_ECC_LANE_DYN_VDL_OVRIDE_BYTE_BIT_W			0x18010f3c
#define DDR_PHY_WORD_LANE_0_READ_DATA_DLY					0x18010b60
#define DDR_PHY_ECC_LANE_READ_DATA_DLY						0x18010f60
#define DDR_PHY_WORD_LANE_0_READ_CONTROL					0x18010b64
#define DDR_PHY_ECC_LANE_READ_CONTROL						0x18010f64
#define DDR_PHY_WORD_LANE_0_IDLE_PAD_CONTROL				0x18010ba0
#define DDR_PHY_ECC_LANE_IDLE_PAD_CONTROL					0x18010fa0
#define DDR_PHY_WORD_LANE_0_DRIVE_PAD_CTL					0x18010ba4
#define DDR_PHY_ECC_LANE_DRIVE_PAD_CTL						0x18010fa4
#define DDR_PHY_WORD_LANE_0_WR_PREAMBLE_MODE				0x18010bac
#define DDR_PHY_ECC_LANE_WR_PREAMBLE_MODE					0x18010fac
#define DDR_PHY_ECC_LANE_READ_FIFO_CLEAR					0x18010f94
#define DDR_PHY_CONTROL_REGS_ZQ_PVT_COMP_CTL				0x1801083c
#define DDR_PHY_CONTROL_REGS_VDL_CALIBRATE					0x18010848
#define DDR_PHY_CONTROL_REGS_VDL_CALIB_STATUS				0x1801084c
#define DDR_PHY_CONTROL_REGS_VDL_OVRIDE_BIT_CTL				0x18010834
#define DDR_DENALI_CTL_43									0x180100ac
#define DDR_PHY_WORD_LANE_1_READ_DATA_DLY					0x18010d60
#define DDR_PHY_WORD_LANE_0_WR_PREAMBLE_MODE				0x18010bac
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT4_W			0x18010a20
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT5_W			0x18010a24
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT6_W			0x18010a28
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE0_BIT7_W			0x18010a2c
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_W				0x18010aa4
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT0_W			0x18010ab0
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT1_W			0x18010ab4
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT2_W			0x18010ab8
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT3_W			0x18010abc
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT4_W			0x18010ac0
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT5_W			0x18010ac4
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT6_W			0x18010ac8
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_BIT7_W			0x18010acc
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_DM_W			0x18010ad0
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_R_P			0x18010aa8
#define DDR_PHY_WORD_LANE_0_VDL_OVRIDE_BYTE1_R_N			0x18010aac
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_W				0x18010c04
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT0_W			0x18010c10
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT1_W			0x18010c14
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT2_W			0x18010c18
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT3_W			0x18010c1c
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT4_W			0x18010c20
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT5_W			0x18010c24
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT6_W			0x18010c28
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_BIT7_W			0x18010c2c
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_DM_W			0x18010c30
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_W				0x18010ca4
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT0_W			0x18010cb0
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT1_W			0x18010cb4
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT2_W			0x18010cb8
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT3_W			0x18010cbc
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT4_W			0x18010cc0
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT5_W			0x18010cc4
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT6_W			0x18010cc8
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_BIT7_W			0x18010ccc
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_DM_W			0x18010cd0
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_R_P			0x18010c08
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE0_R_N			0x18010c0c
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_R_P			0x18010ca8
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE1_R_N			0x18010cac
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_RD_EN_BASE			0xe00
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_W_BASE				0xe04
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_R_P_BASE			0xe08
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_R_N_BASE			0xe0c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_W_BASE		0xe10
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_W_BASE		0xe14
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_W_BASE		0xe18
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_W_BASE		0xe1c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_DM_W_BASE			0xe20
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_R_P_BASE		0xe34
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT0_R_N_BASE		0xe38
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_R_P_BASE		0xe3c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT1_R_N_BASE		0xe40
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_R_P_BASE		0xe44
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT2_R_N_BASE		0xe48
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_R_P_BASE		0xe4c
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT3_R_N_BASE		0xe50
#define DDR_PHY_ECC_LANE_VDL_OVRIDE_BYTE_BIT_RD_EN_BASE		0xe74
#define DDR_PHY_ECC_LANE_READ_DATA_DLY_BASE					0xf60
#define DDR_PHY_ECC_LANE_READ_CONTROL_BASE					0xf64
#define DDR_PHY_ECC_LANE_IDLE_PAD_CONTROL_BASE				0xfa0
#define DDR_PHY_ECC_LANE_DRIVE_PAD_CTL_BASE					0xfa4
#define DDR_PHY_ECC_LANE_WR_PREAMBLE_MODE_BASE				0xfac		
#define DDR_PHY_WORD_LANE_1_WR_PREAMBLE_MODE				0x18010dac
#define DDR_PHY_WORD_LANE_1_VDL_OVRIDE_BYTE_RD_EN			0x18010c00
#define IPROC_DDR_PLL_CTRL_REGISTER_3						0x1803fc0c
#define IPROC_DDR_PLL_CTRL_REGISTER_3__NDIV_INT_R			0
#define IPROC_DDR_PLL_CTRL_REGISTER_3__NDIV_INT_WIDTH		10
#define IPROC_DDR_PLL_CTRL_REGISTER_4						0x1803fc10
#define IPROC_DDR_PLL_CTRL_REGISTER_4__CH0_MDIV_R			0
#define IPROC_DDR_PLL_CTRL_REGISTER_4__CH0_MDIV_WIDTH		8
#define IPROC_DDR_PLL_CTRL_REGISTER_5						0x1803fc14
#define IPROC_DDR_PLL_CTRL_REGISTER_5__IPROC_DDR_PLL_SW_OVWR 16
#define IPROC_DDR_PLL_CTRL_REGISTER_5__IPROC_DDR_PLL_POST_RESETB 12
#define IPROC_DDR_PLL_CTRL_REGISTER_5__IPROC_DDR_PLL_RESETB 11
#define IPROC_DDR_PLL_CTRL_REGISTER_5__CH1_MDIV_R			0
#define IPROC_DDR_PLL_CTRL_REGISTER_5__CH1_MDIV_WIDTH		8
#define IPROC_DDR_PLL_CTRL_REGISTER_4__CH0_MDIV_R			0
#define IPROC_DDR_PLL_CTRL_REGISTER_4__CH0_MDIV_WIDTH		8
#define IPROC_DDR_PLL_STATUS								0x1803fc18
#define IPROC_DDR_PLL_STATUS__IPROC_DDR_PLL_LOCK			12
#define NAND_nand_flash_INTFC_STATUS						0x18026014
#define NAND_ro_ctlr_ready									0x18026f10
#define NAND_nand_flash_CMD_ADDRESS							0x1802600c
#define NAND_nand_flash_CMD_EXT_ADDRESS						0x18026008
#define NAND_nand_flash_INIT_STATUS							0x18026144
#define NAND_nand_flash_FLASH_DEVICE_ID						0x18026194
#define NAND_nand_flash_ONFI_STATUS							0x18026148
#define NAND_nand_flash_CONFIG_CS1							0x18026064
#define NAND_nand_flash_CONFIG_CS0							0x18026054
#define NAND_nand_flash_ACC_CONTROL_CS1						0x18026060
#define NAND_nand_flash_ACC_CONTROL_CS0						0x18026050
#define NAND_nand_flash_FLASH_CACHE0						0x18026400
#define NAND_nand_flash_ECC_UNC_ADDR						0x18026118
#define NAND_nand_flash_CS_NAND_SELECT						0x18026018
#define NAND_nand_flash_SPARE_AREA_READ_OFS_0				0x18026200
#define NAND_nand_flash_SPARE_AREA_WRITE_OFS_0				0x18026280
#define NAND_nand_flash_FLASH_DEVICE_ID_EXT					0x18026198
#define NAND_nand_flash_CMD_START							0x18026004
#define NAND_IDM_IDM_RESET_CONTROL							0x1811b800
#define NAND_nand_flash_UNCORR_ERROR_COUNT					0x180260fc
#define DMU_PCU_IPROC_STRAPS_CAPTURED						0x1803f028
#define DDR_PHY_ECC_LANE_READ_FIFO_STATUS					0x18010f90
#define AMAC_IDM0_IDM_RESET_CONTROL							0x18110800
#define AMAC_IDM1_IDM_RESET_CONTROL							0x18111800
#define ChipcommonB_MII_Management_Control					0x18032000
#define ChipcommonB_MII_Management_Command_Data				0x18032004
#define ChipcommonB_MII_Management_Control__BSY				8
#define ChipcommonB_MII_Management_Control__PRE				7
#define ChipcommonB_MII_Management_Control__EXT				9
#define ChipcommonB_MII_Management_Command_Data__SB_R		30
#define ChipcommonB_MII_Management_Command_Data__PA_R		23
#define ChipcommonB_MII_Management_Command_Data__RA_R		18
#define ChipcommonB_MII_Management_Command_Data__TA_R		16
#define ChipcommonB_MII_Management_Command_Data__OP_R		28
#define CMIC_DEV_REV_ID										0x48010224

#endif /* __SOCREGS_H */
