#include "page_reader.h"

using namespace std;

int64_t PageIndexer::pageToOffset(int64_t page_no) const{
  if(numPages() < 1) return -1;
  if(page_no > numPages()) page_no = numPages();

  return pages.at(page_no - 1);
}

int64_t PageReader::getNumPages() const {
  if(index.invalid) return 0;
  return index.numPages();
}

bool PageReader::jumpToPage(int64_t page_no){
  line_count = 0;
  if(!parser.get()) return false;
  return parser->seek(index.pageToOffset(page_no));
}

bool PageReader::getNextRecord(vector<string> & fields){
  if(parser.get() && line_count < PAGE_SIZE){
    line_count++;
    return parser->getNextRecord(fields);
  }
  
  return false;
}
