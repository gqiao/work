#ifndef __SAA7113H_H
#define __SAA7113H_H

#include <linux/i2c.h>
#include <linux/videodev.h>
#include <linux/video_decoder.h>

//#include "rivatv.h"	/* HACK for register dump! */

#define SAA7113H_DEVICE_NAME "SAA7113H"

/*----------------------------------------------------------------------
 *	SAA7113H register map definition
 *----------------------------------------------------------------------*/

/* Number of SAA7113H registers */
#define SAA7113H_NR_REGISTER	99

typedef struct {
	__u8 ID; /* chip version */
} saa7113h_r00_t, saa7113h_chip_version_t;

typedef struct {
	__u8 IDEL : 1; /*
			 bit0:	disable increment delay
			 other: delay value (NOTE 000 is max, 111 is min)
		       */
	__u8 _X_  : 4; 
} saa7113h_r01_t, saa7113h_increment_delay_t;

typedef struct {
	__u8 MODE   : 4; /*
			   select: operational mode
			   0000: CVBS from analog input A11 (automatic gain)
			   0001: CVBS from analog input A12 (automatic gain)
			   0010: CVBS from analog input A21 (automatic gain)
			   0011: CVBS from analog input A22 (automatic gain)
			   0100: reserved
			   0101: reserved
			   0110: luminance from A11 (auto gain), chrominance from
			   .	  A21 (gain adjustable via GAI2). BYPS should be set.
			   0111: luminance from A12 (auto gain), chrominance from
			   .	  A22 (gain adjustable via GAI2). BYPS should be set.
			   1000: luminance from A11 (auto gain), chrominance from
			   .	  A21 (gain adapted from lum.). BYPS should be set
			   1001: luminance from A12 (auto gain), chrominance from
			   .	  A22 (gain adapted from lum.). BYPS should be set
			   XXXX: Other values reserved
			 */

	__u8 GULD   : 2; /* val: update hysterisis for 9 bit gain: +/- 0/1/2/3 LSB */

	__u8 FUSE   : 2; /*
			   select: amplifier & anti-alias activation
			   00: desactivated
			   01: desactivated
			   10: amplifier active
			   11: amplifier & anti-alias both active
			 */
} saa7113h_r02_t, saa7113h_analog_input_control_1_t;


typedef struct {
	__u8 GAI1_MSB : 1; /* MSB of static gain control for channel 1		   */
	__u8 GAI2_MSB : 1; /* MSB of static gain control for channel 2		   */
	__u8 GAFIX    : 1; /*
			     select: gain control adjustment
			     0 : gain controlled by MODE
			     1 : gain is user programmable
			   */

	__u8 HOLDG    : 1; /*
			     select: automatic gain integration
			     0: active
			     1: freeze
			   */

	__u8 WPOFF    : 1; /*
			     select: white peak off
			     0: white peak off active
			     1: white peak off
			   */

	__u8 VBSL     : 1; /*
			     select: AGC hold during vertical blanking period
			     0: short, AGC disabled during equal and serat. pulses)
			     1: long,  AGC disabled until start of video
			     .	line 22 for NTSC, line 24 for PAL
			   */

	__u8 HLNRS    : 1; /*
			     select: HL not referenced select
			     0: normal clamping if decoder is in unlocked state
			     1: reference select if decoder is in unlocked state
			   */

	__u8 _X_      : 1;
} saa7113h_r03_t, saa7113h_analog_input_control_2_t;


typedef struct {
	__u8 GAI1 : 8; /*
			 val: gain control analog; static gain channel 1
			 .    MSB set by GAI1_MSB
			 . 0: ~ -3dB
			 127: ~ 0
			 511: ~ +6dB
		       */
} saa7113h_r04_t, saa7113h_analog_input_control_3_t;

typedef struct {
	__u8 GAI2 : 8; /*
			 val: gain control analog; static gain channel 2
			 .    MSB set by GAI2_MSB
			 . 0: ~ -3dB
			 127: ~ 0
			 511: ~ +6dB
		       */
} saa7113h_r05_t, saa7113h_analog_input_control_4_t;

