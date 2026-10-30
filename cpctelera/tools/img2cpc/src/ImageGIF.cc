#include "ImageFormats.hpp"

#include <array>

//
// GIF decoder. Same behaviour as FreeImage's default GIF loading:
//   - Only the first frame is decoded, at its own size (not the logical screen size).
//   - The transparent index gives alpha 0 and keeps the palette RGB.
//   - Without any palette, a grey ramp is used.
//
namespace {

using Palette = std::array<RGBA, 256>;

int ReadPalette(ImageFormats::ByteReader& in, std::uint8_t sizeBits, Palette& palette) {
	const int size = 2 << (sizeBits & 0x07);
	for (int i = 0; i < size; ++i) {
		const auto rgb = in.Bytes(3);
		palette[i] = { rgb[0], rgb[1], rgb[2], 0xFF };
	}
	return size;
}

std::vector<std::uint8_t> ReadSubBlocks(ImageFormats::ByteReader& in) {
	std::vector<std::uint8_t> result;
	while (std::uint8_t size = in.U8()) {
		const auto block = in.Bytes(size);
		result.insert(result.end(), block.begin(), block.end());
	}
	return result;
}

void SkipSubBlocks(ImageFormats::ByteReader& in) {
	while (std::uint8_t size = in.U8()) {
		in.Skip(size);
	}
}

// Decodes a GIF LZW stream into at most maxPixels indices.
// A truncated or corrupt stream ends decoding; missing pixels stay at index 0.
std::vector<std::uint8_t> DecodeLZW(std::span<const std::uint8_t> stream, int minCodeSize, std::size_t maxPixels) {
	if (minCodeSize < 1 || minCodeSize > 11) {
		throw ImageError("GIF: invalid LZW code size");
	}
	constexpr int MaxCodes = 4096;
	const int clearCode = 1 << minCodeSize;
	const int endCode = clearCode + 1;

	std::array<std::uint16_t, MaxCodes> prefix{};
	std::array<std::uint8_t,  MaxCodes> suffix{};
	std::array<std::uint8_t,  MaxCodes> firstChar{};
	std::array<std::uint8_t,  MaxCodes> stack{};
	for (int i = 0; i < clearCode; ++i) {
		suffix[i] = firstChar[i] = std::uint8_t(i);
	}

	std::vector<std::uint8_t> out;
	out.reserve(maxPixels);

	int codeSize = minCodeSize + 1;
	int nextCode = endCode + 1;
	int previous = -1;                 // Previous code, -1 after a clear code
	std::uint32_t bitBuffer = 0;
	int bitCount = 0;
	std::size_t pos = 0;

	while (out.size() < maxPixels) {
		while (bitCount < codeSize && pos < stream.size()) {
			bitBuffer |= std::uint32_t(stream[pos++]) << bitCount;
			bitCount += 8;
		}
		if (bitCount < codeSize) {
			break;
		}
		const int code = int(bitBuffer & ((1u << codeSize) - 1));
		bitBuffer >>= codeSize;
		bitCount -= codeSize;

		if (code == clearCode) {
			codeSize = minCodeSize + 1;
			nextCode = endCode + 1;
			previous = -1;
			continue;
		}
		if (code == endCode) {
			break;
		}
		if (previous < 0) {
			if (code > clearCode) {
				break;
			}
			out.push_back(std::uint8_t(code));
			previous = code;
			continue;
		}
		if (code > nextCode || (code == nextCode && nextCode >= MaxCodes)) {
			break;
		}

		// Unwind the string for this code. Code == nextCode is the KwKwK case.
		std::size_t depth = 0;
		int current = code;
		if (code == nextCode) {
			stack[depth++] = firstChar[previous];
			current = previous;
		}
		while (current > endCode) {
			stack[depth++] = suffix[current];
			current = prefix[current];
		}
		stack[depth++] = std::uint8_t(current);
		while (depth > 0 && out.size() < maxPixels) {
			out.push_back(stack[--depth]);
		}

		if (nextCode < MaxCodes) {
			prefix[nextCode] = std::uint16_t(previous);
			suffix[nextCode] = std::uint8_t(current);
			firstChar[nextCode] = firstChar[previous];
			++nextCode;
			if (nextCode == (1 << codeSize) && codeSize < 12) {
				++codeSize;
			}
		}
		previous = code;
	}
	out.resize(maxPixels, 0);
	return out;
}

}

