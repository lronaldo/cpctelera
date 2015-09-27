#ifndef _DSKGEN_OPTIONS_H_
#define _DSKGEN_OPTIONS_H_

#include <string>
#include <fstream>
#include <json/json.h>
#include "Types.hpp"
#include "FileToProcess.hpp"

using namespace std;

class Options {
public:
    Options() { };

    string                     OutputFileName;
    string                     BootFile;
    CatalogType                Catalog;
    DiskType                   OutputDiskType;
    u8                         NumSides;
    struct XDPB                DiskParams;
    vector<FileToProcess>      FilesToProcess;

    void AddFile(const FileToProcess &f) {
        this->FilesToProcess.push_back(f);
    }

    FileToProcess GetBootFile() {
        FileToProcess result;
        for(vector<FileToProcess>::iterator it=FilesToProcess.begin(); it!= FilesToProcess.end();++it) {
            if(it->SourcePath == BootFile) {
                result = *it;
                break;
            }
        }
        return result;
    }

    void SetCatalogType(const string &catStr) {
        this->Catalog = ParseCatalogType(catStr);
    }

    void SetDiskType(const string &diskStr) {
        this->OutputDiskType = ParseDiskType(diskStr);
        if(this->OutputDiskType == DSK_SYSTEM) {
            this->DiskParams.recordsPerTrack = 36;
            this->DiskParams.blockShift = 3;
            this->DiskParams.blockMask = 7;
            this->DiskParams.extentMask = 0;
            this->DiskParams.numBlocks = 170;
            this->DiskParams.dirEntries = 63;
            this->DiskParams.allocationLo = 0x00;
            this->DiskParams.allocationHi = 0xC0;
            this->DiskParams.checksumLength = 16;
            this->DiskParams.reservedTracks = 2;
            this->DiskParams.firstSectorNumber = 0x41;
            this->DiskParams.sectorsPerTrack = 9;
            this->DiskParams.gapRW = 42;
            this->DiskParams.gapF = 82;
            this->DiskParams.fillerByte = 0xE9;
            this->DiskParams.logSectSize = 2;
            this->DiskParams.sectSizeInRecords = 4;
        } else if(this->OutputDiskType == DSK_DATA) {
            this->DiskParams.recordsPerTrack = 36;
            this->DiskParams.blockShift = 3;
            this->DiskParams.blockMask = 7;
            this->DiskParams.extentMask = 0;
            this->DiskParams.numBlocks = 179;
            this->DiskParams.dirEntries = 63;
            this->DiskParams.allocationLo = 0x00;
            this->DiskParams.allocationHi = 0xC0;
            this->DiskParams.checksumLength = 16;
            this->DiskParams.reservedTracks = 0;
            this->DiskParams.firstSectorNumber = 0xC1;
            this->DiskParams.sectorsPerTrack = 9;
            this->DiskParams.gapRW = 42;
            this->DiskParams.gapF = 82;
            this->DiskParams.fillerByte = 0xE9;
            this->DiskParams.logSectSize = 2;
            this->DiskParams.sectSizeInRecords = 4;
        } else if(this->OutputDiskType == DSK_IBM) {
            this->DiskParams.recordsPerTrack = 32;
            this->DiskParams.blockShift = 3;
            this->DiskParams.blockMask = 7;
            this->DiskParams.extentMask = 0;
            this->DiskParams.numBlocks = 155;
            this->DiskParams.dirEntries = 63;
            this->DiskParams.allocationLo = 0x00;
            this->DiskParams.allocationHi = 0xC0;
            this->DiskParams.checksumLength = 16;
            this->DiskParams.reservedTracks = 1;
            this->DiskParams.firstSectorNumber = 0x01;
            this->DiskParams.sectorsPerTrack = 8;
            this->DiskParams.gapRW = 42;
            this->DiskParams.gapF = 80;
            this->DiskParams.fillerByte = 0xE9;
            this->DiskParams.logSectSize = 2;
            this->DiskParams.sectSizeInRecords = 4;
        } else {
            this->DiskParams.recordsPerTrack = 36;
            this->DiskParams.blockShift = 3;
            this->DiskParams.blockMask = 7;
            this->DiskParams.extentMask = 0;
            this->DiskParams.numBlocks = 179;
            this->DiskParams.dirEntries = 63;
            this->DiskParams.allocationLo = 0x00;
            this->DiskParams.allocationHi = 0xC0;
            this->DiskParams.checksumLength = 16;
            this->DiskParams.reservedTracks = 0;
            this->DiskParams.firstSectorNumber = 0xC1;
            this->DiskParams.sectorsPerTrack = 9;
            this->DiskParams.gapRW = 42;
            this->DiskParams.gapF = 82;
            this->DiskParams.fillerByte = 0xE9;
            this->DiskParams.logSectSize = 2;
            this->DiskParams.sectSizeInRecords = 4;
        }
    }

