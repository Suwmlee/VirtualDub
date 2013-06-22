#include <windows.h>
#include <vd2/system/binary.h>
#include <vd2/system/vectors.h>
#include <vd2/system/VDString.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmapops.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "displaydrv.h"

#define VDDEBUG_DISP (void)sizeof printf
//#define VDDEBUG_DISP VDDEBUG

void VDDitherImage(VDPixmap& dst, const VDPixmap& src, const uint8 *pLogPal);

///////////////////////////////////////////////////////////////////////////////

class VDVideoDisplayMinidriverGDI : public VDVideoDisplayMinidriver {
public:
	VDVideoDisplayMinidriverGDI();
	~VDVideoDisplayMinidriverGDI();

	bool Init(HWND hwnd, const VDVideoDisplaySourceInfo& info);
	void Shutdown();

	bool ModifySource(const VDVideoDisplaySourceInfo& info);

	bool IsValid() { return mbValid; }

	bool Update(UpdateMode);
	void Refresh(UpdateMode);
	bool Paint(HDC hdc, const RECT& rClient, UpdateMode mode);
	bool SetSubrect(const vdrect32 *r);
	void SetLogicalPalette(const uint8 *pLogicalPalette) { mpLogicalPalette = pLogicalPalette; }

protected:
	HWND		mhwnd;
	HDC			mhdc;
	HBITMAP		mhbm;
	HGDIOBJ		mhbmOld;
	void *		mpBitmapBits;
	ptrdiff_t	mPitch;
	HPALETTE	mpal;
	const uint8 *mpLogicalPalette;
	bool		mbPaletted;
	bool		mbValid;
	bool		mbUseSubrect;
	int			mScreenFormat;

	vdrect32	mSubrect;

	uint8		mIdentTab[256];

	VDVideoDisplaySourceInfo	mSource;

	void InternalRefresh(HDC hdc, const RECT& rClient, UpdateMode mode);
	static int GetScreenIntermediatePixmapFormat(HDC);
};

IVDVideoDisplayMinidriver *VDCreateVideoDisplayMinidriverGDI() {
	return new VDVideoDisplayMinidriverGDI;
}

VDVideoDisplayMinidriverGDI::VDVideoDisplayMinidriverGDI()
	: mhwnd(0)
	, mhdc(0)
	, mhbm(0)
	, mpal(0)
	, mpLogicalPalette(NULL)
	, mbValid(false)
	, mbUseSubrect(false)
{
	memset(&mSource, 0, sizeof mSource);
}

VDVideoDisplayMinidriverGDI::~VDVideoDisplayMinidriverGDI() {
}

