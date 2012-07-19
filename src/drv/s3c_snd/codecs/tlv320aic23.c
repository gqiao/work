#define LOG_TAG "tlv320aic23.c"
#include "osal.h"
#include "s3c.h"

/*
 * tlv320aic23.c -- TLV320AIC23 ALSA SoC audio driver
 *
 * Copyright 2005 Openedhand Ltd.
 *
 * Author: Richard Purdie <richard@openedhand.com>
 *
 * Based on TLV320AIC23.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <sound/initval.h>

#ifdef CONFIG_HHBF_FAST_REBOOT
#include <asm/reboot.h>
#endif

#include "tlv320aic23_28.h"

#define AUDIO_NAME "TLV320AIC23"
#define TLV320AIC23_VERSION "v0.12"


//#define TLV320AIC23_DEBUG 1

/* codec private data */
//static unsigned tlv320aic23_sysclk;
void (*hhbf_audio_switch)(int flag) = NULL;
EXPORT_SYMBOL(hhbf_audio_switch);

struct tlv320aic23_srate_reg_info {
	u32 sample_rate;
	u8 control;             /* SR3, SR2, SR1, SR0 and BOSR */
	u8 divider;             /* if 0 CLKIN = MCLK, if 1 CLKIN = MCLK/2 */
};

/*
 * tlv320aic23 register cache
 * We can't read the TLV320AIC23 register space when we
 * are using 2 wire for device control, so we cache them instead.
 */
static const u16 tlv320aic23_reg[] = {
#if 1	//Õý³£¹¤×÷×´Ì¬¼Ä´æÆ÷ÉèÖÃ
	0x0117, 0x0117, 0x0179, 0x0179,	/* 0 */
	0x001c, 0x0005, 0x0001, 0x0042,	/* 4 */
	0x000d, 0x0001, 0x0000, 0x0000,	/* 8 */
	0x0000, 0x0000, 0x0000, 0x0000,	/* 12 */
#else	//ÉÏµçÄ¬ÈÏ¼Ä´æÆ÷ÉèÖÃ
	0x0097, 0x0097, 0x00F9, 0x00F9,		/* 0 */
	0x001A, 0x0004, 0x0007, 0x0001,	/* 4 */
	0x0020, 0x0000, 0x0000, 0x0000,	/* 8 */
	0x0000, 0x0000, 0x0000, 0x0000,	/* 12 */
#endif
};

/*
 * read tlv320aic23 register cache
 */
static inline unsigned int tlv320aic23_read_reg_cache(struct snd_soc_codec *codec, unsigned int reg)
{
	u16 *cache = codec->reg_cache;

	__D("\n");
	
    // mhfan
	if (reg > ARRAY_SIZE(tlv320aic23_reg))
		return -1;

	return cache[reg];
}

/*
 * write tlv320aic23 register cache
 */
static inline void tlv320aic23_write_reg_cache(struct snd_soc_codec *codec, unsigned int reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;

	__D("\n");
	
    // mhfan
	if (reg > ARRAY_SIZE(tlv320aic23_reg))
		return;

	cache[reg] = value;
}

static int tlv320aic23_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int value)
{
	u8 data[2];

	__D("\n");
	
	/* TLV320AIC23 has 7 bit address and 9 bits of data
	 * so we need to switch one data bit into reg and rest
	 * of data into val
	 */

	if ((reg < 0 || reg > 9) && (reg != 15)) {
		__E("%s Invalid register R%d\n", __func__, reg);
		return -1;
	}
	/* data is
	 *   D15..D9 TLV320AIC23 register offset
	 *   D8...D0 register data
	 */
	data[0] = (reg << 1) | ((value >> 8) & 0x0001);
	data[1] = value & 0x00ff;

#ifdef	TLV320AIC23_DEBUG
	if (value == tlv320aic23_read_reg_cache(codec, reg))
		return 0;

	__D(AUDIO_NAME ": 0x%02x%02x, R%02d <= 0x%03x\n",
		data[0], data[1], reg, value);
#endif//TLV320AIC23_DEBUG

	tlv320aic23_write_reg_cache (codec, reg, value);

	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
        __E("%s cannot write %03x to register R%d\n",
			__func__, value, reg);

	return -EIO;
}

#define tlv320aic23_reset(c)	tlv320aic23_write(c, TLV320AIC23_RESET, 0)


static const char *rec_src_text[] = { "Line", "Mic" };
static const char *deemph_text[] = {"None", "32Khz", "44.1Khz", "48Khz"};

static const struct soc_enum rec_src_enum =
	SOC_ENUM_SINGLE(TLV320AIC23_ANLG, 2, 2, rec_src_text);

static const struct snd_kcontrol_new tlv320aic23_rec_src_mux_controls =
	SOC_DAPM_ENUM("Input Select", rec_src_enum);

static const struct soc_enum tlv320aic23_rec_src =
	SOC_ENUM_SINGLE(TLV320AIC23_ANLG, 2, 2, rec_src_text);
