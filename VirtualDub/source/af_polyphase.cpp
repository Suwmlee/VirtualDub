//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2003 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "af_base.h"
#include "af_polyphase.h"
#include "gui.h"
#include "resource.h"
#include "audioutil.h"
#include <vd2/system/fraction.h>

namespace {
	// sin(x) = x - x^3/6 + x^5/120 - x^7/5040...
	// sinc(y) = 1 - y^2/6 + y^4/120 - y^6/5040...  y=x*pi
	//
	// The dominant term is y^2/6, and 1+y^2/6 will drop to 1 in IEEE
	// double precision once y^2/6 < 2^-54.  This will happen when
	// y < 1.8e-08.

	static const double pi = 3.1415926535897932;

	// These functions generate Hamming/Blackman windows and windowed-
	// sinc FIR filters.  See "Digital Signal Processing" by Smith
	// for more info.

	static inline double sinc(double y) {
		return fabs(y) < 1.8e-8 ? 1 : sin(y)/y;
	}

	// These are centered around y=0, *not* y=pi!

	static inline double hamming(double z) {
		return 0.54 + 0.46 * cos(z);
	}

	static inline double blackman(double z) {
		return 0.42 + 0.5 * cos(z) + 0.08 * cos(2*z);
	}

	void AudioMakeResampleFilter(float *dst, int halfpoints, double cutoff, double phase) {
		const int points = 2*halfpoints;
		int i;
		double one_over_M = 0.5 / halfpoints;
		double sum = 0.0;

		for(i=0; i<points; ++i) {
			double y = (i - halfpoints + 1 - phase)*2.0*pi;
			double v = sinc(y * cutoff) * blackman(y * one_over_M);

			sum += v;
			dst[i] = v;
		}

		double inv_sum = 1.0/sum;

		for(i=0; i<points; i++)
			dst[i] *= inv_sum;
	}

	void AudioMakeLowpassFilter(float *dst, int halfpoints, double cutoff) {
		int i;
		double one_over_M = 0.5 / halfpoints;
		double sum = 0.0;

		for(i=0; i<=halfpoints; i++) {
			double y = i*2.0*pi;
			double v = sinc(y * cutoff) * blackman(y * one_over_M);

			sum += v;
			dst[i] = v;
		}

		double inv_sum = 0.5/(sum - 0.5*dst[0]);

		for(i=0; i<=halfpoints; i++)
			dst[i] *= inv_sum;
	}

	void AudioMakeHighpassFilter(float *dst, int halfpoints, double cutoff) {
		int i;
		double one_over_M = 0.5 / halfpoints;
		double sum = 0.0;

		for(i=0; i<=halfpoints; i++) {
			double y = i*2.0*pi;
			double v = sinc(y * cutoff) * blackman(y * one_over_M);

			sum += v;
			dst[i] = v;
		}

		double inv_sum = -0.5/(sum - 0.5*dst[0]);

		for(i=0; i<=halfpoints; i++)
			dst[i] *= inv_sum;

		dst[0] += 1.0f;
	}
};

///////////////////////////////////////////////////////////////////////////

VDAudioFilterSymmetricFIR::VDAudioFilterSymmetricFIR() {
}

VDAudioFilterSymmetricFIR::~VDAudioFilterSymmetricFIR() {
}

uint32 VDAudioFilterSymmetricFIR::Prepare() {
	const VDWaveFormat& inFormat = *mpContext->mpInputs[0]->mpFormat;

	if (   inFormat.mTag != VDWaveFormat::kTagPCM
		|| (inFormat.mSampleBits != 8 && inFormat.mSampleBits != 16)
		)
		return kVFAPrepare_BadFormat;


	GenerateFilter(inFormat.mSamplingRate);
	mFilterBank.resize((mFilterBank.size() + 3) & ~3, 0);

	mpContext->mpInputs[0]->mDelay			= (sint64)(mFilterSize*1000000) * inFormat.mBlockSize / inFormat.mDataRate;

	VDWaveFormat *pwf = mpContext->mpAudioCallbacks->CopyWaveFormat(&inFormat);

	if (!pwf)
		mpContext->mpServices->ExceptOutOfMemory();

	mpContext->mpOutputs[0]->mpFormat = pwf;

	pwf->mSampleBits	= 16;
	pwf->mBlockSize		= 2 * pwf->mChannels;
	pwf->mDataRate		= pwf->mSamplingRate * pwf->mBlockSize;

	return 0;
}

