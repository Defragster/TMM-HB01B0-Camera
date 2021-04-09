/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HM01B0 driver.
 */
 /*
  *  Parts of this library were re-worked from the Sparkfun HB01B0 Library for the Artemis Platform
  */
 /*
Copyright (c) 2019 SparkFun Electronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
 
#ifndef __HM01B0_H__
#define __HM01B0_H__

#include <Arduino.h>

#define SensorMonochrome 1

typedef enum {
    PIXFORMAT_INVALID = 0,
    PIXFORMAT_BINARY,    // 1BPP/BINARY
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_BAYER,     // 1BPP/RAW
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} pixformat_t;

typedef enum {
    FRAMESIZE_INVALID = 0,
    // VGA Resolutions
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QVGA,     // 320x240
	FRAMESIZE_320X320,  // 320x320
} framesize_t;

typedef enum {
    GAINCEILING_2X,
    GAINCEILING_4X,
    GAINCEILING_8X,
    GAINCEILING_16X,
} gainceiling_t;

typedef enum {
	LOAD_DEFAULT_REGS,
	LOAD_WALKING1S_REG,
	LOAD_SHM01B0INIT_REGS,
} camera_reg_settings_t;



typedef struct
{
    uint8_t                 ui8AETargetMean;
    uint8_t                 ui8AEMinMean;
    uint8_t                 ui8ConvergeInTh;
    uint8_t                 ui8ConvergeOutTh;
    uint8_t                 ui8AEMean;
} ae_cfg_t;

typedef struct
{
    uint8_t                 ui8IntegrationH;
    uint8_t                 ui8IntegrationL;
    uint8_t                 ui8AGain;
    uint8_t                 ui8DGain_H;
    uint8_t                 ui8DGain_L;
} hm01b0_snr_expo_gain_ctrl_t;


typedef enum {
    HM01B0_ERR_OK               = 0x00,
    HM01B0_ERR_AE_NOT_CONVERGED,
    HM01B0_ERR_PARAMS,
    HM01B0_NUM_ERR
} status_e;

	
class HM01B0
{
  public:
    HM01B0();
	int reset();
	uint8_t cameraReadRegister(uint16_t reg);
	uint8_t cameraWriteRegister(uint16_t reg, uint8_t data) ;
	int set_pixformat( pixformat_t pfmt);
	uint8_t set_framesize(framesize_t framesize);
	int set_framerate(int framerate);
	int set_brightness(int level);
	int set_gainceiling(gainceiling_t gainceiling);
	int set_colorbar(int enable);
	int set_auto_gain(int enable, float gain_db, float gain_db_ceiling);
	int get_vt_pix_clk(uint32_t *vt_pix_clk);
	int get_gain_db(float *gain_db);
	int getCameraClock(uint32_t *vt_pix_clk);
	int set_auto_exposure(int enable, int exposure_us);
	int get_exposure_us(int *exposure_us);
	int set_hmirror(int enable);
	int set_vflip( int enable);
	uint8_t set_mode(uint8_t Mode, uint8_t FrameCnt);
	uint8_t cmdUpdate();
	uint8_t loadSettings(camera_reg_settings_t settings);
	void readFrame(void* buffer);
	uint8_t get_ae( ae_cfg_t *psAECfg);
	uint8_t cal_ae( uint8_t CalFrames, uint8_t* Buffer, uint32_t ui32BufferLen, ae_cfg_t* pAECfg);
	uint16_t get_modelid();

	int init();
	
  	framesize_t framesize;
	pixformat_t pixformat;
	camera_reg_settings_t settings;
	uint16_t w, h;  //width, height

	inline float fast_log2(float x)
	{
	  union { float f; uint32_t i; } vx = { x };
	  union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	  float y = vx.i;
	  y *= 1.1920928955078125e-7f;

	  return y - 124.22551499f - 1.498030302f * mx.f
			   - 1.72587999f / (0.3520887068f + mx.f);
	}

	inline float fast_log(float x)
	{
	  return 0.69314718f * fast_log2 (x);
	}

	inline int fast_floorf(float x)
	{
		int i;
		asm volatile (
				"vcvt.S32.f32  %[r], %[x]\n"
				: [r] "=t" (i)
				: [x] "t"  (x));
		return i;
	}

	inline int fast_ceilf(float x)
	{
		int i;
		x += 0.9999f;
		asm volatile (
				"vcvt.S32.f32  %[r], %[x]\n"
				: [r] "=t" (i)
				: [x] "t"  (x));
		return i;
	}

	inline int fast_roundf(float x)
	{
		int i;
		asm volatile (
				"vcvtr.s32.f32  %[r], %[x]\n"
				: [r] "=t" (i)
				: [x] "t"  (x));
		return i;
	}

	typedef union{
		uint32_t l;
		struct {
			uint32_t m : 20;
			uint32_t e : 11;
			uint32_t s : 1;
		};
	} exp_t;

	inline float fast_expf(float x)
	{
		exp_t e;
		e.l = (uint32_t)(1512775 * x + 1072632447);
		// IEEE binary32 format
		e.e = (e.e -1023 + 127) &0xFF; // rebase

		uint32_t packed = (e.s << 31) | (e.e << 23) | e.m <<3;
		return *((float*)&packed);
	}
	
  private:
  	uint8_t VSYNC_PIN = 33;
	uint8_t PCLK_PIN = 8;
	uint8_t HSYNC_PIN = 32;
	uint8_t MCLK_PIN = 7;
	uint8_t EN_PIN = 2;
	uint8_t G0 = 40;
	uint8_t G1 = 41;
	uint8_t G2 = 42;
	uint8_t G3 = 43;
	uint8_t G4 = 44;
	uint8_t G5 = 45;
	uint8_t G6 = 6;
	uint8_t G7 = 9;

	
	uint32_t _vsyncMask;
	uint32_t _hrefMask;
	uint32_t _pclkMask;
	const volatile uint32_t *_vsyncPort;
	const volatile uint32_t *_hrefPort;
	const volatile uint32_t *_pclkPort;
	
	uint32_t OMV_XCLK_FREQUENCY	= 12000000;


};
//Rest is TBD.

#endif // __HM01B0_H__