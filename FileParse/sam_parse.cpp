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

SamParse::SamParse(const string & filename) : Parse(filename), filein(filename.c_str()), buffer(0), buffer_size(0), buffer_pos(0), file_pos(0) {
  filesize = getFileSize(filename);
  if(filein.is_open()){
    parseHeader();
    file_pos = filein.tellg();
  }
}

bool SamParse::seek(int64_t offset){
  filein.clear();
  filein.seekg(offset, ios::beg);
  return filein.good();
}

bool SamParse::getNextRecordIndex(int64_t & offset){
  if(!buffer.size()) buffer.resize(1024*1024);
  
  offset = file_pos + buffer_pos;

  while(1){
    if(buffer_size == 0 || buffer_pos >= buffer_size){
      file_pos = filein.tellg();
      filein.read( &buffer[0], buffer.size());
      buffer_size = filein.gcount();
      buffer_pos = 0;
    }
    if(!buffer_size) return false;
    if(buffer[buffer_pos] == '\n'){
      buffer_pos++;
      return true;
    }
    buffer_pos++;
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