    void ParseFile(string& configFile) {
        ifstream file(configFile, ifstream::binary);
        Json::Value root;
        file >> root;

        this->SetCatalogType(root.get("catalog", "none").asString());
        this->SetDiskType(root.get("diskType", "system").asString());
        this->NumSides = (u8) root.get("sides", "1").asUInt();

        if (root["boot"] != Json::nullValue) {
            this->BootFile = root["boot"].asString();
        }

        if (this->OutputDiskType == DSK_CUSTOM) {
            Json::Value diskParams = root["diskParams"];
            if (!diskParams.isNull()) {
                // Get the XDPB specified parameters...
                if (diskParams["spt"] != Json::nullValue) {
                    this->DiskParams.recordsPerTrack = (u16) diskParams["spt"].asUInt();
                }
                if (diskParams["bsh"] != Json::nullValue) {
                    this->DiskParams.blockShift = (u8)diskParams["bsh"].asUInt();
                }
                if (diskParams["blm"] != Json::nullValue) {
                    this->DiskParams.blockMask = (u8)diskParams["blm"].asUInt();
                }
                if (diskParams["exm"] != Json::nullValue) {
                    this->DiskParams.extentMask = (u8)diskParams["exm"].asUInt();
                }
                if (diskParams["dsm"] != Json::nullValue) {
                    this->DiskParams.numBlocks = (u16)diskParams["dsm"].asUInt();
                }
                if (diskParams["drm"] != Json::nullValue) {
                    this->DiskParams.dirEntries = (u16)diskParams["drm"].asUInt();
                }
                if (diskParams["al0"] != Json::nullValue) {
                    this->DiskParams.allocationLo = (u8)diskParams["al0"].asUInt();
                }
                if (diskParams["al1"] != Json::nullValue) {
                    this->DiskParams.allocationHi = (u8)diskParams["al1"].asUInt();
                }
                if (diskParams["cks"] != Json::nullValue) {
                    this->DiskParams.checksumLength = (u16)diskParams["cks"].asUInt();
                }
                if (diskParams["off"] != Json::nullValue) {
                    this->DiskParams.reservedTracks = (u16)diskParams["off"].asUInt();
                }
                if (diskParams["fsn"] != Json::nullValue) {
                    this->DiskParams.firstSectorNumber = (u8)diskParams["fsn"].asUInt();
                }
                if (diskParams["sectorsPerTrack"] != Json::nullValue) {
                    this->DiskParams.sectorsPerTrack = (u8)diskParams["sectorsPerTrack"].asUInt();
                }
                if (diskParams["gapRW"] != Json::nullValue) {
                    this->DiskParams.gapRW = (u8)diskParams["gapRW"].asUInt();
                }
                if (diskParams["gapF"] != Json::nullValue) {
                    this->DiskParams.gapF = (u8)diskParams["gapF"].asUInt();
                }
                if (diskParams["fillerByte"] != Json::nullValue) {
                    this->DiskParams.fillerByte = (u8)diskParams["fillerByte"].asUInt();
                }
                if (diskParams["logSectSize"] != Json::nullValue) {
                    this->DiskParams.logSectSize = (u8)diskParams["logSectSize"].asUInt();
                }
                if (diskParams["sectsizeInRecords"] != Json::nullValue) {
                    this->DiskParams.sectSizeInRecords = (u8)diskParams["sectSizeInRecords"].asUInt();
                }
            }
        }

        const Json::Value files = root["files"];
        if(files != Json::Value::null) {
            for (Json::Value::iterator it=files.begin(); it != files.end(); ++it) {
                Json::Value current = *it;
                FileToProcess f;
                if (current["path"] == Json::nullValue) {
                    throw "A file was found without path in configration file.";
                }
                
                f.SetSourcePath(current["path"].asString());
                f.Header = ParseHeaderType(current.get("header", "none").asString());
                f.AmsdosType = ParseAmsdosFileType(current.get("amsdosType", "").asString());
                f.LoadAddress = (u16) current.get("loadAddress", 0).asUInt();
                f.ExecutionAddress = (u16)current.get("executionAddress", 0).asUInt();

                this->FilesToProcess.push_back(f);
               }
        }
    }
};

#endif