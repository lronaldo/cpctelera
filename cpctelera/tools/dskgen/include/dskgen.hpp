#ifndef _DSKGEN_H_
#define _DSKGEN_H_

#include <iostream>
#include <string>
#include <json/json.h>
#include "ezOptionParser.hpp"
#include "Options.hpp"
#include "Types.hpp"
#include "Dsk.hpp"
#include "FileToProcess.hpp"

using namespace std;
using namespace ez;

int initializeParser(ezOptionParser &parser);
void showUsage(ezOptionParser &options);

int extractOptions(ezOptionParser &switches, Options &options);

int addFile(FileToProcess &fileOptions, Dsk &disk);

int main(int argc, const char** argv);

#endif