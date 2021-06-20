#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include <vector>
#include <string>
#include <FreeImage.h>

using namespace std;

class FileUtils {
public:
  static string GetFileName(const string &fileName) {
    string result(fileName);
    size_t slashPos = result.rfind("/");
    if(slashPos != string::npos) {
      // remove until slash.
      result = result.substr(slashPos + 1, string::npos);
    }
    slashPos = result.rfind("\\");

    if(slashPos != string::npos) {
      result = result.substr(slashPos + 1, string::npos);
    }
    return result;
  };

  static string RemoveExtension(const string &fileName) {
    string result(fileName);
    result = GetFileName(result);

    size_t dotPos = result.rfind(".");
    if(dotPos!=0 && dotPos!=string::npos) {
      result = result.substr(0, dotPos);
    }
    return result;
  };

  static string Sanitize(const string &name) {
    string result(name);
    if(!name.empty()) {
      replace_if(result.begin(), result.end(), [](const char c) { return !isalnum(c); }, '_');      
    }
    return result;
  };

  static FIBITMAP* LoadImage(const string &fileName) {
    FIBITMAP *result = NULL;
    const char* lpszFileName = fileName.c_str();
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(lpszFileName, 0);
    if(fif == FIF_UNKNOWN) {
      fif = FreeImage_GetFIFFromFilename(lpszFileName);
    }
    if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
      FIBITMAP *dib = FreeImage_Load(fif, lpszFileName, 0);
      result = FreeImage_ConvertTo32Bits(dib);
      FreeImage_Unload(dib);
    }
    return result;
  };

  static void SaveImage(const string &fileName, int tileWidth, int tileHeight, unsigned char *buffer) {
    FIBITMAP *dib = FreeImage_ConvertFromRawBits(buffer, tileWidth, tileHeight, 3 * tileWidth, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
    FreeImage_Save(FIF_PNG, dib, fileName.c_str());
    FreeImage_Unload(dib);
  }

};

#endif