void VDAudioFilterSymmetricFIR::Start() {
	const VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;

	mFIRBufferReadPoint = 0;
	mFIRBufferWritePoint = 0;
	mFIRBufferLimit = (mFilterSize*2 + format.mSamplingRate + 15) & ~15;
	mFIRBufferChannelStride = mFIRBufferLimit;
	mFIRBuffer.resize(mFIRBufferChannelStride * format.mChannels);
	mOutputBuffer.Init(format.mBlockSize * pin.mBufferSize);

	mMaxQuantum = std::max<int>(format.mSamplingRate / 10, 256);
}

uint32 VDAudioFilterSymmetricFIR::Run() {
	VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;
	bool bInputRead = false;

	// fill up FIR buffer
	while(mFIRBufferWritePoint < mFIRBufferLimit) {
		sint16 buf[4096];
		int samples_req = std::min<int>(mFIRBufferLimit - mFIRBufferWritePoint, 4096 / format.mChannels);

		int samples = mpContext->mpInputs[0]->Read(buf, samples_req, false, kVFARead_PCM16);

		for(int ch=0; ch<format.mChannels; ++ch) {
			sint16 *dst = &mFIRBuffer[mFIRBufferChannelStride * ch + mFIRBufferWritePoint];
			const sint16 *src16 = (const sint16 *)buf + ch;

			for(int i=0; i<samples; ++i)
				dst[i] = src16[i * format.mChannels];
		}

		mFIRBufferWritePoint += samples;

		if (samples < samples_req)
			break;

		bInputRead = true;
	}

	// compute output samples
	int bytes = (mFIRBufferWritePoint - mFIRBufferReadPoint - 2*mFilterSize) * format.mBlockSize;
	sint16 *dst;
	int samples = 0;
	
	if (bytes > 0) {
		dst = (sint16 *)mOutputBuffer.LockWrite(bytes, bytes);
		samples = bytes / format.mBlockSize;
	}

	if (samples > mMaxQuantum)
		samples = mMaxQuantum;

	if (!samples) {
		if (!bInputRead && mpContext->mpInputs[0]->mbEnded && bytes<=0)
			return kVFARun_Finished;

		return 0;
	}

	const VDAudioFilterVtable *pVtbl = VDGetAudioFilterVtable();
	int newReadPoint = mFIRBufferReadPoint + samples;
	bool bShift = (newReadPoint >= (mFIRBufferLimit>>1));

	for(int ch=0; ch<format.mChannels; ++ch) {
		sint16 *src = &mFIRBuffer[mFIRBufferChannelStride * ch];

		pVtbl->FilterPCM16SymmetricArray(dst + ch, format.mChannels, src + mFIRBufferReadPoint, samples, &mFilterBank.front(), (mFilterSize>>2));

		if (bShift)
			memmove(src, src+newReadPoint, sizeof(src[0]) * (mFIRBufferWritePoint - newReadPoint));
	}

	mFIRBufferReadPoint = newReadPoint;

	if (bShift) {
		mFIRBufferReadPoint = 0;
		mFIRBufferWritePoint -= newReadPoint;
	}

	mOutputBuffer.UnlockWrite(samples * format.mBlockSize);

	mpContext->mpOutputs[0]->mCurrentLevel = mOutputBuffer.getLevel() / mpContext->mpOutputs[0]->mpFormat->mBlockSize;

	return 0;
}

