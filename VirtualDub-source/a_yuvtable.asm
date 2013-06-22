;	VirtualDub - Video processing and capture application
;	Copyright (C) 1998-2000 Avery Lee
;
;	This program is free software; you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation; either version 2 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program; if not, write to the Free Software
;	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	.486
	.model	flat
	.const

	public _YUV_Y_table
	public _YUV_U_table
	public _YUV_V_table
	public _YUV_clip_table
	public _YUV_clip_table16

_YUV_Y_table	dd	1FED1FEDh, 1FEF1FEFh, 1FF01FF0h, 1FF11FF1h
		dd	1FF21FF2h, 1FF31FF3h, 1FF41FF4h, 1FF61FF6h
		dd	1FF71FF7h, 1FF81FF8h, 1FF91FF9h, 1FFA1FFAh
		dd	1FFB1FFBh, 1FFD1FFDh, 1FFE1FFEh, 1FFF1FFFh
		dd	20002000h, 20012001h, 20022002h, 20032003h
		dd	20052005h, 20062006h, 20072007h, 20082008h
		dd	20092009h, 200A200Ah, 200C200Ch, 200D200Dh
		dd	200E200Eh, 200F200Fh, 20102010h, 20112011h
		dd	20132013h, 20142014h, 20152015h, 20162016h
		dd	20172017h, 20182018h, 201A201Ah, 201B201Bh
		dd	201C201Ch, 201D201Dh, 201E201Eh, 201F201Fh
		dd	20212021h, 20222022h, 20232023h, 20242024h
		dd	20252025h, 20262026h, 20282028h, 20292029h
		dd	202A202Ah, 202B202Bh, 202C202Ch, 202D202Dh
		dd	202F202Fh, 20302030h, 20312031h, 20322032h
		dd	20332033h, 20342034h, 20362036h, 20372037h
		dd	20382038h, 20392039h, 203A203Ah, 203B203Bh
		dd	203D203Dh, 203E203Eh, 203F203Fh, 20402040h
		dd	20412041h, 20422042h, 20442044h, 20452045h
		dd	20462046h, 20472047h, 20482048h, 20492049h
		dd	204A204Ah, 204C204Ch, 204D204Dh, 204E204Eh
		dd	204F204Fh, 20502050h, 20512051h, 20532053h
		dd	20542054h, 20552055h, 20562056h, 20572057h
		dd	20582058h, 205A205Ah, 205B205Bh, 205C205Ch
		dd	205D205Dh, 205E205Eh, 205F205Fh, 20612061h
		dd	20622062h, 20632063h, 20642064h, 20652065h
		dd	20662066h, 20682068h, 20692069h, 206A206Ah
		dd	206B206Bh, 206C206Ch, 206D206Dh, 206F206Fh
		dd	20702070h, 20712071h, 20722072h, 20732073h
		dd	20742074h, 20762076h, 20772077h, 20782078h
		dd	20792079h, 207A207Ah, 207B207Bh, 207D207Dh
		dd	207E207Eh, 207F207Fh, 20802080h, 20812081h
		dd	20822082h, 20842084h, 20852085h, 20862086h
		dd	20872087h, 20882088h, 20892089h, 208B208Bh
		dd	208C208Ch, 208D208Dh, 208E208Eh, 208F208Fh
		dd	20902090h, 20922092h, 20932093h, 20942094h
		dd	20952095h, 20962096h, 20972097h, 20982098h
		dd	209A209Ah, 209B209Bh, 209C209Ch, 209D209Dh
		dd	209E209Eh, 209F209Fh, 20A120A1h, 20A220A2h
		dd	20A320A3h, 20A420A4h, 20A520A5h, 20A620A6h
		dd	20A820A8h, 20A920A9h, 20AA20AAh, 20AB20ABh
		dd	20AC20ACh, 20AD20ADh, 20AF20AFh, 20B020B0h
		dd	20B120B1h, 20B220B2h, 20B320B3h, 20B420B4h
		dd	20B620B6h, 20B720B7h, 20B820B8h, 20B920B9h
		dd	20BA20BAh, 20BB20BBh, 20BD20BDh, 20BE20BEh
		dd	20BF20BFh, 20C020C0h, 20C120C1h, 20C220C2h
		dd	20C420C4h, 20C520C5h, 20C620C6h, 20C720C7h
		dd	20C820C8h, 20C920C9h, 20CB20CBh, 20CC20CCh
		dd	20CD20CDh, 20CE20CEh, 20CF20CFh, 20D020D0h
		dd	20D220D2h, 20D320D3h, 20D420D4h, 20D520D5h
		dd	20D620D6h, 20D720D7h, 20D920D9h, 20DA20DAh
		dd	20DB20DBh, 20DC20DCh, 20DD20DDh, 20DE20DEh
		dd	20DF20DFh, 20E120E1h, 20E220E2h, 20E320E3h
		dd	20E420E4h, 20E520E5h, 20E620E6h, 20E820E8h
		dd	20E920E9h, 20EA20EAh, 20EB20EBh, 20EC20ECh
		dd	20ED20EDh, 20EF20EFh, 20F020F0h, 20F120F1h
		dd	20F220F2h, 20F320F3h, 20F420F4h, 20F620F6h
		dd	20F720F7h, 20F820F8h, 20F920F9h, 20FA20FAh
		dd	20FB20FBh, 20FD20FDh, 20FE20FEh, 20FF20FFh
		dd	21002100h, 21012101h, 21022102h, 21042104h
		dd	21052105h, 21062106h, 21072107h, 21082108h
		dd	21092109h, 210B210Bh, 210C210Ch, 210D210Dh
		dd	210E210Eh, 210F210Fh, 21102110h, 21122112h
		dd	21132113h, 21142114h, 21152115h, 21162116h

