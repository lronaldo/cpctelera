#include "dskgen.hpp"

int initializeParser(ezOptionParser &parser) {
    parser.overview = "dskgen - (c) 2008-2015 Retroworks.";
    parser.syntax = "dskgen [OPTIONS] fileName";
    parser.example = "dskgen -o disk.dsk -c RAW -t SYSTEM -h NONE --files fileSpec1[;fileSpec2;fileSpecN]\ndskgen -o disk.dsk --config cfg.json\n\n";
    parser.footer = "If you liked this program, drop an email at: augusto.ruiz@gmail.com\n";

    parser.add("disk.dsk", false, 1, 0, "Output file name. Default value is disk.dsk.", "-o", "--outputFileName");
    ezOptionValidator* vCat = new ezOptionValidator("t", "in", "NONE,RAW,CPM", true);
    parser.add("NONE", false, 1, 0, "Catalog type. Valid values are: NONE, RAW or CPM.\n\nNONE: No catalog is added. This can be used if sectors will be read in raw mode, or if the first file to add contains CATART data.\n\nRAW: A catalog is created in RAW mode. This catalog is not understood by the OS, see the docs.\n\nCPM: Standard CPM catalog, understood by AMSDOS.", "-c", "--catalog", vCat);
    ezOptionValidator* vDskType = new ezOptionValidator("t", "in", "SYSTEM,DATA,IBM,CUSTOM", true);
    parser.add("SYSTEM", false, 1, 0, "Disk type. Valid values are SYSTEM, DATA, IBM and CUSTOM.", "-t", "--type", vDskType);
    parser.add("", true, 1, ';', "Specifies the files to insert. File Specified as path,header,loadAddr,exeAddr. Header values can be NONE or AMSDOS. Default header value is NONE, default addresses are 0x0.", "-f", "--files");
    parser.add("2", false, 1, 0, "Specifies the sides of the disk. Default value is 2 sides.", "-s", "--sides");
    parser.add("", false, 1, 0, "Specifies the file that contains boot code por |CPM booting. Only valid for SYSTEM or CUSTOM files.", "-b");
    parser.add("80", false, 1, 0, "Specifies the number of tracks in the disk. Default value is 80.", "--tracks");
    parser.add("9", false, 1, 0, "Specifies the number of sectors per track in the disk. Default value is 9. Only valid for CUSTOM files.", "--sectors");
    parser.add("0xC1", false, 1, 0, "Specifies the initial sector per track. Default value is 0xC1. Only valid for CUSTOM files.", "--initialSector");
    parser.add("", false, 1, 0, "Specifies the initial track for data (except boot file). Default value is 0. Only valid for CUSTOM files.", "--initialTrack");
    parser.add("", false, 1, 0, "Configuration file with all options and files. This is the option to use if some files need headers and others don't, if some files need to be readonly...", "--config");
    parser.add("", false, 0, 0, "Help. Show usage.", "--help");

    return 0;
}

void showUsage(ezOptionParser &options) {
    string usage;
    options.getUsage(usage);
    cout << usage << endl;
}

