#include "utils.h"

#include <fstream>
#include "BGZF.h"

using namespace std;

void Tokenize(const string & str, vector<string> & tokens, const string & delimiters){
  string::size_type firstPos = 0;
  string::size_type lastPos = str.find_first_of(delimiters, firstPos);
  while(string::npos != lastPos){
    tokens.push_back(str.substr(firstPos, lastPos - firstPos));
    firstPos = lastPos + 1;
    lastPos = str.find_first_of(delimiters, firstPos);
  }
  tokens.push_back(str.substr(firstPos, lastPos - firstPos));
}

bool isBAM(const string & filename){
  BamTools::BgzfData bgzf;
  bgzf.Open(filename, "rb");
  if(!bgzf.IsOpen) return false;

  char buffer[5];
  buffer[4] = 0;
  if(bgzf.Read(buffer, 4) == -1) return false;
  bgzf.Close();
  return !strcmp(buffer, "BAM\1");
}

bool isSAM(const string & filename){
  ifstream filein(filename.c_str());
  if(!filein.is_open()) return false;

  string line;
  while(getline(filein, line)){
    string tag = line.substr(0, 3);
    if(tag != "@HD" && tag != "@SQ" && tag != "@RG" && tag != "@PG" && tag != "@CO") break;
  }

  if(!filein.good()){
    filein.close();
    return false;
  }

  filein.close();
  vector<string> fields;
  Tokenize(line, fields, "\t");
  if(fields.size() < 11){
    return false;
  }
  if(fields.at(10) == "*" || fields.at(10).size() == fields.at(9).size()){
    return true;
  }
  return false;
}

int64_t getFileSize(const std::string & filename){
  ifstream filein(filename.c_str());
  int64_t begin = filein.tellg();
  filein.seekg(0, ios::end);
  int64_t end = filein.tellg();
  filein.close();
  return (end-begin);
}

std::string getFileSizeString(const std::string & filename){
  //int64_t siz = getFileSize(filename);
  return "";
  
}