static const struct soc_enum tlv320aic23_deemph =
	SOC_ENUM_SINGLE(TLV320AIC23_DIGT, 1, 4, deemph_text);

static const DECLARE_TLV_DB_SCALE(out_gain_tlv, -12100, 100, 0);
static const DECLARE_TLV_DB_SCALE(input_gain_tlv, -1725, 75, 0);
static const DECLARE_TLV_DB_SCALE(sidetone_vol_tlv, -1800, 300, 0);

static int snd_soc_tlv320aic23_put_volsw(struct snd_kcontrol *kcontrol,
										 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	u16 val, reg;

	__D("\n");
	
	val = (ucontrol->value.integer.value[0] & 0x07);

	/* linear conversion to userspace
	 * 000	=	-6db
	 * 001	=	-9db
	 * 010	=	-12db
	 * 011	=	-18db (Min)
	 * 100	=	0db (Max)
	 */
	val = (val >= 4) ? 4  : (3 - val);

	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_ANLG) & (~0x1C0);
	tlv320aic23_write(codec, TLV320AIC23_ANLG, reg | (val << 6));

	return 0;
}

static int snd_soc_tlv320aic23_get_volsw(struct snd_kcontrol *kcontrol,
										 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	u16 val;

	__D("\n");
	
 	val = tlv320aic23_read_reg_cache(codec, TLV320AIC23_ANLG) & (0x1C0);
	val = val >> 6;
	val = (val >= 4) ? 4  : (3 -  val);
	ucontrol->value.integer.value[0] = val;
	return 0;

}

#define SOC_TLV320AIC23_SINGLE_TLV(xname, reg, shift, max, invert, tlv_array) \
	{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,				\
			.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |					\
			SNDRV_CTL_ELEM_ACCESS_READWRITE,							\
			.tlv.p = (tlv_array),										\
			.info = snd_soc_info_volsw, .get = snd_soc_tlv320aic23_get_volsw, \
			.put = snd_soc_tlv320aic23_put_volsw,						\
			.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }

static const struct snd_kcontrol_new tlv320aic23_snd_controls[] = {
	SOC_DOUBLE_R_TLV("Digital Playback Volume", TLV320AIC23_LCHNVOL,
					 TLV320AIC23_RCHNVOL, 0, 127, 0, out_gain_tlv),
	SOC_SINGLE("Digital Playback Switch", TLV320AIC23_DIGT, 3, 1, 1),
	SOC_DOUBLE_R("Line Input Switch", TLV320AIC23_LINVOL,
				 TLV320AIC23_RINVOL, 7, 1, 0),
	SOC_DOUBLE_R_TLV("Line Input Volume", TLV320AIC23_LINVOL,
					 TLV320AIC23_RINVOL, 0, 31, 0, input_gain_tlv),
	SOC_SINGLE("Mic Input Switch", TLV320AIC23_ANLG, 1, 1, 1),
	SOC_SINGLE("Mic Booster Switch", TLV320AIC23_ANLG, 0, 1, 0),
	SOC_TLV320AIC23_SINGLE_TLV("Sidetone Volume", TLV320AIC23_ANLG,
							   6, 4, 0, sidetone_vol_tlv),
	SOC_ENUM("Playback De-emphasis", tlv320aic23_deemph),
};


/* add non dapm controls */
static int tlv320aic23_add_controls(struct snd_soc_codec *codec)
{
	int err, i;

	__D("\n");
	
	for (i = 0; i < ARRAY_SIZE(tlv320aic23_snd_controls); i++) {
		if ((err = snd_ctl_add(codec->card,
							   snd_soc_cnew(&tlv320aic23_snd_controls[i],codec, NULL))) < 0)
			return err;
	}
	return 0;
}

/* PGA Mixer controls for Line and Mic switch */
static const struct snd_kcontrol_new tlv320aic23_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("Line Bypass Switch", TLV320AIC23_ANLG, 3, 1, 0),
	SOC_DAPM_SINGLE("Mic Sidetone Switch", TLV320AIC23_ANLG, 5, 1, 0),
	SOC_DAPM_SINGLE("Playback Switch", TLV320AIC23_ANLG, 4, 1, 0),
};

static const struct snd_soc_dapm_widget tlv320aic23_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", TLV320AIC23_PWR, 3, 1),
	SND_SOC_DAPM_ADC("ADC", "Capture", TLV320AIC23_PWR, 2, 1),
	SND_SOC_DAPM_MUX("Capture Source", SND_SOC_NOPM, 0, 0,
					 &tlv320aic23_rec_src_mux_controls),
	SND_SOC_DAPM_MIXER("Output Mixer", TLV320AIC23_PWR, 4, 1,
					   &tlv320aic23_output_mixer_controls[0],
					   ARRAY_SIZE(tlv320aic23_output_mixer_controls)),
	SND_SOC_DAPM_PGA("Line Input", TLV320AIC23_PWR, 0, 1, NULL, 0),
	SND_SOC_DAPM_PGA("Mic Input", TLV320AIC23_PWR, 1, 1, NULL, 0),

	SND_SOC_DAPM_OUTPUT("LHPOUT"),
	SND_SOC_DAPM_OUTPUT("RHPOUT"),
	SND_SOC_DAPM_OUTPUT("LOUT"),
	SND_SOC_DAPM_OUTPUT("ROUT"),

	SND_SOC_DAPM_INPUT("LLINEIN"),
	SND_SOC_DAPM_INPUT("RLINEIN"),

	SND_SOC_DAPM_INPUT("MICIN"),
};