_YUV_U_table	dd	1EFE1032h, 1F001032h, 1F021031h, 1F041031h
		dd	1F061030h, 1F081030h, 1F0A1030h, 1F0C102Fh
		dd	1F0E102Fh, 1F10102Fh, 1F12102Eh, 1F14102Eh
		dd	1F16102Dh, 1F18102Dh, 1F1A102Dh, 1F1C102Ch
		dd	1F1E102Ch, 1F20102Bh, 1F22102Bh, 1F24102Bh
		dd	1F26102Ah, 1F28102Ah, 1F2A1029h, 1F2C1029h
		dd	1F2E1029h, 1F301028h, 1F321028h, 1F341027h
		dd	1F361027h, 1F381027h, 1F3A1026h, 1F3C1026h
		dd	1F3E1026h, 1F401025h, 1F421025h, 1F441024h
		dd	1F461024h, 1F481024h, 1F4A1023h, 1F4C1023h
		dd	1F4E1022h, 1F501022h, 1F521022h, 1F541021h
		dd	1F561021h, 1F591020h, 1F5B1020h, 1F5D1020h
		dd	1F5F101Fh, 1F61101Fh, 1F63101Eh, 1F65101Eh
		dd	1F67101Eh, 1F69101Dh, 1F6B101Dh, 1F6D101Dh
		dd	1F6F101Ch, 1F71101Ch, 1F73101Bh, 1F75101Bh
		dd	1F77101Bh, 1F79101Ah, 1F7B101Ah, 1F7D1019h
		dd	1F7F1019h, 1F811019h, 1F831018h, 1F851018h
		dd	1F871017h, 1F891017h, 1F8B1017h, 1F8D1016h
		dd	1F8F1016h, 1F911016h, 1F931015h, 1F951015h
		dd	1F971014h, 1F991014h, 1F9B1014h, 1F9D1013h
		dd	1F9F1013h, 1FA11012h, 1FA31012h, 1FA51012h
		dd	1FA71011h, 1FA91011h, 1FAB1010h, 1FAD1010h
		dd	1FAF1010h, 1FB1100Fh, 1FB3100Fh, 1FB5100Eh
		dd	1FB7100Eh, 1FB9100Eh, 1FBB100Dh, 1FBD100Dh
		dd	1FBF100Dh, 1FC1100Ch, 1FC3100Ch, 1FC5100Bh
		dd	1FC7100Bh, 1FCA100Bh, 1FCC100Ah, 1FCE100Ah
		dd	1FD01009h, 1FD21009h, 1FD41009h, 1FD61008h
		dd	1FD81008h, 1FDA1007h, 1FDC1007h, 1FDE1007h
		dd	1FE01006h, 1FE21006h, 1FE41005h, 1FE61005h
		dd	1FE81005h, 1FEA1004h, 1FEC1004h, 1FEE1004h
		dd	1FF01003h, 1FF21003h, 1FF41002h, 1FF61002h
		dd	1FF81002h, 1FFA1001h, 1FFC1001h, 1FFE1000h
		dd	20001000h, 20021000h, 20040FFFh, 20060FFFh
		dd	20080FFEh, 200A0FFEh, 200C0FFEh, 200E0FFDh
		dd	20100FFDh, 20120FFCh, 20140FFCh, 20160FFCh
		dd	20180FFBh, 201A0FFBh, 201C0FFBh, 201E0FFAh
		dd	20200FFAh, 20220FF9h, 20240FF9h, 20260FF9h
		dd	20280FF8h, 202A0FF8h, 202C0FF7h, 202E0FF7h
		dd	20300FF7h, 20320FF6h, 20340FF6h, 20360FF5h
		dd	20390FF5h, 203B0FF5h, 203D0FF4h, 203F0FF4h
		dd	20410FF3h, 20430FF3h, 20450FF3h, 20470FF2h
		dd	20490FF2h, 204B0FF2h, 204D0FF1h, 204F0FF1h
		dd	20510FF0h, 20530FF0h, 20550FF0h, 20570FEFh
		dd	20590FEFh, 205B0FEEh, 205D0FEEh, 205F0FEEh
		dd	20610FEDh, 20630FEDh, 20650FECh, 20670FECh
		dd	20690FECh, 206B0FEBh, 206D0FEBh, 206F0FEAh
		dd	20710FEAh, 20730FEAh, 20750FE9h, 20770FE9h
		dd	20790FE9h, 207B0FE8h, 207D0FE8h, 207F0FE7h
		dd	20810FE7h, 20830FE7h, 20850FE6h, 20870FE6h
		dd	20890FE5h, 208B0FE5h, 208D0FE5h, 208F0FE4h
		dd	20910FE4h, 20930FE3h, 20950FE3h, 20970FE3h
		dd	20990FE2h, 209B0FE2h, 209D0FE2h, 209F0FE1h
		dd	20A10FE1h, 20A30FE0h, 20A50FE0h, 20A70FE0h
		dd	20AA0FDFh, 20AC0FDFh, 20AE0FDEh, 20B00FDEh
		dd	20B20FDEh, 20B40FDDh, 20B60FDDh, 20B80FDCh
		dd	20BA0FDCh, 20BC0FDCh, 20BE0FDBh, 20C00FDBh
		dd	20C20FDAh, 20C40FDAh, 20C60FDAh, 20C80FD9h
		dd	20CA0FD9h, 20CC0FD9h, 20CE0FD8h, 20D00FD8h
		dd	20D20FD7h, 20D40FD7h, 20D60FD7h, 20D80FD6h
		dd	20DA0FD6h, 20DC0FD5h, 20DE0FD5h, 20E00FD5h
		dd	20E20FD4h, 20E40FD4h, 20E60FD3h, 20E80FD3h
		dd	20EA0FD3h, 20EC0FD2h, 20EE0FD2h, 20F00FD1h
		dd	20F20FD1h, 20F40FD1h, 20F60FD0h, 20F80FD0h
		dd	20FA0FD0h, 20FC0FCFh, 20FE0FCFh, 21000FCEh