uint32 VDAudioFilterSymmetricFIR::Read(unsigned inpin, void *dst, uint32 samples) {
	VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;

	samples = std::min<uint32>(samples, mOutputBuffer.getLevel() / format.mBlockSize);

	if (dst) {
		mOutputBuffer.Read((char *)dst, samples * format.mBlockSize);
		mpContext->mpOutputs[0]->mCurrentLevel = mOutputBuffer.getLevel() / mpContext->mpOutputs[0]->mpFormat->mBlockSize;
	}

	return samples;
}

sint64 VDAudioFilterSymmetricFIR::Seek(sint64 us) {
	mOutputBuffer.Flush();
	mpContext->mpOutputs[0]->mCurrentLevel = 0;
	mFIRBufferReadPoint = 0;
	mFIRBufferWritePoint = 0;
	return us;
}

///////////////////////////////////////////////////////////////////////////

VDAudioFilterPolyphase::VDAudioFilterPolyphase() {
}

VDAudioFilterPolyphase::~VDAudioFilterPolyphase() {
}

uint32 VDAudioFilterPolyphase::Prepare() {
	const VDWaveFormat& inFormat = *mpContext->mpInputs[0]->mpFormat;

	if (   inFormat.mTag != VDWaveFormat::kTagPCM
		|| (inFormat.mSampleBits != 8 && inFormat.mSampleBits != 16)
		)
		return kVFAPrepare_BadFormat;

	VDWaveFormat *pwf = mpContext->mpAudioCallbacks->CopyWaveFormat(&inFormat);

	if (!pwf)
		mpContext->mpServices->ExceptOutOfMemory();

	mpContext->mpOutputs[0]->mpFormat = pwf;

	pwf->mSamplingRate = GenerateFilterBank(inFormat.mSamplingRate);

	pwf->mSampleBits	= 16;
	pwf->mBlockSize		= 2 * pwf->mChannels;
	pwf->mDataRate		= pwf->mSamplingRate * pwf->mBlockSize;

	mpContext->mpInputs[0]->mGranularity	= 1;
	mpContext->mpInputs[0]->mDelay		= (sint64)(mFilterSize*1000000) * inFormat.mBlockSize / inFormat.mDataRate;
	mpContext->mpOutputs[0]->mGranularity = 1;

	return 0;
}

void VDAudioFilterPolyphase::Start() {
	const VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;

	mFIRBufferPoint = 0;
	mFIRBufferLimit = 16384;
	mFIRBufferChannelStride = 16384;
	mFIRBuffer.resize(mFIRBufferChannelStride * format.mChannels);
	mOutputBuffer.Init(format.mBlockSize * pin.mBufferSize);
	mCurrentPhase = 0;

	mRatioSrc = mpContext->mpInputs[0]->mpFormat->mSamplingRate;
	mRatioDst = mpContext->mpOutputs[0]->mpFormat->mSamplingRate;
}

