#ifndef _OUTPUT_GENERATOR_H_
#define _OUTPUT_GENERATOR_H_

#include <vector>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include "ConversionOptions.hpp"
#include "Tile.hpp"
#include "FileUtils.hpp"

using namespace std;

class OutputGenerator {

	string DATA_CREATED_WITH = "Data created with Img2CPC - (c) Retroworks - 2007-2017";
	string ASM_COMMENT_PREFIX = ";; ";
	string ASXXXX_COMMENT_PREFIX = ";; ";
	string C_COMMENT_PREFIX = "// ";

	vector<int> GetPaletteValues(ConversionOptions &options) {
		vector<int> palette;
		switch (options.PaletteFormat) {
		case ConversionOptions::FIRMWARE:
			palette = options.Palette.GetPaletteAsFW();
			break;
		case ConversionOptions::HARDWARE:
			palette = options.Palette.GetPaletteAsHW();
			break;
		default:
			break;
		}
		return palette;
	};

public:
	inline string toHexString(unsigned char data) {
		stringstream ss;
		ss << hex << setw(2) << setfill('0') << (int)data;
		return ss.str();
	};

	void DumpTableASM(unsigned char* table, int tableSize, string name) {
		stringstream ss;
		ss << name << ".asm";
		string fileName = ss.str();

		ofstream ofs(fileName);
		ofs << name << ":";
		for(int i=0;i<tableSize;++i) {
			if(i % 0x10 == 0) {
				ofs << endl << "DEFB ";
			} else {
				if (i > 0) {
					ofs << ", ";
				}				
			}
			ofs << "#" << toHexString(table[i]);
		}
		ofs << endl;
		ofs.close();
	}

	void DumpTableASXXXX(unsigned char* table, int tableSize, string name) {
		stringstream ss;
		ss << name << ".s";
		string fileName = ss.str();

		ofstream ofs(fileName);
		ofs << name << "::";
		for(int i=0;i<tableSize;++i) {
			if(i % 0x10 == 0) {
				ofs << endl << ".db ";
			} else {
				if (i > 0) {
					ofs << ", ";
				}				
			}
			ofs << "#0x" << toHexString(table[i]);
		}
		ofs << endl;
		ofs.close();
	}

	void DumpTableC(unsigned char* table, int tableSize, string name) {
		stringstream css, hss;
		css << name << ".c";
		hss << name << ".h";
		string cFileName = css.str();
		string hFileName = hss.str();
		string sanitizedFileName = FileUtils::Sanitize(hFileName);

		ofstream cofs(cFileName); 
		ofstream hofs(hFileName);

		hofs << "#ifndef _" << sanitizedFileName << "_" << endl;
		hofs << "#define _" << sanitizedFileName << "_" << endl << endl;
		hofs << "extern u8 const " << name << "[" << tableSize << "];" << endl << endl;
		hofs << "#endif" << endl;
		hofs.close();

		cofs << "#include \"" << hFileName << "\"" << endl;
		cofs << "const u8 " << name << "[" << tableSize << "] = { ";
		for(int i=0;i<tableSize;++i) {
			if (i > 0) { 
				cofs << ", ";
			}
			if(i % 0x10 == 0) {
				cofs << endl << "\t";
			}
			cofs << "0x" << toHexString(table[i]);
		}
		cofs << endl << "};" << endl << endl;
		cofs.close();
	}

	void DumpTableBIN(unsigned char* table, int tableSize, string name) {
		stringstream ss;
		ss << name << ".bin";
		string binFileName = ss.str();

		ofstream ofs(binFileName, ios::binary);
		for(int i=0;i<tableSize;++i) {
			ofs << table[i];
		}
		ofs.close();

		stringstream asmSs;
		asmSs << name << ".asm";
		string asmFileName = asmSs.str();

		ofstream asmOfs(asmFileName);
		asmOfs << name << ":" << endl << "INCBIN \"" << binFileName << "\"" << endl;
		asmOfs.close();
	}

