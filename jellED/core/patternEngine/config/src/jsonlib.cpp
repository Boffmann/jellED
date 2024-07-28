#include "jsonlib.h"

// remove all white space from the json string... preserving strings
std::string jsonRemoveWhiteSpace(const std::string& json){
  int i = 0;
  int cursor = 0;
  int quote_count = 0;
  std::string out = "";
  char out_chars[json.length()+1];
  
  for(i=0; i<json.length(); i++){
    if(json[i] == ' ' || json[i] == '\n' || json[i] == '\t' || json[i] == '\r'){
      if(quote_count % 2){ // inside a string
	out_chars[cursor++] = json[i];
      }
      else{ // outside a string!
      }
    }
    else{
      if(json[i] == 34){ // ascii dounble quote
	//check for escaped quote
	if(i > 0 && json[i - 1] == '\\'){
	  //escaped!
	}
	else{ // not escaped
	  quote_count++;
	}
      }
      out_chars[cursor++] = json[i];
    }
  }
  out_chars[cursor] = 0;
  out = std::string(out_chars);
  return out;
}

std::string jsonIndexList(const std::string& json, int idx){
  int count = 1; // number of braces seen { = +1 } = -1
  int i = 1;
  int item_idx = 0;
  int start = i;
  int stop = json.length() - 1;
  
  while(i < json.length() && count > 0){
    if(json.at(i) == ']' or json.at(i) == '}'){
      count--;
    }
    if(json.at(i) == '{' or json.at(i) == '['){
      count++;
    }
    if(count == 1 && json.at(i) == ',' && item_idx == idx){
      //item separator!
      stop = i;
      return json.substr(start, stop);
    }
    if(count == 1 && json.at(i) == ']' && item_idx == idx){ 
	stop = i + 1;
	return json.substr(start, stop);
    }
    if(count == 1 && json.at(i) == ','){
      item_idx++;
      start = i + 1;
    }
    i++;
  }
  return json.substr(start, stop);
}

// return a sub-json struct
std::string jsonExtract(const std::string& json, const std::string& nameArg){
  char next;
  int start = 0, stop = 0;
  static const size_t npos = -1;
  const std::string QUOTE = "\"";
  
  std::string name = QUOTE + nameArg + QUOTE;
  if (json.find(name) == npos) return json.substr(0,0);
  start = json.find(name) + name.length() + 1;
  next = json.at(start);
  
  while(start < json.length() && next == ' ') { // filters blanks before value if there
    start++;
    next = json.at(start);
  }
  
  if(next == '\"'){
    //Serial.println(".. a string");
    start = start + 1;
    stop = json.find('"', start);
  }
  else if(next == '['){
    //Serial.println(".. a list");
    int count = 1;
    int i = start;
    while(count > 0 && i++ < json.length()){
      if(json.at(i) == ']'){
	count--;
      }
      else if(json.at(i) == '['){
	count++;
      }
    }
    stop = i + 1;
  }
  else if(next == '{'){
    //Serial.println(".. a struct");
    int count = 1;
    int i = start;
    while(count > 0 && i++ < json.length()){
      if(json.at(i) == '}'){
	count--;
      }
      else if(json.at(i) == '{'){
	count++;
      }
    }
    stop = i + 1;
  }
  else if(next == 't'){
	//Serial.println("... a boolean true");
	int i = start;
	
	while(i++ < json.length()){
		if(json.at(i) == 't' && json.at(i + 1) == 'r' && json.at(i + 2) == 'u' && json.at(i + 3) == 'e')
		{
			stop = i + 3;
			break;
		}
		else if (json.at(i) == ',' || json.at(i) == '}' || json.at(i) == ']' || json.at(i) == ']') {
			stop = i;
			break;
		}
	}
  } 
  else if(next == 'f' ){
	//Serial.println("... a boolean false");
    int i = start;
	
	if(json.at(i) == 'f' && json.at(i + 1) == 'a' && json.at(i + 2) == 'l' && json.at(i + 3) == 's' && json.at(i + 4) == 'e')
	{
		stop = i + 4;
	}
	else {
		stop = i;
	}	
  } 
  else if(next == 'n' ){
	//Serial.println("... a null");
    int i = start;
	if(json.at(i) == 'n' && json.at(i + 1) == 'u' && json.at(i + 2) == 'l' && json.at(i + 3) == 'l')
	{
		stop = i + 3;
	}
	else {
		stop = i;
	}
  }  
  else if(next == '.' || next == '-' || ('0' <= next  && next <= '9')){
    //Serial.println(".. a number");
    int i = start;
    while(i++ < json.length() && (json.at(i) == '.' || ('0' <= json.at(i)  && json.at(i) <= '9'))){
    }
    stop = i;
  }
  return json.substr(start, stop);
}