uint32 VDAudioFilterPolyphase::Run() {
	VDAudioFilterPin& pin = *mpContext->mpInputs[0];
	VDAudioFilterPin& pinout = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;
	const VDWaveFormat& formatOut = *pinout.mpFormat;
	bool bInputRead = false;

	// fill up FIR buffer
	while(mFIRBufferPoint < mFIRBufferLimit) {
		sint16 buf[4096];
		int samples_req = std::min<int>(mFIRBufferLimit - mFIRBufferPoint, 4096 / format.mChannels);

		int samples = mpContext->mpInputs[0]->Read(buf, samples_req, false, kVFARead_PCM16);

		for(int ch=0; ch<format.mChannels; ++ch) {
			sint16 *dst = &mFIRBuffer[mFIRBufferChannelStride * ch + mFIRBufferPoint];
			const sint16 *src16 = (const sint16 *)buf + ch;

			for(int i=0; i<samples; ++i)
				dst[i] = src16[i * format.mChannels];
		}

		mFIRBufferPoint += samples;

		if (samples < samples_req)
			break;

		bInputRead = true;
	}

	// compute output samples
	int bytes = (((mFIRBufferPoint - mFilterSize + 1)*mRatioDst - mCurrentPhase)/mRatioSrc) * format.mBlockSize;
	sint16 *dst;
	int samples = 0;
	
	if (bytes > 0) {
		dst = (sint16 *)mOutputBuffer.LockWrite(bytes, bytes);
		samples = bytes / format.mBlockSize;
	}

	if (!samples) {
		if (!bInputRead && pin.mbEnded && !bytes)
			return kVFARun_Finished;

		return 0;
	}

	sint32	phasefixed		= (mCurrentPhase * (sint64)0x10000) / mRatioDst;
	sint32	phaseincfixed	= (sint32)((mRatioSrc*(sint64)0x10000) / mRatioDst);

	sint32	newPhase	= mCurrentPhase + mRatioSrc * samples;
	sint32	srcInc		= newPhase / mRatioDst;

	const VDAudioFilterVtable *pVtbl = VDGetAudioFilterVtable();

	for(int ch=0; ch<format.mChannels; ++ch) {
		sint16 *src = &mFIRBuffer[mFIRBufferChannelStride * ch];
		sint16 *dst2 = dst + ch;
		sint32	phase = phasefixed;

		for(int i=0; i<samples; ++i) {
			const sint16 *pFilter = &mFilterBank[mFilterSize * ((phase>>11)&31)];
			const sint16 *src2 = src + (phase>>16);
			*dst2 = pVtbl->FilterPCM16(src2, pFilter, mFilterSize >> 2);
			dst2 += format.mChannels;
			phase += phaseincfixed;
		}

		pVtbl->FilterPCM16End();

		memmove(src, src+srcInc, sizeof(src[0]) * (mFIRBufferPoint - srcInc));
	}

	mCurrentPhase = newPhase - srcInc * mRatioDst;

	mFIRBufferPoint -= srcInc;

	mOutputBuffer.UnlockWrite(samples * format.mBlockSize);

	mpContext->mpOutputs[0]->mCurrentLevel = mOutputBuffer.getLevel() / mpContext->mpOutputs[0]->mpFormat->mBlockSize;

	return 0;
}

uint32 VDAudioFilterPolyphase::Read(unsigned inpin, void *dst, uint32 samples) {
	VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
	const VDWaveFormat& format = *pin.mpFormat;

	samples = std::min<uint32>(samples, mOutputBuffer.getLevel() / format.mBlockSize);

	if (dst) {
		mOutputBuffer.Read((char *)dst, samples * format.mBlockSize);
		mpContext->mpOutputs[0]->mCurrentLevel = mOutputBuffer.getLevel() / mpContext->mpOutputs[0]->mpFormat->mBlockSize;
	}

	return samples;
}

sint64 VDAudioFilterPolyphase::Seek(sint64 us) {
	mOutputBuffer.Flush();
	mpContext->mpOutputs[0]->mCurrentLevel = 0;
	mFIRBufferPoint = 0;
	return us;
}

///////////////////////////////////////////////////////////////////////////

VDAFBASE_BEGIN_CONFIG(Xpass);
VDAFBASE_CONFIG_ENTRY(Xpass, 0, U32, cutoff, L"Cutoff frequency (Hz)",	L"Approximate frequency in Hertz at which filter takes effect (cuts off audio components)." );
VDAFBASE_CONFIG_ENTRY(Xpass, 1, U32, taps, L"Filter tap count",	L"Number of filter taps to use." );
VDAFBASE_END_CONFIG(Xpass, 1);

typedef VDAudioFilterData_Xpass VDAudioFilterXpassConfig;

class VDDialogAudioFilterXpassConfig : public VDDialogBaseW32 {
public:
	VDDialogAudioFilterXpassConfig(VDAudioFilterXpassConfig& config) : VDDialogBaseW32(IDD_AF_XPASS), mConfig(config) {}