bool VDVideoDisplayMinidriverGDI::Init(HWND hwnd, const VDVideoDisplaySourceInfo& info) {
	switch(info.pixmap.format) {
	case nsVDPixmap::kPixFormat_Pal8:
	case nsVDPixmap::kPixFormat_XRGB1555:
	case nsVDPixmap::kPixFormat_RGB565:
	case nsVDPixmap::kPixFormat_RGB888:
	case nsVDPixmap::kPixFormat_XRGB8888:
		break;

	case nsVDPixmap::kPixFormat_YUV422_YUYV:
	case nsVDPixmap::kPixFormat_YUV422_UYVY:
	case nsVDPixmap::kPixFormat_YUV444_Planar:
	case nsVDPixmap::kPixFormat_YUV422_Planar:
	case nsVDPixmap::kPixFormat_YUV420_Planar:
	case nsVDPixmap::kPixFormat_YUV411_Planar:
	case nsVDPixmap::kPixFormat_YUV410_Planar:
	case nsVDPixmap::kPixFormat_Y8:
		if (!info.bAllowConversion)
	default:
			return false;
	}
	
	mhwnd	= hwnd;
	mSource	= info;

	if (HDC hdc = GetDC(mhwnd)) {
		mScreenFormat = GetScreenIntermediatePixmapFormat(hdc);

		mhdc = CreateCompatibleDC(hdc);

		if (mhdc) {
			bool bPaletted = 0 != (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE);

			mbPaletted = bPaletted;

			if (bPaletted) {
				struct {
					BITMAPINFOHEADER hdr;
					RGBQUAD pal[256];
				} bih;

				bih.hdr.biSize			= sizeof(BITMAPINFOHEADER);
				bih.hdr.biWidth			= mSource.pixmap.w;
				bih.hdr.biHeight		= mSource.pixmap.h;
				bih.hdr.biPlanes		= 1;
				bih.hdr.biCompression	= BI_RGB;
				bih.hdr.biBitCount		= 8;

				mPitch = ((mSource.pixmap.w + 3) & ~3);
				bih.hdr.biSizeImage		= mPitch * mSource.pixmap.h;
				bih.hdr.biClrUsed		= 216;
				bih.hdr.biClrImportant	= 216;

				for(int i=0; i<216; ++i) {
					bih.pal[i].rgbRed	= (BYTE)((i / 36) * 51);
					bih.pal[i].rgbGreen	= (BYTE)(((i%36) / 6) * 51);
					bih.pal[i].rgbBlue	= (BYTE)((i%6) * 51);
					bih.pal[i].rgbReserved = 0;
				}

				for(int j=0; j<256; ++j)
					mIdentTab[j] = (uint8)j;

				mhbm = CreateDIBSection(hdc, (const BITMAPINFO *)&bih, DIB_RGB_COLORS, &mpBitmapBits, mSource.pSharedObject, mSource.sharedOffset);
			} else if (mSource.pixmap.format == nsVDPixmap::kPixFormat_Pal8) {
				struct {
					BITMAPINFOHEADER hdr;
					RGBQUAD pal[256];
				} bih;

				bih.hdr.biSize			= sizeof(BITMAPINFOHEADER);
				bih.hdr.biWidth			= mSource.pixmap.w;
				bih.hdr.biHeight		= mSource.pixmap.h;
				bih.hdr.biPlanes		= 1;
				bih.hdr.biCompression	= BI_RGB;
				bih.hdr.biBitCount		= 8;

				mPitch = ((mSource.pixmap.w + 3) & ~3);
				bih.hdr.biSizeImage		= mPitch * mSource.pixmap.h;
				bih.hdr.biClrUsed		= 256;
				bih.hdr.biClrImportant	= 256;

				for(int i=0; i<256; ++i) {
					bih.pal[i].rgbRed	= (uint8)(mSource.pixmap.palette[i] >> 16);
					bih.pal[i].rgbGreen	= (uint8)(mSource.pixmap.palette[i] >> 8);
					bih.pal[i].rgbBlue	= (uint8)mSource.pixmap.palette[i];
					bih.pal[i].rgbReserved = 0;
				}

				mhbm = CreateDIBSection(hdc, (const BITMAPINFO *)&bih, DIB_RGB_COLORS, &mpBitmapBits, mSource.pSharedObject, mSource.sharedOffset);
			} else {
				BITMAPV4HEADER bih = {0};

				bih.bV4Size				= sizeof(BITMAPINFOHEADER);
				bih.bV4Width			= mSource.pixmap.w;
				bih.bV4Height			= mSource.pixmap.h;
				bih.bV4Planes			= 1;
				bih.bV4V4Compression	= BI_RGB;
				bih.bV4BitCount			= (WORD)(mSource.bpp << 3);

				switch(mSource.pixmap.format) {
				case nsVDPixmap::kPixFormat_XRGB1555:
				case nsVDPixmap::kPixFormat_RGB888:
				case nsVDPixmap::kPixFormat_XRGB8888:
					break;
				case nsVDPixmap::kPixFormat_YUV422_YUYV:
				case nsVDPixmap::kPixFormat_YUV422_UYVY:
				case nsVDPixmap::kPixFormat_YUV444_Planar:
				case nsVDPixmap::kPixFormat_YUV422_Planar:
				case nsVDPixmap::kPixFormat_YUV420_Planar:
				case nsVDPixmap::kPixFormat_YUV411_Planar:
				case nsVDPixmap::kPixFormat_YUV410_Planar:
				case nsVDPixmap::kPixFormat_Y8:
				case nsVDPixmap::kPixFormat_RGB565:
					switch(mScreenFormat) {
					case nsVDPixmap::kPixFormat_XRGB1555:
						bih.bV4BitCount			= 16;
						break;
					case nsVDPixmap::kPixFormat_RGB565:
						bih.bV4V4Compression	= BI_BITFIELDS;
						bih.bV4RedMask			= 0xf800;
						bih.bV4GreenMask		= 0x07e0;
						bih.bV4BlueMask			= 0x001f;
						bih.bV4BitCount			= 16;
						break;
					case nsVDPixmap::kPixFormat_RGB888:
						bih.bV4BitCount			= 24;
						break;
					case nsVDPixmap::kPixFormat_XRGB8888:
						bih.bV4BitCount			= 32;
						break;
					}
					break;
				default:
					return false;
				}

				mPitch = ((mSource.pixmap.w * bih.bV4BitCount + 31)>>5)*4;
				bih.bV4SizeImage		= mPitch * mSource.pixmap.h;
				mhbm = CreateDIBSection(hdc, (const BITMAPINFO *)&bih, DIB_RGB_COLORS, &mpBitmapBits, mSource.pSharedObject, mSource.sharedOffset);
			}

			if (mhbm) {
				mhbmOld = SelectObject(mhdc, mhbm);

				if (mhbmOld) {
					ReleaseDC(mhwnd, hdc);
					VDDEBUG_DISP("VideoDisplay: Using GDI for %dx%d %s display.\n", mSource.pixmap.w, mSource.pixmap.h, VDPixmapGetInfo(mSource.pixmap.format).name);
					mbValid = (mSource.pSharedObject != 0);
					return true;
				}

				if (mSource.pSharedObject && mSource.sharedOffset >= 65536)
					UnmapViewOfFile(mpBitmapBits);		// Workaround for GDI memory leak in NT4

				DeleteObject(mhbm);
				mhbm = 0;
			}
			DeleteDC(mhdc);
			mhdc = 0;
		}

		ReleaseDC(mhwnd, hdc);
	}

	Shutdown();
	return false;
}