static const struct snd_soc_dapm_route intercon[] = {
	/* Output Mixer */
	{"Output Mixer", "Line Bypass Switch", "Line Input"},
	{"Output Mixer", "Playback Switch", "DAC"},
	{"Output Mixer", "Mic Sidetone Switch", "Mic Input"},

	/* Outputs */
	{"RHPOUT", NULL, "Output Mixer"},
	{"LHPOUT", NULL, "Output Mixer"},
	{"LOUT", NULL, "Output Mixer"},
	{"ROUT", NULL, "Output Mixer"},

	/* Inputs */
	{"Line Input", "NULL", "LLINEIN"},
	{"Line Input", "NULL", "RLINEIN"},

	{"Mic Input", "NULL", "MICIN"},

	/* input mux */
	{"Capture Source", "Line", "Line Input"},
	{"Capture Source", "Mic", "Mic Input"},
	{"ADC", NULL, "Capture Source"},

};

/* tlv320aic23 related */
static const struct tlv320aic23_srate_reg_info srate_reg_info[] = {
	{4000, 0x06, 1},	/*  4000 */
	{8000, 0x06, 0},	/*  8000 */
	{16000, 0x0C, 1},	/* 16000 */
	{22050, 0x11, 1},	/* 22050 */
	{24000, 0x00, 1},	/* 24000 */
	{32000, 0x0C, 0},	/* 32000 */
	{44100, 0x11, 0},	/* 44100 */
	{48000, 0x00, 0},	/* 48000 */
	{88200, 0x1F, 0},	/* 88200 */
	{96000, 0x0E, 0},	/* 96000 */
};

static int tlv320aic23_add_widgets(struct snd_soc_codec *codec)
{
	int i;

	__D("\n");
	
	for(i = 0; i < ARRAY_SIZE(tlv320aic23_dapm_widgets); i++) {
		snd_soc_dapm_new_control(codec, &tlv320aic23_dapm_widgets[i]);
	}

	/* set up audio path interconnects */
	snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

struct _coeff_div {
	u32 mclk;
	u32 rate;
	u16 fs;
	u8 sr:5;
	u8 usb:1;
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
	/* 8k */
	{12288000, 8000, 1536, 0x6, 0x0},
	{18432000, 8000, 2304, 0x7, 0x0},
	{12000000, 8000, 1500, 0x6, 0x1},

	/*80182 */
	{11289600, 8018, 1408, 0x16, 0x0},
	{16934400, 8018, 2112, 0x17, 0x0},

	/*8.0214k */
	{12000000, 8021, 1496, 0x17, 0x1},

	/*11.025k */
	{11289600, 11025, 1024, 0x18, 0x0},
	{16934400, 11025, 1536, 0x19, 0x0},
	{12000000, 11025, 1088, 0x19, 0x1},
	
	/*12k */
	{12288000, 12000, 1024, 0x8, 0x0},
	{18432000, 12000, 1536, 0x9, 0x0},
	{12000000, 12000, 1000, 0x8, 0x1},

	/* 16k */
	{12288000, 16000, 768, 0xa, 0x0},
	{18432000, 16000, 1152, 0xb, 0x0},
	{12000000, 16000, 750, 0xa, 0x1},

	/* 22.05k */
	{11289600, 22050, 512, 0x1a, 0x0},
	{16934400, 22050, 768, 0x1b, 0x0},
	{12000000, 22050, 544, 0x1b, 0x1},
	
	/*24k */
	{12288000, 24000, 512, 0x1c, 0x0},
	{18432000, 24000, 768, 0x1d, 0x0},
	{12000000, 24000, 500, 0x1c, 0x1},

	/* 32k */
	{12288000, 32000, 384, 0xc, 0x0},
	{18432000, 32000, 576, 0xd, 0x0},
	{12000000, 32000, 375, 0xc, 0x1},	// mhfan

	/* 44.1k */
	{11289600, 44100, 256, 0x10, 0x0},
	{16934400, 44100, 384, 0x11, 0x0},
	{12000000, 44100, 272, 0x11, 0x1},

	/* 48k */
	{12288000, 48000, 256, 0x0, 0x0},
	{18432000, 48000, 384, 0x1, 0x0},
	{12000000, 48000, 250, 0x0, 0x1},

	/* 88.2k */
	{11289600, 88200, 128, 0x1e, 0x0},
	{16934400, 88200, 192, 0x1f, 0x0},
	{12000000, 88200, 136, 0x1f, 0x1},

	/* 96k */
	{12288000, 96000, 128, 0xe, 0x0},
	{18432000, 96000, 192, 0xf, 0x0},
	{12000000, 96000, 125, 0xe, 0x1},
};

static inline int get_coeff(int mclk, int rate)
{
	int i;

	__D("\n");
	
	for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
		if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
			return i;
	}

	__E("tlv320aic23: could not get coeff for mclk %d @ rate %d\n",
		   mclk, rate);
	return -EINVAL;
}

