#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>

void Tokenize(const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters);

bool isBAM(const std::string & filename);
bool isSAM(const std::string & filename);

#endif