typedef struct {
	__u8 HSB : 8; /*
			val: horizontal sync start (step size = 8/LLC)
			.    valid range is limited to [-108;+108]
			recommended: 0b11101001
		      */
} saa7113h_r06_t, saa7113h_horizontal_sync_start_t;

typedef struct {
	__u8 HSS : 8; /*
			val: horizontal sync stop (step size = 8/LLC)
			.    valid range is limited to [-108;+108]
			recommended: 0b00001101
		      */
} saa7113h_r07_t, saa7113h_horizontal_sync_stop_t;

typedef struct {
	__u8 VNOI : 2; /*
			 select: vertical noise reduction
			 00: normal mode (recommended)
			 01: fast mode (for stable sources). AUFD must be disabled.
			 10: free running mode (???)
			 11: vertical noise reduction bypassed.
		       */

#define SAA7113H_HPLL_CLOSED 0
#define SAA7113H_HPLL_OPEN   1

	__u8 HPLL : 1; /*
			 select: PLL mode
			 0: closed
			 1: open, horizontal frequency fixed
		       */

#define SAA7113H_HTC_TV   0
#define SAA7113H_HTC_VTR  1
#define SAA7113H_HTC_FAST 3

	__u8 HTC  : 2; /*
			 select: horizontal constant selection
			 00: TV mode (poor quality signals)
			 01: VTR mode (recommended with a detection control circuit
			 .   is connected directly to SAA7113H
			 10: reserved
			 11: fast locking mode (recommended)
		       */

	__u8 FOET : 1; /*
			 select: forced odd/even toggle
			 0: odd/even signal toggles only with interlaced sources
			 1: odd/even signal toggles even if source in non interlaced
		       */

#define SAA7113H_FSEL_50HZ 0
#define SAA7113H_FSEL_60HZ 1

	__u8 FSEL : 1; /*
			 select: field selection
			 0: PAL/SECAM (50Hz, 625 lines)
			 1: NTSC (60Hz, 525 lines)
		       */

#define SAA7113H_AUFD_AUTO  1
#define SAA7113H_AUFD_FIXED 0

	__u8 AUFD : 1; /*
			 select: automatic field (PAL/SECAM/NTSC) detection
			 0: field state directly controlled via FSEL
			 1: field state fixed via FSEL
		       */
} saa7113h_r08_t, saa7113h_sync_control_t;

typedef struct {
	__u8 APER  : 2; /*
			  select: luminance aperture factor
			  00: factor = 0
			  01: factor = 0.25
			  10: factor = 0.5
			  11: factor = 1
			*/

	__u8 UPTCV : 1; /*
			  select: update time for analog AGC value
			  0: horizontal update (once a line)
			  1: vertical update (once per field)
			*/

	__u8 VBLB  : 1; /*
			  select: vertical blanking luminance bypass
			  0: active luminance processing
			  1: chrominance trap and peaking stage are disabled during
			  .  VBI lines determined by VREF=0
			*/

	__u8 BPSS  : 2; /*
			  select: aperture band-pass (centre frequency)
			  00: 4.1 MHz
			  01: 3.8 Mhz, (not to be used with bypass chrominance trap)
			  01: 2.6 Mhz, (not to be used with bypass chrominance trap)
			  01: 2.9 Mhz, (not to be used with bypass chrominance trap)
			*/

	__u8 PREF  : 1; /* bool: prefilter active */
	__u8 BYPS  : 1; /*
			  select:
			  0: chrominance trap active; default for CVBS mode
			  1: chrominance trap bypassed; default for S-Video !!!!
			*/
} saa7113h_r09_t, saa7113h_luminance_control_t;

typedef struct {
#define SAA7113H_BRIG_NTSCJ 0x95
#define SAA7113H_BRIG_CCIR  0x80
	__u8 BRIG : 8; /* val: brightness level */
} saa7113h_r0a_t, saa7113h_luminance_brightness_t;

