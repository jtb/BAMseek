#ifndef BAM_PARSE_H_
#define BAM_PARSE_H_

#include <map>

#include "parser_base.h"
#include "BGZF.h"

class BamParse : public Parse {
 public:
  BamParse(const std::string & filename);
  
  ~BamParse(){ bgzf.Close(); }
  
  bool seek(int64_t offset);
  bool getNextRecordIndex(int64_t & offset);
  bool getNextRecord(std::vector<std::string> & fields);

  double getProgress() {
    return (double)bgzf.BlockAddress/(double)filesize;
  }

 private:
  bool isBAM();
  void parseHeader();
  void getReferences();

  int64_t filesize;
  BamTools::BgzfData bgzf;
  std::map<int, std::string> refmap;
  std::vector<std::string> sequences;
};

#endif
