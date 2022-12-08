#include "file.hpp"
#include <cstdio>
#include <cassert>

namespace gli{
namespace detail
{
	static unsigned char const FOURCC_KMG100[] = {0xAB, 0x4B, 0x49, 0x4D, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

	struct kmgHeader10
	{
		std::uint32_t Endianness;
		std::uint32_t Format;
		std::uint32_t Target;
		std::uint32_t SwizzleRed;
		std::uint32_t SwizzleGreen;
		std::uint32_t SwizzleBlue;
		std::uint32_t SwizzleAlpha;
		std::uint32_t PixelWidth;
		std::uint32_t PixelHeight;
		std::uint32_t PixelDepth;
		std::uint32_t Layers;
		std::uint32_t Levels;
		std::uint32_t Faces;
		std::uint32_t GenerateMipmaps;
		std::uint32_t BaseLevel;
		std::uint32_t MaxLevel;
	};

	inline texture load_kmg100(char const * Filename)
	{
		FILE* File = detail::open_file(Filename, "rb");
		if(!File)
			return texture();
		
		std::remove_const<decltype(detail::FOURCC_KMG100)>::type FourCC {};
	    	if(std::fread(FourCC, sizeof(FourCC), 1, File) < 1)
            	{
			std::fclose(File);
                	return texture();
            	}
		
		if(memcmp(FourCC, detail::FOURCC_KMG100, sizeof(detail::FOURCC_KMG100)) != 0)
		{
			std::fclose(File);
                	return texture();
		}
		
		detail::kmgHeader10 Header {};
		if(std::fread(&Header, sizeof(Header), 1, File) < 1)
		{
			std::fclose(File);
                	return texture();
		}

		texture Texture(
			static_cast<target>(Header.Target),
			static_cast<format>(Header.Format),
			texture::extent_type(Header.PixelWidth, Header.PixelHeight, Header.PixelDepth),
			Header.Layers,
			Header.Faces,
			Header.Levels,
			texture::swizzles_type(Header.SwizzleRed, Header.SwizzleGreen, Header.SwizzleBlue, Header.SwizzleAlpha));

		for(texture::size_type Layer = 0, Layers = Texture.layers(); Layer < Layers; ++Layer)
		for(texture::size_type Level = 0, Levels = Texture.levels(); Level < Levels; ++Level)
		{
			texture::size_type const FaceSize = static_cast<texture::size_type>(Texture.size(Level));
			for(texture::size_type Face = 0, Faces = Texture.faces(); Face < Faces; ++Face)
			{
				if(std::fread(Texture.data(Layer, Face, Level), FaceSize, 1, File) < 1)
				{
					std::fclose(File);
                			return texture();
				}
			}
		}

		return texture(
			Texture, Texture.target(), Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Header.BaseLevel, Header.MaxLevel, 
			Texture.swizzles());
	}
}//namespace detail

	inline texture load_kmg(std::string const & Filename)
	{
		return detail::load_kmg100(Filename.c_str());
	}
}//namespace gli
