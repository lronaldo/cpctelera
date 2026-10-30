#include "ImageFormats.hpp"

#include <array>
#include <string>

//
// PCX decoder (ZSoft). Supports RLE and raw data and these layouts:
//   - 1, 2, 4 or 8 bits per pixel, 1 plane (packed palette indices).
//   - 1 bit per pixel, 2 to 4 planes (EGA style planar palette indices).
//   - 8 bits per pixel, 3 planes (RGB) or 4 planes (RGBA).
// Palettes: black and white for 1 bit, header palette up to 16 colours,
// trailing 256-colour palette for 8 bits (grey ramp if it is missing).
//
Image ImageFormats::DecodePCX(std::span<const std::uint8_t> data) {
	ByteReader in(data, 2);
	const std::uint8_t encoding = in.U8();
	const std::uint8_t bitsPerPixel = in.U8();
	const std::uint16_t xMin = in.U16();
	const std::uint16_t yMin = in.U16();
	const std::uint16_t xMax = in.U16();
	const std::uint16_t yMax = in.U16();
	in.Skip(4);                                     // Resolution
	const auto headerPalette = in.Bytes(48);
	in.Skip(1);                                     // Reserved
	const std::uint8_t planes = in.U8();
	const std::uint16_t bytesPerLine = in.U16();

	const std::int64_t width = std::int64_t(xMax) - xMin + 1;
	const std::int64_t height = std::int64_t(yMax) - yMin + 1;
	CheckDimensions(width, height, "PCX");
	const auto w = std::uint32_t(width);
	const auto h = std::uint32_t(height);

	const bool trueColor = (bitsPerPixel == 8 && (planes == 3 || planes == 4));
	const bool indexed = (planes == 1 && (bitsPerPixel == 1 || bitsPerPixel == 2 || bitsPerPixel == 4 || bitsPerPixel == 8))
	                  || (bitsPerPixel == 1 && planes >= 2 && planes <= 4);
	if (!trueColor && !indexed) {
		throw ImageError("PCX: unsupported format (" + std::to_string(bitsPerPixel) + " bits, "
		                 + std::to_string(planes) + " planes)");
	}
	if (std::uint64_t(bytesPerLine) * 8 < std::uint64_t(w) * bitsPerPixel) {
		throw ImageError("PCX: invalid bytes per line");
	}

	// Decode all scan lines. Runs may cross line boundaries; missing data stays 0.
	const std::size_t lineLength = std::size_t(bytesPerLine) * planes;
	std::vector<std::uint8_t> raster(lineLength * h, 0);
	in.Seek(128);
	for (std::size_t pos = 0; pos < raster.size() && in.Remaining() > 0; ) {
		std::uint8_t value = in.U8();
		std::size_t count = 1;
		if (encoding == 1 && (value & 0xC0) == 0xC0) {
			count = value & 0x3F;
			if (in.Remaining() == 0) { break; }
			value = in.U8();
		}
		for (; count > 0 && pos < raster.size(); --count) {
			raster[pos++] = value;
		}
	}

	Image image(w, h);

	if (trueColor) {
		for (std::uint32_t y = 0; y < h; ++y) {
			const std::uint8_t* line = &raster[y * lineLength];
			for (std::uint32_t x = 0; x < w; ++x) {
				image.At(x, y) = { line[x], line[x + bytesPerLine], line[x + 2 * bytesPerLine],
				                   planes == 4 ? line[x + 3 * bytesPerLine] : std::uint8_t(0xFF) };
			}
		}
		return image;
	}

	const int bitCount = bitsPerPixel * planes;
	std::array<RGBA, 256> palette{};
	if (bitCount == 1) {
		palette[0] = { 0, 0, 0, 0xFF };
		palette[1] = { 0xFF, 0xFF, 0xFF, 0xFF };
	} else if (bitCount <= 4) {
		for (int i = 0; i < 16; ++i) {
			palette[i] = { headerPalette[i * 3], headerPalette[i * 3 + 1], headerPalette[i * 3 + 2], 0xFF };
		}
	} else if (data.size() >= 128 + 769 && data[data.size() - 769] == 0x0C) {
		const auto vga = data.last(768);
		for (int i = 0; i < 256; ++i) {
			palette[i] = { vga[i * 3], vga[i * 3 + 1], vga[i * 3 + 2], 0xFF };
		}
	} else {
		for (int i = 0; i < 256; ++i) {
			palette[i] = { std::uint8_t(i), std::uint8_t(i), std::uint8_t(i), 0xFF };
		}
	}

	const std::uint8_t pixelMask = std::uint8_t((1 << bitsPerPixel) - 1);
	for (std::uint32_t y = 0; y < h; ++y) {
		const std::uint8_t* line = &raster[y * lineLength];
		for (std::uint32_t x = 0; x < w; ++x) {
			const std::size_t bit = std::size_t(x) * bitsPerPixel;
			const int shift = 8 - bitsPerPixel - int(bit % 8);
			unsigned index = 0;
			for (unsigned plane = 0; plane < planes; ++plane) {
				const unsigned bits = (line[plane * bytesPerLine + bit / 8] >> shift) & pixelMask;
				index |= bits << (plane * bitsPerPixel);
			}
			image.At(x, y) = palette[index];
		}
	}
	return image;
}
