#include "Types.hpp"

CatalogType ParseCatalogType(const string &catStr) {
    CatalogType result = CAT_NONE;
    string tmpStr(catStr);
    transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
    if(tmpStr == "none") {
        result = CAT_NONE;
    } else if(tmpStr == "raw") {
        result = CAT_RAW;    
    } else if(tmpStr == "cpm") {
        result = CAT_CPM;
    } else {
        stringstream ss;
        ss << "Catalog type not recognized: " << catStr << ".\nValid values are: CPM, RAW or NONE.\n";
        throw ss.str();
    }
    return result;
}

DiskType ParseDiskType(const string &diskStr) {
    DiskType result = DSK_SYSTEM;
    string tmpStr(diskStr);
    transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
    if(tmpStr == "system") {
        result = DSK_SYSTEM;
    }
    else if(tmpStr == "data") {
        result = DSK_DATA;
    } else if(tmpStr == "ibm") {
        result = DSK_IBM;
    } else if(tmpStr == "custom") {
        result = DSK_CUSTOM;
    } else {
        stringstream ss;
        ss << "Disk type not recognized: " << diskStr << ".\nValid values are: SYSTEM, DATA, IBM or CUSTOM.\n";
        throw ss.str();
    }
    return result;
}

HeaderType ParseHeaderType(const string &hdrStr) {
    HeaderType result = HDR_NONE;
    string tmpStr(hdrStr);
    transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
    if(tmpStr == "none") {
        result = HDR_NONE;
    } else if(tmpStr == "amsdos") {
        result = HDR_AMSDOS;
    } else {
        stringstream ss;
        ss << "Header type not recognized: " << hdrStr << "\n.Valid values are: AMSDOS or NONE.\n";
        throw ss.str();
    }
    return result;
}

AmsdosFileType ParseAmsdosFileType(const string &fileTypeStr) {
    AmsdosFileType result = AMSDOS_FILE_NONE;
    string tmpStr(fileTypeStr);
    transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), ::tolower);
    if (tmpStr == "bas") {
        result = AMSDOS_FILE_INTERNAL_BASIC;
    }
    else if (tmpStr == "bin") {
        result = AMSDOS_FILE_BINARY;
    }
    else if (tmpStr == "binp") {
        result = AMSDOS_FILE_BINARY_PROTECTED;
    }
    else if (tmpStr == "scr") {
        result = AMSDOS_FILE_SCREEN_IMAGE;
    }
    else if (tmpStr == "asc") {
        result = AMSDOS_FILE_ASCII;
    }
    else if (tmpStr == "") {
        result = AMSDOS_FILE_NONE;
    }
    else {
        stringstream ss;
        ss << "File type not recognized: " << fileTypeStr << ".\nValid values are: BAS, BIN, SCR, ASC, or empty string.\n";
        throw ss.str();
    }
    return result;
}