	bool Activate(VDGUIHandle hParent) {
		return 0 != ActivateDialog(hParent);
	}

	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
		switch(msg) {
		case WM_INITDIALOG:
			SetDlgItemInt(mhdlg, IDC_CUTOFF, mConfig.cutoff, FALSE);
			SetDlgItemInt(mhdlg, IDC_TAPS, mConfig.taps, FALSE);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDOK:
				{
					BOOL valid;

					mConfig.cutoff = GetDlgItemInt(mhdlg, IDC_CUTOFF, &valid, FALSE);
					if (!valid) {
						MessageBeep(MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(mhdlg, IDC_CUTOFF));
						return TRUE;
					}
					mConfig.taps = GetDlgItemInt(mhdlg, IDC_TAPS, &valid, FALSE);
					if (!valid) {
						MessageBeep(MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(mhdlg, IDC_TAPS));
						return TRUE;
					}
					End(TRUE);
				}
				return TRUE;
			case IDCANCEL:
				End(FALSE);
				return TRUE;
			}
		}

		return FALSE;
	}

	VDAudioFilterXpassConfig& mConfig;
};

class VDAudioFilterXpass : public VDAudioFilterSymmetricFIR {
public:
	VDAudioFilterXpass(bool bHighpass) : mbHighpass(bHighpass) {
		mConfig.cutoff = 4000;
		mConfig.taps = 64;
	}

	bool Config(HWND hwnd) {
		VDAudioFilterXpassConfig	config(mConfig);

		if (!hwnd)
			return true;

		if (VDDialogAudioFilterXpassConfig(config).Activate((VDGUIHandle)hwnd)) {
			mConfig = config;
			return true;
		}
		return false;
	}

	void *GetConfigPtr() { return &mConfig; }

	void GenerateFilter(int freq);

	const bool mbHighpass;
	VDAudioFilterXpassConfig	mConfig;
};

void VDAudioFilterXpass::GenerateFilter(int freq) {
	int requested_halfsize = mConfig.taps;

	if (requested_halfsize < 8)
		requested_halfsize = 8;
	if (requested_halfsize > 16384)
		requested_halfsize = 16384;

	int halfsize = requested_halfsize;
	double cutoff = (double)mConfig.cutoff / freq;

	if (cutoff > 1.0)
		cutoff = 1.0;
	if (cutoff < 0)
		cutoff = 0;

	std::vector<float> halfkernel(halfsize+1);

	if (mbHighpass)
		AudioMakeHighpassFilter(&halfkernel[0], halfsize, cutoff);
	else
		AudioMakeLowpassFilter(&halfkernel[0], halfsize, cutoff);

	// Cut off zeroes at the end of the kernel.
	while(halfsize > 1 && fabs(halfkernel[halfsize-1]) < (0.5f / 16384.0f))
		--halfsize;

	halfsize = (halfsize + 3) & ~3;

	if (halfsize < requested_halfsize) {
		VDDEBUG("AudioFilter/XPass: Reduced filter halfkernel size from %d taps to %d due to zero roundoff\n", requested_halfsize, halfsize);
	}

	mFilterSize = halfsize;
	mFilterBank.resize(2*mFilterSize+1);
	for(int i=0; i<=halfsize; ++i) {
		mFilterBank[mFilterSize+i] = mFilterBank[mFilterSize-i] = (int)floor(0.5 + halfkernel[i]*16384);
	}
}

void __cdecl VDAudioFilterLowpassInitProc(const VDAudioFilterContext *pContext) {
	new (pContext->mpFilterData) VDAudioFilterXpass(false);
}

void __cdecl VDAudioFilterHighpassInitProc(const VDAudioFilterContext *pContext) {
	new (pContext->mpFilterData) VDAudioFilterXpass(true);
}

extern const struct VDAudioFilterDefinition afilterDef_lowpass = {
	sizeof(VDAudioFilterDefinition),
	kVFAF_HasConfig,

	sizeof(VDAudioFilterXpass),	1,	1,

	&VDAudioFilterData_Xpass::members.info,

	VDAudioFilterLowpassInitProc,
	&VDAudioFilterBase::sVtbl,
};

