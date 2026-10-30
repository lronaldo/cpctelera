#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include <algorithm>
#include <cctype>
#include <vector>
#include <string>

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
      replace_if(result.begin(), result.end(), [](unsigned char c) { return !isalnum(c); }, '_');      
    }
    return result;
  };

};

#endif