typedef struct {
#define SAA7113H_CONT_NTSCJ 0x48
	__u8 CONT : 8; /* 
			  val: contrast gain
			  0b10000000: -2 (inverse luminance)
			  0b11000000: -1 (inverse luminance)
			  0:	      0	 (luminance off)
			  64->127:    1->1.999
			  0b01000111: 1.109 (CCIR value)
		       */
} saa7113h_r0b_t, saa7113h_luminance_contrast_t;

typedef struct {
	__u8 SATN : 8; /* 
			  val: chrominance saturation control gain
			  0b10000000: -2 (inverse luminance)
			  0b11000000: -1 (inverse luminance)
			  64->127:    0->1.999
			  0b01000000: 1.0 (CCIR value)
		       */
} saa7113h_r0c_t, saa7113h_chrominance_saturation_t;

typedef struct {
	__u8 HUE : 8; /*
			val: chrominance hue control (phase). SIGNED
			-128: -180 degrees
			+127: +178.6 degrees
		      */
} saa7113h_r0d_t, saa7113h_chrominance_hue_control_t;

typedef struct {
	__u8 CHBW : 2; /*
			 select: chrominance bandwidth
			 00: small bandwidth (~ 620 KHz)
			 01: nominal bandwidth (~ 800 KHz)
			 10: medium bandwidth (~ 920 Mhz)
			 11: wide bandwidth (~ 1000 Mhz)
		       */

	__u8 FCTC : 1; /*
			 select: fast colour time constant
			 0: nominal time constant
			 1: fast time constant
		       */

	__u8 DCCF : 1; /*
			 select: disable chrominance comb filter
			 0: filter on (during lines determined by VREF=1)
			 1: filter permanently off
		       */

#define SAA7113H_CSTD_PALB   0
#define SAA7113H_CSTD_NTSCM  0
#define SAA7113H_CSTD_NTSCJ  0
#define SAA7113H_CSTD_NTSC50 1
#define SAA7113H_CSTD_PAL60  1
#define SAA7113H_CSTD_PALN   2
#define SAA7113H_CSTD_NTSC60 2
#define SAA7113H_CSTD_NTSCN  3
#define SAA7113H_CSTD_PALM   3
#define SAA7113H_CSTD_SECAM  5

	__u8 CSTD : 3; /* 
			  select: colour standard definition
			  000: PAL BGHIN	 / NTSC-M or NTSC-Japan
			  .    for Japan: brightness=0x95, contrast=0x48
			  001: NTSC 4.43 (50 Hz) / PAL 4.43 (60 Hz)
			  010: combination PAL-N / NTSC 4.43 (60Hz)
			  011: NTSC-N		 / PAL M
			  101: SECAM		 / reserved
			  xxx: other values should not be set (SECAM ME ?)
		       */

	__u8 CDT0 : 1; /*
			 select: clear DTO
			 0: disabled
			 1: every time CDTO carrier is set, the internal subcarrier
			 .  DTO phase is reset to 0 (degrees) and the RTCO output
			 .  generates a logic 0 at time slot 68... blah blah
		       */
} saa7113h_r0e_t, saa7113h_chrominance_control_t;

typedef struct {
	__u8 CGAIN : 7; /*
			  val: chrominance gain control 0.5 -> 7.5
			  0b0100100: nominal gain
			*/
		  
	__u8 ACGC  : 1; /*
			  select: automatic gain control
			  0: on
			  1: programmable gain via CGAIN
			*/
} saa7113h_r0f_t, saa7113h_chrominance_gain_control_t;

