# img2cpc
Tool to convert images in a lot of formats to data that can be used in Amstrad CPC software

img2cpc - (c) 2007-2015 Retroworks.

## Usage
`img2cpc [OPTIONS] fileNames`

## Options
`--help`
Help. Show usage.

`-bn, --baseName ARG` 
Tile base name. If not set, the filename will be used. The base name will be prepended to the filename, unless the `absoluteBaseName` switch is specified.

`-abn, --absoluteBaseName` 
Absolute tile base name. This switch should be used in conjuction with the `baseName` switch. If specified, the base name specified will be the identifier to use when generating data, and the file name will be ignored.

`-f`
Create a flipped values look-up table for the current palette and mode.

`-fwp, --firmwarePalette ARG1[,ARGn]`
Palette specified in firmware values.

`-g, --generatePNG`
Generates PNG images, one per tile and one per mask (if transparency is set).

`-h, --tileHeight ARG`
Tile height. If not set, the image height is used.

`-hwp, --hardwarePalette ARG1[,ARGn]`
Palette specified in hardware values.

`-im, --interlacedMasks`
Interlaced masks. Mask values will be interlaced with pixel values.

`-m, --mode ARG`
Specifies the CPC Mode to generate data for. Valid values are 0 (default), 1 or 2.

`-map, --tilemap`
Generate tile map

`-nt, --noTileset`
Don't create tileset. Use this if you are creating sprites and do not need a table with all the tile pointers.

`-o, --outputFileName ARG`
Output file name. Default value is gfx. img2cpc will append the proper extension based on format.

`-of, --outputFormat ARG`
Output format. If not set, data will be generated in assembly format.

`--oneFile`
Generate one output file per processed file. Files will be named using the base name (if specified) and the source file name.

`-opfw`
Output palette (firmware values)

`-ophw`
Output palette (hardware values)

`-os, --outputSize`
Output tile size as constants. `#define`s will be created in C, `EQU`s in assembler.

`-p, --palette ARG`
Specifies input palette file.

`-rgbp, --rgbPalette ARG1[,ARGn]`
Palette specified in RGB values. RGB values must be specified as `0xRRGGBB`, where `RR`, `GG` and `BB` are hexadecimal, or as an int value.
                                      
Examples: `0x1122FF`, `255`

`-s, --scanlineOrder ARG1[,ARGn]`
Scanline order. Default value is `0,1,2,3,4,5,6,7`

`-t, --transparentColor ARG`
Specifies transparent color (as index in palette).

`-w, --tileWidth ARG`
Tile width. If not set, the image width is used.

`-z, --zigzag`
Zigzag. Generate data in zigzag order.

## Examples

`img2cpc -w 8 -h 8 --outputFormat asm -bn tile -m 0 tiles.png`

Use palette in `src/config_rgb.json`, output C format, images in mode 0, output file name `gfx.c`, create PNGs, base name: tile, tile size 32x32.
`img2cpc -p src/config_rgb.json -of c -m 0 -o gfx.c -g -bn tile -h 32 -w 32 bee.png`

Use palette in `src/config_rgb.json`, transparent color is 0, output C format, images in mode 0, output file name `gfx.c`, create PNGs, base name: tile, tile size 32x32.
`img2cpc -p src/config_rgb.json -t 0 -of c -m 0 -o gfx.c -g -bn tile -h 32 -w 32 bee.png`

Use palette in `src/config_rgb.json`, transparent color is 12, output ASM format, images in mode 0, output file name `gfx.s`, one tile in image.
`img2cpc -p src/config_rgb.json -t 12 -of asm -m 0 -o gfx.s RYU_STAND_0.png`

Use palette in `src/config_rgb.json`, transparent color is 12, output ASM format, images in mode 0, output file name `gfx.s`, one tile in image, data in zigzag.
`img2cpc -p src/config_rgb.json -t 12 -of asm -m 0 -o gfx.s -z RYU_STAND_0.png`

Use palette in `src/config_rgb.json`, transparent color is 12, output ASM format, images in mode 0, output file name `gfx.s`, one tile in image, data in zigzag, scanline order `0, 1, 2, 3, 7, 6, 5, 4`.
`img2cpc -p src/config_rgb.json -t 12 -of asm -m 0 -o gfx.s -z -s 0,1,2,3,7,6,5,4 RYU_STAND_0.png`

If you liked this program, drop an email at: `augusto.ruiz@gmail.com`