extern const struct VDPluginInfo apluginDef_lowpass = {
	sizeof(VDPluginInfo),
	L"lowpass",
	NULL,
	L"Removes frequency components above a given cutoff using a windowed-sinc filter.",
	0,
	kVDPluginType_Audio,
	0,

	kVDPlugin_APIVersion,
	kVDPlugin_APIVersion,
	kVDPlugin_AudioAPIVersion,
	kVDPlugin_AudioAPIVersion,

	&afilterDef_lowpass
};

extern const struct VDAudioFilterDefinition afilterDef_highpass = {
	sizeof(VDAudioFilterDefinition),
	kVFAF_HasConfig,

	sizeof(VDAudioFilterXpass),	1,	1,

	&VDAudioFilterData_Xpass::members.info,

	VDAudioFilterHighpassInitProc,
	&VDAudioFilterBase::sVtbl,
};

extern const struct VDPluginInfo apluginDef_highpass = {
	sizeof(VDPluginInfo),
	L"highpass",
	NULL,
	L"Removes frequency components below a given cutoff using a windowed-sinc filter.",
	0,
	kVDPluginType_Audio,
	0,

	kVDPlugin_APIVersion,
	kVDPlugin_APIVersion,
	kVDPlugin_AudioAPIVersion,
	kVDPlugin_AudioAPIVersion,

	&afilterDef_highpass
};

///////////////////////////////////////////////////////////////////////////

VDAFBASE_BEGIN_CONFIG(Resample);
VDAFBASE_CONFIG_ENTRY(Resample, 0, U32, newfreq, L"New frequency (Hz)",	L"Target frequency in Hertz." );
VDAFBASE_CONFIG_ENTRY(Resample, 1, U32, taps, L"Filter tap count",	L"Number of filter taps to use." );
VDAFBASE_END_CONFIG(Resample, 1);

typedef VDAudioFilterData_Resample VDAudioFilterResampleConfig;

class VDDialogAudioFilterResampleConfig : public VDDialogBaseW32 {
public:
	VDDialogAudioFilterResampleConfig(VDAudioFilterResampleConfig& config) : VDDialogBaseW32(IDD_AF_RESAMPLE), mConfig(config) {}

	bool Activate(VDGUIHandle hParent) {
		return 0 != ActivateDialog(hParent);
	}

	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
		switch(msg) {
		case WM_INITDIALOG:
			SetDlgItemInt(mhdlg, IDC_FREQ, mConfig.newfreq, FALSE);
			SetDlgItemInt(mhdlg, IDC_TAPS, mConfig.taps, FALSE);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDOK:
				{
					BOOL valid;

					mConfig.newfreq = GetDlgItemInt(mhdlg, IDC_FREQ, &valid, FALSE);
					if (!valid) {
						MessageBeep(MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(mhdlg, IDC_FREQ));
						return TRUE;
					}
					mConfig.taps = GetDlgItemInt(mhdlg, IDC_TAPS, &valid, FALSE);
					if (!valid) {
						MessageBeep(MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(mhdlg, IDC_TAPS));
						return TRUE;
					}
					End(TRUE);
				}
				return TRUE;
			case IDCANCEL:
				End(FALSE);
				return TRUE;
			}
		}

		return FALSE;
	}

	VDAudioFilterResampleConfig& mConfig;
};

class VDAudioFilterResample : public VDAudioFilterPolyphase {
public:
	static void __cdecl InitProc(const VDAudioFilterContext *pContext);

	VDAudioFilterResample() {
		mConfig.newfreq = 44100;
		mConfig.taps = 64;
	}

	bool Config(HWND hwnd) {
		VDAudioFilterResampleConfig	config(mConfig);

		if (!hwnd)
			return true;

		if (VDDialogAudioFilterResampleConfig(config).Activate((VDGUIHandle)hwnd)) {
			mConfig = config;
			return true;
		}
		return false;
	}

