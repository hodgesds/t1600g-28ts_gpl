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
#ifndef __PLAT_IPROC_IRQS_H
#define __PLAT_IPROC_IRQS_H

#define BCM_INT_PRIORITY_MAX  32  /* there are only 32 priority are supported */
#define BCM_INT_SPI_MAX       128 /* there are 128 shared peripheral interrupt*/
/*=====================================================================*/
/* Software Trigger Interrupt IDs                                      */
/*=====================================================================*/
#define BCM_INT_ID_STI0                 0
#define BCM_INT_ID_STI1                 1
#define BCM_INT_ID_STI2                 2
#define BCM_INT_ID_STI3                 3
#define BCM_INT_ID_STI4                 4
#define BCM_INT_ID_STI5                 5
#define BCM_INT_ID_STI6                 6
#define BCM_INT_ID_STI7                 7
#define BCM_INT_ID_STI8                 8
#define BCM_INT_ID_STI9                 9
#define BCM_INT_ID_STI10                10
#define BCM_INT_ID_STI11                11
#define BCM_INT_ID_STI12                12
#define BCM_INT_ID_STI13                13
#define BCM_INT_ID_STI14                14
#define BCM_INT_ID_STI15                15
#define BCM_INT_ID_STI_MAX              16 /* terminating ID */

/*=====================================================================*/
/* Private Peripheral Interrupt IDs                                    */
/*=====================================================================*/
#define BCM_INT_ID_PPI0                 ( 0 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI1                 ( 1 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI2                 ( 2 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI3                 ( 3 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI4                 ( 4 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI5                 ( 5 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI6                 ( 6 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI7                 ( 7 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI8                 ( 8 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI9                 ( 9 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI10                (10 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI11                (11 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI12                (12 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI13                (13 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI14                (14 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI15                (15 + BCM_INT_ID_STI_MAX)
#define BCM_INT_ID_PPI_MAX              (16 + BCM_INT_ID_STI_MAX) /* terminating ID */

/*=====================================================================*/
/* iHost Interrupt IDs                                                 */
/*=====================================================================*/
#define BCM_INT_ID_IHOST_L2CC           32
#define BCM_INT_ID_IHOST_PWRWDOG        33
#define BCM_INT_ID_IHOST_TRAP8          34
#define BCM_INT_ID_IHOST_TRAP1          35
#define BCM_INT_ID_IHOST_COMMTX         36
#define BCM_INT_ID_IHOST_COMMRX         38
#define BCM_INT_ID_IHOST_PMU            40
#define BCM_INT_ID_IHOST_CT             42
#define BCM_INT_ID_IHOST_DEFFLG_CPU0    44
#define BCM_INT_ID_IHOST_DEFFLG_CPU1    45
#define BCM_INT_ID_IHOST_MAX            46 /* terminating ID */
#if defined(CONFIG_MACH_HX4) || defined(CONFIG_MACH_KT2)
#define BCM_INT_ID_IHOST_CPU0_PAR		46
#define BCM_INT_ID_IHOST_CPU1_PAR	    47
#define BCM_INT_ID_IHOST_SCU0_PAR		48
#define BCM_INT_ID_IHOST_SCU1_PAR	    49
#define BCM_INT_ID_IHOST_I2_SEC			50
#endif

/*=====================================================================*/
/* IDM Interrupt IDs                                                   */
/*=====================================================================*/
#define BCM_INT_ID_IHOST_M1             ( 0 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE0_M0             ( 1 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE1_M0             ( 2 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE2_M0             ( 3 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DMA_M0               ( 4 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_AMAC_M0              ( 5 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_AMAC_M1              ( 6 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_AMAC_M2              ( 7 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_AMAC_M3              ( 8 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_USBH_M0              ( 9 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_USBH_M1              (10 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_SDIO_M0              (11 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_I2S_M0               (12 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_A9JTAG_M0            (13 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_INIT_SEQ_M0          (14 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_JTAG_M0              (15 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_IHOST_ACP            (16 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_IHOST_S0             (17 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DDR_S1               (18 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DDR_S2               (19 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE0_S0             (20 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE1_S0             (21 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_PCIE2_S0             (22 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_ROM_S0               (23 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_NAND_S0              (24 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_QPSI_S0              (25 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_A9JTAG_S0            (26 + BCM_INT_ID_IHOST_MAX)
#if defined(CONFIG_MACH_HX4) || defined(CONFIG_MACH_KT2)
#define BCM_INT_ID_SATA_S0				(27 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_SRAM_S0              (28+ BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_APBW					(29 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_APBX					(30 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_APBY					(31 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_APBZ					(32 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_0                 (33 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_1                 (34 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_2                 (35 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_3                 (36 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_4                 (37 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_IDM_MAX              (38 + BCM_INT_ID_IHOST_MAX)
#elif defined(CONFIG_MACH_NS) || defined(CONFIG_MACH_NSP)
#define BCM_INT_ID_APBX                 (27 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_0                 (28 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_1                 (29 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_2                 (30 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_3                 (31 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_DS_4                 (32 + BCM_INT_ID_IHOST_MAX)
#define BCM_INT_ID_IDM_MAX              (33 + BCM_INT_ID_IHOST_MAX)
#endif

