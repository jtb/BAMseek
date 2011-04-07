#include <sstream>
#include "bam_parse.h"
#include "BGZF.h"
#include <assert.h>

using namespace std;
using namespace BamTools;

namespace {
  char getBase(int val){
    if(val <= 0) return '=';
    if(val <= 1) return 'A';
    if(val <= 2) return 'C';
    if(val <= 4) return 'G';
    if(val <= 8) return 'T';
    return 'N';
  }

  char getOp(int val){
    switch(val){
    case 0 : return 'M';
    case 1 : return 'I';
    case 2 : return 'D';
    case 3 : return 'N';
    case 4 : return 'S';
    case 5 : return 'H';
    case 6 : return 'P';
    case 7 : return '=';
    }
    return 'X';
  }

  template <class T>
  string toString(T val){
    stringstream ss (stringstream::out);
    ss << val;
    return ss.str();
  }

  float getFloat(char * buffer, BgzfData & bgzf, int & block_size){
    bgzf.Read(buffer, 4);
    block_size -= 4;
    return BgzfData::UnpackFloat(buffer);
  }

  int getShort(char * buffer, BgzfData & bgzf, int & block_size){
    bgzf.Read(buffer, 2);
    block_size -= 2;
    return (int)BgzfData::UnpackSignedShort(buffer);
  }

  int getInt32(char * buffer, BgzfData & bgzf, int & block_size){
    bgzf.Read(buffer, 4);
    block_size -= 4;
    return BgzfData::UnpackSignedInt(buffer);
  }
  int getInt32(char * buffer, BgzfData & bgzf){
    int block_size = 0;
    return (unsigned int)getInt32(buffer, bgzf, block_size);
  }

  unsigned int getUshort(char * buffer, BgzfData & bgzf, int & block_size){
    bgzf.Read(buffer, 2);
    block_size -= 2;
    return BgzfData::UnpackUnsignedShort(buffer);
  }

  unsigned int getUint32(char * buffer, BgzfData & bgzf, int & block_size){
    bgzf.Read(buffer, 4);
    block_size -= 4;
    return BgzfData::UnpackUnsignedInt(buffer);
  }
  int getUint32(char * buffer, BgzfData & bgzf){
    int block_size = 0;
    return getUint32(buffer, bgzf, block_size);
  }

  string getNullTermString(BgzfData & bgzf, int & block_size){
    char buffer;
    string ans = "";
    while(bgzf.Read(&buffer, 1)){
      block_size--;
      if(buffer == 0){
	break;
      }
      ans += buffer;
    }
    return ans;
  }

  string getHexString(BgzfData & bgzf, int & block_size){
    char buffer;
    string ans = "";
    while(bgzf.Read(&buffer, 1)){
      block_size--;
      if((((buffer) >> 4) & 0x0F) == 0){
	break;
      }
      if(((buffer) & 0x0F) == 0){
	break;
      }
      ans += buffer;
    }
    return ans;
  }

  string getVal(BgzfData & bgzf, int & block_size, char val_type){
    string ans = "";
    switch(val_type){
    case 'A': {
      char buffer;
      bgzf.Read(&buffer, 1);
      block_size -= 1;
      ans += buffer;
      break;
    }
    case 'c': {
      char buffer;
      bgzf.Read(&buffer, 1);
      block_size -= 1;
      ans = toString<int>((int)buffer);
      break;
    }
    case 'C': {
      char buffer;
      bgzf.Read(&buffer, 1);
      block_size -= 1;
      ans = toString<unsigned int>((unsigned int) (unsigned char) buffer);
      break;
    }
    case 's': {
      char buffer[2];
      ans = toString<int>(getShort(buffer, bgzf, block_size));
      break;
    }
    case 'S': {
      char buffer[2];
      ans = toString<unsigned int>(getUshort(buffer, bgzf, block_size));
      break;
    }
    case 'i': {
      char buffer[4];
      ans = toString<int>(getInt32(buffer, bgzf, block_size));
      break;
    }
    case 'I': {
      char buffer[4];
      ans = toString<unsigned int>(getUint32(buffer, bgzf, block_size));
      break;
    }
    case 'f': {
      char buffer[4];
      ans = toString<float>(getFloat(buffer, bgzf, block_size));
      break;      
    }
    case 'Z': {
      ans = getNullTermString(bgzf, block_size);
      break;
    }
    case 'H': {
      ans = getHexString(bgzf, block_size);
      break;
    }
    }
    return ans;
  }
  
}

bool BamParse::isBAM(){
  char buffer[5];
  buffer[4] = 0;
  if(bgzf.Read(buffer, 4) == -1) return false;
  return !strcmp(buffer, "BAM\1");
}

void BamParse::parseHeader(){
  header = "";
  char buffer[4];

  unsigned int headerTextLength = getUint32(buffer, bgzf);
  
  char * headerText = new char[headerTextLength + 1];
  headerText[headerTextLength] = 0;
  bgzf.Read(headerText, headerTextLength);
  header = headerText;
  delete headerText;

}

void BamParse::getReferences(){
  char buffer[4];
  int n_ref = getInt32(buffer, bgzf);
  
  refmap[-1] = "*";
  for(int i = 0; i < n_ref; i++){
    int l_name = getInt32(buffer, bgzf);
    char * name = new char[l_name];
    bgzf.Read(name, l_name);
    string ref_name(name);
    delete name;
    refmap[i] = ref_name;
    bgzf.Read(buffer, 4);//length of reference sequence
  }
}