typedef struct {
	__u8 YDEL : 3; /* val: luminance delay compensation: -4 -> +3 */

	__u8 VRLN : 1; /*
			 select: VREF pulse position and length (ITU number in par.)
			 0 60Hz: len=240 f1=19(22)->258(261) f2=282(285)->521(524)
			 0 50Hz: len=286 f1=24->309 f2=337->622
			 1 60Hz: len=242 f1=19(21)->259(262) f2=281(284)->522(525)
			 1 50Hz: len=288 f1=23->310 f2=336->623
		       */

	__u8 HDEL : 2; /* val: fine position of HS: 0->3 */

	__u8 OFTS : 2; /*
			 select: output format selection
			 00: standard ITU 656 format
			 01: V-flag in SAV/EAV generated by VREF
			 10: V-flag in SAV/EAV generated by data type
			 11: reserved
		       */
} saa7113h_r10_t, saa7113h_format_delay_control_t;

typedef struct {
	__u8 COLO  : 1; /* bool: enable colour */

	__u8 VIPB  : 1; /*
			  select: YUV decoder bypassed
			  0: processed data to VPO output
			  1: ADC data to VPO output depending on mode settings
			*/

#define SAA7113H_OERT_DISABLE 0
#define SAA7113H_OERT_ENABLE  1

	__u8 OERT  : 1; /*
			  select: output enable real-time
			  0: RTS0, RTS1, RTCO high inpedance inputs
			  1: RTS0, RTS1, RTCO active if RTSE13 to RTSE10 = 0000
			*/

#define SAA7113H_OEYC_DISABLE 0
#define SAA7113H_OEYC_ENABLE  1

	__u8 OEYC  : 1; /*
			  select: output enable YUV data
			  0: VPO high impedance
			  1: output VPO active or controlled by RTS1
			*/

	__u8 HLSEL : 1; /*
			  select: selection of horizontal lock indicator for RTS0,
			  .	  RTS1
			  0: standard horizontal lock indicator (low-passed)
			  1: fast lock indicator (use recommended for hi-perf
			  .  signals)
			*/

	__u8 GPSW0 : 1; /* bool: general purpose switch on RTS0 if RTSE=0x0010 */

	__u8 CM99  : 1; /*
			  select: CM99 compatibility to SAA7199
			  0: default value
			  1: to be set only if SAA7199 present
			*/

	__u8 GPSW1 : 1; /* bool: general purpose switch on RTS1 if RTSE=0x0010 */
} saa7113h_r11_t, saa7113h_output_control_1_t;

typedef struct {
	__u8 RTSE0 : 4; /*
			  select: RTS1 output control:
			  0000 - 3-state, RTS1 used as dot input
			  0x0C,
			  .	 VIPB = 0: reserved
			  .	 VIPB = 1: LSBs of the 9-bit ADCs
			  0001 - GPSW1 ???
			  0011 - horizontal lock:
			  .	 HLSEL = 0: standard horizontal lock indicator
			  .	 HLSEL = 1: fast mode (not for VCRs)
			  NOT COMPLETE
			*/
	__u8 RTSE1 : 4;
} saa7113h_r12_t, saa7113h_output_control_2_t;

typedef struct {
	__u8 AOSL  : 2; /*
			  select: AOUT connected to:
			  00 - internal test point 1
			  01 - input AD1
			  02 - input AD2
			  03 - internal test point 2
			*/
	__u8 _X_   : 1;
	__u8 FIDP  : 1; /*
			  select: field ID polarity if RTS set to 0x1111:
			  0 - default
			  1 - inverted
			*/
	__u8 OLDSB : 1; /* bool: old status byte (compatibility) */
	__u8 _XX_  : 2;
	__u8 ADLSB : 1; /* A2D output bits on VPO in bypass mode (VIPB=1, test) */
} saa7113h_r13_t, saa7113h_output_control_3_t;

typedef __u8 saa7113h_r14_t; /* RESERVED */

typedef struct {
	__u8 VSTA : 8;
} saa7113h_r15_t, saa7113h_VGATE_start_t;

typedef struct {
	__u8 VSTO : 8;
} saa7113h_r16_t, saa7113h_VGATE_stop_t;

typedef struct {
	__u8 VSTA_MSB : 1; /* bit: MSB of VSTA */
	__u8 VSTO_MSB : 1; /* bit: MSB of VSTB */
	__u8 _X_      : 6;
} saa7113h_r17_t, saa7113h_VGATE_MSB_t;

