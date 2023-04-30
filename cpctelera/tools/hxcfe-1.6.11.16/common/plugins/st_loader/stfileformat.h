/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
typedef struct stfileformats_t_
{
	unsigned int filesize;
	int numberoftrack;
	int sectorpertrack;
	int numberofside;
	int gap3len;
	int interleave;
}stfileformats_t;

static stfileformats_t stfileformats[]=
{
	{354816,77,9,1,84,1},
	{359424,78,9,1,84,1},
	{364032,79,9,1,84,1},
	{368640,80,9,1,84,1},
	{373248,81,9,1,84,1},
	{377856,82,9,1,84,1},
	{382464,83,9,1,84,1},
	{387072,84,9,1,84,1},
	{391680,85,9,1,84,1},
	{394240,77,10,1,30,1},
	{399360,78,10,1,30,1},
	{404480,79,10,1,30,1},
	{409600,80,10,1,30,1},
	{414720,81,10,1,30,1},
	{419840,82,10,1,30,1},
	{424960,83,10,1,30,1},
	{430080,84,10,1,30,1},
	{435200,85,10,1,30,1},
	{433664,77,11,1,3,2},
	{439296,78,11,1,3,2},
	{444928,79,11,1,3,2},
	{450560,80,11,1,3,2},
	{456192,81,11,1,3,2},
	{461824,82,11,1,3,2},
	{467456,83,11,1,3,2},
	{473088,84,11,1,3,2},
	{478720,85,11,1,3,2},
	{655360,80,8,2,84,1},
	{663552,81,8,2,84,1},
	{709632,77,9,2,84,1},
	{718848,78,9,2,84,1},
	{728064,79,9,2,84,1},
	{737280,80,9,2,84,1},
	{746496,81,9,2,84,1},
	{755712,82,9,2,84,1},
	{764928,83,9,2,84,1},
	{774144,84,9,2,84,1},
	{783360,85,9,2,84,1},
	
	{788480,77,10,2,30,1},
	{798720,78,10,2,30,1},
	{808960,79,10,2,30,1},
	{819200,80,10,2,30,1},
	{829440,81,10,2,30,1},
	{839680,82,10,2,30,1},
	{839680,82,10,2,30,1},
	{844800,82,10,2,30,1},
	{849920,83,10,2,30,1},
	{855040,83,10,2,30,1},
	{860160,84,10,2,30,1},
	{870400,85,10,2,30,1},
	
	{867328,77,11,2,3,2},
	{878592,78,11,2,3,2},
	{889856,79,11,2,3,2},
	{901120,80,11,2,3,2},
	{912384,81,11,2,3,2},
	{923648,82,11,2,3,2},
	{934912,83,11,2,3,2},
	{946176,84,11,2,3,2},
	{957440,85,11,2,3,2},
	
	{1419264,77,18,2,84,1},
	{1437696,78,18,2,84,1},
	{1456128,79,18,2,84,1},
	{1474560,80,18,2,84,1},
	{1492992,81,18,2,84,1},
	{1511424,82,18,2,84,1},
	{1529856,83,18,2,84,1},
	{1548288,84,18,2,84,1},
	{1566720,85,18,2,84,1},
	
	{1498112,77,19,2,70,1},
	{1517568,78,19,2,70,1},
	{1537024,79,19,2,70,1},
	{1556480,80,19,2,70,1},
	{1575936,81,19,2,70,1},
	{1595392,82,19,2,70,1},
	{1614848,83,19,2,70,1},
	{1634304,84,19,2,70,1},
	{1653760,85,19,2,70,1},

	{1576960,77,20,2,40,1},
	{1597440,78,20,2,40,1},
	{1617920,79,20,2,40,1},
	{1638400,80,20,2,40,1},
	{1658880,81,20,2,40,1},
	{1679360,82,20,2,40,1},
	{1699840,83,20,2,40,1},
	{1740800,85,20,2,40,1},

	{1655808,77,21,2,18,1},
	{1677312,78,21,2,18,1},
	{1698816,79,21,2,18,1},
	{1720320,80,21,2,18,1},
	{1741824,81,21,2,18,1},
	{1763328,82,21,2,18,1},
	{1784832,83,21,2,18,1},
	{1806336,84,21,2,18,1},
	{1827840,85,21,2,18,1},


	{2949120,80,36,2,84,1},
	{0,0,0,0,0,0}
};
