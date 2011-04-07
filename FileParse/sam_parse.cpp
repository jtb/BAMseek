#include "sam_parse.h"
#include "utils.h"

using namespace std;

void SamParse::parseHeader(){
  string line;
  header = "";
  int64_t pos = filein.tellg();
  while(getline(filein, line)){
    string tag = line.substr(0, 3);
    if(tag == "@HD" || tag == "@SQ" || tag == "@RG" || tag == "@PG" || tag == "@CO"){
      header += line;
      header += "\n";
      pos = filein.tellg();
    }else{
      break;
    }
  }
  if(filein.good()){
    filein.seekg(pos, ios::beg);
  }
}

SamParse::SamParse(const string & filename) : Parse(filename), filein(filename.c_str()) {
  if(filein.is_open()){
    parseHeader();
  }
}

bool SamParse::seek(int64_t offset){
  filein.clear();
  filein.seekg(offset, ios::beg);
  return filein.good();
}

bool SamParse::getNextRecordIndex(int64_t & offset){
  offset = filein.tellg();
  string line;
  if(getline(filein, line)){
    return true;
  }
  return false;
}

bool SamParse::getNextRecord(std::vector<std::string> & fields){
  fields.clear();
  string line;
  if(getline(filein, line)){
    Tokenize(line, fields, "\t");
    return true;
  }
  return false;
}