_YUV_V_table	dd	1F341068h, 1F351067h, 1F371066h, 1F391066h
		dd	1F3A1065h, 1F3C1064h, 1F3D1063h, 1F3F1062h
		dd	1F401062h, 1F421061h, 1F441060h, 1F45105Fh
		dd	1F47105Eh, 1F48105Dh, 1F4A105Dh, 1F4C105Ch
		dd	1F4D105Bh, 1F4F105Ah, 1F501059h, 1F521059h
		dd	1F541058h, 1F551057h, 1F571056h, 1F581055h
		dd	1F5A1055h, 1F5C1054h, 1F5D1053h, 1F5F1052h
		dd	1F601051h, 1F621050h, 1F641050h, 1F65104Fh
		dd	1F67104Eh, 1F68104Dh, 1F6A104Ch, 1F6C104Ch
		dd	1F6D104Bh, 1F6F104Ah, 1F701049h, 1F721048h
		dd	1F741048h, 1F751047h, 1F771046h, 1F781045h
		dd	1F7A1044h, 1F7C1043h, 1F7D1043h, 1F7F1042h
		dd	1F801041h, 1F821040h, 1F84103Fh, 1F85103Fh
		dd	1F87103Eh, 1F88103Dh, 1F8A103Ch, 1F8B103Bh
		dd	1F8D103Bh, 1F8F103Ah, 1F901039h, 1F921038h
		dd	1F931037h, 1F951036h, 1F971036h, 1F981035h
		dd	1F9A1034h, 1F9B1033h, 1F9D1032h, 1F9F1032h
		dd	1FA01031h, 1FA21030h, 1FA3102Fh, 1FA5102Eh
		dd	1FA7102Eh, 1FA8102Dh, 1FAA102Ch, 1FAB102Bh
		dd	1FAD102Ah, 1FAF1029h, 1FB01029h, 1FB21028h
		dd	1FB31027h, 1FB51026h, 1FB71025h, 1FB81025h
		dd	1FBA1024h, 1FBB1023h, 1FBD1022h, 1FBF1021h
		dd	1FC01021h, 1FC21020h, 1FC3101Fh, 1FC5101Eh
		dd	1FC7101Dh, 1FC8101Ch, 1FCA101Ch, 1FCB101Bh
		dd	1FCD101Ah, 1FCF1019h, 1FD01018h, 1FD21018h
		dd	1FD31017h, 1FD51016h, 1FD71015h, 1FD81014h
		dd	1FDA1014h, 1FDB1013h, 1FDD1012h, 1FDE1011h
		dd	1FE01010h, 1FE2100Fh, 1FE3100Fh, 1FE5100Eh
		dd	1FE6100Dh, 1FE8100Ch, 1FEA100Bh, 1FEB100Bh
		dd	1FED100Ah, 1FEE1009h, 1FF01008h, 1FF21007h
		dd	1FF31007h, 1FF51006h, 1FF61005h, 1FF81004h
		dd	1FFA1003h, 1FFB1002h, 1FFD1002h, 1FFE1001h
		dd	20001000h, 20020FFFh, 20030FFEh, 20050FFEh
		dd	20060FFDh, 20080FFCh, 200A0FFBh, 200B0FFAh
		dd	200D0FF9h, 200E0FF9h, 20100FF8h, 20120FF7h
		dd	20130FF6h, 20150FF5h, 20160FF5h, 20180FF4h
		dd	201A0FF3h, 201B0FF2h, 201D0FF1h, 201E0FF1h
		dd	20200FF0h, 20220FEFh, 20230FEEh, 20250FEDh
		dd	20260FECh, 20280FECh, 20290FEBh, 202B0FEAh
		dd	202D0FE9h, 202E0FE8h, 20300FE8h, 20310FE7h
		dd	20330FE6h, 20350FE5h, 20360FE4h, 20380FE4h
		dd	20390FE3h, 203B0FE2h, 203D0FE1h, 203E0FE0h
		dd	20400FDFh, 20410FDFh, 20430FDEh, 20450FDDh
		dd	20460FDCh, 20480FDBh, 20490FDBh, 204B0FDAh
		dd	204D0FD9h, 204E0FD8h, 20500FD7h, 20510FD7h
		dd	20530FD6h, 20550FD5h, 20560FD4h, 20580FD3h
		dd	20590FD2h, 205B0FD2h, 205D0FD1h, 205E0FD0h
		dd	20600FCFh, 20610FCEh, 20630FCEh, 20650FCDh
		dd	20660FCCh, 20680FCBh, 20690FCAh, 206B0FCAh
		dd	206D0FC9h, 206E0FC8h, 20700FC7h, 20710FC6h
		dd	20730FC5h, 20750FC5h, 20760FC4h, 20780FC3h
		dd	20790FC2h, 207B0FC1h, 207C0FC1h, 207E0FC0h
		dd	20800FBFh, 20810FBEh, 20830FBDh, 20840FBDh
		dd	20860FBCh, 20880FBBh, 20890FBAh, 208B0FB9h
		dd	208C0FB8h, 208E0FB8h, 20900FB7h, 20910FB6h
		dd	20930FB5h, 20940FB4h, 20960FB4h, 20980FB3h
		dd	20990FB2h, 209B0FB1h, 209C0FB0h, 209E0FB0h
		dd	20A00FAFh, 20A10FAEh, 20A30FADh, 20A40FACh
		dd	20A60FABh, 20A80FABh, 20A90FAAh, 20AB0FA9h
		dd	20AC0FA8h, 20AE0FA7h, 20B00FA7h, 20B10FA6h
		dd	20B30FA5h, 20B40FA4h, 20B60FA3h, 20B80FA3h
		dd	20B90FA2h, 20BB0FA1h, 20BC0FA0h, 20BE0F9Fh
		dd	20C00F9Eh, 20C10F9Eh, 20C30F9Dh, 20C40F9Ch
		dd	20C60F9Bh, 20C70F9Ah, 20C90F9Ah, 20CB0F99h

