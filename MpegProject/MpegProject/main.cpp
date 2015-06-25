#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>

#include "TableHandler.h"
#include "header.h"

unsigned char *mpeg_data;
unsigned int len;
int idx = 0;

void readFileBytes(const char *name){
  FILE *fp=NULL;
  fp = fopen(name, "rb");
  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  rewind(fp);
  char *ret = new char[len];
  fread(ret, len, 1, fp);
  mpeg_data = (unsigned char*)ret;
}

int main(int argc, char **argv){

  if (argc != 2){
    printf("<usage>./MpegProject <inputfile>");
    return -1;
  }

  cout << "Input: " << argv[1] << endl;

  TableHandler::initialize();
  //argv[0] = "C:\\Users\\leisure\\Desktop\\phw_mpeg\\IPB_ALL.M1V";
  //argv[0] = "C:\\Users\\leisure\\Desktop\\phw_mpeg\\IP_ONLY.M1V";
  argv[0] = "C:\\Users\\leisure\\Desktop\\phw_mpeg\\I_ONLY.M1V";

  readFileBytes(argv[1]);
  video_sequence();
  play();

  return 0;
}