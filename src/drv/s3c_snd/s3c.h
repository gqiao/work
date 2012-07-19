#ifndef __HAL_S3C_H__
#define __HAL_S3C_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/errno.h> /* error codes */
#include <asm/div64.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/hardware.h>
#include <asm/uaccess.h>
//#include <asm/arch/map.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
typedef volatile u32 v32;

typedef struct {
	v32 APLL_SEL   :1, // [0]
		MPLL_SEL   :1, // [1]
		EPLL_SEL   :1, // [2]
		RESERVED   :1, // [3]
		MFCCLK_SEL :1, // [4]
		UHOST_SEL  :2, // [6:5]
		AUDIO0_SEL :3, // [9:7]
		AUDIO1_SEL :3, // [12:10]
		UART_SEL   :1, // [13]
		SPI0_SEL   :2, // [15:14]
		SPI1_SEL   :2, // [17:16]
		MMC0_SEL   :2, // [19:18]
		MMC1_SEL   :2, // [21:20]
		MMC2_SEL   :2, // [23:22]
		IRDA_SEL   :2, // [25:24]
		LCD_SEL    :2, // [27:26]
		SCALER_SEL :2, // [29:28]
		DAC27_SEL  :1, // [30]
		TV27_SEL   :1; // [31]
}s3c_clk_src_t;

typedef struct {
	struct {
		v32 InYCbCrFormat_LSB    :1, // [0]
			InRGBFormat          :1, // [1]
			INTERLEAVE           :1, // [2]
			InRGB                :1, // [3]
			OutRGBFormat         :1, // [4]
			IRQ_Level            :1, // [5]
			POSTINT              :1, // [6]
			INTEN                :1, // [7]
			SRC420               :1, // [8]
			reserved1            :1, // [9]
			Wide_Narrow          :2, // [11:10]
			Interlace            :1, // [12]
			LCDPathEnable        :1, // [13]
			AutoLoadEnable       :1, // [14]
			InYCbCrFormat_MSB    :1, // [15]
			R2YSel               :1, // [16]
			DST420               :1, // [17]
			OutRGB               :1, // [18]
			OutYCbCrFormat       :2, // [20:19]
			CLKSEL_F             :2, // [22:21]
			CLKDIR               :1, // [23]
			CLKVAL_F             :6, // [29:24]
			CLKVALUP             :1, // [30]
			ExtFIFOIn            :1; // [31]
	}MODE;
	
	struct {
		v32 PreScale_H_Ratio     :7, // [6:0]
			PreScale_V_Ratio     :7, // [13:7]
			reserved1            :18;// [31:14]
	}PreScale_Ratio;

	struct {
		v32 PreScale_DSTWidth    :12, // [11:0]
			PreScale_DSTHeight   :12, // [23:12]
			reserved1            :8;  // [31:24]
	}PreScaleImgSize;
	
	struct  {
		v32 SRCWidth             :12, // [11:0]
			SRCHeight            :12, // [23:12]
			reserved1            :8;  // [31:24]
	}SRCImgSize;
	
	v32 MainScale_H_Ratio;    // [8:0]
	v32 MainScale_V_Ratio;    // [8:0]

	struct {
		v32 DSTWidth             :12, // [11:0]
			DSTHeight            :12, // [23:12]
			reserved1            :8;  // [31:24]
	} DSTImgSize;

	v32 PreScale_SHFactor;   // [3:0]
	v32 ADDRStart_Y;         // [30:0]
	v32 ADDRStart_Cb;        // [30:0]
	v32 ADDRStart_Cr;        // [30:0]
	v32 ADDRStart_RGB;       // [30:0]
	v32 ADDREnd_Y;           // [30:0]
	v32 ADDREnd_Cb;          // [30:0]
	v32 ADDREnd_Cr;          // [30:0]
	v32 ADDREnd_RGB;         // [30:0]
	v32 Offset_Y;            // [23:0]
	v32 Offset_Cb;           // [23:0]
	v32 Offset_Cr;           // [23:0]
	v32 Offset_RGB;          // [23:0]
	v32 reserved1;
	v32 NxtADDRStart_Y;      // [30:0]
	v32 NxtADDRStart_Cb;     // [30:0]
	v32 NxtADDRStart_Cr;     // [30:0]
	v32 NxtADDRStart_RGB;    // [30:0]
	v32 NxtADDREnd_Y;        // [30:0]
	v32 NxtADDREnd_Cb;       // [30:0]
	v32 NxtADDREnd_Cr;       // [30:0]
	v32 NxtADDREnd_RGB;      // [30:0]
	v32 ADDRStart_oCb;       // [30:0]
	v32 ADDRStart_oCr;       // [30:0]
	v32 ADDREnd_oCb;         // [30:0]
	v32 ADDREnd_oCr;         // [30:0]
	v32 Offset_oCb;          // [23:0]
	v32 Offset_oCr;          // [23:0]
	v32 NxtADDRStart_oCb;    // [30:0]
	v32 NxtADDRStart_oCr;    // [30:0]
	v32 NxtADDREnd_oCb;      // [30:0]
	v32 NxtADDREnd_oCr;      // [30:0]

	struct {
		v32 reserved1 : 31,  // [30:0]
			enable : 1;   // [31]
	}POSTENVID;

	struct {
		v32 hardware_trigger     : 3,  // [2:0]
			BC_SEL               : 1,  // [3]
			ADDR_CH_DIS          : 1,  // [4]
			FIFO_OUT_PATH        : 2,  // [6:5]
			reserved             : 25; // [31:7]
	}MODE_2;

} s3c_tvscaler_t;

typedef struct {
	struct {
		v32 TVONOFF              : 1,  // [0]
			reserved1            : 3,  // [3:1]
			TVOUTFMT             : 3,  // [6:4]
			reserved2            : 1,  // [7]
			TVOUTTYPE            : 1,  // [8]
			reserved3            : 3,  // [11:9]
			INTStatus            : 1,  // [12]
			reserved4            : 3,  // [15:13]
			INTFIFOUR            : 1,  // [16]
			reserved5            : 15; // [31:17]
	}TVCTRL;
	
	struct {
		v32 VOFBPD               : 8,  // [7:0]
			reserved1            : 8,  // [15:8]
			VEFBPD               : 9,  // [24:16]
			reserved2            : 7;  // [31:25]
	}VBPORCH;
	struct {
		v32 HBPD                 : 11, // [10:0]
			reserved1            : 5,  // [15:11]
			HSPW                 : 8,  // [23:16]
			reserved2            : 8;  // [31:24]
	}HBPORCH;
	struct {
		v32 HEOV                 : 5,  // [4:0]
			reserved1            : 3,  // [7:5]
			DTOffset             : 3,  // [10:8]
			reserved2            : 5,  // [15:11]
			HACTWinCenCTRL       : 8,  // [23:16]
			VACTWinCenCTRL       : 6,  // [29:24]
			reserved3            : 2;  // [31:30]
	}HEnhOffset;
	struct {
		v32 VDWSP                : 9,  // [8:0]
			reserved1            : 7,  // [15:9]
			VDWS                 : 9,  // [24:16]
			reserved2            : 7;  // [31:25]
	}VDemoWinSize;
	struct {
		v32 HDWSP                : 11, // [10:0]
			reserved1            : 5,  // [15:11]
			HDWEP                : 11, // [26:16]
			reserved2            : 5;  // [31:27]
	}HDemoWinSize;
	
	struct {
		v32 ImageWidth           : 11, // [10:0]
			reserved1            : 5,  // [15:11]
			ImageHeight          : 10, // [25:16]
			reserved2            : 6;  // [31:26]
	}InImageSize;
	v32 PEDCTRL;				// [0]
	struct {
		v32 CBW                  : 2,  // [1:0] {0:1.2MHz,...}
			reserved1            : 2,  // [3:2]
			YBW                  : 3,  // [6:4] {0:6.0MHz(s-video),...}
			reserved2            : 25; // [31:7]
	}YCFilterBW;

	v32 HUECTRL;  // [7:0] {0x00:0 phase shift,-, 0x80:180 phase shift, ...}
	
	v32 FscCTRL;  // [14:0] {current DTO + FscCTRL * (2^9)}
	
	struct {
		v32 FscDTOManual         : 31, // [30:0]
			FscMEn               : 1;  // [31]
	}FscDTOManCTRL;
	
	v32 reserved1;
	struct {
		v32 BGYOFS               : 4,  // [3:0]
			BGCS                 : 3,  // [6:4]
			reserved1            : 1,  // [7]
			SME                  : 1;  // [8]
	}BGCTRL;
	
	struct {
		v32 BGVS                 : 8,  // [7:0]
			BGVL                 : 8,  // [15:8]
			BGHS                 : 8,  // [23:16]
			BGHL                 : 8;  // [31:24]
	}BGHVAVCTRL;
	
	v32 reserved2[2];

	struct {
		v32 CONTRAST             : 8,  // [7:0]
			reserved1            : 8,  // [15:8]
			BRIGHT               : 8;  // [23:16]
	}ContraBright;
	
	struct {
		v32 CBGAIN               : 8,  // [7:0]
			reserved1            : 8,  // [15:8]
			CRGAIN               : 8;  // [23:16]
	}CbCrGainCTRL;
	
	struct {
		v32 BSGn                 : 2,  // [1:0]
			reserved1            : 6,  // [7:2]
			WStOff               : 1,  // [8]
			reserved2            : 3,  // [11:9]
			BStOff               : 1,  // [12]
			reserved3            : 3,  // [15:13]
			FreshEn              : 1,  // [16]
			reserved4            : 7,  // [23:17]
			MVDemo               : 1;  // [24]
	}DemoWinCTRL;
	
	struct {
		v32 FTCAS                : 8,  // [7:0]
			reserved1            : 8,  // [15:8]
			FTCAC                : 8;  // [23:16]
	}FTCA;
	
	v32 reserved3;
	struct {
		v32 BGain                : 4,  // [3:0]
			WGain                : 4;  // [7:4]
	}BWGAIN;
	
	v32 reserved4;
	struct {
		v32 DShpGn               : 6,  // [5:0]
			reserved1            : 2,  // [7:6]
			DShpF0               : 2,  // [9:8]
			reserved2            : 2,  // [11:10]
			SDhCor               : 3,  // [14:12]
			reserved3            : 5,  // [19:15]
			SHARPT               : 8;  // [27:20]
	}SharpCTRL;
	
	struct {
		v32 DCTRAN               : 3,  // [2:0]
			reserved1            : 5,  // [7:3]
			GamMode              : 2,  // [9:8]
			Reserved2            : 2,  // [11:10]
			GamEn                : 1;  // [12]
	}GammaCTRL;
	
	struct {
		v32 Fdrst                : 1,  // [0]
			reserved1            : 3,  // [3:1]
			Phalt                : 1;  // [4]
	}FscAuxCTRL;
	
	v32 SyncSizeCTRL;			// [9:0] SySize {0x3D:NTSC, 0x3E:PAL}
	
	struct {
		v32 BuSt                 : 10, // [9:0]
			reserved1            : 6,  // [15:10]
			BuEnd                : 10; // [25:16]
	}BurstCTRL;
	
	v32 MacroBurstCTRL;			// {0x41:NTSC, 0x42:PAL}

	struct {
		v32 AvonSt               : 10, // [9:0]
			reserved1            : 6,  // [15:10]
			AvonEnd              : 10; // [25:16]
	}ActVidPoCTRL;
	
	v32 EncCTRL; // [0] {0:Disable, 1:Enable}
	
	struct {
		v32 MuteOnOff            : 1,  // [0]
			reserved1            : 7,  // [7:1]
			MuteY                : 8,  // [15:8]
			MuteCb               : 8,  // [23:16]
			MuteCr               : 8;  // [31:24]
	}MuteCTRL;
} s3c_tvenc_t;