typedef __u8 saa7113h_r18_t;
typedef __u8 saa7113h_r19_t;
typedef __u8 saa7113h_r1a_t;
typedef __u8 saa7113h_r1b_t;
typedef __u8 saa7113h_r1c_t;
typedef __u8 saa7113h_r1d_t;
typedef __u8 saa7113h_r1e_t;

typedef struct {
	__u8 RDCAP : 1; /* bool: ready to capture (PLLs locked)		      */
	__u8 COPRO : 1; /* bool: copy protected source detected		      */
	__u8 WIPA  : 1; /* bool: white peak loop active			      */
	__u8 GLIMB : 1; /* bool: channel's luminance gain has a minimum limit */
	__u8 GLIMT : 1; /* bool: channel's luminance gain has a maximum limit */
	__u8 FIDT  : 1; /* flag: detected frequency: 0 = 50Hz / 1 = 60Hz      */
	__u8 HLVLN : 1; /* bool: horiz & vert PLLs locked		      */
	__u8 INTL  : 1; /* bool: interlace detected			      */
} saa7113h_r1f_t, saa7113h_decoder_status_t;

typedef __u8 saa7113h_r20_t;
typedef __u8 saa7113h_r21_t;
typedef __u8 saa7113h_r22_t;
typedef __u8 saa7113h_r23_t;
typedef __u8 saa7113h_r24_t;
typedef __u8 saa7113h_r25_t;
typedef __u8 saa7113h_r26_t;
typedef __u8 saa7113h_r27_t;
typedef __u8 saa7113h_r28_t;
typedef __u8 saa7113h_r29_t;
typedef __u8 saa7113h_r2a_t;
typedef __u8 saa7113h_r2b_t;
typedef __u8 saa7113h_r2c_t;
typedef __u8 saa7113h_r2d_t;
typedef __u8 saa7113h_r2e_t;
typedef __u8 saa7113h_r2f_t;
typedef __u8 saa7113h_r30_t;
typedef __u8 saa7113h_r31_t;
typedef __u8 saa7113h_r32_t;
typedef __u8 saa7113h_r33_t;
typedef __u8 saa7113h_r34_t;
typedef __u8 saa7113h_r35_t;
typedef __u8 saa7113h_r36_t;
typedef __u8 saa7113h_r37_t;
typedef __u8 saa7113h_r38_t;
typedef __u8 saa7113h_r39_t;
typedef __u8 saa7113h_r3a_t;
typedef __u8 saa7113h_r3b_t;
typedef __u8 saa7113h_r3c_t;
typedef __u8 saa7113h_r3d_t;
typedef __u8 saa7113h_r3e_t;
typedef __u8 saa7113h_r3f_t;

typedef struct {
	__u8 _X_    : 1;
	__u8 _SLCK_ : 2; /*
			   select: data slicer clock selection
			   00: reserved
			   01: 13.5 MHz (default)
			   10: reserved
			   11: reserved
			 */

	__u8 _AMPS_ : 1; /* bool: amplitude search active */

	__u8 _XX_   : 1;

	__u8 FCE    : 1; /*
			   select: framing code error
			   0: one framing code error allowed
			   1: no framing code errors allowed
			 */

	__u8 HAM_N  : 1; /*
			   select: hamming check
			   0: hamming check for 2 bytes after framming code (default)
			   1: no hamming check
			 */

	__u8 FISET  : 1; /*
			   select: field size select
			   0: 50 Hz
			   1: 60 Hz
			 */
} saa7113h_r40_t, saa7113h_slicer_control_1_t;

/*
  LCR registers

  depending on the value for FOFF:
  0: field1 = D7->D4 field2 = D0->D3
  1: field2 = D0->D3 field1 = D7->D4
  
  0000: WST625
  0001: European closed caption
  0010: video programming service
  0011: wide screen signaling bits
  0100: US teletext
  0101: US closed caption
  0110: video component signal, VBI region
  0111: oversampled CVBS data
  1000: general text
  1001: VITC625
*/

