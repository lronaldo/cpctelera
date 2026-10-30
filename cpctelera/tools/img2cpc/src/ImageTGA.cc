#include "ImageFormats.hpp"

#include <string>

//
// TGA (Truevision Targa) decoder.
// Image types: colour-mapped (1, 9), true-colour (2, 10) and greyscale (3, 11), raw or RLE.
// Pixel depths: 8, 15, 16, 24 and 32 bits. Any origin (bottom/top, left/right).
//
namespace {

// Converts a little-endian pixel value of the given depth (15, 16, 24, 32) to RGBA.
RGBA TrueColor(std::uint32_t value, int depth, bool useAlphaBit) {
	switch (depth) {
		case 15:
		case 16: {
			auto expand = [](std::uint32_t v) { return std::uint8_t(v * 0xFF / 0x1F); };
			const std::uint8_t alpha = (useAlphaBit && !(value & 0x8000)) ? 0 : 0xFF;
			return { expand((value >> 10) & 0x1F), expand((value >> 5) & 0x1F), expand(value & 0x1F), alpha };
		}
		case 24:
			return { std::uint8_t(value >> 16), std::uint8_t(value >> 8), std::uint8_t(value), 0xFF };
		default:
			return { std::uint8_t(value >> 16), std::uint8_t(value >> 8), std::uint8_t(value), std::uint8_t(value >> 24) };
	}
}

std::uint32_t ReadValue(ImageFormats::ByteReader& in, int bytes) {
	std::uint32_t value = 0;
	for (int i = 0; i < bytes; ++i) {
		value |= std::uint32_t(in.U8()) << (8 * i);
	}
	return value;
}

}

Image ImageFormats::DecodeTGA(std::span<const std::uint8_t> data) {
	ByteReader in(data);
	const std::uint8_t idLength = in.U8();
	const std::uint8_t colorMapType = in.U8();
	const std::uint8_t imageType = in.U8();
	const std::uint16_t colorMapFirst = in.U16();
	const std::uint16_t colorMapLength = in.U16();
	const std::uint8_t colorMapDepth = in.U8();
	in.Skip(4);                                         // X and Y origin
	const std::uint16_t width = in.U16();
	const std::uint16_t height = in.U16();
	const std::uint8_t depth = in.U8();
	const std::uint8_t descriptor = in.U8();
	in.Skip(idLength);

	const bool rle = imageType >= 9;
	const int baseType = rle ? imageType - 8 : imageType;
	if (baseType < 1 || baseType > 3 || colorMapType > 1) {
		throw ImageError("TGA: unsupported image type " + std::to_string(imageType));
	}
	CheckDimensions(width, height, "TGA");

	const bool validDepth = (baseType == 1 && (depth == 8 || depth == 16))
	                     || (baseType == 2 && (depth == 15 || depth == 16 || depth == 24 || depth == 32))
	                     || (baseType == 3 && (depth == 8 || depth == 16));
	if (!validDepth) {
		throw ImageError("TGA: unsupported pixel depth " + std::to_string(depth));
	}
	const int alphaBits = descriptor & 0x0F;

	std::vector<RGBA> palette;
	if (colorMapType == 1) {
		if (colorMapDepth != 15 && colorMapDepth != 16 && colorMapDepth != 24 && colorMapDepth != 32) {
			throw ImageError("TGA: unsupported colour map depth " + std::to_string(colorMapDepth));
		}
		palette.resize(std::size_t(colorMapFirst) + colorMapLength, RGBA{ 0, 0, 0, 0xFF });
		for (std::size_t i = 0; i < colorMapLength; ++i) {
			palette[colorMapFirst + i] = TrueColor(ReadValue(in, (colorMapDepth + 7) / 8), colorMapDepth, alphaBits > 0);
		}
	}
	if (baseType == 1 && palette.empty()) {
		throw ImageError("TGA: colour-mapped image without colour map");
	}

	const int bytesPerPixel = (depth + 7) / 8;
	auto toRGBA = [&](std::uint32_t value) -> RGBA {
		switch (baseType) {
			case 1:
				return value < palette.size() ? palette[value] : RGBA{ 0, 0, 0, 0xFF };
			case 3: {
				const auto grey = std::uint8_t(value);
				return { grey, grey, grey, depth == 16 ? std::uint8_t(value >> 8) : std::uint8_t(0xFF) };
			}
			default:
				return TrueColor(value, depth, alphaBits > 0);
		}
	};

	// Pixels are stored left to right, bottom to top unless the descriptor says otherwise.
	const bool rightToLeft = descriptor & 0x10;
	const bool topToBottom = descriptor & 0x20;
	Image image(width, height);
	const std::size_t total = std::size_t(width) * height;
	auto store = [&](std::size_t n, RGBA color) {
		const auto x = std::uint32_t(n % width);
		const auto y = std::uint32_t(n / width);
		image.At(rightToLeft ? width - 1 - x : x, topToBottom ? y : height - 1 - y) = color;
	};

	for (std::size_t n = 0; n < total; ) {
		if (!rle) {
			store(n++, toRGBA(ReadValue(in, bytesPerPixel)));
			continue;
		}
		const std::uint8_t packet = in.U8();
		const std::size_t count = (packet & 0x7F) + 1u;
		if (packet & 0x80) {
			const RGBA color = toRGBA(ReadValue(in, bytesPerPixel));
			for (std::size_t i = 0; i < count && n < total; ++i) {
				store(n++, color);
			}
		} else {
			for (std::size_t i = 0; i < count && n < total; ++i) {
				store(n++, toRGBA(ReadValue(in, bytesPerPixel)));
			}
		}
	}
	return image;
}
