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

///////////////////////////////////////////////////////////////////////////
//
//                                 WARNING
//
// This code is heavily based off of the Java MPEG video player written by
// Joerg Anders.  Because his code was released under the GNU GPL v2, this
// means VirtualDub must also be released under GNU GPL v2 when MPEG
// support is included.
//
// (Like that's any different.)
//
// This code is really nasty...
//
///////////////////////////////////////////////////////////////////////////

#include "mpeg_tables.h"

const char mpeg_macro_block_inc_decode[96*2]={
0,8,	0,8,	0,8,	0,8,	0,8,	0,8,	0,8,	0,8,
-2,11,	0,11,	0,10,	0,10,	0,10,	0,10,	0,11,	-1,11,
0,8,	0,8,	0,8,	0,8,	0,8,	0,8,	0,8,	0,8,
33,11,	32,11,	31,11,	30,11,	29,11,	28,11,	27,11,	26,11,
25,11,	24,11,	23,11,	22,11,	21,10,	21,10,	20,10,	20,10,
19,10,	19,10,	18,10,	18,10,	17,10,	17,10,	16,10,	16,10,
15,8,	15,8,	15,8,	15,8,	15,8,	15,8,	15,8,	15,8,
14,8,	14,8,	14,8,	14,8,	14,8,	14,8,	14,8,	14,8,
13,8,	13,8,	13,8,	13,8,	13,8,	13,8,	13,8,	13,8,
12,8,	12,8,	12,8,	12,8,	12,8,	12,8,	12,8,	12,8,
11,8,	11,8,	11,8,	11,8,	11,8,	11,8,	11,8,	11,8,
10,8,	10,8,	10,8,	10,8,	10,8,	10,8,	10,8,	10,8,
};

const char mpeg_macro_block_inc_decode2[58*2]={
												9,7,	8,7,
7,5,	7,5,	7,5,	7,5,	6,5,	6,5,	6,5,	6,5,
5,4,	5,4,	5,4,	5,4,	5,4,	5,4,	5,4,	5,4,
4,4,	4,4,	4,4,	4,4,	4,4,	4,4,	4,4,	4,4,
3,3,	3,3,	3,3,	3,3,	3,3,	3,3,	3,3,	3,3,
3,3,	3,3,	3,3,	3,3,	3,3,	3,3,	3,3,	3,3,
2,3,	2,3,	2,3,	2,3,	2,3,	2,3,	2,3,	2,3,
2,3,	2,3,	2,3,	2,3,	2,3,	2,3,	2,3,	2,3,
};

const unsigned char mpeg_block_pattern_decode1[128*2]={
0,8,	0,8,	39,9,	27,9,	59,9,	55,9,	47,9,	31,9,
58,8,	58,8,	54,8,	54,8,	46,8,	46,8,	30,8,	30,8,
57,8,	57,8,	53,8,	53,8,	45,8,	45,8,	29,8,	29,8,
38,8,	38,8,	26,8,	26,8,	37,8,	37,8,	25,8,	25,8,
43,8,	43,8,	23,8,	23,8,	51,8,	51,8,	15,8,	15,8,
42,8,	42,8,	22,8,	22,8,	50,8,	50,8,	14,8,	14,8,
41,8,	41,8,	21,8,	21,8,	49,8,	49,8,	13,8,	13,8,
35,8,	35,8,	19,8,	19,8,	11,8,	11,8,	7,8,	7,8,
34,7,	34,7,	34,7,	34,7,	18,7,	18,7,	18,7,	18,7,
10,7,	10,7,	10,7,	10,7,	6,7,	6,7,	6,7,	6,7,
33,7,	33,7,	33,7,	33,7,	17,7,	17,7,	17,7,	17,7,
9,7,	9,7,	9,7,	9,7,	5,7,	5,7,	5,7,	5,7,
63,6,	63,6,	63,6,	63,6,	63,6,	63,6,	63,6,	63,6,
3,6,	3,6,	3,6,	3,6,	3,6,	3,6,	3,6,	3,6,
36,6,	36,6,	36,6,	36,6,	36,6,	36,6,	36,6,	36,6,
24,6,	24,6,	24,6,	24,6,	24,6,	24,6,	24,6,	24,6,
};

const unsigned char mpeg_block_pattern_decode0[24*2]={
62,5,	2,5,	61,5,	1,5,	56,5,	52,5,	44,5,	28,5,
40,5,	20,5,	48,5,	12,5,	32,4,	32,4,	16,4,	16,4,
8,4,	8,4,	4,4,	4,4,	60,3,	60,3,	60,3,	60,3,
};

const unsigned char mpeg_dct_size_luminance_decode[64*2]={
0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,
0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,	0,	3,
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,
4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,	4,	3,
5,	4,	5,	4,	5,	4,	5,	4,	5,	4,	5,	4,	5,	4,	5,	4,
6,	5,	6,	5,	6,	5,	6,	5,	7,	6,	7,	6,	8,	7,	0,	7,
};

