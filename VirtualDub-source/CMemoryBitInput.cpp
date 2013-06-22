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

#include "CMemoryBitInput.h"

CMemoryBitInput::CMemoryBitInput() {
}

CMemoryBitInput::CMemoryBitInput(char *buffer) {
	buf = buf_start = (unsigned char *)buffer;
//	bitcnt = 0;

	bitheap = ((unsigned long)buf[0] << 24) | ((unsigned long)buf[1] << 16) | ((unsigned long)buf[2] << 8) | (unsigned long)buf[3];
//	bitcnt = 32;
	bitcnt = -8;
	buf += 4;
}
