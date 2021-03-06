#ifndef SAM_PARSE_H_
#define SAM_PARSE_H_

#include <fstream>
#include "parser_base.h"

class SamParse: public Parse {
 public:
  SamParse(const std::string & filename);

  ~SamParse(){ filein.close(); }

  bool seek(int64_t offset);
  bool getNextRecordIndex(int64_t & offset);
  bool getNextRecord(std::vector<std::string> & fields);

  double getProgress() {
    return (double)filein.tellg()/(double)filesize;
  }
  
 private:
  void parseHeader();

  int64_t filesize;
  std::ifstream filein;

  std::vector<char> buffer;
  int buffer_size;
  int buffer_pos;
  int64_t file_pos;
};

#endif
