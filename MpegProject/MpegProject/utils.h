#include "define.h"

extern unsigned char *mpeg_data;
extern int idx;

unsigned long int res = 0;
unsigned int resNum = 0;

void fillNBit(int nbit)
{
  while ((int)resNum<nbit){
    res <<= 8;
    res |= mpeg_data[idx++];
    resNum += 8;
  }
}

unsigned int getNBit(int nbit){
  fillNBit(nbit);
  unsigned int result = res >> (resNum - nbit);
  resNum -= nbit;
  res &= (1U << resNum) - 1;
  return result;
}

unsigned int lookNBit(int nbit){
  fillNBit(nbit);
  int result = res >> (resNum - (nbit));
  return result;
}

void skipNBit(int nbit){
  fillNBit(nbit);
  resNum -= nbit;
  res &= (1U << resNum) - 1;
}

unsigned int nextbits(int nbit){
  return lookNBit(nbit);
}

void next_start_code(){
  unsigned int check = getNBit(resNum % 8);
  if (check != 0)
    printf("[Next Start Code]: skip none zero bit\n");
  while (nextbits(24) != start_code_begin){
    check = getNBit(8);
    if (check!=0)
      printf("[Next Start Code]: skip none zero byte\n");
  }
}