static int tlv320aic23_set_dai_clkdiv(struct snd_soc_codec_dai *codec_dai,
									  int div_id, int div)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 reg;

	__D("\n");
	
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_SRATE) & 0x003f;
	tlv320aic23_write(codec, TLV320AIC23_SRATE, reg | div);

	return 0;
}

static int tlv320aic23_set_dai_fmt(struct snd_soc_codec_dai *codec_dai,
								   unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 iface_reg;

	__D("\n");
	
	iface_reg =
	    tlv320aic23_read_reg_cache(codec, TLV320AIC23_DIGT_FMT) & (~0x03);

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface_reg |= TLV320AIC23_MS_MASTER;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;

	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface_reg |= TLV320AIC23_FOR_I2S;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface_reg |= TLV320AIC23_FOR_DSP;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface_reg |= TLV320AIC23_FOR_LJUST;
		break;
	default:
		return -EINVAL;

	}

	tlv320aic23_write(codec, TLV320AIC23_DIGT_FMT, iface_reg);

	return 0;
}

static int tlv320aic23_set_dai_sysclk(struct snd_soc_codec_dai *codec_dai,
									  int clk_id, unsigned int freq, int dir)
{
	__D("\n");
	
	switch (freq) {
	case 12000000:
		return 0;
	}
	return -EINVAL;
}

static int tlv320aic23_pcm_hw_params(struct snd_pcm_substream *substream,
									 struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd    = substream->private_data;
	struct snd_soc_device      *socdev = rtd->socdev;
	struct snd_soc_codec       *codec  = socdev->codec;
	u16 iface_reg, data;
	u8 count = 0;

	__D("\n");
	
	iface_reg =
	    tlv320aic23_read_reg_cache(codec,
								   TLV320AIC23_DIGT_FMT) & ~(0x03 << 2);

	/* Search for the right sample rate */
	/* Verify what happens if the rate is not supported
	 * now it goes to 96Khz */
	while ((srate_reg_info[count].sample_rate != params_rate(params)) &&
	       (count < ARRAY_SIZE(srate_reg_info))) {
		count++;
	}

	data =  (srate_reg_info[count].divider << TLV320AIC23_CLKIN_SHIFT) |
		(srate_reg_info[count]. control << TLV320AIC23_BOSR_SHIFT) |
		TLV320AIC23_USB_CLK_ON;

	tlv320aic23_write(codec, TLV320AIC23_SRATE, data);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		iface_reg |= (0x01 << 2);
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iface_reg |= (0x02 << 2);
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		iface_reg |= (0x03 << 2);
		break;
	}
	tlv320aic23_write(codec, TLV320AIC23_DIGT_FMT, iface_reg);

	return 0;
}

static int tlv320aic23_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd    = substream->private_data;
	struct snd_soc_device      *socdev = rtd->socdev;
	struct snd_soc_codec       *codec  = socdev->codec;

	__D("\n");
	
	/* set active */
	tlv320aic23_write(codec, TLV320AIC23_ACTIVE, 0x0001);

	return 0;
}

static void tlv320aic23_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd    = substream->private_data;
	struct snd_soc_device      *socdev = rtd->socdev;
	struct snd_soc_codec       *codec  = socdev->codec;

	__D("\n");
	
	/* deactivate */
	if (!codec->active) {
		udelay(50);
		tlv320aic23_write(codec, TLV320AIC23_ACTIVE, 0x0);
	}
}

static int tlv320aic23_mute(struct snd_soc_codec_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	u16 reg;

	__D("\n");
	
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_DIGT);
	if (mute)
		reg |= TLV320AIC23_DACM_MUTE;

	else
		reg &= ~TLV320AIC23_DACM_MUTE;

	tlv320aic23_write(codec, TLV320AIC23_DIGT, reg);

	return 0;
}

static int tlv320aic23_dapm_event(struct snd_soc_codec *codec, int event)
{
	__D("\n");
	
	return 0;
}

static int tlv320aic23_set_bias_level(struct snd_soc_codec *codec,
									  enum snd_soc_bias_level level)
{
	u16 reg;

	__D("\n");

	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_PWR) & 0xff7f;

	switch (level) {
	case SND_SOC_BIAS_ON:
		/* vref/mid, osc on, dac unmute */
		tlv320aic23_write(codec, TLV320AIC23_PWR, (reg & 0x0001));
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		/* everything off except vref/vmid, */
		tlv320aic23_write(codec, TLV320AIC23_PWR, ((reg & 0x0001) | 0x00fc));
		break;
	case SND_SOC_BIAS_OFF:
		/* everything off, dac mute, inactive */
		tlv320aic23_write(codec, TLV320AIC23_ACTIVE, 0x0);
		tlv320aic23_write(codec, TLV320AIC23_PWR, 0xffff);
		break;
	}
	codec->bias_level = level;
	return 0;
}

