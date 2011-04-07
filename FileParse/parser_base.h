#ifndef PARSER_BASE_H_
#define PARSER_BASE_H_

#include <string>
#include <vector>

class Parse {

 public:
 Parse(const std::string & filename) : filename(filename), header("") {}

  virtual ~Parse(){}

  virtual bool seek(int64_t offset) = 0;
  virtual bool getNextRecordIndex(int64_t & offset) = 0;
  virtual bool getNextRecord(std::vector<std::string> & fields) = 0;

  virtual double getProgress() const = 0;

  virtual std::string getHeader() const { return header; }
  virtual std::string getFilename() const { return filename; }

 protected:
  const std::string filename;
  std::string header;
  
};

class ParseFactory {
 public:
  static Parse * NewParse(const std::string & filename);
};


#endif