	void *GetConfigPtr() { return &mConfig; }

	int GenerateFilterBank(int freq);

	VDAudioFilterResampleConfig	mConfig;
};

void __cdecl VDAudioFilterResample::InitProc(const VDAudioFilterContext *pContext) {
	new (pContext->mpFilterData) VDAudioFilterResample;
}

int VDAudioFilterResample::GenerateFilterBank(int freq) {
	int requested_halfsize = mConfig.taps;

	if (requested_halfsize < 8)
		requested_halfsize = 8;
	if (requested_halfsize > 16384)
		requested_halfsize = 16384;

	const int halfsize = requested_halfsize;
	const int fullsize = 2*halfsize;
	double cutoff = 0.5 * (double)mConfig.newfreq / freq;

	if (cutoff > 0.5)
		cutoff = 0.5;
	if (cutoff < 0)
		cutoff = 0;

	std::vector<float> kernel(fullsize);
	std::vector<sint16> ikernels(32 * fullsize);
	sint16 *dst = &ikernels.front();
	int phase, i;
	int lorange = fullsize, hirange = 0;

	for(phase=0; phase<32; ++phase) {
		AudioMakeResampleFilter(&kernel[0], halfsize, cutoff, phase / 32.0);

		for(i=0; i<fullsize; ++i) {
			int v = (int)floor(0.5 + kernel[i]*16384);
			*dst++ = v;

			if (v) {
				if (lorange > i)
					lorange = i;
				if (hirange < i)
					hirange = i;
			}
		}
	}

	int trim = std::min<int>(lorange, hirange) & ~3;

	mFilterSize = fullsize - trim;
	if (mFilterSize < 8) {
		mFilterSize = 8;
		trim = halfsize - 4;
	}

	if (mFilterSize != fullsize) {
		VDDEBUG("AudioFilter/Resample: Reduced filter kernel size from %d taps to %d due to zero roundoff\n", fullsize, mFilterSize);
	}

	mFilterBank.resize(32 * mFilterSize);

	dst = &mFilterBank.front();
	for(phase=0; phase<32; ++phase) {
		std::copy(&ikernels[2*halfsize*phase + trim], &ikernels[2*halfsize*phase + trim + mFilterSize], dst);
		dst += mFilterSize;
	}

	return mConfig.newfreq;
}

extern const struct VDAudioFilterDefinition afilterDef_resample = {
	sizeof(VDAudioFilterDefinition),
	kVFAF_HasConfig,

	sizeof(VDAudioFilterResample),	1,	1,

	&VDAudioFilterData_Resample::members.info,

	VDAudioFilterResample::InitProc,
	&VDAudioFilterBase::sVtbl,
};

extern const struct VDPluginInfo apluginDef_resample = {
	sizeof(VDPluginInfo),
	L"resample",
	NULL,
	L"Resamples audio to a new sampling frequency using a 32-phase filter bank.",
	0,
	kVDPluginType_Audio,
	0,

	kVDPlugin_APIVersion,
	kVDPlugin_APIVersion,
	kVDPlugin_AudioAPIVersion,
	kVDPlugin_AudioAPIVersion,

	&afilterDef_resample
};

///////////////////////////////////////////////////////////////////////////

VDAFBASE_BEGIN_CONFIG(Stretch);
VDAFBASE_CONFIG_ENTRY(Stretch, 0, Double, ratio, L"Stretch ratio",	L"Stretch ratio (>1.0 is longer)" );
VDAFBASE_END_CONFIG(Stretch, 0);

typedef VDAudioFilterData_Stretch VDAudioFilterStretchConfig;

class VDDialogAudioFilterStretchConfig : public VDDialogBaseW32 {
public:
	VDDialogAudioFilterStretchConfig(VDAudioFilterStretchConfig& config) : VDDialogBaseW32(IDD_AF_STRETCH), mConfig(config) {}