#define AIC23_RATES     SNDRV_PCM_RATE_8000_96000
#define AIC23_FORMATS   (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
                         SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

struct snd_soc_codec_dai tlv320aic23_dai = {
	.name = "TLV320AIC23",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = AIC23_RATES,
		.formats = AIC23_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = AIC23_RATES,
		.formats = AIC23_FORMATS,},
	.ops = {
		.prepare = tlv320aic23_pcm_prepare,
		.hw_params = tlv320aic23_pcm_hw_params,
		.shutdown = tlv320aic23_shutdown,
	},
	.dai_ops = {
		.digital_mute = tlv320aic23_mute,
		.set_fmt = tlv320aic23_set_dai_fmt,
		.set_sysclk = tlv320aic23_set_dai_sysclk,
		.set_clkdiv = tlv320aic23_set_dai_clkdiv,
	},
};
EXPORT_SYMBOL_GPL(tlv320aic23_dai);

static void tlv320aic23_work(struct work_struct *work)
{
	__D("\n");	
}

static int tlv320aic23_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec  *codec  = socdev->codec;

	__D("\n");
	
	tlv320aic23_write(codec, TLV320AIC23_ACTIVE, 0x0);
	tlv320aic23_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int tlv320aic23_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec  *codec  = socdev->codec;
	int i;
	u16 reg;

	__D("\n");
	
	/* Sync reg_cache with the hardware */
	for (reg = 0; reg < ARRAY_SIZE(tlv320aic23_reg); i++) {
		u16 val = tlv320aic23_read_reg_cache(codec, reg);
		tlv320aic23_write(codec, reg, val);
	}

	tlv320aic23_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	tlv320aic23_set_bias_level(codec, codec->suspend_bias_level);

	return 0;
}

/*
 * initialise the TLV320AIC23 driver
 * register the mixer and dsp interfaces with the kernel
 */
static int tlv320aic23_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->codec;
	int reg, ret = 0;
	
	__D("\n");
	
	codec->name           = "TLV320AIC23";
	codec->owner          = THIS_MODULE;
	codec->read           = tlv320aic23_read_reg_cache;
	codec->write          = tlv320aic23_write;
	codec->set_bias_level = tlv320aic23_set_bias_level;
	codec->dapm_event     = tlv320aic23_dapm_event;
	codec->dai            = &tlv320aic23_dai;
	codec->num_dai        = 1;

	codec->reg_cache      = (void*)tlv320aic23_reg;
	codec->reg_cache_size = ARRAY_SIZE(tlv320aic23_reg);

	tlv320aic23_write(codec, TLV320AIC23_RESET, 0);

	/* register pcms */
	if ((ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1)) < 0) {
		__E("tlv320aic23: failed to create pcms\n");
		goto pcm_err;
	}

	/*  REG 15 : Reset codec*/
	tlv320aic23_write(codec, TLV320AIC23_RESET, 0);
	/* power on device */
	/*  REG 6 : Power Down Control*/
	tlv320aic23_set_bias_level(codec, SND_SOC_BIAS_ON);


	/* Unmute input */
	/* REG 0/1  : Unmute left/right line-in input*/
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_LINVOL);
	tlv320aic23_write(codec, TLV320AIC23_LINVOL,
					  (reg & (~TLV320AIC23_LIM_MUTED)) |(TLV320AIC23_LRS_ENABLED));

	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_RINVOL);
	tlv320aic23_write(codec, TLV320AIC23_RINVOL,
					  (reg & (~TLV320AIC23_LIM_MUTED)) | TLV320AIC23_LRS_ENABLED);


	/* Default output volume */
	/*  REG 2/3 :Default Left/Right Channel Headphone Volume Control */
	tlv320aic23_write(codec, TLV320AIC23_LCHNVOL,
					  (TLV320AIC23_DEFAULT_OUT_VOL & TLV320AIC23_OUT_VOL_MASK) |TLV320AIC23_LRS_ENABLED);
	tlv320aic23_write(codec, TLV320AIC23_RCHNVOL,
					  (TLV320AIC23_DEFAULT_OUT_VOL & TLV320AIC23_OUT_VOL_MASK) |TLV320AIC23_LRS_ENABLED);
	/* REG 4: Analog Audio Path Control, can be change*/
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_ANLG);
	tlv320aic23_write(codec, TLV320AIC23_ANLG,
					  ((reg) & (~TLV320AIC23_BYPASS_ON) &
					   (~TLV320AIC23_MICM_MUTED)) | TLV320AIC23_INSEL_MIC);
	
	/* REG5: Digital Audio Path Control */
	tlv320aic23_write(codec, TLV320AIC23_DIGT, TLV320AIC23_DEEMP_44K);

	/*  REG 7 :Digital Audio Path Format*/
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_DIGT_FMT);
	tlv320aic23_write(codec, TLV320AIC23_DIGT_FMT, reg);
	/*  REG 8 :Sample rate control*/
	reg = tlv320aic23_read_reg_cache(codec, TLV320AIC23_SRATE);
	tlv320aic23_write(codec, TLV320AIC23_SRATE, reg);
	/* REG 9  :Active Device*/
	tlv320aic23_write(codec, TLV320AIC23_ACTIVE, 0x1);

	tlv320aic23_add_controls(codec);
	tlv320aic23_add_widgets(codec);

	if ((ret = snd_soc_register_card(socdev)) < 0) {
		__E("tlv320aic23: failed to register card\n");
		goto card_err;
	}

	return ret;

  card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
  pcm_err:
	return ret;
}

