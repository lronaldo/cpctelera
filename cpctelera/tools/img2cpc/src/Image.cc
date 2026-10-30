#include "Image.hpp"
#include "ImageFormats.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

enum class ImageFormat { Unknown, PNG, GIF, BMP, PCX, TGA };

bool StartsWith(std::span<const std::uint8_t> data, std::string_view prefix) {
	return data.size() >= prefix.size()
	    && std::equal(prefix.begin(), prefix.end(), data.begin(),
	                  [](char c, std::uint8_t b) { return std::uint8_t(c) == b; });
}

ImageFormat FormatFromSignature(std::span<const std::uint8_t> data) {
	using namespace std::string_view_literals;

	if (StartsWith(data, "\x89PNG\r\n\x1A\n"sv))               { return ImageFormat::PNG; }
	if (StartsWith(data, "GIF87a"sv) || StartsWith(data, "GIF89a"sv)) { return ImageFormat::GIF; }
	if (StartsWith(data, "BM"sv))                              { return ImageFormat::BMP; }

	// PCX: manufacturer 0x0A, version 0-5, encoding 0-1, 1/2/4/8 bits per plane.
	if (data.size() >= 128 && data[0] == 0x0A && data[1] <= 5 && data[2] <= 1
	    && (data[3] == 1 || data[3] == 2 || data[3] == 4 || data[3] == 8)) {
		return ImageFormat::PCX;
	}

	// TGA has no header signature. Version 2 files end with a fixed footer.
	constexpr auto tgaFooter = "TRUEVISION-XFILE.\0"sv;
	if (data.size() >= 18 + tgaFooter.size()
	    && StartsWith(data.last(tgaFooter.size()), tgaFooter)) {
		return ImageFormat::TGA;
	}
	return ImageFormat::Unknown;
}

ImageFormat FormatFromExtension(const std::filesystem::path& fileName) {
	std::string ext = fileName.extension().string();
	std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });

	if (ext == ".png")                                  { return ImageFormat::PNG; }
	if (ext == ".gif")                                  { return ImageFormat::GIF; }
	if (ext == ".bmp" || ext == ".dib")                 { return ImageFormat::BMP; }
	if (ext == ".pcx")                                  { return ImageFormat::PCX; }
	if (ext == ".tga" || ext == ".targa")               { return ImageFormat::TGA; }
	return ImageFormat::Unknown;
}

}

Image::Image(std::uint32_t width, std::uint32_t height)
	: _width(width), _height(height), _pixels(std::size_t(width) * height) {}

Image Image::Load(const std::filesystem::path& fileName) {
	std::ifstream file(fileName, std::ios::binary);
	if (!file) {
		throw ImageError("cannot open file");
	}
	std::vector<std::uint8_t> data{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	if (file.bad()) {
		throw ImageError("error reading file");
	}
	return Decode(data, fileName);
}

Image Image::Decode(std::span<const std::uint8_t> data, const std::filesystem::path& nameHint) {
	ImageFormat format = FormatFromSignature(data);
	if (format == ImageFormat::Unknown) {
		format = FormatFromExtension(nameHint);
	}

	switch (format) {
		case ImageFormat::PNG: return ImageFormats::DecodePNG(data);
		case ImageFormat::GIF: return ImageFormats::DecodeGIF(data);
		case ImageFormat::BMP: return ImageFormats::DecodeBMP(data);
		case ImageFormat::PCX: return ImageFormats::DecodePCX(data);
		case ImageFormat::TGA: return ImageFormats::DecodeTGA(data);
		case ImageFormat::Unknown: break;
	}
	throw ImageError("unknown or unsupported image format (supported formats: PNG, GIF, BMP, PCX, TGA)");
}

void Image::SavePNG(const std::filesystem::path& fileName) const {
	const std::vector<std::uint8_t> png = ImageFormats::EncodePNG(*this);
	std::ofstream file(fileName, std::ios::binary);
	file.write(reinterpret_cast<const char*>(png.data()), std::streamsize(png.size()));
	if (!file) {
		throw ImageError("cannot write file '" + fileName.string() + "'");
	}
}

Image Image::Crop(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) const {
	Image result(width, height);
	for (std::uint32_t row = 0; row < height; ++row) {
		auto source = _pixels.begin() + std::ptrdiff_t(std::size_t(y + row) * _width + x);
		std::copy_n(source, width, result._pixels.begin() + std::ptrdiff_t(std::size_t(row) * width));
	}
	return result;
}

void ImageFormats::CheckDimensions(std::int64_t width, std::int64_t height, const char* format) {
	if (width <= 0 || height <= 0 || width > Image::MaxDimension || height > Image::MaxDimension) {
		throw ImageError(std::string(format) + ": invalid image size "
		                 + std::to_string(width) + "x" + std::to_string(height));
	}
}