Image ImageFormats::DecodeGIF(std::span<const std::uint8_t> data) {
	ByteReader in(data, 6);
	in.Skip(4);                     // Logical screen width and height
	const std::uint8_t screenFlags = in.U8();
	in.Skip(2);                     // Background colour and aspect ratio

	Palette globalPalette{};
	int globalSize = 0;
	if (screenFlags & 0x80) {
		globalSize = ReadPalette(in, screenFlags, globalPalette);
	}

	int transparentIndex = -1;
	for (;;) {
		const std::uint8_t blockType = in.U8();
		if (blockType == 0x21) {                                // Extension
			const std::uint8_t label = in.U8();
			if (label == 0xF9) {                                // Graphic control extension
				const auto gce = in.Bytes(in.U8());
				transparentIndex = -1;
				if (gce.size() >= 4 && (gce[0] & 0x01)) {
					transparentIndex = gce[3];
				}
			}
			SkipSubBlocks(in);
		} else if (blockType == 0x2C) {                         // Image descriptor
			break;
		} else {
			throw ImageError("GIF: no image found");
		}
	}

	in.Skip(4);                                                 // Frame left and top
	const std::uint16_t width = in.U16();
	const std::uint16_t height = in.U16();
	const std::uint8_t frameFlags = in.U8();
	CheckDimensions(width, height, "GIF");

	Palette palette{};
	int paletteSize = 0;
	if (frameFlags & 0x80) {
		paletteSize = ReadPalette(in, frameFlags, palette);
	} else if (globalSize > 0) {
		palette = globalPalette;
		paletteSize = globalSize;
	} else {
		for (int i = 0; i < 256; ++i) {
			palette[i] = { std::uint8_t(i), std::uint8_t(i), std::uint8_t(i), 0xFF };
		}
	}
	// FreeImage stored 1, 4 or 8 bits per pixel depending on the palette size,
	// and masked the decoded indices with it.
	const int bitsPerPixel = (paletteSize == 0 || paletteSize > 16) ? 8 : (paletteSize <= 2 ? 1 : 4);
	const std::uint8_t indexMask = std::uint8_t((1 << bitsPerPixel) - 1);

	for (int i = 0; i < 256; ++i) {
		palette[i].a = 0xFF;
	}
	if (transparentIndex >= 0 && transparentIndex <= indexMask) {
		palette[transparentIndex].a = 0;
	}

	const int minCodeSize = in.U8();
	const std::vector<std::uint8_t> stream = ReadSubBlocks(in);
	const std::vector<std::uint8_t> indices = DecodeLZW(stream, minCodeSize, std::size_t(width) * height);

	// Rows come in order, or in 4 interlaced passes.
	std::vector<std::uint32_t> rowOrder;
	rowOrder.reserve(height);
	if (frameFlags & 0x40) {
		constexpr std::array<std::pair<int, int>, 4> passes{{ {0, 8}, {4, 8}, {2, 4}, {1, 2} }};
		for (auto [start, step] : passes) {
			for (std::uint32_t y = std::uint32_t(start); y < height; y += std::uint32_t(step)) {
				rowOrder.push_back(y);
			}
		}
	} else {
		for (std::uint32_t y = 0; y < height; ++y) {
			rowOrder.push_back(y);
		}
	}

	Image image(width, height);
	auto index = indices.begin();
	for (std::uint32_t y : rowOrder) {
		for (std::uint32_t x = 0; x < width; ++x) {
			image.At(x, y) = palette[*index++ & indexMask];
		}
	}
	return image;
}