/* If the i2c layer weren't so broken, we could pass this kind of data
   around */
static struct snd_soc_device *tlv320aic23_socdev;

void hhbf_audio_close(void)
{
	__D("\n");

    if (!tlv320aic23_socdev) return;
    cancel_delayed_work(&tlv320aic23_socdev->delayed_work);
    tlv320aic23_reset(tlv320aic23_socdev->codec);
}
EXPORT_SYMBOL(hhbf_audio_close);

#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)

#include <asm/arch/gpio.h>
#include <linux/proc_fs.h>
static struct proc_dir_entry *w1_proc_entry;			//wireless1 dial indicate
static struct proc_dir_entry *w2_proc_entry;			//wireless1 dial indicate
static struct proc_dir_entry *ctrl_proc_entry;			//transfer indicate
static struct proc_dir_entry *data_proc_entry;		//encoder indicate
static short w1_led = '0';
static short w2_led = '0';
static short ctrl_led = '0';
static short encoder_led = '0';

static ssize_t w1_LED_write( struct file *filp, const char __user *buff,
							 unsigned long len, void *data)
{
	__D("\n");
	
	if (len > 2) {
		__E("len(%d) > 2!\n", (int)len);
		return -ENOSPC;
	}
	if (copy_from_user( &w1_led, buff, len )) {
		return -EFAULT;
	}

	if((w1_led & 0xff) == '0'){			//Ãð
		gpio_direction_output(S3C_GPP10, 0);
		gpio_direction_output(S3C_GPP13, 0);
	}
	else if ((w1_led & 0xff) == '1'){	//ÂÌ
		gpio_direction_output(S3C_GPP10, 0);
		gpio_direction_output(S3C_GPP13, 1);
	}
	else if ((w1_led & 0xff) == '2'){	//ºì
		gpio_direction_output(S3C_GPP10, 1);
		gpio_direction_output(S3C_GPP13, 0);
	}
	return len;
}
static int w1_LED_read( char *page, char **start, off_t off,
						int count, int *eof, void *data )
{
	int len;

	__D("\n");
	
	if (off > 0) {
		*eof = 1;
		return 0;
	}
	len = sprintf(page, "%d\n", ((w1_led & 0xff) == '0') ? 0 : (((w1_led & 0xff) == '1') ? 1 : 2));

	return len;
}
static ssize_t w2_LED_write( struct file *filp, const char __user *buff,
							 unsigned long len, void *data)
{
	__D("\n");
	
	if (len > 2) {
		__E("len(%d) > 2!\n", (int)len);
		return -ENOSPC;
	}
	if (copy_from_user( &w2_led, buff, len )) {
		return -EFAULT;
	}

	if((w2_led & 0xff) == '0'){			//Ãð
		gpio_direction_output(S3C_GPP12, 0);
		gpio_direction_output(S3C_GPP11, 0);
	}
	else if ((w2_led & 0xff) == '1'){	//ÂÌ
		gpio_direction_output(S3C_GPP12, 0);
		gpio_direction_output(S3C_GPP11, 1);
	}
	else if ((w2_led & 0xff) == '2'){	//ºì
		gpio_direction_output(S3C_GPP12, 1);
		gpio_direction_output(S3C_GPP11, 0);
	}
	return len;
}
static int w2_LED_read( char *page, char **start, off_t off,
						int count, int *eof, void *data )
{
	int len;

	__D("\n");
		
	if (off > 0) {
		*eof = 1;
		return 0;
	}
	len = sprintf(page, "%d\n", ((w2_led & 0xff) == '0') ? 0 : (((w2_led & 0xff) == '1') ? 1 : 2));
	return len;
}
static int ctrl_LED_write( struct file *filp, const char __user *buff,
						   unsigned long len, void *data )
{
	__D("\n");
	
	if (len > 2) {
		__E("ctrl_LED_write: len(%d) > 2!\n", (int)len);
		return -ENOSPC;
	}
	if (copy_from_user( &ctrl_led, buff, len )) {
		return -EFAULT;
	}

	if((ctrl_led & 0xff) == '0'){			//Ãð
		gpio_direction_output(S3C_GPP10, 0);
		gpio_direction_output(S3C_GPP13, 0);
		gpio_direction_output(S3C_GPP12, 0);
		gpio_direction_output(S3C_GPP11, 0);
	}
	else{
		if ((w1_led & 0xff) == '1'){		//ÂÌ
			gpio_direction_output(S3C_GPP10, 0);
			gpio_direction_output(S3C_GPP13, 1);
		}
		if ((w1_led & 0xff) == '2'){		//ºì
			gpio_direction_output(S3C_GPP10, 1);
			gpio_direction_output(S3C_GPP13, 0);
		}
		if ((w2_led & 0xff) == '1'){	//ÂÌ
			gpio_direction_output(S3C_GPP12, 0);
			gpio_direction_output(S3C_GPP11, 1);
		}
		if ((w2_led & 0xff) == '2'){	//ºì
			gpio_direction_output(S3C_GPP12, 1);
			gpio_direction_output(S3C_GPP11, 0);
		}
	}
	
	return len;
}
static int ctrl_LED_read( char *page, char **start, off_t off,
						  int count, int *eof, void *data )
{
	int len;
	if (off > 0) {
		*eof = 1;
		return 0;
	}
	len = sprintf(page, "%d\n", ((ctrl_led & 0xff) == '0') ? 0 : 1);
	return len;
}