	bool Activate(VDGUIHandle hParent) {
		return 0 != ActivateDialog(hParent);
	}

	BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
		char buf[256];

		switch(msg) {
		case WM_INITDIALOG:
			sprintf(buf, "%.4f", mConfig.ratio);
			SetDlgItemText(mhdlg, IDC_RATIO, buf);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDOK:
				{
					double v;

					if (!GetDlgItemText(mhdlg, IDC_RATIO, buf, sizeof buf) || (v=atof(buf))<0.1 || (v>10.0)) {
						MessageBeep(MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(mhdlg, IDC_RATIO));
						return TRUE;
					}
					mConfig.ratio = v;
					End(TRUE);
				}
				return TRUE;
			case IDCANCEL:
				End(FALSE);
				return TRUE;
			}
		}

		return FALSE;
	}

	VDAudioFilterStretchConfig& mConfig;
};

class VDAudioFilterStretch : public VDAudioFilterPolyphase {
public:
	static void __cdecl InitProc(const VDAudioFilterContext *pContext);

	VDAudioFilterStretch() {
		mConfig.ratio = 1.0;
	}
	
	bool Config(HWND hwnd) {
		VDAudioFilterStretchConfig	config(mConfig);

		if (!hwnd)
			return true;

		if (VDDialogAudioFilterStretchConfig(config).Activate((VDGUIHandle)hwnd)) {
			mConfig = config;
			return true;
		}
		return false;
	}

	void Start() {
		VDAudioFilterPolyphase::Start();
		mRatioSrc = (sint32)(0.5 + 0x10000 / mConfig.ratio);
		mRatioDst = 0x10000;

		VDAudioFilterPin& pin = *mpContext->mpOutputs[0];
		pin.mLength = VDFraction(mRatioSrc, mRatioDst).scale64ir(pin.mLength);
	}

	void *GetConfigPtr() { return &mConfig; }

	sint64 Seek(sint64 pos) {
		return VDFraction(mRatioSrc, mRatioDst).scale64r(pos);
	}

	int GenerateFilterBank(int freq);

	VDAudioFilterStretchConfig	mConfig;
};

void __cdecl VDAudioFilterStretch::InitProc(const VDAudioFilterContext *pContext) {
	new (pContext->mpFilterData) VDAudioFilterStretch;
}

int VDAudioFilterStretch::GenerateFilterBank(int freq) {
	int halfsize = 64;
	double cutoff = 0.5 * mConfig.ratio;

	if (cutoff > 0.5)
		cutoff = 0.5;
	if (cutoff < 0)
		cutoff = 0;

	mFilterSize = 2*halfsize;
	mFilterBank.resize(32 * mFilterSize);

	sint16 *dst = &mFilterBank.front();
	std::vector<float> kernel(2*halfsize);

	for(int phase=0; phase<32; ++phase) {
		AudioMakeResampleFilter(&kernel[0], halfsize, cutoff, phase / 32.0);

		for(int i=0; i<mFilterSize; ++i)
			*dst++ = (int)floor(0.5 + kernel[i]*16384);
	}

	return freq;
}

extern const struct VDAudioFilterDefinition afilterDef_stretch = {
	sizeof(VDAudioFilterDefinition),
	kVFAF_HasConfig,

	sizeof(VDAudioFilterStretch),	1,	1,

	&VDAudioFilterData_Stretch::members.info,

	VDAudioFilterStretch::InitProc,
	&VDAudioFilterBase::sVtbl,
};

extern const struct VDPluginInfo apluginDef_stretch = {
	sizeof(VDPluginInfo),
	L"stretch",
	NULL,
	L"Resamples audio to a different length without changing sampling frequency, using a 32-phase, 129-tap filter bank.",
	0,
	kVDPluginType_Audio,
	0,

	kVDPlugin_APIVersion,
	kVDPlugin_APIVersion,
	kVDPlugin_AudioAPIVersion,
	kVDPlugin_AudioAPIVersion,

	&afilterDef_stretch
};