/* PA base = 0x78000000 <=> S3C_VA_CAMIF */
typedef struct {
	/* 0x00 */
	struct {
		v32 SrcVsize_CAM           :13, // [12:0]
			reserved1              :1,  // [13]
			Order422_CAM           :2,  // [15:14]
			SrcHsize_CAM           :13, // [28:16]
			reserved2              :1,  // [29]
			UVOffset               :1,  // [30]
			ITU601_656n            :1;  // [31]
	} CISRCFMT;// camera input source format
	/* 0x04 */
	struct {
		v32 WinVerOfst             :11, // [10:0]
			reserved1              :1,  // [11]
			ClrOvPrFiCr            :1,  // [12]    1: clear the overflow indication flag of input PREVIEW FIFO Cr, 0: normal
			ClrOvPrFiCb            :1,  // [13]    1: clear the overflow indication flag of input PREVIEW FIFO Cb, 0: normal
			ClrOvCoFiCr            :1,  // [14]    1: clear the overflow indication flag of input CODEC FIFO Cr, 0: normal
			ClrOvCoFiCb            :1,  // [15]    1: clear the overflow indication flag of input CODEC FIFO Cb, 0: normal
			WinHorOfst             :11, // [26:16]
			ClrOvPrFiY             :1,  // [27]
			ClrOvRLB_Pr            :1,  // [28]
			reserved2              :1,  // [29]
			ClrOvCoFiY             :1,  // [30]    1: clear the overflow indication flag of input CODEC FIFO Y, 0: normal
			WinOfsEn               :1;  // [31]
	} CIWDOFST;// window offset register

	/* 0x08 */
	struct {
		v32 Cam_Interface          :1,  // [0]
			InvPolFIELD            :1,  // [1]
			FIELDMODE              :1,  // [2]
			reserved1              :15, // [17:3]
			IRQ_CLR_p              :1,  // [18]
			IRQ_CLR_c              :1,  // [19]
			IRQ_Level              :1,  // [20]
			Href_mask              :1,  // [21]
			IRQ_Ovfen              :1,  // [22]
			reserved2              :1,  // [23]
			InvPolHREF             :1,  // [24]    1: inverse the polarity of HREF 0: normal
			InvPolVSYNC            :1,  // [25]    1: inverse the polarity of VSYNC 0: normal
			InvPolPCLK             :1,  // [26]    1: inverse the polarity of PCLK, 0: normal
			TestPattern            :2,  // [28:27]
			reserved3              :1,  // [29]
			CamRst                 :1,  // [30]
			SwRst                  :1;  // [31]
	} CIGCTRL;// Global control register

	
	v32 reserved1[2];
	
	/* 0x14 */
	struct {
		v32 WinVerOfst2            :11, // [10:0]
			reserved1              :5,  // [15:11]
			WinHorOfst2            :11; // [26:16]
	} CIWDOFST2;// window offset register 2

	/* 0x18: Y / YCbCr */
	/* 0x1C:*/
	/* 0x20 */
	/* 0x24 */
	v32 CICOYSA[4];   // CICOYSA1~4. Non-Interleave Y, Interleave YCbCr, RGB: 1st,2nd,3rd,4th frame start address for codec DMA
	
	/* 0x28 Cb */
	/* 0x2C */
	/* 0x30 */
	/* 0x34 */
	v32 CICOCBSA[4];  // Cb 1st,2nd,3rd,4th frame start address for codec DMA

	/* 0x38 Cr */
	/* 0x3C */
	/* 0x40 */
	/* 0x44 */
	v32 CICOCRSA[4];  // Cr 1st,2nd,3rd,4th frame start address for codec DMA

	/* 0x48 */
	struct {
		v32 TargetVsize_Co         :13, // [12:0]  Vertical pixel number of target image for codec DMA. Minimum number is 4
			reserved1              :1,  // [13]
			FlipMd_Co              :2,  // [15:14] Image mirror and rotation for codec DMA. 00: Normal, 01: X-axis mirror, 10: Y-axis mirror, 11: 180° rotation (XY-axis mirror)
			TargetHsize_Co         :13, // [28:16] Horizontal pixel number of target image for codec DMA (16’s multiple. minimum 16)
			OutFormat_Co           :2;  // [30:29] 00: YCbCr 4:2:0 codec output image format. (Non-interleave), 01: YCbCr 4:2:2 codec output image format. (Non-interleave), 10: YCbCr 4:2:2 codec output image format. (Interleave), 11: RGB codec output image format. (cf. RGB format register → OutRGB_FMT_Pr)
	} CICOTRGFMT; // Target image format of codec DMA

	/* 0x4C */
	struct {
		v32 Order422_Co            :2,  // [1:0]
			LastIRQEn_Co           :1,  // [2]     1: enable last IRQ at the end of frame capture (to check the done signal of capturing image for JPEG. One pulse), 0: normal
			Reserved1              :1,  // [3]
			Cburst2_Co             :5,  // [8:4]   Remained burst length for codec Cb/Cr frames
			Cburst1_Co             :5,  // [13:9]  Main burst length for codec Cb/Cr frames
			Yburst2_Co             :5,  // [18:14] Remained burst length for codec Y frames
			Yburst1_Co             :5;  // [23:19] Main burst length for codec Y frames
	} CICOCTRL; // Codec DMA control register

	/* 0x50 */
	struct {
		v32 PreVerRatio_Co         :7,  // [6:0]   Vertical ratio of codec pre-scaler
			reserved1              :9,  // [15:7]
			PreHorRatio_Co         :7,  // [22:16] Horizontal ratio of codec pre-scaler
			reserved2              :5,  // [27:23]
			SHfactor_Co            :4;  // [31:28] Shift factor for codec pre-scaler
	} CICOSCPRERATIO; // Codec pre-scaler ratio control

	/* 0x54 */
	struct {
		v32 PreDstHeight_Co        :12, // [11:0]  Destination height for codec pre-scaler
			reserved1              :4,  // [15:12] 
			PreDstWidth_Co         :12; // [27:16] Destination width for codec pre-scaler
	} CICOSCPREDST;

	/* 0x58 */
	struct {
		v32 MainVerRatio_Co        :9,  // [8:0]   Vertical scale ratio for codec main-scaler
			One2One_Co             :1,  // [9]
			Ext_RGB_Co             :1,  // [10]
			OutRGB_FMT_Co          :2,  // [12:11] Output RGB format for Codec write DMA. 00: RGB565 , 01: RGB666 , 10: RGB888 , 11: Reserved
			InRGB_FMT_Co           :2,  // [14:13]
			CoScalerStart          :1,  // [15]    Codec scaler start 1: start 0:  stop
			MainHorRatio_Co        :9,  // [24:16] Horizontal scale ratio for codec main-scaler
			Interlace_Co           :1,  // [25]
			LCDPathEn_Co           :1,  // [26]
			CSCY2R_c               :1,  // [27]
			CSCR2Y_c               :1,  // [28]
			ScaleUp_V_Co           :1,  // [29]    Vertical scale up/down flag for codec scaler (In 1:1 scale ratio, this bit must be “1”) 1: up, 0:down
			ScaleUp_H_Co           :1,  // [30]    Horizontal scale up/down flag for codec scaler (In 1:1 scale ratio, this bit must be “1”) 1: up, 0:down
			ScalerBypass_Co        :1;  // [31]    Codec scaler bypass. In this case, ImgCptEn_CoSC must be 0, but ImgCptEn must be 1.
	} CICOSCCTRL; // Codec main-scaler control

	/* 0x5C */
	v32 CICOTAREA; // [25:0] Target area for codec DMA

	/* 0x60 */
	v32 reserved2;
	
	/* 0x64 */
	struct {
		v32 reserved1              :17, // [16:0]
			FrameEnd_Co            :1,  // [17]
			reserved2              :2,  // [19:18]
			VSYNC_A                :1,  // [20]
			ImgCptEn_CoSC          :1,  // [21]
			ImgCptEn               :1,  // [22]
			FlipMd_Co              :2,  // [24:23]
			WinOfstEn_Co           :1,  // [25]
			FrameCnt_Co            :2,  // [27:26]  Frame count of codec DMA (It means the next frame number)
			VSYNC                  :1,  // [28]     Camera VSYNC (This bit can be referred by CPU for first SFR setting after external camera muxing. It can be seen in the ITUR BT 656 mode)
			OvFiCr_Co              :1,  // [29]     Overflow state of codec FIFO Cr
			OvFiCb_Co              :1,  // [30]     Overflow state of codec FIFO Cb
			OvFiY_Co               :1;  // [31]     Overflow state of codec FIFO Y
	} CICOSTATUS;

	/* 0x68 */
	v32 reserved3;

	/* 0x6C */
	/* 0x... */
	/* 0x78 */
	v32 CIPRYSA[4];  // Non-Interleave Y, Interleave YCbCr, RGB: 1st,2nd,3rd,4th frame start address for preview DMA

	/* 0x7C */
	/* 0x... */
	/* 0x88 */
	v32 CIPRCBSA[4];

	/* 0x8C */
	/* 0x... */
	/* 0x98 */
	v32 CIPRCRSA[4];

	/* 0x9C */
	struct {
		v32 TargetVsize_Pr        :13, // [12:0]  Vertical pixel number of target image for preview DMA. Minimum number is 4. (When Rot90_Pr is set, 8’s multiple but, 4’s multiple if RGB888/666 mode & H_WIDTH > 160)
			Rot90_Pr              :1,  // [13]    1: Rotate clockwise 90°, 0: Rotator bypass
			FlipMd_Pr             :2,  // [15:14] Image mirror and rotation for preview DMA. 00: normal, 01: x-axis mirror, 10: y-axis mirror, 11: 180° rotation
			TargetHsize_Pr        :13, // [28:16] Horizontal pixel number of target image for preview DMA (16’s multiple)
			OutFormat_Pr          :2;  // [30:29]
	} CIPRTRGFMT;

	/* 0xA0 */
	struct {
		v32 Order422_Pr           :2,  // [1:0]
			LastIRQEn_Pr          :1,  // [2]    1: enable last IRQ at the end of frame capture (One pulse), 0 : normal
			reserved1             :1,  // [3]
			Cburst2_Pr            :5,  // [8:4]
			Cburst1_Pr            :5,  // [13:9]
			Yburst2_Pr            :5,  // [18:14]
			Yburst1_Pr            :5;  // [23:19]
	} CIPRCTRL;

	/* 0xA4 */
	struct {
		v32 PreVerRatio_Pr        :7,  // [6:0]   Vertical ratio of preview pre-scaler
			reserved1             :9,  // [15:7]
			PreHorRatio_Pr        :7,  // [22:16] Horizontal ratio of preview pre-scaler
			reserved2             :5,  // [27:23]
			SHfactor_Pr           :4;  // [31:28] Shift factor for preview pre-scaler
	} CIPRSCPRERATIO; // Preview Pre-scaler ratio

	/* 0xA8 */
	struct {
		v32 PreDstHeight_Pr       :12,  // [11:0]  Destination height for preview pre-scaler
			reserved1             :4,   // [15:12]
			PreDstWidth_Pr        :12;  // [27:16] Destination width for preview pre-scaler
	} CIPRSCPREDST; // Preview Pre-scaler destination format

	/* 0xAC */
	struct {
		v32 MainVerRatio_Pr       :9,  // [8:0]
			One2One_Pr            :1,  // [9]
			Ext_RGB_Pr            :1,  // [10]
			OutRGB_FMT_Pr         :2,  // [12:11] Output RGB format for Preview write DMA. 00: RGB565, 01: RGB666, 10: RGB888 , 11: Forbidden
			InRGB_FMT_Pr          :2,  // [14:13]
			PrScalerStart         :1,  // [15]    Preview scaler start. Must be zero in preview scaler-bypass mode. 1: start, 0: stop
			MainHorRatio_Pr       :9,  // [24:16] Horizontal scale ratio for preview main-scaler
			Interlace_Pr          :1,  // [25]
			LCDPathEn_Pr          :1,  // [26]
			CSCY2R_Pr             :1,  // [27]
			CSCR2Y_Pr             :1,  // [28]
			ScaleUp_V_Pr          :1,  // [29]    Vertical scale up/down flag for preview scaler (In 1:1 scale ratio, this bit must be “1”) 1: up, 0:down
			ScaleUp_H_Pr          :1,  // [30]    Horizontal scale up/down flag for preview scaler (In 1:1 scale ratio, this bit must be “1”) 1: up, 0:down
			ScalerBypass_Pr       :1;  // [31]
	} CIPRSCCTRL;// Preview main-scaler control

	/* 0xB0 */
	v32 CIPRTAREA;                     // [25:0]

	/* 0xB4 */
	v32 reserved5;
	
	/* 0xB8 */
	struct {
		v32 reserved1            :19,  // [18:0]
			FrameEnd_Pr          :1,   // [19]
			OvRLB_Pr             :1,   // [20]
			ImgCptEn_PrSC        :1,   // [21]
			reserved2            :1,   // [22]
			FlipMd_Pr            :2,   // [24:23]
			reserved3            :1,   // [25]
			FrameCnt_Pr          :2,   // [27:26]  Frame count of preview DMA
			reserved4            :1,   // [28]
			OvFiCr_Pr            :1,   // [29]     Overflow state of preview FIFO Cr
			OvFiCb_Pr            :1,   // [30]     Overflow state of preview FIFO Cb
			OvFiY_Pr             :1;   // [31]     Overflow state of preview FIFO Y
	} CIPRSTATUS;

	/* 0xBC */
	v32 reserved6;
	
	/* 0xC0 */
	struct {
		v32 reserved1            :10,  // [9:0]
			Cpt_FrCnt            :8,   // [17:10]
			Cpt_FrMod            :1,   // [18]
			Cpt_FrPtr            :5,   // [23:19]
			Cpt_FrEn_pr          :1,   // [24]    Capture preview frame control. (only camera input is applied) 1: Enable (Step-by-Step frame one shot mode), 0 : Disable (Free Run mode)
			Cpt_FrEn_Co          :2,   // [26:25]    Capture codec frame control. (only camera input is applied) 1: Enable (Step-by-Step frame one shot mode), 0: Disable (FreeRun mode)
			reserved2            :2,   // [28:27]
			ImgCptEn_PrSc        :1,   // [29]    capture enable for preview scaler. Must be zero in preview scaler-bypass mode.
			ImgCptEn_CoSc        :1,   // [30]    capture enable for codec scaler. Must be zero in codec scaler-bypass mode
			ImgCptEn             :1;   // [31]    camera interface global capture enable
	} CIIMGCPT;

	/* 0xC4 */
	v32 CICPTSEQ; // [31:0] capture sequence pattern

	/* 0xC8, 0xCC */
	v32 reserved7[2];
	
	/* 0xD0 */
	struct {
		v32 PAT_Cr               :8,  // [7:0]
			reserved1            :5,  // [12:8]
			PAT_Cb               :8,  // [20:13]
			reserved2            :5,  // [25:21]
			FIN                  :3,  // [28:26] Image Effect selection. (Common preview & codec path) 3’d0 : Bypass, 3’d1 : Arbitrary Cb/Cr, 3’d2 : Negative, 3’d3 : Art Freeze, 3’d4 : Embossing, 3’d5 : Silhouette
			IE_AFTER_SC          :1,  // [29]
			IE_ON_Co             :1,  // [30]    0: image effect function disable in codec path , 1: enable
			IE_ON_Pr             :1;  // [31]    0: image effect function disable in preview path, 1: enable
	} CIIMGEFF;

	/* 0xD4 */
	v32 MSCOY0SA;  // [30:0]  DMA start address for Y component (non-interleave YCbCr 4:2:0, 4:2:2). DMA start address for Interleave YCbCr 4:2:2 / RGB component

	/* 0xD8 */
	v32 MSCOCB0SA;  // [30:0] DMA start address for Cb component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xDC */
	v32 MSCOCR0SA;  // [30:0] DMA start address for Cr component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xE0 */
	v32 MSCOY0END;  // [30:0] DMA End address for Y component (non-interleave YCbCr 4:2:0, 4:2:2). DMA End address for Interleave YCbCr 4:2:2 / RGB component

	/* 0xE4 */
	v32 MSCOCB0END; // [30:0] DMA End address for Cb component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xE8 */
	v32 MSCOCR0END; // [30:0] DMA End address for Cr component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xEC */
	v32 MSCOYOFF;   // [23:0] Offset of Y component for fetching source image (non-interleave YCbCr 4:2:0, 4:2:2). Offset of Interleave YCbCr 4:2:2 / RGB component for fetching source image

	/* 0xF0 */
	v32 MSCOCBOFF;  // [23:0] Offset of Cb component for fetching source image(non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xF4 */
	v32 MSCOCROFF;  // [23:0] Offset of Cr component for fetching source image(non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0xF8 */
	struct {
		v32 MSCOWIDTH      :12, // [11:0]  MSDMA source image horizontal pixel size (must be 8’s multiple. Must be 4’s multiple of PreHorRatio. minimum 16)
			reserved1      :4,  // [15:12]
			MSCOHEIGHT     :12, // [27:16] MSDMA source image vertical pixel size. minimum 8. Must be multiple of PreVerRatio.
			reserved2      :2,  // [29:28]
			ADDR_CH_DIS    :1,  // [30]    MSDMA Address Change Disable (Only Software trigger mode). At the first frame start needs ADDR_CH_DIS = ‘0’. 0: Address change enable , 1: Address change disable
			AutoLoadEnable :1;  // [31]    MSDMA Automatically restart (Only Software trigger mode). At the first frame start requires ENVID start setting. After first frame, next frame does not need ENVID setting. 0: AutoLoad Disable , 1: AutoLoad Enable
	} MSCOWIDTH;

	/* 0xFC */
	struct {
		v32 ENVID_M_C      :1,  // [0]    MSDMA operation start. (When triggered Low to High bysoftware setting) Hardware doesn’t clear automatically. It is allowed set only Software Trigger mode. If Hardware trigger mode, this bit is read only. 1) SEL_DMA_CAM = ‘0’ , ENVID don’t care (using external camera signal for codec path). 2) SEL_DMA_CAM = ‘1’, ENVID is set (0→1) then MSDMA operation start for codec
			InFormat_M_C   :2,  // [2:1]  Source image format for MSDMA. 00: YCbCr 4:2:0, 01: YCbCr 4:2:2 (non-interleave), 10: YCbCr 4:2:2 (interleave), 11: RGB (cf. RGB format register → InRGB_FMT_Co)
			SEL_DMA_CAM_C  :1,  // [3]    Codec path input data selection. 0: External camera input path, 1: Memory data input path (MSDMA)
			Order422_M_C   :2,  // [5:4]
			EOF_M_C        :1;  // [6]
	} MSCOCTRL;

	/* 0x100 */
	v32 MSPRY0SA; // [30:0] MSDMA Y0 start address related. DMA start address for Y component (non-interleave YCbCr 4:2:0, 4:2:2), DMA start address for Interleave YCbCr 4:2:2 / RGB component

	/* 0x104 */
	v32 MSPRCB0SA; // [30:0] MSDMA Cb0 start address related. DMA start address for Cb component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0x108 */
	v32 MSPRCR0SA; // [30:0] MSDMA Cr0 start address related. DMA start address for Cr component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0x10C */
	v32 MSPRY0END; // [30:0] MSDMA Y0 end address related. DMA End address for Y component (non-interleave YCbCr 4:2:0,4:2:2), DMA End address for Interleave YCbCr 4:2:2 / RGB component

	/* 0x110 */
	v32 MSPRCB0END; // [30:0] MSDMA Cb0 end address related. DMA End address for Cb component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0x114 */
	v32 MSPRCR0END; // [30:0] MSDMA Cr0 end address related. DMA End address for Cr component (non-interleave YCbCr 4:2:0, 4:2:2)

	/* 0x118 */
	v32 MSPRYOFF;   // [23:0]

	/* 0x11C */
	v32 MSPRCBOFF;  // [23:0]

	/* 0x120 */
	v32 MSPRCROFF;  // [23:0]

	/* 0x124 */
	struct {
		v32 MSPRWIDTH        :12, // [11:0]  MSDMA source image horizontal pixel size (must be 8’s multiple. Also, must be 4’s multiple of PreHorRatio. minimum 16)
			reserved1        :4,  // [15:12]
			MSPRHEIGHT       :12, // [27:16] MSDMA source image vertical pixel size. minimum 8. Also, must be multiple of PreVerRatio. If InRot90_Pr = 1, must be 8’s multiple. Also, must be 4’s multiple of PreHorRatio. minimum 16
			reserved2        :2,  // [29:28]
			ADDR_CH_DIS      :1,  // [30]
			AutoLoadEnable   :1;  // [31]
	} MSPRWIDTH;

	/* 0x128 */
	struct {
		v32 ENVID_M_P        :1,  // [0]   MSDMA operation start. (When triggered Low to High by software setting) Hardware clear automatically. This bit is allowed set only Software Trigger mode. Hardware trigger mode only can read.
			InFormat_M_P     :2,  // [2:1]
			SEL_DMA_CAM_P    :1,  // [3]
			Order422_M_P     :2,  // [5:4]
			EOF_M_P          :1;  // [6]   When MSDMA operation done, End Of Frame will be generated.(read only this signal)
	} MSPRCTRL;

	/* 0x12C */
	struct {
		v32 Line_Yoffset_Co     :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Yoffset_Co  :13; // [28:16]
	} CICOSCOSY;

	/* 0x130 */
	struct {
		v32 Line_Cboffset_Co    :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Cboffset_Co :13; // [28:16]
	} CICOSCOSCB;

	/* 0x134 */
	struct {
		v32 Line_Croffset_Co    :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Croffset_Co :13; // [28:16]
	} CICOSCOSCR;

	/* 0x138 */
	struct {
		v32 Line_Yoffset_Pr     :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Yoffset_Pr  :13; // [28:16]
	} CIPRSCOSY;

	/* 0x13C */
	struct {
		v32 Line_Cboffset_Pr    :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Cboffset_Pr :13; // [28:16]
	} CIPRSCOSCB;

	/* 0x140 */
	struct {
		v32 Line_Croffset_Pr    :13, // [12:0]
			reserved1           :3,  // [15:13]
			Initial_Croffset_Pr :13; // [28:16]
	} CIPRSCOSCR;
	
} s3c_cam_t;