/*=====================================================================*/
/* DDR Interrupt IDs                                                   */
/*=====================================================================*/
#define BCM_INT_ID_DDR_CONT             (0 + BCM_INT_ID_IDM_MAX)
#define BCM_INT_ID_DDR_MAX              (1 + BCM_INT_ID_IDM_MAX)

/*=====================================================================*/
/* DMAC Interrupt IDs                                                  */
/*=====================================================================*/
#define BCM_INT_ID_DMAC                 (0 + BCM_INT_ID_DDR_MAX)
#if defined(CONFIG_MACH_HX4) || defined(CONFIG_MACH_KT2)
#define BCM_INT_ID_DMAC_ABORT           (16 + BCM_INT_ID_DDR_MAX)
#define BCM_INT_ID_DMAC_MAX             (17 + BCM_INT_ID_DDR_MAX)
#elif defined(CONFIG_MACH_NS) || defined(CONFIG_MACH_NSP)
#define BCM_INT_ID_DMAC_MAX             (16 + BCM_INT_ID_DDR_MAX)
#endif

/*=====================================================================*/
/* NAND Interrupt IDs                                                  */
/*=====================================================================*/
#define BCM_INT_ID_NAND2CORE_RD_MISS         ( 0 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_ER_COMP         ( 1 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_CB_COMP         ( 2 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_PP_COMP         ( 3 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_ROCTL_RDY       ( 4 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_NAND_RBB        ( 5 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_ECC_MIPS_UNCOR  ( 6 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_ECC_MIPS_COR    ( 7 + BCM_INT_ID_DMAC_MAX)
#define BCM_INT_ID_NAND2CORE_MAX             ( 8 + BCM_INT_ID_DMAC_MAX)

/*=====================================================================*/
/* QPSI Interrupt IDs                                                  */
/*=====================================================================*/
#define BCM_INT_ID_QPSI2CORE_FULL_RCHD        ( 0 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_TRUNCATED        ( 1 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_IMAPTIENT        ( 2 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_SES_DONE         ( 3 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_OVERREAD         ( 4 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_MPSI_DONE        ( 5 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_MPSI_HLT_SET     ( 6 + BCM_INT_ID_NAND2CORE_MAX)
#define BCM_INT_ID_QPSI2CORE_MAX              ( 7 + BCM_INT_ID_NAND2CORE_MAX)

/*=====================================================================*/
/* USB2 Host Interrupt IDs                                             */
/*=====================================================================*/
#define BCM_INT_ID_USB2H2CORE_USB2_INT  (0 + BCM_INT_ID_QPSI2CORE_MAX)
#define BCM_INT_ID_USB2H2CORE_MAX       (1 + BCM_INT_ID_QPSI2CORE_MAX)

/*=====================================================================*/
/* USB3 Host Interrupt IDs                                             */
/*=====================================================================*/
#define BCM_INT_ID_USB3H2CORE_USB2_INT0  (0 + BCM_INT_ID_USB2H2CORE_MAX)
#define BCM_INT_ID_USB3H2CORE_USB2_INT1  (1 + BCM_INT_ID_USB2H2CORE_MAX)
#define BCM_INT_ID_USB3H2CORE_USB2_INT2  (2 + BCM_INT_ID_USB2H2CORE_MAX)
#define BCM_INT_ID_USB3H2CORE_USB2_INT3  (3 + BCM_INT_ID_USB2H2CORE_MAX)
#define BCM_INT_ID_USB3H2CORE_USB2_HSE   (4 + BCM_INT_ID_USB2H2CORE_MAX)
#define BCM_INT_ID_USB3H2CORE_MAX        (5 + BCM_INT_ID_USB2H2CORE_MAX)

/*=====================================================================*/
/* CCA Interrupt IDs                                                   */
/*=====================================================================*/
#define BCM_INT_ID_CCA_INT   (0 + BCM_INT_ID_USB3H2CORE_MAX)
#define BCM_INT_ID_CCA_MAX   (1 + BCM_INT_ID_USB3H2CORE_MAX)

/*=====================================================================*/
/* CCB Interrupt IDs                                                   */
/*=====================================================================*/
#define BCM_INT_ID_CCB_UART0       (0 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_GPIO        (1 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_I2S         (2 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_SMBUS       (3 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_TIM0_INT1   (4 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_TIM0_INT2   (5 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_TIM1_INT1   (6 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_TIM1_INT2   (7 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_RNG         (8 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_SW_SOC      (9 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_PCIE_INT0   (10 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_PCIE_INT1   (11 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_PCIE_INT2   (12 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_SDIO2CORE   (13 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_CTF         (14 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_GMAC_INT0   (15 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_GMAC_INT1   (16 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_GMAC_INT2   (17 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_GMAC_INT3   (18 + BCM_INT_ID_CCA_MAX)
#define BCM_INT_ID_CCB_MAX         (19 + BCM_INT_ID_CCA_MAX)

#define BCM_INT_ID_FA		   178


#ifdef CONFIG_ARCH_REQUIRE_GPIOLIB
#define IPROC_NR_IRQS           (256)
#define IPROC_IRQ_GPIO_0        (IPROC_NR_IRQS)
#define IPROC_NR_GPIO_IRQS      (32 + 4)
#define NR_IRQS                 (IPROC_NR_IRQS + IPROC_NR_GPIO_IRQS)
#else 
#define NR_IRQS               256
#endif

#endif /* __PLAT_IPROC_IRQS_H */
