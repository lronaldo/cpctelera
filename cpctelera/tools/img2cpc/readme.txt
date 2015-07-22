Img2CPC
(C) CEZ Team 2007.

If you liked this program, drop an email at: augusto.ruiz@gmail.com
-------------------------------------------------------------------

Usage:

Img2CPC [/w=width] [/h=height] [/g] [/bn=tileBaseName] [/m=mode] [/t] [/f] [/o=output] [/c=80] [/cbb] files

	w	Tile Width. If not specified, it will process the
		whole image width.
	h	Tile Height. If not specified, it will process the
		whole image height.
	g	If specified, it will generate one bmp file per tile.
	bn	Base name to be used for asm labels.
	m	Pixel mode. Valid values are 0,1,2.
		If not specified defaults to mode 0.
	t	Transparency. Creates masks for transparent pixels.
	i	Interlace masks. Creates masks for transparent pixels, mixing them with sprite data.
	f	Flipped values LUT. Creates a look-up table with
		flipped bytes for the current mode.
	o	Output file name. Default value is gfx.h
	c	Create compiled sprites. Screen width in bytes can (should) be specified.
		(Default width is 80). File name will be compiled.[outputFileName]
	cbb	Create backcuffer compiled sprites. File name will be bb.compiled.[outputFileName]
	files	List of files to process. Wildcards can be used (*.png)
ENJOY!
