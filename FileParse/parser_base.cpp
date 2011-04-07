#include "parser_base.h"
#include "bam_parse.h"
#include "sam_parse.h"
#include "utils.h"

Parse * ParseFactory::NewParse(const std::string & filename){
  if(isBAM(filename)){
    return new BamParse(filename);
  }
  if(isSAM(filename)){
    return new SamParse(filename);
  }
  return NULL;
}
