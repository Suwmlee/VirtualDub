//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2000 Avery Lee
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

#include <stdio.h>

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include "filter.h"
#include "f_convolute.h"
#include "ScriptInterpreter.h"
#include "ScriptError.h"
#include "ScriptValue.h"

extern HINSTANCE g_hInst;

struct MyFilterData {
	ConvoluteFilterData cfd;
	LONG height;
	char direction;
	BOOL rounded;
};

static int emboss_init(FilterActivation *fa, const FilterFunctions *ff) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	mfd->direction		= 0;
	mfd->height			= 16;
	mfd->cfd.bias	= 128*256 + 128;
	mfd->cfd.fClip	= TRUE;

	mfd->cfd.m[5] = -16;
	mfd->cfd.m[3] = 16;

	return 0;
}

static BOOL APIENTRY embossDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {
    switch (message)
    {
        case WM_INITDIALOG:
			{
				MyFilterData *mfd = (MyFilterData *)lParam;
				HWND hWnd;

				hWnd = GetDlgItem(hDlg, IDC_HEIGHT);
				SendMessage(hWnd, TBM_SETTICFREQ, 16, 0);
				SendMessage(hWnd, TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(1, 256));
				SendMessage(hWnd, TBM_SETPOS, (WPARAM)TRUE, mfd->height);
				CheckDlgButton(hDlg, IDC_DIR_MIDDLERIGHT+mfd->direction, TRUE);

				SetWindowLong(hDlg, DWL_USER, (LONG)mfd);
			}
            return (TRUE);

        case WM_COMMAND:                      
            if (LOWORD(wParam) == IDOK) {
				MyFilterData *mfd = (struct MyFilterData *)GetWindowLong(hDlg, DWL_USER);
				int i;

				mfd->height = SendMessage(GetDlgItem(hDlg, IDC_HEIGHT), TBM_GETPOS, 0, 0);

				for(i=0; i<8; i++)
					if (IsDlgButtonChecked(hDlg, IDC_DIR_MIDDLERIGHT+i)) {
						mfd->direction = i;
						break;
					}

				mfd->rounded = IsDlgButtonChecked(hDlg, IDC_ROUNDED);

				EndDialog(hDlg, 0);
				return TRUE;
			} else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, 1);  
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static const char translate[]={ 5,2,1,0,3,6,7,8 };

static int emboss_config(FilterActivation *fa, const FilterFunctions *ff, HWND hWnd) {
//	static char translate[]={ 3,6,7,8,5,2,1,0 };
//	static char translate[]={ 5,8,7,6,3,0,1,2 };
	MyFilterData *mfd;
	int ret;

	if (!(mfd = (MyFilterData *)fa->filter_data)) {
		if (!(fa->filter_data = (void *)new MyFilterData)) return 0;
		mfd = (MyFilterData *)fa->filter_data;

		memset(mfd, 0, sizeof MyFilterData);
		mfd->height = 16;
	}

	ret = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER_EMBOSS), hWnd, embossDlgProc, (LONG)fa->filter_data);

	memset(mfd->cfd.m, 0, sizeof mfd->cfd.m);
	mfd->cfd.bias	= 128*256 + 128;
	mfd->cfd.fClip	= TRUE;

	mfd->cfd.m[translate[mfd->direction]] = -mfd->height;
	mfd->cfd.m[translate[(mfd->direction+4) & 7]] = mfd->height;
	if (mfd->rounded) {
		mfd->cfd.m[translate[(mfd->direction+1) & 7]] =
		mfd->cfd.m[translate[(mfd->direction+7) & 7]] = -(mfd->height+1)/2;
		mfd->cfd.m[translate[(mfd->direction+3) & 7]] =
		mfd->cfd.m[translate[(mfd->direction+5) & 7]] = (mfd->height+1)/2;
	}

	return ret;
}

static void emboss_string(const FilterActivation *fa, const FilterFunctions *ff, char *buf) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	sprintf(buf," (%c%c light, height %d)", "MTTTMBBB"[mfd->direction], "RRCLLLCR"[mfd->direction], mfd->height);
}

static void emboss_script_config(IScriptInterpreter *isi, void *lpVoid, CScriptValue *argv, int argc) {
	FilterActivation *fa = (FilterActivation *)lpVoid;
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	mfd->direction	= argv[0].asInt();
	mfd->height		= argv[1].asInt();

	memset(mfd->cfd.m, 0, sizeof mfd->cfd.m);
	mfd->cfd.bias	= 128*256 + 128;
	mfd->cfd.fClip	= TRUE;

	mfd->cfd.m[translate[mfd->direction]] = -mfd->height;
	mfd->cfd.m[translate[(mfd->direction+4) & 7]] = mfd->height;
	if (mfd->rounded) {
		mfd->cfd.m[translate[(mfd->direction+1) & 7]] =
		mfd->cfd.m[translate[(mfd->direction+7) & 7]] = -(mfd->height+1)/2;
		mfd->cfd.m[translate[(mfd->direction+3) & 7]] =
		mfd->cfd.m[translate[(mfd->direction+5) & 7]] = (mfd->height+1)/2;
	}
}

static ScriptFunctionDef emboss_func_defs[]={
	{ (ScriptFunctionPtr)emboss_script_config, "Config", "0ii" },
	{ NULL },
};

static CScriptObject emboss_obj={
	NULL, emboss_func_defs
};

static bool emboss_script_line(FilterActivation *fa, const FilterFunctions *ff, char *buf, int buflen) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	_snprintf(buf, buflen, "Config(%d,%d)", mfd->direction, mfd->height);

	return true;
}

FilterDefinition filterDef_emboss={
	0,0,NULL,
	"emboss",
	"Converts edges and gradiations in an image to shades, producing a 3D-like emboss effect.\n\n[Assembly optimized] [Dynamic compilation]",
	NULL,NULL,
	sizeof(MyFilterData),
	emboss_init,				NULL,
	filter_convolute_run,
	filter_convolute_param,
	emboss_config,
	emboss_string,
	filter_convolute_start,
	filter_convolute_end,
	&emboss_obj,
	emboss_script_line,
};