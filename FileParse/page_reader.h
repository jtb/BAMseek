#ifndef PAGE_READER_H_
#define PAGE_READER_H_

#include <string>
#include <vector>
#include <memory>

#include <QApplication>
#include <QProgressDialog>

#include "parser_base.h"

static const int PAGE_SIZE = 1000;

class PageIndexer {
 public:
 PageIndexer(const std::string & filename, QProgressDialog * progress) : invalid(true) {
    std::auto_ptr<Parse> parser(ParseFactory::NewParse(filename));
    if(!parser.get()){
      invalid = true;
      return;
    }
    
    int64_t offset;
    int count = 0;
    
    while(parser->getNextRecordIndex(offset)){
                
      if(!count){
	pages.push_back(offset);
	progress->setValue(100*(parser->getProgress()));
	
	qApp->processEvents();
	if(progress->wasCanceled()){
	  progress->setValue(100);
	  invalid = false;
	  return;
	}
      }
      count++;
      if(count == PAGE_SIZE) count = 0;
    }
    progress->setValue(100);
    invalid = false;
  }
  
  ~PageIndexer(){}

  int64_t pageToOffset(int64_t page_no) const;
  
  int64_t numPages() const {
    return pages.size();
  }


  bool invalid;

 private:
  
  std::vector<int64_t> pages;
};

class PageReader {
 public:
 PageReader(const std::string & filename, QProgressDialog * progress) : invalid(true), index(filename, progress), line_count(0), parser(0) {
    parser.reset(ParseFactory::NewParse(filename));
    invalid = index.invalid;
  }
  ~PageReader(){}

  int64_t getNumPages() const;
  bool jumpToPage(int64_t page_no);
  
  std::string getHeader() {
    if(!parser.get()) return "";
    return parser->getHeader();
  }

  std::string getFilename() {
    if(!parser.get()) return "";
    return parser->getFilename();
  }

  bool getNextRecord(std::vector<std::string> & fields);
  bool invalid;

 private:
  PageIndexer index;
  int line_count;
  std::auto_ptr<Parse> parser;
};


#endif