void VDVideoDisplayMinidriverGDI::Shutdown() {
	if (mhbm) {
		SelectObject(mhdc, mhbmOld);
		DeleteObject(mhbm);
		if (mSource.pSharedObject && mSource.sharedOffset >= 65536)
			UnmapViewOfFile(mpBitmapBits);		// Workaround for GDI memory leak in NT4
		mhbm = 0;
	}

	if (mhdc) {
		DeleteDC(mhdc);
		mhdc = 0;
	}

	mbValid = false;
}

bool VDVideoDisplayMinidriverGDI::ModifySource(const VDVideoDisplaySourceInfo& info) {
	if (!mhdc)
		return false;

	if (!mSource.pSharedObject && mSource.pixmap.w == info.pixmap.w && mSource.pixmap.h == info.pixmap.h && mSource.pixmap.format == info.pixmap.format) {
		mSource = info;
		return true;
	}

	return false;
}

bool VDVideoDisplayMinidriverGDI::Update(UpdateMode mode) {
	if (!mSource.pixmap.data)
		return false;

	if (!mSource.pSharedObject) {
		GdiFlush();

		VDPixmap source(mSource.pixmap);

		char *dst = (char *)mpBitmapBits + mPitch*(source.h - 1);
		ptrdiff_t dstpitch = -mPitch;

		if (mSource.bInterlaced && (mode & kModeFieldMask) != kModeAllFields) {
			if ((mode & kModeFieldMask) == kModeOddField) {
				source.data = (char *)source.data + source.pitch;
				source.h >>= 1;
				dst += dstpitch;
			} else {
				source.h = (source.h + 1) >> 1;
			}

			source.pitch += source.pitch;
			dstpitch += dstpitch;
		}

		VDPixmap dstbm = { dst, NULL, source.w, source.h, dstpitch, source.format };

		if (mbPaletted) {
			dstbm.format = nsVDPixmap::kPixFormat_Pal8;

			VDDitherImage(dstbm, source, mIdentTab);
		} else {
			switch(source.format) {
			case nsVDPixmap::kPixFormat_YUV422_UYVY:
			case nsVDPixmap::kPixFormat_YUV422_YUYV:
			case nsVDPixmap::kPixFormat_YUV444_Planar:
			case nsVDPixmap::kPixFormat_YUV422_Planar:
			case nsVDPixmap::kPixFormat_YUV420_Planar:
			case nsVDPixmap::kPixFormat_YUV411_Planar:
			case nsVDPixmap::kPixFormat_YUV410_Planar:
			case nsVDPixmap::kPixFormat_Y8:
				dstbm.format = mScreenFormat;
				break;
			}

			VDPixmapBlt(dstbm, source);
		}

		if (mbDisplayDebugInfo) {
			int saveIndex = SaveDC(mhdc);
			if (saveIndex) {
				SetTextColor(mhdc, RGB(255, 255, 0));
				SetBkColor(mhdc, RGB(0, 0, 0));
				SetBkMode(mhdc, OPAQUE);
				SetTextAlign(mhdc, TA_BOTTOM | TA_LEFT);
				SelectObject(mhdc, GetStockObject(DEFAULT_GUI_FONT));

				VDStringA desc;
				GetFormatString(mSource, desc);
				VDStringA s;
				s.sprintf("GDI minidriver - %s", desc.c_str());

				TextOut(mhdc, 10, source.h - 10, s.data(), s.size());
				RestoreDC(mhdc, saveIndex);
			}
		}

		mbValid = true;
	}

	return true;
}