int extractOptions(ezOptionParser &switches, Options &options) {
    switches.get("-o")->getString(options.OutputFileName);
    if(switches.isSet("--config")) {
        string configFile;
        switches.get("--config")->getString(configFile);
        options.ParseFile(configFile);
    } else {
        string tmpStr;
        switches.get("-c")->getString(tmpStr);
        options.SetCatalogType(tmpStr);
        switches.get("-t")->getString(tmpStr);
        options.SetDiskType(tmpStr);

        int sides;
        switches.get("-s")->getInt(sides);
        options.NumSides = (u8)sides;

        if(switches.isSet("--initialTrack")) {
            if(options.OutputDiskType != DSK_CUSTOM) {
                cout << "WARNING: Custom initial track specification is only allowed for CUSTOM files. This setting will be ignored." << endl;                
            } else {
                int initialTrack;
                switches.get("--initialTrack")->getInt(initialTrack);
                options.DiskParams.reservedTracks = (u16) initialTrack;
            }
        }

        if(switches.isSet("-b")) {
            if(options.DiskParams.reservedTracks == 0) {
                cout << "WARNING: Boot files are only allowed for disks with reserved tracks. This setting will be ignored." << endl;
            } else {
                switches.get("-b")->getString(options.BootFile);
            }
        }

        if(switches.isSet("--tracks")) {
            if(options.OutputDiskType != DSK_CUSTOM) {
                    cout << "WARNING: Custom track number specification is only allowed for CUSTOM files. This setting will be ignored." << endl;
            } else {
                int tracks = 0;
                switches.get("--tracks")->getInt(tracks);
                int blockSize = DSK_RECORD_SIZE * (1 << options.DiskParams.blockShift);
                int sectorSize = options.DiskParams.sectSizeInRecords * DSK_RECORD_SIZE;
                int sectorsPerBlock = (blockSize / sectorSize);
                options.DiskParams.numBlocks = (u8) (((tracks - options.DiskParams.reservedTracks) * options.DiskParams.sectorsPerTrack) / sectorsPerBlock) - 1;
            }
        }        
        if(switches.isSet("--sectors")) {
            if(options.OutputDiskType != DSK_CUSTOM) {
                cout << "WARNING: Custom sector number specification is only allowed for CUSTOM files. This setting will be ignored." << endl;                
            } else {
                int sectorsPerTrack;
                int blockSize = DSK_RECORD_SIZE * (1 << options.DiskParams.blockShift);
                int sectorSize = options.DiskParams.sectSizeInRecords * DSK_RECORD_SIZE;
                int sectorsPerBlock = (blockSize / sectorSize);

                int tracks = (((options.DiskParams.numBlocks + 1) * sectorsPerBlock) / options.DiskParams.sectorsPerTrack) + options.DiskParams.reservedTracks;
                switches.get("--sectors")->getInt(sectorsPerTrack);
                options.DiskParams.sectorsPerTrack = (u8) options.DiskParams.sectorsPerTrack;
                options.DiskParams.numBlocks = (u8) (((tracks - options.DiskParams.reservedTracks) * options.DiskParams.sectorsPerTrack) / sectorsPerBlock) - 1;
            }
        }
        if(switches.isSet("--initialSector")) {
            if(options.OutputDiskType != DSK_CUSTOM) {
                cout << "WARNING: Custom initial sector specification is only allowed for CUSTOM files. This setting will be ignored." << endl;                
            } else {
                int initalSector;
                switches.get("--initialSector")->getInt(initalSector);
                options.DiskParams.firstSectorNumber = (u8)initalSector;
            }
        }

        if (switches.isSet("-f")) {
            vector<string> fileSpecs;
            switches.get("-f")->getStrings(fileSpecs);
            for (vector<string>::iterator fileIter = fileSpecs.begin(); fileIter != fileSpecs.end(); ++fileIter) {
                vector<string> fileSpecParts;

                // Extract the file spec.
                stringstream ss(*fileIter); // Turn the string into a stream.
                string token;
                while (getline(ss, token, ',')) {
                    fileSpecParts.push_back(token);
                }

                // Add the file to process.
                FileToProcess f;
                f.SetSourcePath(fileSpecParts[0]);
                f.Header = fileSpecParts.size() > 1 ? ParseHeaderType(fileSpecParts[1]) : HDR_NONE;
                f.AmsdosType = fileSpecParts.size() > 2 ? ParseAmsdosFileType(fileSpecParts[2]) : AMSDOS_FILE_NONE;
                f.LoadAddress = (u16)fileSpecParts.size() > 3 ? strtoul(fileSpecParts[3].c_str(), nullptr, 0) : 0;
                f.ExecutionAddress = (u16)fileSpecParts.size() > 4 ? strtoul(fileSpecParts[4].c_str(), nullptr, 0) : 0;
                options.AddFile(f);
            }
        }
    }

    return 0;
}

int main(int argc, const char** argv)
{
    ezOptionParser switches;
    if (initializeParser(switches)) {
        return -1;
    }

    switches.parse(argc, argv);

    if (switches.isSet("--help")) {
        showUsage(switches);
        return 0;
    }

    Options opt;
    if (extractOptions(switches, opt)) {
        cout << "ERROR: No files specified." << endl;
        showUsage(switches);
        return -1;
    }
 
    Dsk disk(opt.NumSides, opt.DiskParams, opt.Catalog);
    if(!opt.BootFile.empty()) {
        if(disk.AddBootFile(opt.BootFile)) {
            cout << "ERROR: Couldn't process file '" << opt.BootFile << "'." << endl;            
        }
    }

    for(vector<FileToProcess>::iterator fileIter = opt.FilesToProcess.begin(); fileIter!=opt.FilesToProcess.end(); ++fileIter) {
        if(disk.AddFile(*fileIter)) {
            cout << "ERROR: Couldn't process file '" << fileIter->SourcePath << "'." << endl;
        }
    }
    disk.Save(opt.OutputFileName);

    return 0;
}