typedef struct {
	__u8 LCR : 8;
} saa7113h_r41_t, saa7113h_line_control_2_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r42_t, saa7113h_line_control_3_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r43_t, saa7113h_line_control_4_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r44_t, saa7113h_line_control_5_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r45_t, saa7113h_line_control_6_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r46_t, saa7113h_line_control_7_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r47_t, saa7113h_line_control_8_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r48_t, saa7113h_line_control_9_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r49_t, saa7113h_line_control_10_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4a_t, saa7113h_line_control_11_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4b_t, saa7113h_line_control_12_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4c_t, saa7113h_line_control_13_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4d_t, saa7113h_line_control_14_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4e_t, saa7113h_line_control_15_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r4f_t, saa7113h_line_control_16_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r50_t, saa7113h_line_control_17_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r51_t, saa7113h_line_control_18_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r52_t, saa7113h_line_control_19_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r53_t, saa7113h_line_control_20_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r54_t, saa7113h_line_control_21_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r55_t, saa7113h_line_control_22_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r56_t, saa7113h_line_control_23_t;

typedef struct {
	__u8 LCR : 8;
} saa7113h_r57_t, saa7113h_line_control_24_t;

typedef struct {
	__u8 FC : 8;
} saa7113h_r58_t, saa7113h_programmable_framming_code_t;

typedef struct {
	__u8 HOFF : 8;
} saa7113h_r59_t, saa7113h_slicer_horizontal_offset_t;

typedef struct {
	__u8 VOFF : 8;
} saa7113h_r5a_t, saa7113h_slicer_vertical_offset_t;

typedef struct {
	__u8 HOFF_MSB : 2;
	__u8 _X_      : 1;
	__u8 VOFF_MSB : 1;
	__u8 _XX_     : 2;
	__u8 FOFF     : 1;
} saa7113h_r5b_t, saa7113h_slicer_extra_offsets_t;

typedef __u8 saa7113h_r5c_t;
typedef __u8 saa7113h_r5d_t;

typedef struct {
	__u8 SDID : 6;
	__u8 _X_  : 2;
} saa7113h_r5e_t, saa7113h_sliced_data_id_code_t;

typedef __u8 saa7113h_r5f_t;

typedef struct {
	__u8 _X_  : 2;
	__u8 CV	  : 1;
	__u8 PPV  : 1;
	__u8 VPSV : 1;
	__u8 FC7V : 1;
	__u8 FC8V : 1;
	__u8 _XX_ : 1;
} saa7113h_r60_t, saa7113h_slicer_status_1_t;

typedef struct {
	__u8 LNHI  : 5;
	__u8 F21_N : 1;
	__u8 _X_   : 2;
} saa7113h_r61_t, saa7113h_slicer_status_2_t;

typedef struct {
	__u8 DT	  : 4;
	__u8 LNLO : 4;
} saa7113h_r62_t, saa7113h_slicer_status_3_t;


