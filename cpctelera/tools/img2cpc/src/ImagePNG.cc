#include "ImageFormats.hpp"
#include "lodepng.h"

#include <array>
#include <cmath>
#include <string>

namespace {

// FreeImage asked libpng to correct gamma for a 2.2 screen when a gAMA chunk
// was present. This replicates libpng's 8-bit gamma table, including the
// threshold under which libpng considers the correction not significant.
void ApplyGamma(Image& image, unsigned fileGamma) {
	if (fileGamma == 0) {
		return;
	}
	constexpr double screenGamma = 220000.0;  // 2.2 in libpng fixed point
	const double product = std::floor(double(fileGamma) * screenGamma / 100000.0 + 0.5);
	if (std::fabs(product - 100000.0) <= 5000.0) {
		return;
	}
	const double exponent = std::floor(1e15 / double(fileGamma) / screenGamma + 0.5) * 0.00001;

	std::array<std::uint8_t, 256> table{};
	for (int i = 0; i < 256; ++i) {
		table[i] = (i == 0 || i == 255)
		         ? std::uint8_t(i)
		         : std::uint8_t(std::floor(255.0 * std::pow(i / 255.0, exponent) + 0.5));
	}
	for (RGBA& p : image.Pixels()) {
		p.r = table[p.r];
		p.g = table[p.g];
		p.b = table[p.b];
	}
}

}

Image ImageFormats::DecodePNG(std::span<const std::uint8_t> data) {
	lodepng::State state;
	state.info_raw.colortype = LCT_RGBA;
	state.info_raw.bitdepth = 8;

	std::vector<unsigned char> rgba;
	unsigned width = 0, height = 0;
	if (unsigned error = lodepng::decode(rgba, width, height, state, data.data(), data.size())) {
		throw ImageError(std::string("PNG: ") + lodepng_error_text(error));
	}
	CheckDimensions(width, height, "PNG");

	Image image(width, height);
	auto source = rgba.begin();
	for (RGBA& p : image.Pixels()) {
		p = { source[0], source[1], source[2], source[3] };
		source += 4;
	}
	if (state.info_png.gama_defined) {
		ApplyGamma(image, state.info_png.gama_gamma);
	}
	return image;
}

std::vector<std::uint8_t> ImageFormats::EncodePNG(const Image& image) {
	std::vector<unsigned char> rgb;
	rgb.reserve(image.Pixels().size() * 3);
	for (const RGBA& p : image.Pixels()) {
		rgb.insert(rgb.end(), { p.r, p.g, p.b });
	}

	std::vector<unsigned char> png;
	if (unsigned error = lodepng::encode(png, rgb, image.Width(), image.Height(), LCT_RGB, 8)) {
		throw ImageError(std::string("PNG: ") + lodepng_error_text(error));
	}
	return png;
}
