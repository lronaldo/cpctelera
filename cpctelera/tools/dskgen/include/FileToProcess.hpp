#ifndef _DSKGEN_FILE_TO_PROCESS_H_
#define _DSKGEN_FILE_TO_PROCESS_H_

#include <string>
#include <algorithm>
#include <cstring>

#include "Types.hpp"

class FileToProcess {
public:
    HeaderType Header;
    AmsdosFileType AmsdosType;
    string SourcePath;

    u8 AmsDosName[11];
    u8 Side;
    u8 InitialTrack;
    u8 InitialSectorOffset;
    // File length includes header!
    u16 Length;

    u16 LoadAddress;
    u16 ExecutionAddress;

    void SetSourcePath(string path) {
        string tmpStr(path);
        this->SourcePath = path;
        transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::toupper);
        std::size_t found = tmpStr.find_last_of("/\\");
        if(found != string::npos) {
            tmpStr = tmpStr.substr(found+1);
        }
        // The file name is in 8+3, padded with spaces
        // tmpStr contains the file Name.
        std::size_t dotPosition = tmpStr.find_last_of(".");
        std::size_t maxNameSize = (size_t)8;

        string fileName = tmpStr.substr(0, min(maxNameSize, dotPosition));
        std::size_t nameSize = fileName.size();

        if(nameSize < maxNameSize) {
            fileName.append(maxNameSize - nameSize, ' ');
        }

        string extension;

        if(dotPosition != string::npos) {
            extension = tmpStr.substr(dotPosition + 1);
        }
        std::size_t maxExtSize = (size_t)3;
        std::size_t extSize = extension.size();
        if(extSize < maxExtSize) {
            fileName.append(maxExtSize - extSize, ' ');
        }

        memcpy(this->AmsDosName, fileName.c_str(), maxNameSize);
        memcpy(this->AmsDosName + maxNameSize, extension.c_str(), maxExtSize);        
    }
};

#endif