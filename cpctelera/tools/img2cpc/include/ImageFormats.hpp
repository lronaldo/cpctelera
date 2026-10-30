#ifndef _IMAGE_FORMATS_H_
#define _IMAGE_FORMATS_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
#include "Image.hpp"

//
// Internal interface between Image and the per-format decoders.
// Decoders reproduce the behaviour img2cpc had with FreeImage:
// pixels keep their RGB values even when fully transparent.
//
namespace ImageFormats {

	Image DecodePNG(std::span<const std::uint8_t> data);
	Image DecodeGIF(std::span<const std::uint8_t> data);
	Image DecodeBMP(std::span<const std::uint8_t> data);
	Image DecodePCX(std::span<const std::uint8_t> data);
	Image DecodeTGA(std::span<const std::uint8_t> data);

	std::vector<std::uint8_t> EncodePNG(const Image& image);

	// Throws ImageError if width or height are zero or too big.
	void CheckDimensions(std::int64_t width, std::int64_t height, const char* format);

	// Little-endian reader with bounds checking. Throws ImageError on truncated data.
	class ByteReader {
	public:
		explicit ByteReader(std::span<const std::uint8_t> data, std::size_t pos = 0) noexcept
			: _data(data), _pos(pos) {}

		std::uint8_t U8() {
			Require(1);
			return _data[_pos++];
		}
		std::uint16_t U16() {
			Require(2);
			std::uint16_t v = std::uint16_t(_data[_pos] | (_data[_pos + 1] << 8));
			_pos += 2;
			return v;
		}
		std::uint32_t U32() {
			Require(4);
			std::uint32_t v = std::uint32_t(_data[_pos])
			                | std::uint32_t(_data[_pos + 1]) << 8
			                | std::uint32_t(_data[_pos + 2]) << 16
			                | std::uint32_t(_data[_pos + 3]) << 24;
			_pos += 4;
			return v;
		}
		std::int32_t I32() { return std::int32_t(U32()); }

		std::span<const std::uint8_t> Bytes(std::size_t count) {
			Require(count);
			auto result = _data.subspan(_pos, count);
			_pos += count;
			return result;
		}
		void Skip(std::size_t count) { Require(count); _pos += count; }
		void Seek(std::size_t pos) {
			if (pos > _data.size()) { throw ImageError("truncated or corrupt file"); }
			_pos = pos;
		}

		std::size_t Position()  const noexcept { return _pos; }
		std::size_t Remaining() const noexcept { return _pos < _data.size() ? _data.size() - _pos : 0; }

	private:
		void Require(std::size_t count) const {
			if (count > Remaining()) { throw ImageError("truncated or corrupt file"); }
		}

		std::span<const std::uint8_t> _data;
		std::size_t _pos;
	};

}

#endif