/* S3C2410_PRIORITY <=> S3C2410_IRQREG(0x00C) */
typedef struct {
	v32 ARB_MODE0               :1,  // [0]
		ARB_MODE1               :1,  // [1]
		ARB_MODE2               :1,  // [2]
		ARB_MODE3               :1,  // [3]
		ARB_MODE4               :1,  // [4]
		ARB_MODE5               :1,  // [5]
		ARB_MODE6               :1,  // [6]
		ARB_SEL0                :2,  // [8:7]
		ARB_SEL1                :2,  // [10:9]
		ARB_SEL2                :2,  // [12:11]
		ARB_SEL3                :2,  // [14:13]
		ARB_SEL4                :2,  // [16:15]
		ARB_SEL5                :2,  // [18:17]
		ARB_SEL6                :2;  // [20:19]
} s3c_priority_t;

typedef struct {
	/* 0x00 */
	v32 APLL_LOCK;    // [15:0] PLL_LOCKTIME: Required period to generate a stable clock output

	/* 0x04 */
	v32 MPLL_LOCK;   // [15:0] PLL_LOCKTIME: Required period to generate a stable clock output

	/* 0x08 */
	v32 EPLL_LOCK;   // PLL_LOCKTIME [15:0] : Required period to generate a stable clock output

	/* 0x0C */
	struct {
		v32 SDIV                :3,  // [2:0]   PLL S divide value
			reserved1           :5,  // [7:3]
			PDIV                :6,  // [13:8]  PLL P divide value
			reserved2           :2,  // [15:14]
			MDIV                :10, // [25:16] PLL M divide value
			reserved3           :5,  // [30:26]
			ENABLE              :1;  // [31]    PLL enable control (0: disable, 1: enable)
	} APLL_CON;

	/* 0x10 */
	struct {
		v32 SDIV                :3,  // [2:0]   PLL S divide value
			reserved1           :5,  // [7:3]
			PDIV                :6,  // [13:8]  PLL P divide value
			reserved2           :2,  // [15:14]
			MDIV                :10, // [25:16] PLL M divide value
			reserved3           :5,  // [30:26]
			ENABLE              :1;  // [31]    PLL enable control (0: disable, 1: enable)
	} MPLL_CON;

	/* 0x14 */
	struct {
		v32 SDIV                :3,  // [2:0]   PLL S divide value
			reserved1           :5,  // [7:3]
			PDIV                :6,  // [13:8]  PLL P divide value
			reserved2           :2,  // [15:14]
			MDIV                :8,  // [23:16] PLL M divide value
			reserved3           :7,  // [30:24]
			ENABLE              :1;  // [31]    PLL enable control (0: disable, 1: enable)
	} EPLL_CON0;

	/* 0x18 */
	v32 EPLL_CON1;   // [15:0] KDIV : PLL K divide value

	/* 0x1C */
	struct {
		v32 APLL_SEL            :1,  // [0]     Control MUXapll (0:FINapll, 1:FOUTapll)
			MPLL_SEL            :1,  // [1]     Control MUXmpll (0:FINmpll, 1:FOUTmpll)
			EPLL_SEL            :1,  // [2]     Control MUXepll (0:FINepll, 1:FOUTepll)
			reserved1           :1,  // [3]
			MFCCLK_SEL          :1,  // [4]     Control MUXMFC, which is the source clock of MFC
			UHOST_SEL           :2,  // [6:5]   Control MUXuhost, which is the source clock of USB Host
			AUDIO0_SEL          :3,  // [9:7]   Control MUXaudio0, which is the source clock of IIS0, PCM0 and AC97 0
			AUDIO1_SEL          :3,  // [12:10] Control MUXaudio1, which is the source clock of IIS1, PCM1 and AC97 1
			UART_SEL            :1,  // [13]    Control MUXuart0, which is the source clock of UART
			SPI0_SEL            :2,  // [15:14] Control MUXspi0, which is the source clock of SPI0
			SPI1_SEL            :2,  // [17:16] Control MUXspi1, which is the source clock of SPI1
			MMC0_SEL            :2,  // [19:18] Control MUXmmc0, which is the source clock of MMC0
			MMC1_SEL            :2,  // [21:20] Control MUXmmc1, which is the source clock of MMC1
			MMC2_SEL            :2,  // [23:22] Control MUXmmc2, which is the source clock of MMC2
			IRDA_SEL            :2,  // [25:24] Control MUXirda, which is the source clock of IRDA
			LCD_SEL             :2,  // [27:26] Control MUXlcd, which is the source clock of LCD
			SCALER_SEL          :2,  // [29:28] Control MUXscaler, which is the source clock of TVSCALER
			DAC27_SEL           :1,  // [30]    Control MUXDAC27, which is the source clock of DAC27MHz
			TV27_SEL            :1;  // [31]    Control MUXTV27, which is the source clock of TV27MHz			
	} CLK_SRC;

	/* 0x20 */
	struct {
		v32 ARM_RATIO           :4,  // [3:0]   ARMCLK = DOUTapll / (ARM_RATIO + 1)
			MPLL_RATIO          :1,  // [4]     DOUTMPLL = MOUTmpll / (MPLL_RATIO + 1)
			reserved1           :3,  // [7:5]
			HCLK_RATIO          :1,  // [8]     HCLK = HCLKX2 / (HCLK_RATIO + 1)
			HCLKX2_RATIO        :3,  // [11:9]  HCLKX2 = HCLKX2in / (HCLKX2_RATIO + 1)
			PCLK_RATIO          :4,  // [15:12] PCLK clock divider ratio, which must be odd value. S3C6410 supports only even divider ratio.PCLK = HCLKX2 / (PCLK_RATIO + 1)
			reserved2           :2,  // [17:16]
			SECUR_RATIO         :2,  // [19:18] Security clock divider ratio, which must be 0x1 or 0x3.CLKSECUR = HCLKX2 / (SECUR_RATIO + 1)
			CAM_RATIO           :4,  // [23:20] CLKCAM = HCLKX2 / (CAM_RATIO + 1)
			JPEG_RATIO          :4,  // [27:24] JPEG clock divider ratio, which must be odd value. S3C6410 supports only even divider ratio.CLKJPEG = HCLKX2 / (JPEG_RATIO + 1)
			MFC_RATIO           :4;  // [31:28] CLKMFC = CLKMFCin / (MFC_RATIO + 1)
	} CLK_DIV0;

	/* 0x24 */
	struct {
		v32 MMC0_RATIO          :4,  // [3:0]   CLKMMC0 = CLKMMC0in / (MMC0_RATIO + 1)
			MMC1_RATIO          :4,  // [7:4]   CLKMMC1 = CLKMMC1in / (MMC1_RATIO + 1)
			MMC2_RATIO          :4,  // [11:8]  CLKMMC2 = CLKMMC2in / (MMC2_RATIO + 1)
			LCD_RATIO           :4,  // [15:12] CLKLCD = CLKLCDin / (LCD_RATIO + 1)
			SCALER_RATIO        :4,  // [19:16] CLKSCALER = CLKSCALERin / (SCALER_RATIO + 1)
			UHOST_RATIO         :4,  // [23:20] CLKUHOST = CLKUHOSTin / (UHOST_RATIO + 1)
			FIMC_RATIO          :4;  // [27:24] CLKFIMC = HCLK / (FIMC_RATIO + 1)
	} CLK_DIV1;

	/* 0x28 */
	struct {
		v32 SPI0_RATIO          :4,  // [3:0]   CLKSPI0 = CLKSPI0in / (SPI0_RATIO + 1)
			SPI1_RATIO          :4,  // [7:4]   CLKSPI1 = CLKSPI1in / (SPI1_RATIO + 1)
			AUDIO0_RATIO        :4,  // [11:8]  CLKAUDIO0 = CLKAUDIO0in / (AUDIO0_RATIO + 1)
			AUDIO1_RATIO        :4,  // [15:12] CLKAUDIO1 = CLKAUDIO1in / (AUDIO1_RATIO + 1)
			UART_RATIO          :4,  // [19:16] CLKUART = CLKUARTin / (UART_RATIO + 1)
			IRDA_RATIO          :4,  // [23:20] CLKIRDA = CLKIRDAin / (IRDA_RATIO + 1)
			AUDIO2_RATIO        :4;  // [27:24] CLKAUDIO2 = CLKAUDIO2in / (AUDIO2_RATIO + 1)
	} CLK_DIV2;

	/* 0x2C */
	struct {
		v32 DCLKEN              :1,  // [0]     Enable DCLK (0:disable, 1:enable)
			DCLKSEL             :1,  // [1]     Select DCLK source clock (0:PCLK, 1:48MHz)
			reserved1           :2,  // [3:2]
			DCLKDIV             :4,  // [7:4]   DCLK frequency = source clock / (DCLKDIV + 1)
			DCLKCMP             :4,  // [11:8]  changes the clock duty of DCLK. Smaller than DCLKDIV. Valid only when CLKSEL is 0x7.If the DCLKCMP is n, low level duration is (n+1).High is ((DCLKDIV + 1) – (n+1))
			CLKSEL              :4,  // [15:12] 0000 = FOUTAPLL/4, 0001 = FOUTEPLL, 0010 = HCLK, 0011 = CLK48M, 0100 = CLK27M, 0101 = RTC, ...
			DIVVAL              :4;  // [19:16] Divide ratio = DIVVAL + 1
	} CLK_OUT;

	/* 0x30 */
	struct {
		v32 HCLK_MFC            :1,  // [0] Gating HCLK for MFC (0: mask, 1: pass)
			HCLK_INTC           :1,  // [1] ... vectored interrupt controller (0: mask, 1: pass)
			HCLK_TZIC           :1,  // [2] ... trust interrupt controller (0: mask, 1: pass)
			HCLK_LCD            :1,  // [3] ... LCD controller (0: mask, 1: pass)
			HCLK_ROT            :1,  // [4] ... rotator (0: mask, 1: pass)
			HCLK_POST0          :1,  // [5] ... POST0 (0: mask, 1: pass)
			reserved1           :1,  // [6]
			HCLK_TV             :1,  // [7] ... TV encoder (0: mask, 1: pass)
			HCLK_2D             :1,  // [8] ... 2D (0: mask, 1: pass)
			HCLK_SCALER         :1,  // [9] ... scaler (0: mask, 1: pass)
			HCLK_CAMIF          :1,  // [10] ... camera interface (0: mask, 1: pass)
			HCLK_JPEG           :1,  // [11] ... JPEG (0: mask, 1: pass)
			HCLK_DMA0           :1,  // [12] ... DMA0 (0: mask, 1: pass)
			HCLK_DMA1           :1,  // [13] ... DMA1 (0: mask, 1: pass)
			HCLK_IHOST          :1,  // [14] ... indirect HOST interface (0: mask, 1: pass)
			HCLK_DHOST          :1,  // [15] ... direct HOST interface (0: mask, 1: pass)
			HCLK_MDP            :1,  // [16] ... MDP (0: mask, 1: pass)
			HCLK_HSMMC0         :1,  // [17] ... HSMMC0 (0: mask, 1: pass)
			HCLK_HSMMC1         :1,  // [18] ... HSMMC1 (0: mask, 1: pass)
			HCLK_HSMMC2         :1,  // [19] ... HSMMC2 (0: mask, 1: pass)
			HCLK_USB            :1,  // [20] ... USB OTG (0: mask, 1: pass)
			HCLK_MEM0           :1,  // [21] ... SROM, OneNAND, NFCON, CFCON (0: mask,1: pass)
			HCLK_MEM1           :1,  // [22] ... DMC1 (0: mask, 1: pass)
			reserved2           :1,  // [23]
			HCLK_DDR1           :1,  // [24] ... DDR1 (0: mask, 1: pass)
			HCLK_IROM           :1,  // [25] ... IROM (0: mask, 1: pass)
			HCLK_SDMA0          :1,  // [26] ... SDMA0 (0: mask, 1: pass)
			HCLK_SDMA1          :1,  // [27] ... SDMA1 (0: mask, 1: pass)
			HCLK_SECUR          :1,  // [28] ... security sub-system (0: mask, 1: pass)
			HCLK_UHOST          :1,  // [29] ... UHOST (0: mask, 1: pass)
			reserved3           :1,  // [30]
			HCLK_3DSE           :1;  // [31] ... 3D (0: mask, 1: pass)
	} HCLK_GATE;

	/* 0x34 */
	struct {
		v32 PCLK_MFC            :1,  // [0] Gating PCLK for MFC (0: mask, 1: pass)
			PCLK_UART0          :1,  // [1] ... UART0 (0: mask, 1: pass)
			PCLK_UART1          :1,  // [2] ... UART1 (0: mask, 1: pass)
			PCLK_UATR2          :1,  // [3] ... UART2 (0: mask, 1: pass)
			PCLK_UART3          :1,  // [4] ... UART3 (0: mask, 1: pass)
			PCLK_WDT            :1,  // [5] ... watch dog timer (0: mask, 1: pass)
			PCLK_RTC            :1,  // [6] ... RTC (0: mask, 1: pass)
			PCLK_PWM            :1,  // [7] ... PWM (0: mask, 1: pass)
			PCLK_PCM0           :1,  // [8] ... PCM0 (0: mask, 1: pass)
			PCLK_PCM1           :1,  // [9] ... PCM1 (0: mask, 1: pass)
			PCLK_IRDA           :1,  // [10] ... IRDA (0: mask, 1: pass)
			PCLK_KEYPAD         :1,  // [11] ... Key PAD (0: mask, 1: pass)
			PCLK_TSADC          :1,  // [12] ... touch screen ADC (0: mask, 1: pass)
			PCLK_TZPC           :1,  // [13] ... TZPC (0: mask, 1: pass)
			PCLK_AC97           :1,  // [14] ... AC97 (0: mask, 1: pass)
			PCLK_IIS0           :1,  // [15] ... IIS0 (0: mask, 1: pass)
			PCLK_IIS1           :1,  // [16] ... IIS1 (0: mask, 1: pass)
			PCLK_IIC0           :1,  // [17] ... IIC0 (0: mask, 1: pass)
			PCLK_GPIO           :1,  // [18] ... GPIO (0: mask, 1: pass)
			PCLK_HSITX          :1,  // [19] ... HSI transmitter (0: mask, 1: pass)
			PCLK_HSIRX          :1,  // [20] ... HSI receiver (0: mask, 1: pass)
			PCLK_SPI0           :1,  // [21] ... SPI0 (0: mask, 1: pass)
			PCLK_SPI1           :1,  // [22] ... SPI1 (0: mask, 1: pass)
			PCLK_CHIPID         :1,  // [23] ... chip ID (0: mask, 1: pass)
		    PCLK_SKEY           :1,  // [24] ... security key (0: mask, 1: pass)
			reserved1           :1,  // [25]
			PCLK_IIS2           :1,  // [26] ... IIS2 (0: mask, 1: pass)
			PCLK_IIC1           :1;  // [27] ... IIC1 (0: mask, 1: pass)
	} PCLK_GATE;

	/* 0x38 */
	struct {
		v32 reserved1          :1,  // [0]
			SCLK_JPEG          :1,  // [1] Gating special clock for JPEG (0: mask, 1: pass)
			SCLK_CAM           :1,  // [2] ... camera interface (0: mask, 1: pass)
			SCLK_MFC           :1,  // [3] ... MFC (0: mask, 1: pass)
			reserved2          :1,  // [4] 
			SCLK_UART          :1,  // [5] ... UART0~3 (0: mask, 1: pass)
			SCLK_IRDA          :1,  // [6] ... IRDA (0: mask, 1: pass)
			SCLK_SECUR         :1,  // [7] ... security block (0: mask, 1: pass)
			SCLK_AUDIO0        :1,  // [8] ... PCM0, IIS0 (0: mask, 1: pass)
			SCLK_AUDIO1        :1,  // [9] ... PCM1, IIS1 (0: mask, 1: pass)
			SCLK_POST0         :1,  // [10] ... POST0 (0: mask, 1: pass)
			SCLK_AUDIO2        :1,  // [11] ... IIS2 (0: mask, 1: pass)
			SCLK_POST0_27      :1,  // [12] ... POST0 (0: mask, 1: pass)
			SCLK_FIMC          :1,  // [13] ... camera & LCD (0: mask, 1: pass)
			SCLK_LCD           :1,  // [14] ... LCD controller (0: mask, 1: pass)
			SCLK_LCD27         :1,  // [15] ... LCD controller (0: mask, 1: pass)
			SCLK_SCALER        :1,  // [16] ... scaler (0: mask, 1: pass)
			SCLK_SCALER27      :1,  // [17] ... scaler27 (0: mask, 1: pass)
			SCLK_TV27          :1,  // [18] ... TV encoder (0: mask, 1: pass)
			SCLK_DAC27         :1,  // [19] ... DAC (0: mask, 1: pass)
			SCLK_SPI0          :1,  // [20] ... SPI (0: mask, 1: pass)
			SCLK_SPI1          :1,  // [21] ... SPI (0: mask, 1: pass)
			SCLK_SPI0_48       :1,  // [22] ... SPI (0: mask, 1: pass)
			SCLK_SPI1_48       :1,  // [23] ... SPI (0: mask, 1: pass)
			SCLK_MMC0          :1,  // [24] ... MMC0 (0: mask, 1: pass)
			SCLK_MMC1          :1,  // [25] ... MMC1 (0: mask, 1: pass)
			SCLK_MMC2          :1,  // [26] ... MMC2 (0: mask, 1: pass)
			SCLK_MMC0_48       :1,  // [27] ... MMC0 (0: mask, 1: pass)
			SCLK_MMC1_48       :1,  // [28] ... MMC1 (0: mask, 1: pass)
			SCLK_MMC2_48       :1,  // [29] ... MMC2 (0: mask, 1: pass)
			SCLK_UHOST         :1,  // [30] ... USB-HOST (0: mask, 1: pass)
			reserved3          :1;  // [31]
	} SCLK_GATE;

	/* 0x3C */
	struct {
		v32 reserved1          :1,  // [0]
			HCLK_SROM          :1,  // [1] Gating special clock for SROM (0: mask, 1: pass)
			HCLK_NFCON         :1,  // [2] ... NFCON (0: mask, 1: pass)
			HCLK_OneNAND0      :1,  // [3] ... OneNAND0 (0: mask, 1: pass)
			HCLK_OneNAND1      :1,  // [4] ... OneNAND1 (0: mask, 1: pass)
			HCLK_CFCON         :1;  // [5] ... CFCON (0: mask, 1: pass)
	} MEMO_CLK_GATE;

	/* 0x40 - 0xFC */
	v32 reserved1[48];

	/* 0x100 */
	struct {
		v32 FIX_PRIOR_F        :3,  // [2:0]   Fixed priority order for AHB-F
			reserved1          :1,  // [3]
			PRIOR_TYPE_F       :2,  // [5:4]   Arbitration type for AHB-F
			reserved2          :1,  // [6]
			DISABLE_HLOCK_F    :1,  // [7]     Control HLOCK for F-block (0: disable, 1:enable) 0
			FIX_PRIOR_X        :3,  // [10:8]  Fixed priority order for AHB-X
			reserved3          :1,  // [11]
			PRIOR_TYPE_X       :2,  // [13:12] Arbitration type for AHB-X
			reserved4          :1,  // [14]
			DISABLE_HLOCK_X    :1,  // [15]    Control HLOCK for X-block (0: disable, 1:enable)
			FIX_PRIOR_P        :3,  // [18:16] Fixed priority order for AHB-P
			reserved5          :1,  // [19]
			PRIOR_TYPE_P       :2,  // [21:20] Arbitration type for AHB-P
			reserved6          :1,  // [22]
			DISABLE_HLOCK_P    :1,  // [23]    Control HLOCK for P-block (0: disable, 1:enable)
			FIX_PRIOR_I        :3,  // [26:24] Fixed priority order for AHB-I
			reserved7          :1,  // [27]
			PRIOR_TYPE_I       :2,  // [29:28] Arbitration type for AHB-I
			reserved8          :1,  // [30]
			DISABLE_HLOCK_I    :1;  // [31]    Control HLOCK for I-block (0: disable, 1:enable)
	} AHB_CON0;

	/* 0x104 */
	struct {
		v32 FIX_PRIOR_T0       :3,  // [2:0]   Fixed priority order for AHB-T0
			reserved1          :1,  // [3]
			PRIOR_TYPE_T0      :2,  // [5:4]   Arbitration type for AHB-T0
			reserved2          :2,  // [7:6]
			FIX_PRIOR_T1       :3,  // [10:8]  Fixed priority order for AHB-T1
			reserved3          :1,  // [11]
			PRIOR_TYPE_T1      :2,  // [13:12] Arbitration type for AHB-T1
			reserved4          :1,  // [14]
			DISABLE_HLOCK_T    :1,  // [15]    Control HLOCK for T-block (0: disable, 1:enable)
			FIX_PRIOR_M0       :3,  // [18:16] Fixed priority order for AHB-M0
			reserved5          :1,  // [19]
			PRIOR_TYPE_M0      :2,  // [21:20] Arbitration type for AHB-M0
			reserved6          :1,  // [23:22]
			FIX_PRIOR_M1       :3,  // [26:24] Fixed priority order for AHB-M1
			reserved7          :1,  // [27]
			PRIOR_TYPE_M1      :2,  // [29:28] Arbitration type for AHB-M1
			reserved8          :1,  // [30]
			DISABLE_HLOCK_M    :1;  // [31]    Control HLOCK for M-block (0: disable, 1:enable)
	} AHB_CON1;
	
	/* 0x108 */
	struct {
		v32 FIX_PRIOR_S0       :3,  // [2:0]   Fixed priority order for AHB-F
			reserved1          :1,  // [3]
			PRIOR_TYPE_S0      :2,  // [5:4]   Arbitration type for AHB-F
			reserved2          :2,  // [7:6]
			FIX_PRIOR_S1       :3,  // [10:8]  Fixed priority order for AHB-X
			reserved3          :1,  // [11]
			PRIOR_TYPE_S1      :2,  // [13:12] Arbitration type for AHB-X
			reserved4          :1,  // [14]
			DISABLE_HLOCK_S    :1,  // [15]    Control HLOCK for X-block (0: disable, 1:enable)
			reserved5          :7,  // [22:16]
			DISABLE_HLOCK_R    :1,  // [23]    Control HLOCK for P-block (0: disable, 1:enable)
			reserved6          :8;  // [31:24]
	} AHB_CON2;

	/* 0x10C */
	v32 CLK_SRC2;   // [2:0] AUDIO2_SEL : Control MUXaudio2, which is the source clock of AUDIO2

	/* 0x110 */
	struct {
		v32 UART0_0            :1,  // [0]     DMA selection for UART0 (0: SDMA0, 1: DMA0) 0
			UART0_1            :1,  // [1]     .... UART0 (0: SDMA0, 1: DMA0)
			UART1_0            :1,  // [2]     .... UART1 (0: SDMA0, 1: DMA0)
			UART1_1            :1,  // [3]     .... UART1 (0: SDMA0, 1: DMA0)
			UART2_0            :1,  // [4]     .... UART2 (0: SDMA0, 1: DMA0)
			UART2_1            :1,  // [5]     .... UART2 (0: SDMA0, 1: DMA0)
			UART3_0            :1,  // [6]     .... UART3 (0: SDMA0, 1: DMA0)
			UART3_1            :1,  // [7]     .... UART3 (0: SDMA0, 1: DMA0)
			PCM0_TX            :1,  // [8]     .... PCM0 Tx (0: SDMA0, 1: DMA0)
			PCM0_RX            :1,  // [9]     .... PCM0 Rx (0: SDMA0, 1: DMA0)
			I2S0_TX            :1,  // [10]    .... I2S0 Tx (0: SDMA0, 1: DMA0)
			I2S0_RX            :1,  // [11]    .... I2S0 Rx (0: SDMA0, 1: DMA0)
			SPI0_TX            :1,  // [12]    .... SPI0 Tx (0: SDMA0, 1: DMA0)
			SPI0_RX            :1,  // [13]    .... SPI0 Rx (0: SDMA0, 1: DMA0)
			HSI_TX             :1,  // [14]    .... HSI Tx (0: SDMA0, 1: DMA0)
			HSI_RX             :1,  // [15]    .... HSI Rx (0: SDMA0, 1: DMA0)
			PCM1_TX            :1,  // [16]     .... PCM1 Tx (0: SDMA1, 1: DMA1)
			PCM1_RX            :1,  // [17]     .... PCM1 Rx (0: SDMA1, 1: DMA1)
			I2S1_TX            :1,  // [18]     .... I2S1 Tx (0: SDMA1, 1: DMA1)
			I2S1_RX            :1,  // [19]     .... I2S1 Rx (0: SDMA1, 1: DMA1)
			SPI1_TX            :1,  // [20]     .... SPI1 Tx (0: SDMA1, 1: DMA1)
			SPI1_RX            :1,  // [21]     .... SPI1 Rx (0: SDMA1, 1: DMA1)
			AC_PCMOUT          :1,  // [22]     .... PCM output (0: SDMA1, 1: DMA1)
			AC_PCMIN           :1,  // [23]     .... PCM input (0: SDMA1, 1: DMA1)
			AC_MICIN           :1,  // [24]     .... IrDA (0: SDMA1, 1: DMA1)
			PWM                :1,  // [25]     .... IrDA (0: SDMA1, 1: DMA1)
			IRDA               :1,  // [26]     .... IrDA (0: SDMA1, 1: DMA1)
			EXTERNAL           :1,  // [27]     .... external (0: SDMA1, 1: DMA1)
			reserved1          :2,  // [29:28]
			SECURITY_RX        :1,  // [30]     .... security Rx (always selects SDMA1 regardless of SECURITY_TX field)
			SECURITY_TX        :1;  // [31]     .... security Tx (always selects SDMA1 regardless of SECURITY_TX field)
	} SDMA_SEL;

	/* 0x114 */
	v32 reserved2;

	/* 0x118 */
	struct {
		v32 Pass               :4,  // [3:0]   Layout revision
			Revision           :4,  // [7:4]   Specification revision
			ID                 :24; // [31:8]  ID
	} SYS_ID;

	/* 0x11C */
	struct {
		v32 MODEM_TX0_SDMA_SEL :1,  // [0] DMA selection for MODEM TX0 (0: SDMA0, 1: DMA0)
			MODEM_TX1_SDMA_SEL :1,  // [1] DMA selection for MODEM TX1 (0: SDMA0, 1: DMA0)
			MODEM_RX0_SDMA_SEL :1,  // [2] DMA selection for MODEM RX0 (0: SDMA0, 1: DMA0)
			MODEM_RX1_SDMA_SEL :1,  // [3] DMA selection for MODEM RTX1 (0: SDMA0, 1: DMA0)
			reserved1          :1,  // [4]
			PMU_IRQ_ENABLE     :1;  // [5] Enable ARM nPMUIRQ (Performance Monitor Unit IRQ) interrupt (0: disable, 1: enable)
	} SYS_OTHERS;

	/* 0x120 */
	struct {
		v32 MP0_CS_CFG         :6,  // [5:0]   Set static memory chip selection multiplexing of memory port0
			reserved1          :1,  // [6]
			ADDR_EXPAND        :1,  // [7]     Set usage of Xm1DATA[31:16] pins
			EBI_FIX_PRI        :3,  // [10:8]  Set EBI fixed priority setting
			EBI_PRI            :1,  // [11]    Set EBI priority scheme
			BUS_WIDTH          :1,  // [12]    Select initial state of SROMC CS0 memory bus width. 0:  8-bit data width, 1: 16-bit data width.
			reserved2          :1,  // [13]
			INDEP_CF           :1;  // [14]    Use CF interface independently.0: Use memory port 0 shared by EBI, 1: Use independent CF interface			
	} MEM_SYS_CFG;

	/* 0x124 */
	v32 reserved3;

	/* 0x128 */
	v32 QOS_OVERRIDE1;   // [15:0] v32 QOS_OV_ID : Override Quality of Service for DMC1

	/* 0x12C */
	struct {
		v32 reserved1          :1,  // [0]
			CFG_BUS_WIDTH,     :1,  // [1]     Show SROMC CS0 memory bus width initial setting.0: 8-bit, 1: 16-bit
			reserved2          :2,  // [3:2]
			CFG_ADDR_EXPAND    :1,  // [4]     Show whether Xm1DATA[31:16] pins are used for SROMC address field or not
			CFG_BOOT_LOC       :2,  // [6:5]   Show with which area 0x00000000 address area is aliased
			reserved3          :1,  // [7]
			CFG_NOR_BOOT       :1,  // [8]     0: 16-bit width NOR booting is not selected, 1 = 16-bit width NOR booting is selected
			CFG_MUX_FLASH      :1,  // [9]     NAND Flash type setting. 0: Use OneNAND Controller, 1: reserved
			CFG_INDEP_CF       :1,  // [10]    Show CF interface independently
			reserved4          :1,  // [11]
			CFG_FIX_PRI_TYPE   :3,  // [14:12] Current EBI fixed priority setting. See the EBI_FIX_PRI field of MEM_SYS_CFG register
			CFG_PRI_TYPE       :1;  // [15]    Current EBI priority scheme. See the EBI_PRI field of MEM_SYS_CFG register			
	} MEM_CFG_STAT;

	/* 0x130 */
	v32 reserved4[437];

	/* 0x804 */
	struct {
		v32 OSC27_EN           :1,  // [0]     Control 27MHz X-tal oscillator pad (0: clock disable, 1: clock enable)
			reserved1          :1,  // [1]
			CFG_BATF_WKUP      :1,  // [2]     Configure wakeup source after XnBATF is cleared. 1: use all SLEEP wakeup sources as BATF_WKUP sources
			CFG_BATFLT         :2,  // [4:3]   Configure nBAT_FLT 00: ignore, 01 : generate interrupt (BATF_WAKEUP_MASK field has 1), 11:  SLEEP mode or E-SLEEP mode
			CFG_STANDBYWFI     :2,  // [6:5]   Configure ARM1176 STADNBYWFI. 00: ignore, 01: IDLE mode, 10: STOP mode, 11: SLEEP mode
			BATF_WAKEUP_MASK   :1,  // [7]     BATF wake-up source (0: disable, 1: use as a wakeup source when CFG_BATFLT field has 01)
			KEY_WAKEUP_MASK    :1,  // [8]     Key pad wake-up source (0: use as a wakeup source, 1: disable)
			MSM_WAKEUP_MASK    :1,  // [9]     MSM modem wake-up source (0: use as a wakeup source, 1: disable)
			ALARM_WAKEUP_MASK  :1,  // [10]    RTC alarm wake-up source (0: use as a wakeup source, 1: disable)
			TICK_WAKEUP_MASK   :1,  // [11]    RTC TICK wake-up source (0: use as a wakeup source, 1: disable)
			TS_WAKEUP_MASK     :1,  // [12]    Touch screen wake-up source (0: use as a wakeup source, 1: disable)
			HSI_WAKEUP_MASK    :1,  // [13]    HSI wake-up source (0: use as a wakeup source, 1: disable)
			MMC0_WAKEUP_MASK   :1,  // [14]    MMC0 wake-up source (0: use as a wakeup source, 1: disable)
			MMC1_WAKEUP_MASK   :1,  // [15]    MMC1 wake-up source (0: use as a wakeup source, 1: disable)
			MMC2_WAKEUP_MASK   :1,  // [16]    MMC2 wake-up source (0: use as a wakeup source, 1: disable)
			OSCotg_DISABLE     :1;  // [17]    Control OSCotg clock pad (0: clock enable, 1: clock disable)
	} PWR_CFG;

	/* 0x808 */
	v32 EINT_MASK;   // [27:0] EINT_WAKEUP_MASK: External interrupt wake-up mask EINT[27:0] (0: use as a wakeup source, 1: disable) Affects on NORMAL mode. This field must clear when EINT is used as a normal external interrupt source

	/* 0x80C */
	v32 reserved5;

	/* 0x810 */
	struct {
		v32 reserved1         :9,   // [8:0]
			DOMAIN_V          :1,   // [9]   0: LP mode(OFF), 1: active mode(ON)
			DOMAIN_G          :1,   // [10]  0: LP mode(OFF), 1: active mode(ON)
			reserved2         :1,   // [11]
			DOMAIN_I          :1,   // [12]  0: LP mode(OFF), 1: active mode(ON)
			DOMAIN_P          :1,   // [13]  0: LP mode(OFF), 1: active mode(ON)
			DOMAIN_F          :1,   // [14]  0: LP mode(OFF), 1: active mode(ON)
			DOMAIN_S          :1,   // [15]  0: LP mode(OFF), 1: active mode(ON)
			DOMAIN_ETM        :1,   // [16]  0: LP mode(OFF), 1: active mode(ON)
			reserved3         :13,  // [29:17]
			IROM              :1;   // [30]  0: LP mode(OFF), 1: active mode(ON)
	} NORMAL_CFG;

	/* 0x814 */
	struct {
		v32 OSC_EN            :1,   // [0]     Control X-tal oscillator pad in STOP mode (0: disable, 1: enable)
			reserved1         :7,   // [7:1]
			TOP_LOGIC         :1,   // [8]     0: LP mode(Retention), 1: active mode(ON), must be ‘0’ before entering STOP mode
			reserved2         :8,   // [16:9]
			ARM_LOGIC         :1,   // [17]    0: LP mode(OFF), 1: active mode(ON)
			reserved3         :2,   // [19:18]
			TOP_MEMORY        :1,   // [20]
			reserved4         :8,   // [28:21]
			MEMORY_ARM        :1;   // [29]
	} STOP_CFG;

	/* 0x818 */
	v32 SLEEP_CFG;   // [0] OSC_EN: Control X-tal oscillator pad in SLEEP mode (0: disable, 1: enable)

	/* 0x81C */
	struct {
		v32 NFCON_CFG         :1,   // [0] NFCON Memory control (0: Off, 1 : Retention)
			IrDA_CFG          :1,   // [1] IrDA memory control (0: Off, 1 : Retention)
			IROM_CFG          :1,   // [2] IROM control (0: Off, 1 : Retention)
			HSMMC_CFG         :1,   // [3] HSMMC Memory control (0: Off, 1 : Retention)
			OTG_CFG           :1,   // [4] OTG Memory control (0: Off, 1 : Retention)
			HOSTIF_CFG        :1,   // [5] HOST IF Block Memory control (0: Off, 1 : Retention)
			MODEMIF_CFG       :1;   // [6] MODEM IF Block Memory control (0: Off, 1 : Retention)
	} STOP_MEM_CFG;

	/* 0x820 */
	v32 OSC_FREQ;   // [3:0] OSC_FREQ_VALUE: Oscillator frequency scale counter (OSC_FREQ_VALUE / oscillator_frequency > 200ns)

	/* 0x824 */
	struct {
		v32 OSC_CNT_VALUE     :4,   // [3:0]   Oscillation pad stable counter value (Exponential Scale)
			OSC_CNT_VALUE_1   :16;  // [19:4]  Mapping to counter value 19 to 4 when STABLE COUNTER Type is ’1’ (OSC_CNT_VALUE[3:0] should stay at reset values, OSC_CNT_VALUE[19:4] must be smaller than PWR_CNT_VALUE[19:4])
	} OSC_STABLE;

	/* 0x828 */
	struct {
		v32 PWR_CNT_VALUE     :4,   // [3:0]   Power stable counter value (Exponential Scale)
			PWR_CNT_VALUE_1   :16;  // [19:4]  Mapping to counter value 19 to 4 when STABLE COUNTER Type is ’1’ (PWR_CNT_VALUE[3:0] should stay at reset values, OSC_CNT_VALUE[19:4] must be smaller than PWR_CNT_VALUE[19:4]
	} PWR_STABLE;

	/* 0x82C */
	v32 reserved6;

	/* 0x830 */
	struct {
		v32 DOMAIN_TOP        :4,   // [3:0]   Memory power stabilization counter for domain TOP
			DOMAIN_V          :4,   // [7:4]   Memory power stabilization counter for domain V
			DOMAIN_I          :4,   // [11:8]  Memory power stabilization counter for domain I
			DOMAIN_P          :4,   // [15:12] Memory power stabilization counter for domain P
			DOMAIN_F          :4,   // [19:16] Memory power stabilization counter for domain F
			DOMAIN_S          :4,   // [23:20] Memory power stabilization counter for domain S
			DOMAIN_ETM        :4,   // [27:24] Memory power stabilization counter for domain ETM
			DOMAIN_G          :4;   // [31:28] Memory power stabilization counter for domain G
	} MTC_STABLE;

	/* 0x834 */
	v32 reserved7;

	/* 0x838 */
	struct {
		v32	CACHEABLE_AHB_I     :1,  // [0]     Sets whether master transaction from AHB_I sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_F     :1,  // [1]     Sets whether master transaction from AHB_F sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_P     :1,  // [2]     Sets whether master transaction from AHB_P sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_AXI_V :1,  // [3]     Sets whether master write transaction from AXI_V subblock is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_X     :1,  // [4]     Sets whether master transaction from AHB_X sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_T     :1,  // [5]     Sets whether master transaction from AHB_T sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_M     :1,  // [6]     Sets whether master transaction from AHB_M sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			CACHEABLE_AHB_S     :1,  // [7]     Sets whether master transaction from AHB_S sub-block is cacheable or not. 0: non-cacheable ,1: cacheable
			reserved1           :3,  // [10:8]
			CACHEABLE_AHB_CF    :1,  // [11]    Sets whether master transaction from CF controller is cacheable or not. 0: non-cacheable ,1: cacheable
			reserved2           :6,  // [17:12]
			STOP_EXIT_CKE_L     :1,  // [18]    0: Normal, 1: CKE Keep L until Stop Exit
			SYNC667             :1;  // [19]    0 : Normal Moe, 1 : Sync 667MHz Mode
	} MISC_CON;

	/* 0x83C - 0x8FC */
	v32 reserved8[49];

	/* 0x900 */
	struct {
		v32 CP15DISABLE         :1,  // [0]     Disables write asses to some system control processor registers of ARM1176. (0: enable, 1: disable)
			reserved1           :5,  // [5:1]
			SYNCMUXSEL          :1,  // [6]
			SYNCMODE            :1,  // [7]
			SYNCACK             :4,  // [11:8]  SYNC mode acknowledge (Read Only)
			CLEAR_BATF_INT      :1,  // [12]    Clear interrupt caused by battery fault when this bit is set
			CLEAR_DBGACK        :1,  // [13]    Clear DBGACK signal when this field has 1. ARM1176 asserts DBGACK signal to indicate the system has entered Debug state. If DBGACK is asserted, this state is store in SYSCON until software clear it using this field
			reserved2           :2,  // [15:14]
			USB_SIG_MASK        :1,  // [16]    USB signal mask to prevent unwanted leakage.(This bit must set before USB PHY is used.)
			reserved3           :6,  // [22:17]
			STABLE_COUNTER_TYPE :1;  // [23]    Indicate OSC_STABLE, PWR_STABLE counter type. 0: Exponential Scale, 1: Set by SFR
	} OTHERS;
	
	/* 0x904 */
	struct {
		v32 HW_RESET            :1,  // [0] External reset by XnRESET
			reserved1           :1,  // [1]
			WDT_RESET           :1,  // [2] Watch dog timer reset by WDTRST
			SLEEP_WAKEUP        :1,  // [3] Reset by SLEEP mode wake-up
			ESLEEP_WAKEUP      :1,  // [4] Reset by E-SLEEP mode wake-up
			reserved2           :1,  // [5]
			DEEP_STOP_WAKEUP    :1;  // [6] Reset by DEEP_STOP mode wake-up
	} RST_STAT;

	/* 0x908 */
	struct {
		v32 EINT_WAKEUP         :1,  // [0]  Wake-up by external interrupts. This is cleared by writing 1
			RTC_ALARM_WAKEUP    :1,  // [1]  Wake-up by RTC alarm. This is cleared by writing 1.
			RTC_TICK_WAKEUP     :1,  // [2]  Wake-up by tick interrupt. This is cleared by writing 1
			TS_WAKEUP           :1,  // [3]  Wake-up by touch screen. This is cleared by writing 1
			KEY_WAKEUP          :1,  // [4]  Wake-up by Key PAD. This is cleared by writing 1
			MSM_WAKEUP          :1,  // [5]  Wake-up by MSM modem. This is cleared by writing 1
			BATFLT_WAKEUP       :1,  // [6]  Wake-up by battery fault. This is cleared by writing 1
			reserved1           :1,  // [7]
			HSI_WAKEUP          :1,  // [8]  Wake-up by HSI. This is cleared by writing 1
			MMC0_WAKEUP         :1,  // [9]  Wake-up by MMC0. This is cleared by writing 1
			MMC1_WAKEUP         :1,  // [10] Wake-up by MMC1. This is cleared by writing 1
			MMC2_WAKEUP         :1;  // [11] Wake-up by MMC2. This is cleared by writing 1			
	} WAKEUP_STAT;

	/* 0x90C */
	struct {
		v32 BLK_TOP             :1,  // [0] Block TOP power ready
			BLK_V               :1,  // [1] Block V power ready
			BLK_I               :1,  // [2] Block I power ready
			BLK_P               :1,  // [3] Block P power ready
			BLK_F               :1,  // [4] Block F power ready
			BLK_S               :1,  // [5] Block S power ready
			BLK_ETM             :1,  // [6] Block ETM power ready
			BLK_G               :1;  // [7] Block G power ready
	} BLK_PWR_STAT;

	/* 0x910 */
	v32 reserved9[60];
	
	/* 0xA00 - 0xA0C */
	v32 INFORM[4];       // Information register 0~3. User defined information. INFORM0~3 registers are cleared by asserting XnRESET pin

} s3c_syscon_t;


	/* 0x7F002000 */
