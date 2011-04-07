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

  double getProgress() const {
    return 0;
  }
  
 private:
  void parseHeader();
  std::ifstream filein;
};

#endif