_YUV_clip_table	db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 1, 2, 3
		db	 4, 5, 6, 7
		db	 8, 9, 10, 11
		db	 12, 13, 14, 15
		db	 16, 17, 18, 19
		db	 20, 21, 22, 23
		db	 24, 25, 26, 27
		db	 28, 29, 30, 31
		db	 32, 33, 34, 35
		db	 36, 37, 38, 39
		db	 40, 41, 42, 43
		db	 44, 45, 46, 47
		db	 48, 49, 50, 51
		db	 52, 53, 54, 55
		db	 56, 57, 58, 59
		db	 60, 61, 62, 63
		db	 64, 65, 66, 67
		db	 68, 69, 70, 71
		db	 72, 73, 74, 75
		db	 76, 77, 78, 79
		db	 80, 81, 82, 83
		db	 84, 85, 86, 87
		db	 88, 89, 90, 91
		db	 92, 93, 94, 95
		db	 96, 97, 98, 99
		db	 100, 101, 102, 103
		db	 104, 105, 106, 107
		db	 108, 109, 110, 111
		db	 112, 113, 114, 115
		db	 116, 117, 118, 119
		db	 120, 121, 122, 123
		db	 124, 125, 126, 127
		db	 128, 129, 130, 131
		db	 132, 133, 134, 135
		db	 136, 137, 138, 139
		db	 140, 141, 142, 143
		db	 144, 145, 146, 147
		db	 148, 149, 150, 151
		db	 152, 153, 154, 155
		db	 156, 157, 158, 159
		db	 160, 161, 162, 163
		db	 164, 165, 166, 167
		db	 168, 169, 170, 171
		db	 172, 173, 174, 175
		db	 176, 177, 178, 179
		db	 180, 181, 182, 183
		db	 184, 185, 186, 187
		db	 188, 189, 190, 191
		db	 192, 193, 194, 195
		db	 196, 197, 198, 199
		db	 200, 201, 202, 203
		db	 204, 205, 206, 207
		db	 208, 209, 210, 211
		db	 212, 213, 214, 215
		db	 216, 217, 218, 219
		db	 220, 221, 222, 223
		db	 224, 225, 226, 227
		db	 228, 229, 230, 231
		db	 232, 233, 234, 235
		db	 236, 237, 238, 239
		db	 240, 241, 242, 243
		db	 244, 245, 246, 247
		db	 248, 249, 250, 251
		db	 252, 253, 254, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255
		db	 255, 255, 255, 255