typedef struct {
	/* 0x00 */
	struct {
		v32 I2SACTIVE           :1,  // [0] IIS interface active (start operation). 0: Inactive, 1: Active
			RXDMAACTIVE         :1,  // [1] Rx DMA active (start DMA request). This bit is set from high to low, the DMA operation will be forced to stop immediately. 0: Inactive, 1: Active
			TXDMACTIVE          :1,  // [2] Tx DMA active (start DMA request). This bit is set from high to low, the DMA operation will be forced to stop immediately. 0: Inactive, 1: Active
			RXCHPAUSE           :1,  // [3] Rx channel operation pause command. This bit is activated at any time, the channel operation will be halted after left-right channel data transfer is completed.0: No pause operation, 1: Pause operation
			TXCHPAUSE           :1,  // [4] Tx channel operation pause command. Note that when this bit is activated at any time, the channel operation will be halted after left-right channel data transfer is completed. 0: No pause operation, 1: Pause operation
			RXDMAPAUSE          :1,  // [5] Rx DMA operation pause command. This bit is activated at any time, the DMA request will be halted after current on-going DMA transfer is completed. 0: No pause DMA operation, 1: Pause DMA operation
			TXDMAPAUSE          :1,  // [6] Tx DMA operation pause command. This bit is activated at any time, the DMA request will be halted after current on-going DMA transfer is completed. 0: No pause DMA operation, 1: Pause DMA operation
			FRXFULL             :1,  // [7] Rx FIFO full status indication. 0: FIFO is not full (ready for receive data from channel), 1: FIFO is full (not ready for receive data from channel)
			FTXFULL             :1,  // [8] Tx FIFO full status indication. 0: FIFO is not full 1: FIFO is full
			FRXEMPT             :1,  // [9] Rx FIFO empty status indication. 0: FIFO is not empty, 1: FIFO is empty
			FTXEMPT             :1,  // [10] Tx FIFO empty status indication. 0: FIFO is not empty (ready for transmit data to channel), 1: FIFO is empty (not ready for transmit data to channel)
			LRI                 :1,  // [11] Left/Right channel clock indication. It is dependent on the value of LRP bit of I2SMOD register
			reserved1           :4,  // [15:12] Reserved. Program to zero
			FTXURINTEN          :1,  // [16] TX FIFO Under-run Interrupt Enable. 0: TXFIFO Under-run INT disable, 1: TXFIFO Under-run INT enable
			FTXURSTATUS         :1,  // [17] TX FIFO under-run interrupt status. And this is used by interrupt clear bit. When this is high, you can do interrupt clear by writing ‘1’. 0: Interrupt didn’t be occurred, 1 : Interrupt was occurred
			FRXORINTEN          :1,  // [18] RX FIFO Over-run Interrupt Enable. 0: RXFIFO Over-run INT disable, 1: RXFIFO Over-run INT enable
			FRXORSTATUS         :1;  // [19] RX FIFO over-run interrupt status. And this is used by interrupt clear bit. When this is high, you can do interrupt clear by writing ‘1’. 0: Interrupt didn’t be occurred, 1: Interrupt was occurred
	} IISCON; /* IIS interface control register */

	/* 0x04 */
	struct {
		v32 reserved1           :1,  // [0]     Reserved. Program to zero
			BFS                 :2,  // [2:1]   Bit clock frequency select. 00: 32 fs, where fs is sampling frequency, 01: 48 fs, 10: 16 fs, 11: 24 fs
			RFS                 :2,  // [4:3]   IIS root clock (codec clock) frequency select. 00: 256 fs, where fs is sampling frequency, 01: 512 fs, 10: 384 fs, 11: 768 fs
			SDF                 :2,  // [6:5]   Serial data format. 00: IIS format, 01: MSB-justified (left-justified) format, 10: LSB-justified (right-justified) format, 11: Reserved
			LRP                 :1,  // [7]     Left/Right channel clock polarity select. 0: Low for left channel and high for right channel, 1: High for left channel and low for right channel
			TXR                 :2,  // [9:8]   Transmit or receive mode select. 00: Transmit only mode, 01: Receive only mode, 10: Transmit and receive simultaneous mode, 11: Reserved
			IMS                 :2,  // [11:10] IIS master (internal/external) or slave mode select. 00: Master mode (, using PCLK), 01: Master mode (, using CLKAUDIO[x]), 10: Slave mode (divide mode, using PCLK), 11: Slave mode (bypass mode, using I2SCLK)
			CDCLKCON            :1,  // [12] Determine codec clock source. 0: Use internal codec clock source, 1: Get codec clock source from external codec chip
			BLC                 :2;  // [14:13] Bit Length Control Bit Which decides transmission of 8/16 bits per audio channel. 00: 16 Bits per channel 01: 8 Bits Per Channel, 10: 24 Bits Per Channel,  11:Reserved
	} IISMOD; /* IIS interface mode register */

	/* 0x08 */
	struct {
		v32 FRXCNT              :5,  // [4:0]   RX FIFO data count. FIFO has 16 depth, so value ranges from 0 to 16. N: Data count N of FIFO
			reserved1           :2,  // [6:5]   Reserved. Program to zero
			RFLUSH              :1,  // [7]     RX FIFO flush command. 0: No flush, 1: Flush
			FTXCNT              :5,  // [12:8]  TX FIFO data count. FIFO has 16 depth, so value ranges from 0 to 16. N: Data count N of FIFO
			reserved2           :2,  // [14:13] Reserved. Program to zero
			TFLUSH              :1;  // [15]    TX FIFO flush command. 0: No flush, 1: Flush
	} IISFIC; /* IIS interface FIFO control register */

	/* 0x0C */
	struct {
		v32 reserved1           :8,  // [7:0]  Reserved. Program to zero
			PSVALA              :6,  // [13:8] Pre-scaler (Clock divider) division value. N: Division factor is N+1
			reserved2           :1,  // [14]   Reserved. SBZ
			PSRAEN              :1;  // [15]   Pre-scaler (Clock divider) active. 0: Inactive, 1: Active
	} IISPSR; /* IIS interface clock divider control register */

	/* 0x10 */
	v32 IISTXD; /* IIS interface transmit data register. TX FIFO write data. The left/right channel data is allocated as the following bit fields. R[31:16],L[15:0] when 16-bit BLC. R[23:16],L[7:0] when 8-bit BLC */

	/* 0x14 */
	v32 IISRXD; /* IIS interface receive data register. RX FIFO read data. The left/right channel data is allocated as the following bit fields. R[31:16],L[15:0] when 16-bit BLC. R[23:16],L[7:0] when 8-bit BLC */
	
} s3c_iis_t;

    /* 0x7E002000 */