const unsigned char mpeg_dct_size_chrominance_decode[64*2]={
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,
4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,
5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,
6,	6,	6,	6,	6,	6,	6,	6,	7,	7,	7,	7,	8,	8,	0,	8,
};

const unsigned char mpeg_dct_coeff_decode2[512*4]={
0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,
0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,	0,0,5+1,0,
1,18,9+1,0,	1,17,9+1,0,	1,16,9+1,0,	1,15,9+1,0,	6,3,9+1,0,	16,2,9+1,0,	15,2,9+1,0,	14,2,9+1,0,
13,2,9+1,0,	12,2,9+1,0,	11,2,9+1,0,	31,1,9+1,0,	30,1,9+1,0,	29,1,9+1,0,	28,1,9+1,0,	27,1,9+1,0,
0,40,8+1,0,	0,40,8+1,0,	0,39,8+1,0,	0,39,8+1,0,	0,38,8+1,0,	0,38,8+1,0,	0,37,8+1,0,	0,37,8+1,0,
0,36,8+1,0,	0,36,8+1,0,	0,35,8+1,0,	0,35,8+1,0,	0,34,8+1,0,	0,34,8+1,0,	0,33,8+1,0,	0,33,8+1,0,
0,32,8+1,0,	0,32,8+1,0,	1,14,8+1,0,	1,14,8+1,0,	1,13,8+1,0,	1,13,8+1,0,	1,12,8+1,0,	1,12,8+1,0,
1,11,8+1,0,	1,11,8+1,0,	1,10,8+1,0,	1,10,8+1,0,	1,9,8+1,0,	1,9,8+1,0,	1,8,8+1,0,	1,8,8+1,0,
0,31,7+1,0,	0,31,7+1,0,	0,31,7+1,0,	0,31,7+1,0,	0,30,7+1,0,	0,30,7+1,0,	0,30,7+1,0,	0,30,7+1,0,
0,29,7+1,0,	0,29,7+1,0,	0,29,7+1,0,	0,29,7+1,0,	0,28,7+1,0,	0,28,7+1,0,	0,28,7+1,0,	0,28,7+1,0,
0,27,7+1,0,	0,27,7+1,0,	0,27,7+1,0,	0,27,7+1,0,	0,26,7+1,0,	0,26,7+1,0,	0,26,7+1,0,	0,26,7+1,0,
0,25,7+1,0,	0,25,7+1,0,	0,25,7+1,0,	0,25,7+1,0,	0,24,7+1,0,	0,24,7+1,0,	0,24,7+1,0,	0,24,7+1,0,
0,23,7+1,0,	0,23,7+1,0,	0,23,7+1,0,	0,23,7+1,0,	0,22,7+1,0,	0,22,7+1,0,	0,22,7+1,0,	0,22,7+1,0,
0,21,7+1,0,	0,21,7+1,0,	0,21,7+1,0,	0,21,7+1,0,	0,20,7+1,0,	0,20,7+1,0,	0,20,7+1,0,	0,20,7+1,0,
0,19,7+1,0,	0,19,7+1,0,	0,19,7+1,0,	0,19,7+1,0,	0,18,7+1,0,	0,18,7+1,0,	0,18,7+1,0,	0,18,7+1,0,
0,17,7+1,0,	0,17,7+1,0,	0,17,7+1,0,	0,17,7+1,0,	0,16,7+1,0,	0,16,7+1,0,	0,16,7+1,0,	0,16,7+1,0,
10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,	10,2,6+1,0,
9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,	9,2,6+1,0,
5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,	5,3,6+1,0,
3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,	3,4,6+1,0,
2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,	2,5,6+1,0,
1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,	1,7,6+1,0,
1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,	1,6,6+1,0,
0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,	0,15,6+1,0,
0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,	0,14,6+1,0,
0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,	0,13,6+1,0,
0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,	0,12,6+1,0,
26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,	26,1,6+1,0,
25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,	25,1,6+1,0,
24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,	24,1,6+1,0,
23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,	23,1,6+1,0,
22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,	22,1,6+1,0,
0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,
0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,	0,11,5+1,0,
8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,
8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,	8,2,5+1,0,
4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,
4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,	4,3,5+1,0,
0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,
0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,	0,10,5+1,0,
2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,
2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,	2,4,5+1,0,
7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,
7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,	7,2,5+1,0,
21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,
21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,	21,1,5+1,0,
20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,
20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,	20,1,5+1,0,
0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,
0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,	0,9,5+1,0,
19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,
19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,	19,1,5+1,0,
18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,
18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,	18,1,5+1,0,
1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,
1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,	1,5,5+1,0,
3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,
3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,	3,3,5+1,0,
0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,
0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,	0,8,5+1,0,
6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,
6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,	6,2,5+1,0,
17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,
17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,	17,1,5+1,0,
};