static ssize_t enc_LED_write( struct file *filp, const char __user *buff,
							  unsigned long len, void *data)
{
	__D("\n");
	
	if (len > 2) {
		__E("w1_LED_write: len(%d) > 2!\n", (int)len);
		return -ENOSPC;
	}
	if (copy_from_user( &encoder_led, buff, len )) {
		return -EFAULT;
	}

	//set direction
	if((encoder_led & 0xff) == '0'){			//Ãð
		gpio_direction_output(S3C_GPP9, 0);
		gpio_direction_output(S3C_GPP8, 0);
	}
	else if ((encoder_led & 0xff) == '1'){	//ÂÌ
		gpio_direction_output(S3C_GPP9, 0);
		gpio_direction_output(S3C_GPP8, 1);
	}
	else if ((encoder_led & 0xff) == '2'){	//ºì
		gpio_direction_output(S3C_GPP9, 1);
		gpio_direction_output(S3C_GPP8, 0);
	}
	return len;
}
static int enc_LED_read( char *page, char **start, off_t off,
						 int count, int *eof, void *data )
{
	int len = 0;
	
	__D("\n");
	
	if (off > 0) {
		*eof = 1;
		return 0;
	}
	
	len = sprintf(page, "%d\n", ((encoder_led & 0xff) == '0') ? 0 : (((encoder_led & 0xff) == '1') ? 1 : 2));
	
	return len;
}

void create_proc_led(void)
{
	int ret;

	__D("\n");
	
	//proc for w1 led ctrl
	w1_proc_entry = create_proc_entry( "wireless1", 0644, NULL );
	if (w1_proc_entry == NULL) {
		ret = -ENOMEM;
		__E("wireless1 led: Couldn't create proc entry\n");
	} else {
		w1_proc_entry->read_proc = w1_LED_read;
		w1_proc_entry->write_proc = w1_LED_write;
		w1_proc_entry->owner = THIS_MODULE;
		__E("wireless1 led: Module loaded.\n");
	}
	//proc for w2 led ctrl
	w2_proc_entry = create_proc_entry( "wireless2", 0644, NULL );
	if (w1_proc_entry == NULL) {
		ret = -ENOMEM;
		__E("wireless2 led: Couldn't create proc entry\n");
	} else {
		w2_proc_entry->read_proc = w2_LED_read;
		w2_proc_entry->write_proc = w2_LED_write;
		w2_proc_entry->owner = THIS_MODULE;
		__E("wireless2 led: Module loaded.\n");
	}
	//proc for data transfer ctrl
	ctrl_proc_entry = create_proc_entry( "transfer", 0644, NULL );
	if (ctrl_proc_entry == NULL) {
		ret = -ENOMEM;
		__E("transfer led: Couldn't create proc entry\n");
	} else {
		ctrl_proc_entry->read_proc = ctrl_LED_read;
		ctrl_proc_entry->write_proc = ctrl_LED_write;
		ctrl_proc_entry->owner = THIS_MODULE;
		__E("transfer led: Module loaded.\n");
	}
	//proc for encoder led
	data_proc_entry = create_proc_entry( "encoder", 0644, NULL );
	if (data_proc_entry == NULL) {
		ret = -ENOMEM;
		__E("encoder led: Couldn't create proc entry\n");
	} else {
		data_proc_entry->read_proc = enc_LED_read;
		data_proc_entry->write_proc = enc_LED_write;
		data_proc_entry->owner = THIS_MODULE;
		__E("encoder led: Module loaded.\n");
	}

	//set default status
	gpio_direction_output(S3C_GPP10, 0);//Wireless1
	gpio_direction_output(S3C_GPP13, 0);//Wireless1
	gpio_direction_output(S3C_GPP12, 0);//Wireless2
	gpio_direction_output(S3C_GPP11, 0);//Wireless2

	gpio_direction_output(S3C_GPP9, 0);//Data
	gpio_direction_output(S3C_GPP8, 0);//Data
}