typedef struct {
	/* DECODER */
	saa7113h_r00_t chip_version;
	saa7113h_r01_t increment_delay;
	saa7113h_r02_t analog_input_control_1;
	saa7113h_r03_t analog_input_control_2;
	saa7113h_r04_t analog_input_control_3;
	saa7113h_r05_t analog_input_control_4;
	saa7113h_r06_t horizontal_sync_start;
	saa7113h_r07_t horizontal_sync_stop;
	saa7113h_r08_t sync_control;
	saa7113h_r09_t luminance_control;
	saa7113h_r0a_t luminance_brightness;
	saa7113h_r0b_t luminance_contrast;
	saa7113h_r0c_t chrominance_saturation;
	saa7113h_r0d_t chrominance_hue_control;
	saa7113h_r0e_t chrominance_control;
	saa7113h_r0f_t chrominance_gain_control;
	saa7113h_r10_t format_delay_control;
	saa7113h_r11_t output_control_1;
	saa7113h_r12_t output_control_2;
	saa7113h_r13_t output_control_3;
	saa7113h_r14_t reserved_r14;
	saa7113h_r15_t VGATE_start;
	saa7113h_r16_t VGATE_stop;
	saa7113h_r17_t VGATE_MSB;
	saa7113h_r18_t reserved_r18;
	saa7113h_r19_t reserved_r19;
	saa7113h_r1a_t reserved_r1a;
	saa7113h_r1b_t reserved_r1b;
	saa7113h_r1c_t reserved_r1c;
	saa7113h_r1d_t reserved_r1d;
	saa7113h_r1e_t reserved_r1e;
	saa7113h_r1f_t decoder_status;

	/* SECRET */
	saa7113h_r20_t reserved_r20;
	saa7113h_r21_t reserved_r21;
	saa7113h_r22_t reserved_r22;
	saa7113h_r23_t reserved_r23;
	saa7113h_r24_t reserved_r24;
	saa7113h_r25_t reserved_r25;
	saa7113h_r26_t reserved_r26;
	saa7113h_r27_t reserved_r27;
	saa7113h_r28_t reserved_r28;
	saa7113h_r29_t reserved_r29;
	saa7113h_r2a_t reserved_r2a;
	saa7113h_r2b_t reserved_r2b;
	saa7113h_r2c_t reserved_r2c;
	saa7113h_r2d_t reserved_r2d;
	saa7113h_r2e_t reserved_r2e;
	saa7113h_r2f_t reserved_r2f;
	saa7113h_r30_t reserved_r30;
	saa7113h_r31_t reserved_r31;
	saa7113h_r32_t reserved_r32;
	saa7113h_r33_t reserved_r33;
	saa7113h_r34_t reserved_r34;
	saa7113h_r35_t reserved_r35;
	saa7113h_r36_t reserved_r36;
	saa7113h_r37_t reserved_r37;
	saa7113h_r38_t reserved_r38;
	saa7113h_r39_t reserved_r39;
	saa7113h_r3a_t reserved_r3a;
	saa7113h_r3b_t reserved_r3b;
	saa7113h_r3c_t reserved_r3c;
	saa7113h_r3d_t reserved_r3d;
	saa7113h_r3e_t reserved_r3e;
	saa7113h_r3f_t reserved_r3f;

	/* SLICER */
	saa7113h_r40_t slicer_control_1;
	saa7113h_r41_t line_control_2;
	saa7113h_r42_t line_control_3;
	saa7113h_r43_t line_control_4;
	saa7113h_r44_t line_control_5;
	saa7113h_r45_t line_control_6;
	saa7113h_r46_t line_control_7;
	saa7113h_r47_t line_control_8;
	saa7113h_r48_t line_control_9;
	saa7113h_r49_t line_control_10;
	saa7113h_r4a_t line_control_11;
	saa7113h_r4b_t line_control_12;
	saa7113h_r4c_t line_control_13;
	saa7113h_r4d_t line_control_14;
	saa7113h_r4e_t line_control_15;
	saa7113h_r4f_t line_control_16;
	saa7113h_r50_t line_control_17;
	saa7113h_r51_t line_control_18;
	saa7113h_r52_t line_control_19;
	saa7113h_r53_t line_control_20;
	saa7113h_r54_t line_control_21;
	saa7113h_r55_t line_control_22;
	saa7113h_r56_t line_control_23;
	saa7113h_r57_t line_control_24;
	saa7113h_r58_t programmable_framming_code;
	saa7113h_r59_t slicer_horizontal_offset;
	saa7113h_r5a_t slicer_vertical_offset;
	saa7113h_r5b_t slicer_extra_offsets;
	saa7113h_r5c_t reserved_r5c;
	saa7113h_r5d_t reserved_r5d;
	saa7113h_r5e_t sliced_data_id_code;
	saa7113h_r5f_t reserved_r5f;
	saa7113h_r60_t slicer_status_1;
	saa7113h_r61_t slicer_status_2;
	saa7113h_r61_t slicer_status_3;
} saa7113h_reg_map_t;

#endif /* __SAA7113H_H */