void VDVideoDisplayMinidriverGDI::Refresh(UpdateMode mode) {
	if (mbValid) {
		if (HDC hdc = GetDC(mhwnd)) {
			RECT r;

			GetClientRect(mhwnd, &r);
			InternalRefresh(hdc, r, mode);
			ReleaseDC(mhwnd, hdc);
		}
	}
}

bool VDVideoDisplayMinidriverGDI::Paint(HDC hdc, const RECT& rClient, UpdateMode mode) {
	InternalRefresh(hdc, rClient, mode);
	return true;
}

bool VDVideoDisplayMinidriverGDI::SetSubrect(const vdrect32 *r) {
	if (r) {
		mbUseSubrect = true;
		mSubrect = *r;
	} else
		mbUseSubrect = false;

	return true;
}

void VDVideoDisplayMinidriverGDI::InternalRefresh(HDC hdc, const RECT& rClient, UpdateMode mode) {
	SetStretchBltMode(hdc, COLORONCOLOR);

	const VDPixmap& source = mSource.pixmap;

	vdrect32 r;
	if (mbUseSubrect)
		r = mSubrect;
	else
		r.set(0, 0, source.w, source.h);

	if (mColorOverride) {
		SetBkColor(hdc, VDSwizzleU32(mColorOverride) >> 8);
		SetBkMode(hdc, OPAQUE);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rClient, "", 0, NULL);
		return;
	}

	if (mSource.bInterlaced) {
		int fieldMode = mode & kModeFieldMask;
		uint32 vinc		= (r.height() << 16) / rClient.bottom;
		uint32 vaccum	= (vinc >> 1) + (r.top << 16);
		uint32 vtlimit	= (((r.height() + 1) >> 1) << 17) - 1;
		int fieldbase	= (fieldMode == kModeOddField ? 1 : 0);
		int ystep		= (fieldMode == kModeAllFields) ? 1 : 2;

		vaccum += vinc*fieldbase;
		vinc *= ystep;

		for(int y = fieldbase; y < rClient.bottom; y += ystep) {
			int v;

			if (y & 1) {
				uint32 vt = vaccum < 0x8000 ? 0 : vaccum - 0x8000;

				v = (y&1) + ((vt>>16) & ~1);
			} else {
				uint32 vt = vaccum + 0x8000;

				if (vt > vtlimit)
					vt = vtlimit;

				v = (vt>>16) & ~1;
			}

			StretchBlt(hdc, 0, y, rClient.right, 1, mhdc, r.left, v, r.width(), 1, SRCCOPY);
			vaccum += vinc;
		}
	} else {
		StretchBlt(hdc, 0, 0, rClient.right, rClient.bottom, mhdc, r.left, r.top, r.width(), r.height(), SRCCOPY);
	}
}