const unsigned char mpeg_dct_coeff_decode1[8*2]={
16,1,
5,2,
0,7,
2,3,
1,4,
15,1,
14,1,
4,2,
};

const unsigned char mpeg_dct_coeff_decode0[88*4]={
 3,2,8,0x10,	 3,2,8,0x10,	10,1,8,0x10,	10,1,8,0x10,
 1,4,8,0x10,	 1,4,8,0x10,	 9,1,8,0x10,	 9,1,8,0x10,

 8,1,7,0x20,	 8,1,7,0x20,	 8,1,7,0x20,	 8,1,7,0x20,
 7,1,7,0x20,	 7,1,7,0x20,	 7,1,7,0x20,	 7,1,7,0x20,
 2,2,7,0x20,	 2,2,7,0x20,	 2,2,7,0x20,	 2,2,7,0x20,
 6,1,7,0x20,	 6,1,7,0x20,	 6,1,7,0x20,	 6,1,7,0x20,

14,1,9,0x08,	 1,6,9,0x08,	13,1,9,0x08,	12,1,9,0x08,
 4,2,9,0x08,	 2,3,9,0x08,	 1,5,9,0x08,	11,1,9,0x08,

 1,3,6,0x40,	 1,3,6,0x40,	 1,3,6,0x40,	 1,3,6,0x40,
 1,3,6,0x40,	 1,3,6,0x40,	 1,3,6,0x40,	 1,3,6,0x40,
 5,1,6,0x40,	 5,1,6,0x40,	 5,1,6,0x40,	 5,1,6,0x40,
 5,1,6,0x40,	 5,1,6,0x40,	 5,1,6,0x40,	 5,1,6,0x40,
 4,1,6,0x40,	 4,1,6,0x40,	 4,1,6,0x40,	 4,1,6,0x40,
 4,1,6,0x40,	 4,1,6,0x40,	 4,1,6,0x40,	 4,1,6,0x40,

 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,
 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,
 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,
 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,	 1,2,5,0x80,
 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,
 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,
 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,
 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,	 3,1,5,0x80,
};

const unsigned char mpeg_p_type_mb_type_decode[32*2]={
	0x00,	6,
	0x11,	6,
	0x12,	5,
	0x12,	5,
	0x1a,	5,
	0x1a,	5,
	0x01,	5,
	0x01,	5,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x08,	3,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
	0x02,	2,
};

const unsigned char mpeg_b_type_mb_type_decode[32*2]={
	0,		6,
	17,		6,
	22,		6,
	26,		6,
	30,		5,
	30,		5,
	1,		5,
	1,		5,
	8,		4,
	8,		4,
	8,		4,
	8,		4,
	10,		4,
	10,		4,
	10,		4,
	10,		4,
	4,		3,
	4,		3,
	4,		3,
	4,		3,
	4,		3,
	4,		3,
	4,		3,
	4,		3,
	6,		3,
	6,		3,
	6,		3,
	6,		3,
	6,		3,
	6,		3,
	6,		3,
	6,		3,
};

const char mpeg_motion_code_decode[96*2]={
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	7,
0,	8,
0,	8,
0,	8,
0,	8,
0,	8,
0,	8,
0,	8,
0,	8,
16,	11,
-16,	11,
15,	11,
-15,	11,
14,	11,
-14,	11,
13,	11,
-13,	11,
12,	11,
-12,	11,
11,	11,
-11,	11,
10,	10,
10,	10,
-10,	10,
-10,	10,
9,	10,
9,	10,
-9,	10,
-9,	10,
8,	10,
8,	10,
-8,	10,
-8,	10,
7,	8,
7,	8,
7,	8,
7,	8,
7,	8,
7,	8,
7,	8,
7,	8,
-7,	8,
-7,	8,
-7,	8,
-7,	8,
-7,	8,
-7,	8,
-7,	8,
-7,	8,
6,	8,
6,	8,
6,	8,
6,	8,
6,	8,
6,	8,
6,	8,
6,	8,
-6,	8,
-6,	8,
-6,	8,
-6,	8,
-6,	8,
-6,	8,
-6,	8,
-6,	8,
5,	8,
5,	8,
5,	8,
5,	8,
5,	8,
5,	8,
5,	8,
5,	8,
-5,	8,
-5,	8,
-5,	8,
-5,	8,
-5,	8,
-5,	8,
-5,	8,
-5,	8,
};

const char mpeg_motion_code_decode2[58*2]={
4,	7,	-4,	7,
3,	5,	3,	5,
3,	5,	3,	5,
-3,	5,	-3,	5,
-3,	5,	-3,	5,
2,	4,	2,	4,
2,	4,	2,	4,
2,	4,	2,	4,
2,	4,	2,	4,
-2,	4,	-2,	4,
-2,	4,	-2,	4,
-2,	4,	-2,	4,
-2,	4,	-2,	4,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
1,	3,	1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
-1,	3,	-1,	3,
};