void release_proc_led(void)
{
	__D("\n");
	
	remove_proc_entry("wireless1", &proc_root);
	remove_proc_entry("wireless2", &proc_root);
	remove_proc_entry("transfer", &proc_root);
	remove_proc_entry("encoder", &proc_root);
}
/*
 * WM8731 2 wire address is determined by GPIO5
 * state during powerup.
 *    low  = 0x1a
 *    high = 0x1b
 */
static unsigned short normal_i2c[] = { 0, I2C_CLIENT_END };

/* Magic definition of all other variables and things */
I2C_CLIENT_INSMOD;

static struct i2c_driver tlv320aic23_i2c_driver;
static struct i2c_client client_template;

static int tlv320aic23_codec_probe(struct i2c_adapter *adap, int addr, int kind)
{
	struct snd_soc_device          *socdev = tlv320aic23_socdev;
	struct tlv320aic23_setup_data  *setup = socdev->codec_data;
	struct snd_soc_codec           *codec = socdev->codec;
	struct i2c_client              *i2c;
	int ret;
	
	__D("\n");
	
	//create proc file for 2 led
	create_proc_led();
	if (addr != setup->i2c_address)
		return -ENODEV;

	client_template.adapter = adap;
	client_template.addr = addr;

	i2c = &client_template;

	i2c_set_clientdata(i2c, codec);
	codec->control_data = i2c;

	if ((ret = i2c_attach_client(i2c)) < 0) {
		__E("failed to attach codec at addr %x\n", addr);
		goto err;
	}

	if ((ret = tlv320aic23_init(socdev)) < 0) {
		__E("failed to initialise TLV320AIC23\n");
		goto err;
	}

	return ret;

  err:
	kfree(codec);
	return ret;
}

static int tlv320aic23_i2c_detach(struct i2c_client *client)
{
	__D("\n");
	
	i2c_detach_client(client);
	return 0;
}

static int tlv320aic23_i2c_attach(struct i2c_adapter *adap)
{
	__D("\n");
	
	return i2c_probe(adap, &addr_data, tlv320aic23_codec_probe);
}

/* corgi i2c codec control layer */
static struct i2c_driver tlv320aic23_i2c_driver = {
	.driver = {
		.name = "TLV320AIC23 I2C Codec",
		.owner = THIS_MODULE,
	},
	.id             = I2C_DRIVERID_WM8753 + 6,
	.attach_adapter = tlv320aic23_i2c_attach,
	.detach_client  = tlv320aic23_i2c_detach,
	.command        = NULL,
};

static struct i2c_client client_template = {
	.name =   "TLV320AIC23",
	.driver = &tlv320aic23_i2c_driver,
};

#endif

static int tlv320aic23_probe(struct platform_device *pdev)
{
	struct snd_soc_device         *socdev = platform_get_drvdata(pdev);
	struct tlv320aic23_setup_data *setup  = socdev->codec_data;
	struct snd_soc_codec          *codec;
	int ret = 0;

	__D("\n");
	__I("Audio Codec Driver %s\n", TLV320AIC23_VERSION);
	if ((codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL)) == NULL)
		return -ENOMEM;

	socdev->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);
	tlv320aic23_socdev = socdev;
	INIT_DELAYED_WORK(&codec->delayed_work, tlv320aic23_work);

#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)
	if (setup->i2c_address) {
		normal_i2c[0]   = setup->i2c_address;
		codec->hw_write = (hw_write_t)i2c_master_send;
		if ((ret = i2c_add_driver(&tlv320aic23_i2c_driver)) != 0)
			__E("can't add i2c driver");
	}
#endif

	return ret;
}

/*
 * This function forces any delayed work to be queued and run.
 */
static int run_delayed_work(struct delayed_work *dwork)
{
	int ret = 0;

	__D("\n");
	
	/* cancel any work waiting to be queued. */
	/* if there was any work waiting then we run it now and
	 * wait for it's completion */
	if ((ret = cancel_delayed_work(dwork)) != 0) {
		schedule_delayed_work(dwork, 0);
		flush_scheduled_work();
	}
	return ret;
}

/* power down chip */
static int tlv320aic23_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec  *codec  = socdev->codec;

	__D("\n");
	
	//release proc file for 2 led
	release_proc_led();
	if (codec->control_data)
		tlv320aic23_set_bias_level(codec, SND_SOC_BIAS_OFF);
	run_delayed_work(&codec->delayed_work);
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
	
#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)
	i2c_del_driver(&tlv320aic23_i2c_driver);
#endif
	
//	kfree(codec->private_data); //liang
	kfree(codec);
	return 0;
}

struct snd_soc_codec_device soc_codec_dev_tlv320aic23 = {
	.probe   = tlv320aic23_probe,
	.remove  = tlv320aic23_remove,
	.suspend = tlv320aic23_suspend,
	.resume  = tlv320aic23_resume,
};

EXPORT_SYMBOL_GPL(soc_codec_dev_tlv320aic23);

MODULE_DESCRIPTION("ASoC TLV320AIC23 driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");
