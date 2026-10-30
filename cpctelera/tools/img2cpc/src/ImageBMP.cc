#include "ImageFormats.hpp"

#include <algorithm>
#include <bit>
#include <cstdlib>

//
// BMP decoder.
// Headers: OS/2 1.x (12 bytes), OS/2 2.x (16-64 bytes), Windows v1 to v5 (40-124 bytes).
// Pixel formats: 1, 4, 8, 16, 24 and 32 bits per pixel; RLE4, RLE8 and bit fields.
//
namespace {

enum Compression : std::uint32_t {
	BI_RGB = 0, BI_RLE8 = 1, BI_RLE4 = 2, BI_BITFIELDS = 3, BI_ALPHABITFIELDS = 6
};

// Extracts one colour channel described by a bit mask and scales it to 8 bits.
struct Channel {
	std::uint32_t mask = 0;
	int shift = 0;
	std::uint64_t maxValue = 0;

	explicit Channel(std::uint32_t m = 0) : mask(m) {
		if (mask != 0) {
			shift = std::countr_zero(mask);
			maxValue = (std::uint64_t(1) << std::popcount(mask >> shift)) - 1;
		}
	}
	std::uint8_t Extract(std::uint32_t pixel, std::uint8_t ifMissing = 0) const {
		if (mask == 0) {
			return ifMissing;
		}
		const std::uint64_t value = (pixel & mask) >> shift;
		return std::uint8_t(value * 255 / maxValue);
	}
};

// Decodes RLE4 / RLE8 data into palette indices, bottom-up. Undefined pixels stay at index 0.
std::vector<std::uint8_t> DecodeRLE(ImageFormats::ByteReader& in, std::uint32_t width, std::uint32_t height, bool rle4) {
	std::vector<std::uint8_t> indices(std::size_t(width) * height, 0);
	std::uint32_t x = 0, y = 0;
	auto put = [&](std::uint8_t index) {
		if (x < width && y < height) {
			indices[std::size_t(y) * width + x] = index;
		}
		++x;
	};

	while (in.Remaining() >= 2 && y < height) {
		const std::uint8_t count = in.U8();
		const std::uint8_t value = in.U8();
		if (count > 0) {                                     // Encoded run
			for (int i = 0; i < count; ++i) {
				put(rle4 ? std::uint8_t((i & 1) ? (value & 0x0F) : (value >> 4)) : value);
			}
		} else if (value == 0) {                             // End of line
			x = 0;
			++y;
		} else if (value == 1) {                             // End of bitmap
			break;
		} else if (value == 2) {                             // Delta
			x += in.U8();
			y += in.U8();
		} else {                                             // Absolute run
			const std::size_t bytes = rle4 ? (value + 1u) / 2 : value;
			const auto run = in.Bytes(bytes);
			for (int i = 0; i < value; ++i) {
				put(rle4 ? std::uint8_t((i & 1) ? (run[i / 2] & 0x0F) : (run[i / 2] >> 4)) : run[i]);
			}
			if (bytes & 1) {
				in.Skip(std::min<std::size_t>(1, in.Remaining()));   // Runs are word aligned
			}
		}
	}
	return indices;
}

}