	void DumpPaletteC(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "// Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;
			
			ofs << "const unsigned char ";
			if(!options.BaseName.empty()) {
				ofs << options.BaseName << "_";
			}
			ofs << "palette["<< numColors <<"] = { 0x" << toHexString(palette[0]);
			
			for (unsigned int i = 1; i<numColors; ++i) {
				ofs << ", 0x" << toHexString(palette[i]);
			}
			ofs << "}\n\n";
		}
	}

	void DumpPaletteASM(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "; Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;
			
			if(!options.BaseName.empty()) {
				ofs << options.BaseName << "_";
			}
			ofs << "palette:" << endl << "DEFB ";
			
			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "#" << toHexString(palette[i]);
			}
			ofs << endl << endl;
		}
	}

	void DumpPaletteASXXXX(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "; Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;
			
			if(!options.BaseName.empty()) {
				ofs << "_" << options.BaseName << "_";
			}
			ofs << "palette::" << endl << ".db ";

			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "#0x" << toHexString(palette[i]);
			}
			ofs << endl << endl;
		}
	}

	unsigned int DumpPaletteBIN(ConversionOptions &options, ofstream &ofs) {
		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		for (unsigned int i = 0; i<numColors; ++i) {
			ofs << (unsigned char)palette[i];
		}
		return numColors;
	}

	void DumpTileMapC(vector<Tile*> tiles, ConversionOptions &options, ofstream &ofs) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			ofs << options.BaseName << "_tileset["<< numTiles <<"] = { " << tiles[0]->Name;
			for (unsigned int i = 1; i < numTiles; ++i) {
				ofs << ", " << tiles[i]->Name;
			}
			ofs << "}\n\n";
			
			if (options.Palette.TransparentIndex >= 0 && !(options.NoMaskData || options.InterlaceMasks)) {
				ofs << options.BaseName <<"_masks_tileset["<< numTiles << "] = { " << tiles[0]->Name << "_mask";
				
				for (unsigned int i = 0; i < numTiles; ++i) {
					ofs << ", " << tiles[i]->Name << "_mask";
				}
				ofs << "}\n\n";
			}
		}
	}

	void DumpTileMap(vector<Tile*> tiles, ConversionOptions &options, ofstream &ofs) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			ofs << options.BaseName << "tileset:" << endl << "DEFW ";
			for (unsigned int i = 0; i < numTiles; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << tiles[i]->Name;
			}
			ofs << endl << endl;
			if (options.Palette.TransparentIndex >= 0 && !(options.NoMaskData || options.InterlaceMasks)) {
				
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "masks_tileset:" << endl << "DEFW ";
				
				for (unsigned int i = 0; i < numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i]->Name << "_mask";
				}
				ofs << endl << endl;
			}
		}
	}

	void DumpTileMapASXXXX(vector<Tile*> tiles, ConversionOptions &options, ofstream &ofs) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			ofs << "_" << options.BaseName << "tileset::" << endl << ".dw ";
			for (unsigned int i = 0; i < numTiles; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << tiles[i]->Name;
			}
			ofs << endl << endl;
			if (options.Palette.TransparentIndex >= 0 && ! (options.NoMaskData || options.InterlaceMasks)) {
				if(!options.BaseName.empty()) {
					ofs << "_" << options.BaseName;	
				}
				ofs << "_masks_tileset::" << endl << ".dw ";
				for (unsigned int i = 0; i < numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i]->Name << "_mask";
				}
				ofs << endl << endl;
			}
		}
	}

	void GenerateASM(vector<Tile*> tiles, ConversionOptions &options) {
		stringstream ss;

		ss << options.OutputFileName << ".asm";
		string fileName = ss.str();

		ofstream ofs(fileName);

		ofs << ASM_COMMENT_PREFIX << DATA_CREATED_WITH << endl;
		DumpPaletteASM(options, ofs);

		if(options.CreateTileset) {
			DumpTileMap(tiles, options, ofs);
		}

		for (Tile* t : tiles) {
			generateTileASM(t, ofs, options);
		}
		ofs.close();
	};

	void generateTileASM(Tile* t, ofstream& ofs, ConversionOptions &options) {
		int numBytes = t->Data.size();
		if (numBytes > 0) {
			int horizLimit = 0, vertLimit = 0;
			if(options.PixelOrder == ConversionOptions::ROW) {
				horizLimit = t->TileWidthInBytes;
				vertLimit = t->TileHeight;
			}
			if(options.PixelOrder == ConversionOptions::COLUMN) {
				horizLimit = t->TileHeight;
				vertLimit = t->TileWidthInBytes;
			}
			ofs << ";; Tile " << t->Name << " - " << t->TileWidth << "x" << t->TileHeight << " pixels, " << t->TileWidthInBytes << "x" << t->TileHeight << (options.InterlaceMasks && options.Palette.TransparentIndex >= 0 ? "x2" : "") << " bytes." << endl;

			if(options.OutputSize) {
				string defineBase = t->Name;
				transform(defineBase.begin(), defineBase.end(), defineBase.begin(), ::toupper);
				ofs << defineBase << "_W EQU " << t->TileWidthInBytes << endl;
				ofs << defineBase << "_H EQU " << t->TileHeight << endl;
			}

			ofs << t->Name << ":" << endl;
			if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
				ofs << ";; Mask data is interlaced (MASK BYTE, DATA BYTE)." << endl;
				int currentByte = 0;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << "DEFB #" << toHexString(t->MaskData[currentByte]) << ", #" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", #" << toHexString(t->MaskData[currentByte]) << ", #" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					ofs << endl;
				}
			}
			else if(options.RLE) {
				ofs << ";; " << numBytes << " bytes in RLE." << endl;
				for (int x = 0; x < numBytes; ++x) {
					if((x & 0xF) == 0) {
						ofs << endl << "DEFB #" << toHexString(t->Data[x]);
					} else {
						ofs << ", #" << toHexString(t->Data[x]);
					}
				}
				ofs << endl;
			} else {
				int currentByte = 0;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << "DEFB #" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", #" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					ofs << endl;
				}
				if (!options.NoMaskData && options.Palette.TransparentIndex >= 0) {
					currentByte = 0;
					ofs << t->Name << "_MASK:" << endl;
					for (int y = 0; y < vertLimit; ++y) {
						ofs << "DEFB #" << toHexString(t->MaskData[currentByte]);
						currentByte++;
						for (int x = 0; x < horizLimit - 1; ++x) {
							ofs << ", #" << toHexString(t->MaskData[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
				}
			}
			ofs << endl;
		}
	}

	void GenerateASXXXX(vector<Tile*> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".s";
		string fileName = ss.str();

		ofstream ofs(fileName);

		ofs << ASXXXX_COMMENT_PREFIX << DATA_CREATED_WITH << endl;
		DumpPaletteASXXXX(options, ofs);

		if(options.CreateTileset) {
			DumpTileMapASXXXX(tiles, options, ofs);
		}

		for (Tile* t : tiles) {
			generateTileASXXX(t, ofs, options);
		}
		ofs.close();
	};

	void generateTileASXXX(Tile* t, ofstream& ofs, ConversionOptions &options) {
		int numBytes = t->Data.size();
		if (numBytes > 0) {
			int horizLimit = 0, vertLimit = 0;
			if(options.PixelOrder == ConversionOptions::ROW) {
				horizLimit = t->TileWidthInBytes;
				vertLimit = t->TileHeight;
			}
			if(options.PixelOrder == ConversionOptions::COLUMN) {
				horizLimit = t->TileHeight;
				vertLimit = t->TileWidthInBytes;
			}
			ofs << ";; Tile " << t->Name << " - " << t->TileWidth << "x" << t->TileHeight << " pixels, " << t->TileWidthInBytes << "x" << t->TileHeight << (options.InterlaceMasks && options.Palette.TransparentIndex >= 0 ? "x2" : "") << " bytes." << endl;

			if(options.OutputSize) {
				string defineBase = t->Name;
				transform(defineBase.begin(), defineBase.end(), defineBase.begin(), ::toupper);
				ofs << defineBase << "_W == #0x" << toHexString(t->TileWidthInBytes) << endl;
				ofs << defineBase << "_H == #0x" << toHexString(t->TileHeight) << endl;						
			}

			ofs << t->Name << "::" << endl;
			if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
				ofs << ";; Mask data is interlaced (MASK BYTE, DATA BYTE)." << endl;
				int currentByte = 0;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << ".db #0x" << toHexString(t->MaskData[currentByte]) << ", #0x" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", #0x" << toHexString(t->MaskData[currentByte]) << ", #0x" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					ofs << endl;
				}
			}
			else if(options.RLE) {
				ofs << ";; " << numBytes << " bytes in RLE." << endl;
				for (int x = 0; x < numBytes; ++x) {
					if((x & 0xF) == 0) {
						ofs << endl << ".db #0x" << toHexString(t->Data[x]);
					} else {
						ofs << ", #0x" << toHexString(t->Data[x]);
					}
				}
				ofs << endl;
			} else {
				int currentByte = 0;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << ".db #0x" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", #0x" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					ofs << endl;
				}
				if (!options.NoMaskData && options.Palette.TransparentIndex >= 0) {
					currentByte = 0;
					ofs << t->Name << "_MASK::" << endl;
					for (int y = 0; y < vertLimit; ++y) {
						ofs << ".db #0x" << toHexString(t->MaskData[currentByte]);
						currentByte++;
						for (int x = 0; x < horizLimit - 1; ++x) {
							ofs << ", #0x" << toHexString(t->MaskData[currentByte]);
							currentByte++;
						}
						ofs << endl;
					}
				}
			}
			ofs << endl;
		}

	}
	
	void GenerateMultipleBIN(vector<Tile*> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".asm";
		string fileName = ss.str();

		string baseOutputFileName = FileUtils::RemoveExtension(options.OutputFileName);

		ofstream os(fileName);
		os << ASM_COMMENT_PREFIX << DATA_CREATED_WITH << endl;
		
		DumpPaletteASM(options, os);
		
		if(options.CreateTileset) {
			DumpTileMap(tiles, options, os);
		}

		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			for (Tile* t : tiles) {
				int numBytes = t->Data.size();
				if (numBytes > 0) {
					stringstream ss;
					ss << options.OutputFileName << t->Name << ".bin";
					string binFileName = ss.str();
					ofstream ofs(binFileName, ios::binary);

					os << ";; Tile " << t->Name << " - " << t->TileWidth << "x" << t->TileHeight << " pixels, " << t->TileWidthInBytes << "x" << t->TileHeight << (options.InterlaceMasks && options.Palette.TransparentIndex >= 0 ? "x2" : "") << " bytes." << endl;

					if(options.OutputSize) {
						string defineBase = t->Name;
						transform(defineBase.begin(), defineBase.end(), defineBase.begin(), ::toupper);
						os << defineBase << "_W EQU " << t->TileWidthInBytes << endl;
						os << defineBase << "_H EQU " << t->TileHeight << endl;						
					}

					os << t->Name << ":" << endl;
					os << "INCBIN \"" << FileUtils::GetFileName(binFileName) << "\"" << endl;

					if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
						os << ";; Mask data is interlaced (MASK BYTE, DATA BYTE)." << endl;
						for (int i = 0; i < numBytes; ++i) {
							ofs << t->MaskData[i] << t->Data[i];
						}
					}
					else {
						for (int i = 0; i < numBytes; ++i) {
							ofs << t->Data[i];
						}

						if (options.Palette.TransparentIndex >= 0 && !(options.NoMaskData)) {
							stringstream ssMask;
							ssMask << options.OutputFileName << t->Name << "_mask.bin";
							string maskFileName = ssMask.str();
							ofstream ofsMask(maskFileName, ios::binary);

							os << t->Name << "_mask:" << endl;
							os << "INCBIN \"" << FileUtils::GetFileName(maskFileName) << "\"" << endl;

							for (int i = 0; i < numBytes; ++i) {
								ofsMask << t->MaskData[i];
							}
							ofsMask.close();
						}
					}
					ofs.close();
				}
			}
		}
		os.close();
	};


	void GenerateBIN(vector<Tile*> tiles, ConversionOptions &options) {
		// Setup .bin, .h and .h.s files
		stringstream ss;
		ss << options.OutputFileName << ".h.s"; ofstream os(ss.str()); ss.str("");
		ss << options.OutputFileName << ".h";   ofstream oh(ss.str()); ss.str("");
		ss << options.OutputFileName << ".bin"; ofstream ofs(ss.str(), ios::binary);

		//string baseOutputFileName = FileUtils::RemoveExtension(options.OutputFileName);

		// Initial comments
		os << ASM_COMMENT_PREFIX << DATA_CREATED_WITH << '\n';
		oh << C_COMMENT_PREFIX << DATA_CREATED_WITH << '\n';
		
		// Generate Palettes 
		unsigned int totalBytes = DumpPaletteBIN(options, ofs);
		ss.str("");
		ss << "Palete constants\n";
		os << ";; " << ss.str();
		oh << "// " << ss.str();
		ss.str("");
		if(!options.BaseName.empty()) ss << options.BaseName << "_";
		ss << "palette";
		string palette_name = ss.str();
		transform(palette_name.begin(), palette_name.end(), palette_name.begin(), ::toupper);
		os << palette_name << "_OFF  == 0\n";
		os << palette_name << "_SIZE == " << totalBytes << "\n\n";
		oh << "#define " << palette_name << "_OFF    0\n";
		oh << "#define " << palette_name << "_SIZE   " << totalBytes << "\n\n";

		// Generate Tileset
		if(options.CreateTileset) {
			ss.str("");
			ss << " No tileset has been generated, because tilesets require absolute pointers and binary generation can only generate offsets.\n";
			os << ";; " << ss.str();
			oh << "// " << ss.str();
			ss.str("");
			ss << " However, you can calculate your own tileset by adding up offsets of tiles to the address where the first tile starts.\n\n";
			os << ";; " << ss.str();
			oh << "// " << ss.str();
		}

		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			for (Tile* t : tiles) {
				int numBytes = t->Data.size();
				if (numBytes > 0) {
					string defineBase = t->Name;
					transform(defineBase.begin(), defineBase.end(), defineBase.begin(), ::toupper);

					// Tile information in the comment
					ss.str(""); 
					ss << "Tile " << t->Name << " - " << t->TileWidth << "x" << t->TileHeight 
						<< " pixels, " << t->TileWidthInBytes << "x" << t->TileHeight 
						<< (options.InterlaceMasks && options.Palette.TransparentIndex >= 0 ? "x2" : "") 
						<< " bytes." << '\n';
					os << ";; " << ss.str();
					oh << "// " << ss.str();

					// Tile constants
					if(options.OutputSize) {
						os << defineBase << "_OFF      == " << totalBytes << '\n';
						os << defineBase << "_SIZE     == " << numBytes << '\n';
						os << defineBase << "_W        == " << t->TileWidthInBytes << '\n';
						os << defineBase << "_H        == " << t->TileHeight << '\n';
						oh << "#define " << defineBase << "_OFF        " << totalBytes << '\n';
						oh << "#define " << defineBase << "_SIZE       " << numBytes << '\n';
						oh << "#define " << defineBase << "_W          " << t->TileWidthInBytes << '\n';
						oh << "#define " << defineBase << "_H          " << t->TileHeight << '\n';
					}

					// Mask Data
					if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
						ss.str("");
						ss << "Mask data is interlaced (MASK BYTE, DATA BYTE)." << '\n';
						os << ";; " << ss.str();
						oh << "// " << ss.str();
						for (int i = 0; i < numBytes; ++i) {
							ofs << t->MaskData[i] << t->Data[i];
						}
					} else {
						for (int i = 0; i < numBytes; ++i) {
							ofs << t->Data[i];
						}

						// Non-interlaced Mask Data
						if (options.Palette.TransparentIndex >= 0 && !(options.NoMaskData)) {
							//os << t->Name << "_mask:" << '\n';
							totalBytes += numBytes;
							os << defineBase << "_MASK_OFF == " << totalBytes << '\n';
							oh << "#define " << defineBase << "_MASK_OFF   " << totalBytes << '\n';

							for (int i = 0; i < numBytes; ++i) {
								ofs << t->MaskData[i];
							}
						}
					}
				}
				// Update totalbytes
				totalBytes += numBytes;
			}
		}
		os.close();
		oh.close();
		ofs.close();
	};

	void GenerateH(vector<Tile*> tiles, ConversionOptions &options) {
		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			stringstream ss;
			ss << options.OutputFileName << ".h";
			string fileName = ss.str();

			ofstream ofs(fileName);

			ofs << C_COMMENT_PREFIX << DATA_CREATED_WITH << endl;

			string sanitizedFileName = FileUtils::Sanitize(fileName);
			transform(sanitizedFileName.begin(), sanitizedFileName.end(), sanitizedFileName.begin(), ::toupper);

			ofs << "#ifndef _" << sanitizedFileName << "_" << endl;
			ofs << "#define _" << sanitizedFileName << "_" << endl << endl;

			for(string i : options.AdditionalIncludes) {
				ofs << "#include " << i << endl;
			}

			vector<int> palette = GetPaletteValues(options);
			unsigned int numColors = palette.size();
			if (numColors > 0) {
				ofs << "extern const u8 ";
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "palette[" << numColors << "];" << endl << endl;
			}

			if(options.CreateTileset) {
				ofs << "extern u8* const " ;
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "tileset[" << numTiles << "];" << endl << endl;
				if (!(options.InterlaceMasks || options.NoMaskData) && options.Palette.TransparentIndex >= 0) {
					ofs << "extern u8* const "; 
					if(!options.BaseName.empty()) {
						ofs << options.BaseName << "_";		
					}
					ofs << "masks_tileset[" << numTiles << "];" << endl << endl;
				}
			}

			for (Tile* t : tiles) {
				int numBytes = t->Data.size();
				if (numBytes > 0) {

					if(options.OutputSize) {
						string defineBase = t->Name;
						transform(defineBase.begin(), defineBase.end(), defineBase.begin(), ::toupper);
						ofs << "#define " << defineBase << "_W " << t->TileWidthInBytes << endl;
						ofs << "#define " << defineBase << "_H " << t->TileHeight << endl;						
					}

					if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
						ofs << "extern const u8 " << t->Name << "[2 * " << t->TileWidthInBytes << " * " << t->TileHeight << "];" << endl;
					}
					else if(options.RLE) {
						ofs << "extern const u8 " << t->Name << "[" << numBytes << "];" << endl;
					} else {
						ofs << "extern const u8 " << t->Name << "[" << t->TileWidthInBytes << " * " << t->TileHeight << "];" << endl;
						if (!options.NoMaskData && options.Palette.TransparentIndex >= 0) {
							ofs << "extern const u8 " << t->Name << "_mask[" << t->TileWidthInBytes << " * " << t->TileHeight << "];" << endl;
						}
					}
				}
			}
			ofs << endl << "#endif" << endl;
			ofs.close();
		}
	}

	void GenerateC(vector<Tile*> tiles, ConversionOptions &options) {
		stringstream ss;
		ss << options.OutputFileName << ".c";
		string fileName = ss.str();

		ofstream ofs(fileName);
		ofs << "#include \"" << FileUtils::GetFileName(options.OutputFileName) << ".h\"" << endl;
		ofs << C_COMMENT_PREFIX << DATA_CREATED_WITH << endl;

		vector<int> palette = GetPaletteValues(options);
		unsigned int numColors = palette.size();
		if (numColors > 0) {
			ofs << "// Palette uses " << ConversionOptions::ToString(options.PaletteFormat) << " values." << endl;

			ofs << "const u8 ";
			if(!options.BaseName.empty()) { 
				ofs << options.BaseName << "_";
			}
			ofs << "palette[" << numColors << "] = { ";
			for (unsigned int i = 0; i<numColors; ++i) {
				if (i > 0) {
					ofs << ", ";
				}
				ofs << "0x" << toHexString(palette[i]);
			}
			ofs << " };" << endl << endl;
		}

		unsigned int numTiles = tiles.size();
		if (numTiles > 0) {
			if(options.CreateTileset) {
				ofs << "u8* const ";
				if(!options.BaseName.empty()) {
					ofs << options.BaseName << "_";
				}
				ofs << "tileset[" << numTiles << "] = { " << endl << "\t";
				for (unsigned int i = 0; i<numTiles; ++i) {
					if (i > 0) {
						ofs << ", ";
					}
					ofs << tiles[i]->Name;
				}
				ofs << endl << "};" << endl;
				if (options.Palette.TransparentIndex >= 0 && !(options.InterlaceMasks || options.NoMaskData)) {
					ofs << "u8* const " << options.BaseName << "_masks_tileset[" << numTiles << "] = { " << endl << "\t";
					for (unsigned int i = 0; i<numTiles; ++i) {
						if (i > 0) {
							ofs << ", ";
						}
						ofs << tiles[i]->Name << "_mask";
					}
					ofs << endl << "};" << endl;
				}
			}
			
			for (Tile* t : tiles) {
				generateTileC(t, ofs, options);
			}
		}
		ofs.close();
	};
	
	void generateTileC(Tile* t, ofstream& ofs, ConversionOptions &options) {
		int numBytes = t->Data.size();
		if (numBytes > 0) {
			int horizLimit = 0, vertLimit = 0;
			if(options.PixelOrder == ConversionOptions::ROW) {
				horizLimit = t->TileWidthInBytes;
				vertLimit = t->TileHeight;
			}
			if(options.PixelOrder == ConversionOptions::COLUMN) {
				horizLimit = t->TileHeight;
				vertLimit = t->TileWidthInBytes;
			}

			ofs << "// Tile " << t->Name << ": " << t->TileWidth << "x" << t->TileHeight << " pixels, " << t->TileWidthInBytes << "x" << t->TileHeight << (options.InterlaceMasks && options.Palette.TransparentIndex >= 0 ?  "x2" : "") << " bytes." << endl;

			if (options.InterlaceMasks && options.Palette.TransparentIndex >= 0) {
				ofs << "// Mask data is interlaced (MASK BYTE, DATA BYTE)." << endl;
				ofs << "const u8 " << t->Name << "[2 * " << t->TileWidthInBytes << " * " << t->TileHeight << "] = {" << endl;
				int currentByte = 0;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << "\t0x" << toHexString(t->MaskData[currentByte]) << ", 0x" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", 0x" << toHexString(t->MaskData[currentByte]) << ", 0x" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					if (currentByte < numBytes) {
						ofs << ",";
					}
					ofs << endl;
				}
				ofs << "};" << endl;
			}
			else if(options.RLE) {
				ofs << "const u8 " << t->Name << "[" << numBytes << "] = {" << endl;

				ofs << "0x" << toHexString(t->Data[0]);
				for (int x = 1; x < numBytes; ++x) {
					if((x & 0xF) == 0) {
						ofs << endl;
					} 
					ofs << ", 0x" << toHexString(t->Data[x]);
				}
				ofs << "};" << endl;
			} else {
				int currentByte = 0;
				ofs << "const u8 " << t->Name << "[" << t->TileWidthInBytes << " * " << t->TileHeight << "] = {" << endl;
				for (int y = 0; y < vertLimit; ++y) {
					ofs << "\t0x" << toHexString(t->Data[currentByte]);
					currentByte++;
					for (int x = 0; x < horizLimit - 1; ++x) {
						ofs << ", 0x" << toHexString(t->Data[currentByte]);
						currentByte++;
					}
					if (currentByte < numBytes) {
						ofs << ",";
					}
					ofs << endl;
				}
				ofs << "};" << endl;

				if (!options.NoMaskData && options.Palette.TransparentIndex >= 0) {
					currentByte = 0;
					ofs << "const u8 " << t->Name << "_mask[" << t->TileWidthInBytes << " * " << t->TileHeight << "] = {" << endl;
					for (int y = 0; y < vertLimit; ++y) {
						ofs << "\t0x" << toHexString(t->MaskData[currentByte]);
						currentByte++;
						for (int x = 0; x < horizLimit - 1; ++x) {
							ofs << ", 0x" << toHexString(t->MaskData[currentByte]);
							currentByte++;
						}
						if (currentByte < numBytes) {
							ofs << ",";
						}
						ofs << endl;
					}
					ofs << "};" << endl;
				}
			}
			ofs << endl;
		}
	}
};

#endif