_YUV_clip_table16	db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 0, 0, 0, 0
		db	 1, 1, 1, 1
		db	 1, 1, 1, 1
		db	 2, 2, 2, 2
		db	 2, 2, 2, 2
		db	 3, 3, 3, 3
		db	 3, 3, 3, 3
		db	 4, 4, 4, 4
		db	 4, 4, 4, 4
		db	 5, 5, 5, 5
		db	 5, 5, 5, 5
		db	 6, 6, 6, 6
		db	 6, 6, 6, 6
		db	 7, 7, 7, 7
		db	 7, 7, 7, 7
		db	 8, 8, 8, 8
		db	 8, 8, 8, 8
		db	 9, 9, 9, 9
		db	 9, 9, 9, 9
		db	 10, 10, 10, 10
		db	 10, 10, 10, 10
		db	 11, 11, 11, 11
		db	 11, 11, 11, 11
		db	 12, 12, 12, 12
		db	 12, 12, 12, 12
		db	 13, 13, 13, 13
		db	 13, 13, 13, 13
		db	 14, 14, 14, 14
		db	 14, 14, 14, 14
		db	 15, 15, 15, 15
		db	 15, 15, 15, 15
		db	 16, 16, 16, 16
		db	 16, 16, 16, 16
		db	 17, 17, 17, 17
		db	 17, 17, 17, 17
		db	 18, 18, 18, 18
		db	 18, 18, 18, 18
		db	 19, 19, 19, 19
		db	 19, 19, 19, 19
		db	 20, 20, 20, 20
		db	 20, 20, 20, 20
		db	 21, 21, 21, 21
		db	 21, 21, 21, 21
		db	 22, 22, 22, 22
		db	 22, 22, 22, 22
		db	 23, 23, 23, 23
		db	 23, 23, 23, 23
		db	 24, 24, 24, 24
		db	 24, 24, 24, 24
		db	 25, 25, 25, 25
		db	 25, 25, 25, 25
		db	 26, 26, 26, 26
		db	 26, 26, 26, 26
		db	 27, 27, 27, 27
		db	 27, 27, 27, 27
		db	 28, 28, 28, 28
		db	 28, 28, 28, 28
		db	 29, 29, 29, 29
		db	 29, 29, 29, 29
		db	 30, 30, 30, 30
		db	 30, 30, 30, 30
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31
		db	 31, 31, 31, 31


	end