int VDVideoDisplayMinidriverGDI::GetScreenIntermediatePixmapFormat(HDC hdc) {
	int pxformat = 0;

	// First, get the depth of the screen and guess that way.
	int depth = GetDeviceCaps(hdc, BITSPIXEL);

	if (depth < 24)
		pxformat = nsVDPixmap::kPixFormat_RGB565;
	else if (depth < 32)
		pxformat = nsVDPixmap::kPixFormat_RGB888;
	else
		pxformat = nsVDPixmap::kPixFormat_XRGB8888;

	// If the depth is 16-bit, attempt to determine the exact format.
	if (HBITMAP hbm = CreateCompatibleBitmap(hdc, 1, 1)) {
		struct {
			BITMAPV5HEADER hdr;
			RGBQUAD buf[256];
		} format={0};

		if (GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)&format, DIB_RGB_COLORS)
			&& GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)&format, DIB_RGB_COLORS))
		{
			if (format.hdr.bV5Size >= sizeof(BITMAPINFOHEADER)) {
				const BITMAPV5HEADER& hdr = format.hdr;

				if (hdr.bV5Planes == 1) {
					if (hdr.bV5Compression == BI_BITFIELDS) {
						if (hdr.bV5BitCount == 16 && hdr.bV5RedMask == 0x7c00 && hdr.bV5GreenMask == 0x03e0 && hdr.bV5BlueMask == 0x7c00)
							pxformat = nsVDPixmap::kPixFormat_XRGB1555;
						else if (hdr.bV5BitCount == 16 && hdr.bV5RedMask == 0xf800 && hdr.bV5GreenMask == 0x07e0 && hdr.bV5BlueMask == 0x7c00)
							pxformat = nsVDPixmap::kPixFormat_RGB565;
						else if (hdr.bV5BitCount == 24 && hdr.bV5RedMask == 0xff0000 && hdr.bV5GreenMask == 0x00ff00 && hdr.bV5BlueMask == 0x0000ff)
							pxformat = nsVDPixmap::kPixFormat_RGB888;
						else if (hdr.bV5BitCount == 32 && hdr.bV5RedMask == 0x00ff0000 && hdr.bV5GreenMask == 0x0000ff00 && hdr.bV5BlueMask == 0x000000ff)
							pxformat = nsVDPixmap::kPixFormat_XRGB8888;
					} else if (hdr.bV5Compression == BI_RGB) {
						if (hdr.bV5BitCount == 16)
							pxformat = nsVDPixmap::kPixFormat_XRGB1555;
						else if (hdr.bV5BitCount == 24)
							pxformat = nsVDPixmap::kPixFormat_RGB888;
						else if (hdr.bV5BitCount == 32)
							pxformat = nsVDPixmap::kPixFormat_XRGB8888;
					}
				}
			}
		}

		DeleteObject(hbm);
	}

	return pxformat;
}