typedef struct {
	/* 0x000 */
	v32 CodeRun;  // 0: BIT Processor stop execution, 1: BIT Processor start execution
	
	/* 0x004 */
	struct {
		v32 CodeData            :16, // [15:0]  16-bit BIT code download data
			CodeAddr            :13; // [28:16] 13-bit BIT code download address. BIT code word address (16-bit address) Current design has 4 K code word space (8 KB). Therefore CodeAddr[12:0] must be less than 4095
	} CodeDownLoad;  // Code Download Data register

	/* 0x008 */
	v32 HostIntReq; // Host Interrupt Request to BIT processor. Host can write '1' to this register to request interrupt to BIT. Current firmware version does not use Interrupt from Host to BIT. Therefore this register is not used

	/* 0x00C */
	v32 BitIntClear; // BIT Interrupt Clear. Writing '1' to this register clear BIT interrupt to host

	/* 0x010 */
	v32 bitIntSts;   // BIT Interrupt Status. 1 means that BIT interrupts to host is asserted. This bit is cleared when Host writes bitIntClear Register'1'

	/* 0x014 */
	v32 BitCodeReset; // BIT Code Reset. It host write'1' to this register, program counter of BIT is set to '0'. Therefore restart at initial routine

	/* 0x018 */
	v32 BitCurPc;     // BIT Current PC. Current program counter of BIT processor. This register may be used for only debugging purpose

	/* 0x01C ~ 0xFC */
	v32 reserved1[57];

	/* 0x100 */
	v32 CodeBufAddr;  // CODE Table SDRAM Address. BIT firmware code image start byte address, which resides in SDRAM. Host must set start SDRAM byte address of BIT code image to this register before start executing BIT processor. Current design uses 80 KB for code buffer

	/* 0x104 */
	v32 WorkBufAddr;  // Working Buffer SDRAM Address. BIT processor working buffer SDRAM byte address. Host must reserve working buffer in SDRAM for BIT processor encoding/decoding

	/* 0x108 */
	v32 ParaBufAddr;  // Argument/Return Parameter Buffer SDRAM Address. BIT processor parameter buffer SDRAM byte address. Host must reserve parameter buffer in SDRAM for BIT processor command execution argument and return data. Current design uses 8 KB for parameter buffer

	/* 0x10C */
	struct {
		v32 SelBigEndian            :1, // [0] 0: bit stream buffer is 4 byte little endian format, 1: bit stream buffer is big endian
			BufStsCheckDis          :1, // [1] 0: bit stream buffer overflow/underflow check enable, 1: ... disable. BIT processor stop bit stream loading if bit stream buffer underflow occurs in decoding case and stop bit stream saving if bit stream buffer overflow occurs in encoding case. If this flag is “1”, BIT does not check bit stream buffer overflow/underflow status
			BufPicFlush             :1, // [2] 1: bit stream buffer is flushed at every end of encoding picture. In encoding case, after encoding one picture internal bit stream buffer is flushed to external SDRAM. Therefore entire encoded bit strea data is available to host. 0: internal bit stream buffer is flushed only when internal bit stream buffer is filled to its maximal size (512 byte). Therefore at the end of encoding one picture, the some (less than 512 byte) last encoded data is not flushed to external SDRAM and only resides in internal bit stream buffer. To flush remaining encoded data residing in internal bit stream buffer, host must execute ENC_SEQ_END command. This flag is valid only when [BufPicReset] flag is '0'. In decoding case, this flag is ignored
			BufPicReset             :1, // [3] 1: bit stream buffer is reset at every picture encoding/decoding command. In encoding case, after encoding one picture bit stream buffer is flushed to external SDRAM and next picture encoded data is over-written to the start of bit stream buffer. Therefore host must get encoded data at every end of encoding picture. If this flag is “0”, [BufPicFlush] bit is ignored. In decoding case, this flag is ignored
			EncDynbufAllocEn        :1; // [4] Enable dynamic picture stream buffer allocation in encoder operations. BufPicReset should also be enabled to use this option. If not, this value will be ignored. When this option is enabled, encoder stream buffer can be dynamically allocated at every picture encoding stage. This option will be helpful to achieve higher efficiency of buffering encoded picture stream
	} BitStreamCtrl; // Bit Stream Buffer Control

	/* 0x110 */
	v32 FrameMemCtrl;  // Frame Memory Control. [0]SelBigEndian 0: frame memory is 4 byte little endian format, 1: frame memory is big endian

	/* 0x114 */
	v32 DecFuncCtrl;   // Decoder Function Control. [0]StreamEnd 0: There is more bitstream to be given to the decoder. 1: The whole bitstream has been given to the decoder. In Picture Run state, BIT processor can know the end of bitstream by checking this bit. Host must set this flag after writing the whole bitstream to get the last picture of bitstream. Host also can clear busy state while BIT is waiting for the rest of bitstream corresponding to one picture and get one picture by setting this flag. Host can set/clear this flag at any stage in decoding process after BIT is initialized. Once this bit is set, the decoder will not accept more streams; therefore HOST must clear this flag to 0 before starting decoding and set this flag to 1 after writing the whole bitstream. In Seq init state, this flag is used for signaling to BIT processor to escape from SEQ_INIT stall state. When this bit is set by 1, then BIT processor gives up parsing more stream data and escape from stall state with return value 0 (Fail). This flag is ignored in encoding case.

	/* 0x118 */
	v32 reserved2;

	/* 0x11C */
	v32 BitWorkBufCtrl; // Work Buf Control. [0]WorkBufConfig 0: Work Buffer configurable setting disable. 1: Work Buffer configurable setting enable

	/* 0x120 */
	v32 BitStreamRdPtr0; // Bit Stream Buffer Read Address of Run Index 0. In decode case, current external SDRAM Bit Stream Buffer read address of process index 0 is set to this register by BIT processor. In encode case; host must set current external Bit Stream Buffer read address of process index 0 to this register. This register is updated at every bit stream data load by BIT processor and wrapped around by automatically. Current design load 512 bytes to internal buffer for each transfer. Therefore Bit Stream Read Pointer is increased to 512 after loading data completion

	/* 0x124 */
	v32 BitStreamWrPtr0; // Bit Stream Buffer Write Address of Run Index 0. In decode case; host must set current external Bit Stream Buffer write address of process index 0 to this register. In encode case, current external SDRAM Bit Stream Buffer write address of process index 0 is set to this register by BIT processor. This register is updated at every bit stream data save by BIT processor and wrapped around by automatically. Current design save 512 bytes from internal buffer for each transfer. Therefore Bit Stream Write Pointer is increased to 512 after saving data completion

	/* 0x128 */
	v32 BitStreamRdPtr1; // External SDRAM Bit Stream Buffer read address of process index 1

	/* 0x12C */
	v32 BitStreamWrPtr1; // External SDRAM Bit Stream Buffer write address of process index 1

	/* 0x130 */
	v32 BitStreamRdPtr2; // External SDRAM Bit Stream Buffer read address of process index 2

	/* 0x134 */
	v32 BitStreamWrPtr2; // External SDRAM Bit Stream Buffer write address of process index 2

	/* 0x138 */
	v32 BitStreamRdPtr3; // External SDRAM Bit Stream Buffer read address of process index 3

	/* 0x13C */
	v32 BitStreamWrPtr3; // External SDRAM Bit Stream Buffer write address of process index 3

	/* 0x140 */
	v32 BitStreamRdPtr4; // External SDRAM Bit Stream Buffer read address of process index 4

	/* 0x144 */
	v32 BitStreamWrPtr4; // External SDRAM Bit Stream Buffer write address of process index 4

	/* 0x148 */
	v32 BitStreamRdPtr5; // External SDRAM Bit Stream Buffer read address of process index 5

	/* 0x14C */
	v32 BitStreamWrPtr5; // External SDRAM Bit Stream Buffer write address of process index 5

	/* 0x150 */
	v32 BitStreamRdPtr6; // External SDRAM Bit Stream Buffer read address of process index 6

	/* 0x154 */
	v32 BitStreamWrPtr6; // External SDRAM Bit Stream Buffer writes address of process index 6

	/* 0x158 */
	v32 BitStreamRdPtr7; // External SDRAM Bit Stream Buffer read address of process index 7

	/* 0x15C */
	v32 BitStreamWrPtr7; // External SDRAM Bit Stream Buffer writes address of process index 7

	/* 0x160 */
	v32 BusyFlag;    // Processor Busy Flag. 0: BIT processor is ready for host command. 1: BIT processor is executing host command and not completed yet. Host must check this bit before write RunCommand register. If this bit is ‘1’, host must wait until the value of ‘0’ to set command.
	
	/* 0x164 */
	v32 RunCommand;  // Host writes the command code to this register. Command code: 3’b001 (SEQ_INIT), 3’b010 (SEQ_END), 3’b011 (PICURE_RUN), 3’b100 (SET_FRAME_BUF), 3’b101 (ENCODE HEADER), 3’b110 (ENC PARA SET), 3’b111 (DEC PARA SET), 4’b1001 Reserved, 4’b1010 (MFC_SLEEP), 4’b1011 (MFC_WAKEUP), 4’b1111 (GET F/W VER)

	/* 0x168 */
	v32 RunIndex;    // Run Process Index. Host writes the codec process index to this register before every writing run command. BIT processor can execute max 4 encoding/decoding processes simultaneously. If more than one process is running, each process must be assigned different process index by this register

	/* 0x16C */
	v32 RunCodStd;   // Run Codec Standard. Host writes the codec standard index code to this register before every writing run command. 3’b000 : MPEG4/H.263 DECODER, 3’b001 : MPEG4/H.263 ENCODER, 3’b010 : H.264 DECODER, 3’b011 : H.264 ENCODER, 3’b100 : VC-1 DECODER

	/* 0x170 */
	v32 IntEnable;   // Interrupt Enable Flag register. Each bit of this register is interrupt enable flag of various interrupt. 1: interrupt enable so BIT generates interrupt; 0: interrupt disable 0th bit (LSB),Initialize complete. This interrupt is generated at once after BIT run. 1st bit : SEQ_INIT command execution complete, 2nd bit: SEQ_END..., 3rd bit: PIC_RUN..., 4th bit: SET_FRAME_BUF..., 5th bit: ENC_HEADER..., 6th bit: ENC_PARA_SET..., 7th bit: DEC_PARA_SET, 8th ~ 9th bit: Reserved, 10th bit: MFC_SLEEP, 11th bit : MFC_WAKEUP, 12th ~ 13th bits: Reserved, 14th bit: External bit stream buffer is empty status in decoding case, 15th bit: External bit stream buffer is full status in encoding case
	
	/* 0x174 */
	v32 IntReason;   // Interrupt Reason Flag register. Each bit of this register is interrupt report flag of each interrupt. 1: interrupt is generated, 0: not generated. BIT writes “1” to the bit of each interrupt when generates interrupt request and host may acknowledge which interrupt is generated by reading this register at interrupt service routine. Host is responsible for resetting this register to “0” for next interrupt The interrupt matching of each bit field is same with IntEnable register

	/* 0x178 ~0x17C */
	v32 reserved3[2];

	/* 0x180 ~ 0x1D8 Command I/O registers */
	/* 0x180 */
	/* v32 CMD_ENC_SEQ_BIT_BUF_START; // BitBufAddr. Bitstream buffer SDRAM byte address. Bitstream buffer must be 512 byte-aligned. Host must write this register before executing DEC_SEQ_INIT command */

	/* 0x180 */
	struct {
		v32 PostRotMode     :4,  // [3:0] Post rotation mode. PostRotMode[3:0] = {HorMir, VerMir, RotAng[1:0]}. HorMir : Horizontal mirroring, VerMir : Vertical mirroring, RotAng[1:0], 0 : 0 degree counterclockwise rotate, 1 : 90 degree counterclockwise rotate, 2 : 180 degree counterclockwise rotate, 3 : 270 degree counterclockwise rotate. If this field is 4’b0000, post rotation is enabled but just copy the decoded image
			PostRotEn       :1;  // [4] Post rotation enable. If this field is “1”, the rotated image is stored to DecPicRotAddrY, DecPicRotAddrCb, DecPicRotAddrCr address addition to decoded image store for future reference. If this field is “0”, the post rotation is disabled and PostRotMode field is ignored
	} CMD_DEC_PIC_ROT_MODE; 

	/* 0x180 */
	// v32 CMD_ENC_PIC_SRC_ADDR_Y;  // Encoding source frame address of luminance. Host must write this register before executing ENC_PIC_RUN command. Host must set SDRAM frame buffer start address to this register before every encoding picture
	
	/* 0x184 */
	// v32 CMD_ENC_SEQ_BIT_BUF_SIZE;  //BitBufSize. Bitstream buffer size in kilo bytes count. Host must write this register before executing ENC_SEQ_INIT command. Maximal bitstream buffer size is 4G byte

	/* 0x184 */
	v32 CMD_ENC_PIC_SRC_ADDR_CB;  // Encoding source frame address of Cb
	
	/* 0x184 */
	// v32 DecRotAddrY;  // Rotated display frame address of luminance. If PostRotEn field is “1”, the rotated image is stored to this address In VC-1 mode, this register is not used. The rotated output will be one of the frames which previously allocated by RET_DEC_SEQ_FRAME_NEED(0x1CC)
	
	/* 0x188 */
	/* struct { */
	/* 	v32 Mp4DbkOn            :1,  // [0] Enable MPEG4 De-blocking filter.  */
	/* 		RecorderEn          :1,  // [1] Enable display buffer reordering in H.264 decode case.  */
	/* 		FilePlayEn          :1,  // [2] Enable file-play mode in decoder operation with frame-based streaming */
	/* 		DecDynBufAllocEn    :1,  // [3] Enable dynamic picture stream buffer allocation in file-play mode.  */
	/* 		IgnoreVUI           :1,  // [4] Enable decoding with ignoring VUI syntax element */
	/* 		reserved1           :1, // [5] Set to 0 */
	/* 		VC1_Reorder_Disable :1; // [6] Re-ordering disable in VC-1 decoding */
	/* } CMD_DEC_SEQ_OPTION; */

	/* 0x188 */
	struct {
		v32 MbBitReport         :1, // [0] Bit position of every MB is stored to SDRAM buffer
			SliceInfoReport     :1, // [1] Bit position of every MB is stored to SDRAM buffer
			AUDEnable           :1, // [2] Encode H.264 Access Unit Delimiter RBSP enable
			MbQpReport          :1, // [3] Enable MB QP Store to SDRAM buffer
			reserved1           :1, // [4]
			ConstIntrQp         :1; // [5] Enable I-frame Quantization parameter value setting
	} CMD_ENC_SEQ_OPTION;

	/* 0x188 */
	// v32 CMD_ENC_PIC_SRC_ADDR_CR;  // Encoding source frame address of Cr
	
	/* 0x188 */
	// v32 DecRotAddrCb;  // Rotated display frame address of Cb. In VC-1 mode, this register is not used. The rotated output will be one of the frames which previously allocated by RET_DEC_SEQ_FRAME_NEED(0x1CC)
	
	/* 0x18C */
	//v32 CMD_DEC_SEQ_PRO_BUF;   // Process buffer SDRAM byte address. Process buffer must be 256 byte-aligned. Host must write this register before executing DEC_SEQ_INIT command. Process buffer is used as PS data save buffer for AVC and MV direct prediction buffer for VC1 and not used for mpeg4. Process buffer must be larger than MB number * 4 for VC1. In case of AVC, process buffer size is not estimated. Therefore, host process allocates some amount temporarily

	/* 0x18C */
	v32 CMD_ENC_SEQ_COD_STD;  // Encode Coding Standard. 0: MPEG4 Simple Profile, 1: MPEG4 Short Video Header / H.263+, 2: H.264. Host must writes this register before executing SEQ_INIT command

	/* 0x18C */
	// v32 CMD_ENC_PIC_QS;  // Picture quantized step parameter for encoding process. In MPEG4/H.263 mode, allowed range is 1 to 31. In H.264 mode, allowed range is 0 to 51. If rate control is enabled, this register is ignored. If rate control is disabled, BIT encodes whole MBs in current picture with this value. Host may apply its own picture-level rate control algorithm by regulating this value picture-by-picture basis
	
	/* 0x18C */
	// v32 CMD_DEC_PIC_ROT_ADDR_CR;  // Rotated display frame address of Cr. In VC-1 mode, this register is not used. The rotated output will be one of the frames which previously allocated by RET_DEC_SEQ_FRAME_NEED(0x1CC)
		
	/* 0x190 ~ 0x1A0*/
	//v32 CMD_DEC_SEQ_TMP_BUF[5]; // Temporary buffer SDRAM byte address. Temporary buffer must be 256 byte-aligned. Host must writes this register before executing DEC_SEQ_INIT command. Temporary buffer 1 is used as ACDC prediction buffer for mpeg4 and Intra prediction Y buffer for AVC and ACDC prediction buffer for VC1. Temporary buffer 2 is used as data partition part 1 save buffer for mpeg4 and Intra prediction Cb buffer for AVC and deblocking buffer for VC1. Temporary buffer 3 is used as data partition part 2 save buffer for mpeg4 and Intra prediction Cr buffer for AVC and not used for VC1. Temporary buffer 4 is used as slice information save buffer for AVC and not used for VC1 and mpeg4. Temporary buffer 5 is used as slice information save buffer for AVC and not used for VC1 and mpeg4

	/* 0x190 */
	/* struct { */
	/* 	v32 PictureHeight      :10, // Encode source picture height size in pixel. Source picture height must be a multiple of 16, less than or equal to 576 */
	/* 		PictureWidth       :10; // Encode source picture width size in pixel. Source picture width must be a multiple of 16, less than or equal to 720 */
	/* } CMD_ENC_SEQ_SRC_SIZE; */

	/* 0x190 */
	struct {
		v32 PreRotMode         :4,  // [3:0] Pre-rotation mode. PreRotMode[3:0] = {HorMir, VerMir, RotAng[1:0]}, HorMir: Horizontal mirroring, VerMir: Vertical mirroring RotAng[1:0]. 0 : 0 degree counterclockwise rotate, 1: 90 degree counterclockwise rotate, 2: 180 degree counterclockwise rotate, 3: 270 degree counterclockwise rotate. If this field is 4’b0000, pre-rotation is disabled
			PreRotEn           :1;  // [4]   Pre-rotation enable. If this field is “1”, the source image is rotated prior to encoding, If this field is “0”, the pre-rotation is disabled and PreRotMode field is ignored
	} CMD_ENC_PIC_ROT_MODE;

	/* 0x190 */
	// v32 DecDbkAddrY;  // Deblocked display frame address of luminance. If Mp4DbkOn field is “1”, the deblocked image is stored to this address
	
	/* 0x194 */
	struct {
		v32 FrameRateRes       :16, // Encode source frame rate residual
			FrameRateDivMinus1 :16; // Encode source frame rate unit number in Hz minus 1
	} CMD_ENC_SEQ_SRC_F_RATE;

	/* 0x194 */
	/* struct { */
	/* 	v32 PicSkipEn          :1,  // Picture skip flag. If this field is “1”, EncSrcAddrY, EncSrcAddrCb, EncSrcAddrCr are ignored and one skipped picture is encoded. In that case, the reconstructed image at decoder side is a copy of previous picture The skipped picture is encoded as P type (Inter) picture regardless of [EncGopNum].Host may set this field as “1” when next source frame to be encoded is not available */
	/* 		IdrPic             :1;  // If this field is “1”, the source image is encoded as IDR(Instantaneous Decoding Refresh) picture at H.264 or I(Intra) picture at MPEG-4/H.263 regardless of [EncGopNum] value. The IDR picture is I(Intra) picture with zero frame_num value and all of decoding status(ex. reference picture list) are reset. The first frame in bit stream is encoded IDR picture automatically. After encoding IDR picture, I picture period calculation is reset to initial state */
	/* } CMD_ENC_PIC_OPTION; */
	
	/* 0x194 */
	// v32 DecDbkAddrCb;  // Deblocked display frame address of Cb DEC
	
	/* 0x198 */
	struct {
		v32 DataPartEn          :1,  // [0] 0: Data Partition Disable, 1: Data Patition Enable
			RevVlcEn            :1,  // [1] 0: Normal VLC table used, 1: Reversible VLC table used. This bit is ignored if DataPartEn bit is ‘0’
			IntraDcVlcThr       :3;  // [4:2] MPEG4 Intra DC VLC Threshold code. The allowed range is [0 ~ 7]
	} CMD_ENC_SEQ_MP4_PARA;

	/* 0x198 */
	// v32 CMD_ENC_PIC_BB_START;  // Byte address of the encoder output picture stream buffer. This value will only be valid if encoder dynamic buffer allocation option is enabled as well as stream buffer reset option. In this case, this address will be used as the start address of the bitstream data

	/* 0x198 */
	// v32 DecDbkAddrCr;  // Deblocked display frame address of Cr DEC

	/* 0x19C */
	struct {
		v32 AnnexT              :1,  // [0] 0: Annex T off, 1: Annex T on
			AnnexK              :1,  // [1] 0: Annex K off, 1: Annex K on
			AnnexJ              :1,  // [2] 0: Annex J off, 1: AnnexJ on
			AnnexI              :1;  // [3] 0: Annex I off, 1: AnnexI on. Current design does not supports Annex I for encoding mode. Therefore this flag must be set to 0
	} CMD_ENC_SEQ_263_PARA;

	/* 0x19C */
	// v32 CMD_ENC_PIC_BB_SIZE;  // Byte size of encoded picture stream buffer. This value will only be valid if encoder dynamic buffer allocation option is enabled as well as stream buffer reset option. In this case, it will be used as the pointer of the end of picture stream
	
	/* 0x19C */
	// v32 DecRotStride;  // Rotated display frame Stride

	/* 0x1A0 */
	struct {
		v32 ChromaQpOffset      :5,  // [4:0]   chroma_qp_index_offset in Picture Parameter set range -12 to +12. 2's complement signed 5 bit. 1_0100: -12, 1_0101: -11, …, 0_1100: +12
			ConstIntraFlag      :1,  // [5]     constrained_intra_pred_flag in Picture Parameter set. 0: intra prediction use inter MB data, 1: intra prediction does not use inter MB data
			DisableDeblk        :2,  // [7:6]   disable_deblocking_filter_idc in slice header. 0: enable deblocking filter, 1: disable deblocking filter, 2: enable deblocking filter except slice boundary
			DeblkAlphaOffset    :4,  // [11:8]  slice_alpha_c0_offset_div2 in slice header range -6 to +6. 2’s complement signed 4 bit
			DeblkBetaOffset     :4;  // [15:12] slice_beta_offset_div2 in slice header range -6 to +6. 2’s complement signed 4 bit
	} CMD_ENC_SEQ_AVC_PARA;
		
	/* 0x1A4 */
	//v32 CMD_DEC_SEQ_START_BYTE; // Byte Address of valid bitstream in input stream buffer
	
	/* 0x1A4 */
	struct {
		v32 SliceMode           :1,  // [0] 0: one slicke per picture, 1: multiple slices per picture. MPEG4 mode, re-sync marker and packet header is inserted between slice boundary. H.263 mode with Annex K = 0, GOB header is inserted at every GOB layer start. H.263 mode with Annex K = 1, multiple slices are generated. H.264 mode, multiple slice layer RBSP is generated
			SliceSizeMode       :1,  // [1] 0: Slice is changed by encoded slice bit number. 1: Slice is changed by encoded macro-block number. This bit is ignored if SliceMode bit is 0. In H.263 Mode with Annex K = 0, this bit is ignored
			SlickSizeNum        :14; // [15:2] If SliceSizeMode is 0, macro-block number of one slice must be set to this register. If SliceSizeMode is 1, encoded bit count of one slice must be set to this register. This bit is ignored if SliceMode bit is 0. In H.263 Mode with Annex K = 0, this bit is ignored
	} CMD_ENC_SEQ_SLICE_MODE;
	
	/* 0x1A8 ~ 0x1BC */
	//v32 reserved4[6];

	/* 0x1A8 */
	v32 CMD_ENC_SEQ_GOP_NUM;  // Encode GOP number. I picture is inserted at every GOP picture number. Maximum GOP number is 60. 0 − I, P, P, P, … (only first picture is I); 1 − I, I, I, … (no P picture); 2 − I, P, I, P, …; 3 − I, P, P, I, P, P, I, …

	/* 0x1A8 */
	// v32 DecPicChunkSize;  // Byte size of picture stream data. This value will be only valid in file-play mode, and it will be used as size of stream data when BIT processor update write pointer of picture stream buffer.
	
	/* 0x1AC */
	struct {
		v32 RcEnable            :1,  // [0]     Rate Control Enable. If this flag is set to 0, the value of register PictureQs is used as a Quantization Step in whole sequence
			BitRate             :15, // [15:1]  Target Bit Rate in kilo bit per seconds (kbps). This value is ignored if RcEnable = 0. Maximum allowed value is 32767 (0x7FFF)
			InitDelay           :15, // [30:16] Reference Decoder initial buffer removal delay in millisecond (ms). This value is ignored if RcEnable = 0. Maximum allowed value is 32767 (0x7FFF). 0: do not check Reference decoder buffer delay constraint
			SkipDisable         :1;  // Rate control automatic skip disable. If this flag is “0”, BIT processor may skip one picture if available bits are insufficient for accommodate the bit budget. If this flag is “1”, BIT processor never skip the picture but encoded bitstream may be overflow than target bit rate at hard-to-encode sequences. This flag is ignored if [RcEnable] is “0”
	} CMD_ENC_SEQ_RC_PARA;

	/* 0x1AC */
	// v32 DecPicBitBufStart;  // 4-byte aligned byte address of the decoder input picture stream buffer. This value will only be valid if decoder dynamic buffer allocation option is enabled as well as file-play mode option. In this case, this address will be used as the start address of the bitstream data when BIT processor update write pointer of picture stream buffer
	
	/* 0x1B0 */
	v32 CMD_ENC_SEQ_RC_BUF_SIZE; // Reference Decoder buffer size in bits. This value is ignored if RcEnable = 0 or InitDelay = 0. Maximum allowed value is 0x7FFF_FFFF. 0 : do not check Reference decoder buffer size constraint

	/* 0x1B0 */
	// v32 CMD_DEC_PIC_START_BYTE;  // Byte Address of valid bitstream in input picture stream buffer
	
	/* 0x1B4 */
	v32 CMD_ENC_SEQ_INTRA_MB;   // Intra MB refresh number. Must be less than encoded (PictureHeight*PictureWidt/256). 0: Intra MB refresh is not used, N: At least N number of MBs are encoded as Intra mode at every picture

	/* 0x1B8 */
	struct {
		v32 FmoEnable            :1,  // [0]    0: FMO disable, 1: FMO enable
			FmoSliceNr           :4,  // [4:1]  Number of Slice Groups (It must be a value between 2 and 8)
			FmoType              :1,  // [5]    0: Type0 (interleaved), 1: Type1 (Dispersed)
			reserved1            :2,  // [7:6]
			FmoSliceBufSize      :1;  // [23:8] Fmo slice save work buffer size. This bits means work buffer size for 1 slice in KB size. So, if this bits value is 2, total fmo slice work buffer is 2*8=16KB
	} CMD_ENC_SEQ_FMO;

	/* 0x1BC */
	v32 CMD_ENC_INTRA_QP;  // Intra frame picture quantized step parameter for encoding process. In MPEG-4/H.263 mode, allowed range is 1 to 31. In H.264 mode, allowed range is 0 to 51. If rate control disabled, this value is ignored
	
	/* 0x1C0 */
	//v32 RET_DEC_SEQ_SUCCESS;   // 0: DEC_SEQ_INIT command executed with error, 1: ...successfully

	/* 0x1C0 */
	v32 RET_ENC_SEQ_SUCCESS;  // 0: ENC_SEQ_INIT command executed with error, 1: ...successfully

	/* 0x1C0 */
	// v32 RET_ENC_PIC_FRAME_NUM;  // Encoded frame number. After BIT encodes one frame, BIT increase frame number and then stores frame number to this register

	/* 0x1C0 */
	// v32 RET_DEC_PIC_FRAME_NUM;  // Decoded frame number. After BIT decodes one frame, BIT increase frame number and then stores frame number to this register
	
	/* 0x1C4 */
	struct {
		v32 PictureHeight      :10, // [9:0]   Decoded picture height size in pixel
			PictureWidth       :10; // [19:10] Decoded picture width size in pixel
	} RET_DEC_SEQ_SRC_SIZE;

	/* 0x1C4 */
	// v32 EncPicType;  // The picture type of current encoded picture. 0: I (Intra) picture, 1: P (Inter) pictur
	
	/* 0x1C4 */
	// v32 RET_DEC_PIC_IDX;  // Display frame index. After BIT decodes one frame, BIT return display frame index to this register
	
	/* 0x1C8 */
	struct {
		v32 FrameRateRes       :16, // [15:0]  Decoded picture frame rate residual. Number of time units of a clock operating at thefrequency [FrameRateDiv] Hz
			FrameRateDivMinux1 :16; // [31:16] Decoded picture frame rate unit number in Hz minus 1. [FrameRateDiv] is derived by adding this value to 1
	} RET_DEC_SEQ_SRC_F_RATE;

	/* 0x1C8 */
	// v32 RET_ENC_PIC_IDX;  // Reconstructed frame index. After BIT encodes one frame, BIT return reconstructed frame index to this register.Reconstructed frame is used for reference of future frame
	
	/* 0x1C8 */
	// v32 RET_DEC_PIC_ERR_MB_NUM;  // Error MB number in current decoded picture. If BIT recognizes the stream error, BIT performs error concealment in MB basis and returns concealed MB number in whole picture. If this value is “0”, the frame is decoded with no error
	
	/* 0x1CC */
	v32 RET_DEC_SEQ_FRAME_NEED; // Minimum decoded frame buffer need to decode stream successfully. In MPEG4/H.263 case, this value will be 2 (one for motion compensation reference, one for current frame store). In H.264 case, this value may be bigger than 2 and maximal value may be 18 (16 for reference, 1 for current, 1 for display). Host must reserve frame buffer with the amount of minimum this value. In VC-1 case, the rotated output frame is included in this value. Therefore additional rotated frame is not required when rotator is enabled in VC-1

	/* 0x1CC */
	// v32 RET_DEC_PIC_TYPE;  // The picture type of current decoded picture. 0: I (Intra) picture, 1: P (Inter) picture
	
	/* 0x1D0 */
	v32 RET_DEC_SEQ_FRAME_DELAY; // Maximum display frame buffer delay for buffering decoded picture reorder. BIT processor may delay decoded picture display for display reordering when H.264, pic_order_cnt_type “0” or “1” case or VC-1 decode case

	/* 0x1D0 */
	/* v32 CMD_ENC_SEQ_TMP_BUF_1; // Temporary buffer SDRAM byte address. Temporary buffer must be 256 byte-aligned. Host must write this register before executing ENC_SEQ_INIT command. Temporary buffer 1 is used as ACDC prediction Buffer for Mpeg4 and Intra Prediction Y buffer for AVC. Temporary buffer 1 must be larger than picwidth*8 for
mpeg4, stride*picheight/16 for AVC*/
	
	/* 0x1D4 */
	struct {
		v32 DataPartEn         :1,   // 0: Data Partition Disable, 1: Data Partition Enable. After executing completion of decode DEC_SEQ_INIT command, BIT write to this register with data partition enable flag from decoded sequence header information. In encode case, this register is not used
			RevVlcEn           :1,  // 0: Normal VLC table used, 1: Reversible VLC table used. This bit is ignored if DataPartEn bit is ‘0’
			ShortVideoHeader   :1,  // 0: Normal MPEG4 Stream, 1: Short Video Header Stream
			H263AnnexJ         :1;  // 0: Annex J off, 1: Annex J On
	} RET_DEC_SEQ_INFO;

	/* 0x1D4 */
	/* v32 CMD_ENC_SEQ_TMP_BUF_2;  // Temporary buffer SDRAM byte address. Temporary buffer must be 256 byte-aligned. Host must write this register before executing ENC_SEQ_INIT command. Temporary buffer 2 is used as data partition part 2 save. Buffer for Mpeg4 and Intra Prediction Cb buffer for AVC. Temporary buffer 2 must be larger than (stride/2)*(picheight/2)/8 for AVC. In case of mpeg4, temporary buffer 2 size is not estimated */

	/* 0x1D8 */
	// v32 CMD_ENC_SEQ_TMP_BUF_3; // Temporary buffer SDRAM byte address. Temporary buffer must be 256 byte-aligned. Host must write this register before executing ENC_SEQ_INIT command. Temporary buffer 3 is used as data partition part 3 save. Buffer for Mpeg4 and Intra Prediction Cr buffer for AVC. Temporary buffer 3 must be larger than (stride/2)*(picheight/2)/8 for AVC. In case of mpeg4, temporary buffer 3 size is not estimated

	/* 0x1D8 */
	struct {
		v32 RetStatus         :1,  // [0] 0: DEC_PIC_RUN command executed with error, 1: ... successfully
			InvalidPPS        :1;  // [1] This bit is set when there is no valid PPS in given stream. This bit is set only in file play mode and valid when RetStatus bit is 0
	} RET_DEC_PIC_SUCCESS; 

	/* 0x1DC */
	// v32 CMD_ENC_SEQ_TMP_BUF_4; // Temporary buffer SDRAM byte address. Temporary buffer must be 4 byte-aligned. Host must write this register before executing ENC_SEQ_INIT command. Temporary buffer 4 is used when FMO option is enabled in H.264 encoder. The size of this buffer must be given by (32 KB * FmoSliceNr)

	/* 0x1DC */
	v32 RET_DEC_PIC_CUR_IDX;  // Decoded frame index. After BIT decodes one frame, BIT return decoded frame index to this register. The frame index is the index of array of frame buffer address that host informs by SET_FRAME_BUF command. BIT returns -1 (0xFFFF), if BIT doesn’t decode picture at this picture run command
} s3c_mfc_sfr;

#endif	/*  __HAL_S3C_H__ */