Image ImageFormats::DecodeBMP(std::span<const std::uint8_t> data) {
	ByteReader in(data, 10);
	const std::uint32_t pixelOffset = in.U32();
	const std::uint32_t headerSize = in.U32();

	std::int64_t width = 0, height = 0;
	std::uint16_t bitCount = 0;
	std::uint32_t compression = BI_RGB;
	std::uint32_t colorsUsed = 0;
	std::uint32_t masks[4] = { 0, 0, 0, 0 };              // Red, green, blue, alpha
	bool coreHeader = false;

	if (headerSize == 12) {                                // OS/2 1.x
		coreHeader = true;
		width = in.U16();
		height = in.U16();
		in.Skip(2);
		bitCount = in.U16();
	} else if (headerSize >= 16 && headerSize <= 124) {
		width = in.I32();
		height = in.I32();
		in.Skip(2);
		bitCount = in.U16();
		if (headerSize >= 20) { compression = in.U32(); }
		if (headerSize >= 36) { in.Skip(12); colorsUsed = in.U32(); }
		// OS/2 2.x uses the same compression numbers for different (unsupported) methods.
		if (headerSize == 64 && compression >= BI_BITFIELDS) {
			throw ImageError("BMP: unsupported OS/2 compression");
		}
		if (headerSize >= 52) {
			in.Seek(14 + 40);
			for (int i = 0; i < (headerSize >= 56 ? 4 : 3); ++i) { masks[i] = in.U32(); }
		} else if (compression == BI_BITFIELDS || compression == BI_ALPHABITFIELDS) {
			in.Seek(14 + headerSize);                       // Masks follow a v1 header
			for (int i = 0; i < (compression == BI_ALPHABITFIELDS ? 4 : 3); ++i) { masks[i] = in.U32(); }
		}
	} else {
		throw ImageError("BMP: unsupported header size " + std::to_string(headerSize));
	}

	const bool topDown = height < 0;
	height = std::abs(height);
	CheckDimensions(width, height, "BMP");
	const auto w = std::uint32_t(width);
	const auto h = std::uint32_t(height);

	// Palette: right after the header and the masks.
	std::vector<RGBA> palette;
	if (bitCount <= 8) {
		std::size_t paletteOffset = 14 + headerSize;
		if (!coreHeader && headerSize < 52) {
			paletteOffset += (compression == BI_BITFIELDS) ? 12 : (compression == BI_ALPHABITFIELDS) ? 16 : 0;
		}
		const std::size_t entrySize = coreHeader ? 3 : 4;
		std::size_t entries = (colorsUsed > 0 && colorsUsed <= 256) ? colorsUsed : (std::size_t(1) << bitCount);
		if (pixelOffset > paletteOffset) {
			entries = std::min(entries, (pixelOffset - paletteOffset) / entrySize);
		}
		in.Seek(paletteOffset);
		for (std::size_t i = 0; i < entries; ++i) {
			const auto bgr = in.Bytes(entrySize);
			palette.push_back({ bgr[2], bgr[1], bgr[0], 0xFF });
		}
	}
	palette.resize(256, RGBA{ 0, 0, 0, 0xFF });

	Image image(w, h);
	auto pixel = [&](std::uint32_t x, std::uint32_t row) -> RGBA& {
		return image.At(x, topDown ? row : h - 1 - row);
	};

	if (pixelOffset != 0) {
		in.Seek(pixelOffset);
	}

	if (compression == BI_RLE8 || compression == BI_RLE4) {
		if (bitCount != (compression == BI_RLE8 ? 8 : 4)) {
			throw ImageError("BMP: invalid RLE bit depth");
		}
		const auto indices = DecodeRLE(in, w, h, compression == BI_RLE4);
		for (std::uint32_t row = 0; row < h; ++row) {
			for (std::uint32_t x = 0; x < w; ++x) {
				image.At(x, h - 1 - row) = palette[indices[std::size_t(row) * w + x]];
			}
		}
		return image;
	}
	if (compression != BI_RGB && compression != BI_BITFIELDS && compression != BI_ALPHABITFIELDS) {
		throw ImageError("BMP: unsupported compression " + std::to_string(compression));
	}

	const std::size_t stride = ((std::size_t(w) * bitCount + 31) / 32) * 4;

	switch (bitCount) {
		case 1: case 2: case 4: case 8: {
			const std::uint8_t indexMask = std::uint8_t((1 << bitCount) - 1);
			for (std::uint32_t row = 0; row < h; ++row) {
				const auto line = in.Bytes(stride);
				for (std::uint32_t x = 0; x < w; ++x) {
					const std::size_t bit = std::size_t(x) * bitCount;
					const int shift = 8 - bitCount - int(bit % 8);
					pixel(x, row) = palette[(line[bit / 8] >> shift) & indexMask];
				}
			}
			break;
		}
		case 16: case 32: {
			if (compression == BI_RGB) {
				masks[0] = (bitCount == 16) ? 0x7C00 : 0xFF0000;
				masks[1] = (bitCount == 16) ? 0x03E0 : 0x00FF00;
				masks[2] = (bitCount == 16) ? 0x001F : 0x0000FF;
				masks[3] = 0;
			}
			const Channel red(masks[0]), green(masks[1]), blue(masks[2]), alpha(masks[3]);
			const std::size_t bytesPerPixel = bitCount / 8;
			for (std::uint32_t row = 0; row < h; ++row) {
				const auto line = in.Bytes(stride);
				for (std::uint32_t x = 0; x < w; ++x) {
					std::uint32_t value = 0;
					for (std::size_t i = 0; i < bytesPerPixel; ++i) {
						value |= std::uint32_t(line[x * bytesPerPixel + i]) << (8 * i);
					}
					pixel(x, row) = { red.Extract(value), green.Extract(value), blue.Extract(value), alpha.Extract(value, 0xFF) };
				}
			}
			break;
		}
		case 24: {
			for (std::uint32_t row = 0; row < h; ++row) {
				const auto line = in.Bytes(stride);
				for (std::uint32_t x = 0; x < w; ++x) {
					pixel(x, row) = { line[x * 3 + 2], line[x * 3 + 1], line[x * 3], 0xFF };
				}
			}
			break;
		}
		default:
			throw ImageError("BMP: unsupported bit depth " + std::to_string(bitCount));
	}
	return image;
}