BamParse::BamParse(const string & filename) : Parse(filename) {
  bgzf.Open(filename, "rb");

  if(bgzf.IsOpen){
    if(!isBAM()){
      bgzf.Close();
    }else{
      parseHeader();
      getReferences();
    }
  }
 
}

bool BamParse::seek(int64_t offset){
  return bgzf.Seek(offset);
}

bool BamParse::getNextRecordIndex(int64_t & offset){
  offset = bgzf.Tell();

  char buffer[4];
  if(bgzf.Read(buffer, 4) != 4) return false;

  int block_size = BgzfData::UnpackSignedInt(buffer);
  char * record = new char[block_size];
  bgzf.Read(record, block_size);
  delete record;

  return true;
}

bool BamParse::getNextRecord(std::vector<std::string> & fields){
  char buffer[4];
  if(bgzf.Read(buffer, 4) != 4) return false;//block_size
  int block_size = BgzfData::UnpackSignedInt(buffer);

  int refID = getInt32(buffer, bgzf, block_size);
  string RNAME = "*";
  if(refID >= 0){
    map<int, string>::const_iterator it;
    it = refmap.find(refID);
    if(it != refmap.end()){
      RNAME = it->second;
    }
  }
  
  int POS = getInt32(buffer, bgzf, block_size) + 1;
  //fields.push_back(toString<int>(pos));

  unsigned int bin_mq_nl = getUint32(buffer, bgzf, block_size);
  int l_read_name = (bin_mq_nl & 0xFF);
  int MAPQ = (bin_mq_nl >> 8) & 0xFF;
  //fields.push_back(toString<int>(l_read_name));
  //fields.push_back(toString<int>(MAPQ));
  
  unsigned int flag_nc = getUint32(buffer, bgzf, block_size);
  int n_cigar_op = (flag_nc & 0xFFFF);
  int FLAG = (flag_nc >> 16);
  //fields.push_back(toString<int>(n_cigar_op));
  //fields.push_back(toString<int>(FLAG));

  int l_seq = getInt32(buffer, bgzf, block_size);
  
  //RNEXT, PNEXT
  int next_refID = getInt32(buffer, bgzf, block_size);
  string RNEXT = "*";
  if(next_refID >= 0){
    map<int, string>::const_iterator it;
    it = refmap.find(next_refID);
    if(it != refmap.end()){
      RNEXT = it->second;
      if(RNEXT == RNAME) RNEXT = "=";
    }
  }
  int PNEXT = getInt32(buffer, bgzf, block_size) + 1;
  int TLEN = getInt32(buffer, bgzf, block_size);

  char * record = new char[l_read_name];
  bgzf.Read(record, l_read_name);
  string QNAME = record;
  delete record;
  block_size = block_size - l_read_name;

  string CIGAR = "";
  for(int i = 0; i < n_cigar_op; i++){
    unsigned int cig = getUint32(buffer, bgzf, block_size);
    char op = getOp(cig & 0xF);
    CIGAR += toString<unsigned int>(cig >> 4);
    CIGAR += op;
  }
  if(CIGAR.empty()) CIGAR = "*";

  string SEQ = "";
  if(l_seq != 0){
    int len = (l_seq + 1)/2;
    char * seq = new char[len];
    bgzf.Read(seq, len);
    for(int i = 0; i < len; i++){
      uint8_t val = seq[i];
      SEQ += getBase(val >> 4);
      SEQ += getBase(val & 0xF);
    }
    delete seq;
    block_size = block_size - len;
  }
  if(SEQ.empty()) SEQ = "*";
  
  string QUAL = "";
  if(l_seq != 0){
    char * qual = new char[l_seq];
    bgzf.Read(qual, l_seq);
    for(int i = 0; i < l_seq; i++){
      QUAL += (char)(qual[i] + 33);
    }
    delete qual;
    block_size = block_size - l_seq;
  }
  if(QUAL.empty()) QUAL = "*";

  fields.clear();
  fields.push_back(QNAME);
  fields.push_back(toString<int>(FLAG));
  fields.push_back(RNAME);
  fields.push_back(toString<int>(POS));
  fields.push_back(toString<int>(MAPQ));
  fields.push_back(CIGAR);
  fields.push_back(RNEXT);
  fields.push_back(toString<int>(PNEXT));
  fields.push_back(toString<int>(TLEN));
  fields.push_back(SEQ);
  fields.push_back(QUAL);

  while(block_size > 0){
    string ans = "";
    char tag[2];
    bgzf.Read(tag, 2);
    ans += tag[0];
    ans += tag[1];
    ans += ':';
    block_size -= 2;
        
    char val_type;
    bgzf.Read(&val_type, 1);
    block_size -=1;
    string value =  getVal(bgzf, block_size, val_type);
    
    if(val_type != 'A' && val_type != 'f' && val_type != 'Z' && val_type != 'H'){
      val_type = 'i';
    }
    ans += val_type;
    ans += ':';
    ans += value;
        
    fields.push_back(ans);
  }

  assert(block_size == 0);
  if(block_size > 0){
    char * rest = new char[block_size];
    bgzf.Read(rest, block_size);
    delete rest;
  }
  
  return true;
}
