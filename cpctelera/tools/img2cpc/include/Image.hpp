#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cstdint>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <vector>

//
// Self-contained image loading for img2cpc.
// Supported input formats: PNG, GIF, BMP, PCX and TGA.
// Images are always stored as 8-bit RGBA, top-down, row-major.
//

struct RGBA {
	std::uint8_t r = 0, g = 0, b = 0, a = 0;

	friend constexpr bool operator==(const RGBA&, const RGBA&) = default;
};

class ImageError : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

class Image {
public:
	// Largest accepted width or height. Protects against corrupt headers.
	static constexpr std::uint32_t MaxDimension = 16384;

	Image() = default;
	Image(std::uint32_t width, std::uint32_t height);

	// Loads an image file. Format is detected by signature, then by extension.
	// Throws ImageError on failure.
	static Image Load(const std::filesystem::path& fileName);

	// Decodes an image held in memory. Throws ImageError on failure.
	static Image Decode(std::span<const std::uint8_t> data, const std::filesystem::path& nameHint = {});

	// Saves the image as an 8-bit RGB PNG (alpha is dropped). Throws ImageError on failure.
	void SavePNG(const std::filesystem::path& fileName) const;

	// Returns a copy of the rectangle (x, y, width, height). The rectangle must be inside the image.
	Image Crop(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) const;

	std::uint32_t Width()  const noexcept { return _width; }
	std::uint32_t Height() const noexcept { return _height; }
	bool Contains(std::int64_t x, std::int64_t y) const noexcept {
		return x >= 0 && y >= 0 && x < _width && y < _height;
	}

	RGBA&       At(std::uint32_t x, std::uint32_t y)       noexcept { return _pixels[std::size_t(y) * _width + x]; }
	const RGBA& At(std::uint32_t x, std::uint32_t y) const noexcept { return _pixels[std::size_t(y) * _width + x]; }

	std::span<RGBA>       Pixels()       noexcept { return _pixels; }
	std::span<const RGBA> Pixels() const noexcept { return _pixels; }

private:
	std::uint32_t _width  = 0;
	std::uint32_t _height = 0;
	std::vector<RGBA> _pixels;
};

#endif
