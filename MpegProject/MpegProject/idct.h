#include <math.h>

#define  C1 0.9808
#define  C2 0.9239
#define  C3 0.8315
#define  C4 0.7071
#define  C5 0.5556
#define  C6 0.3827
#define  C7 0.1951

void idctrow(int *blk){
  double tmp[16];

  // first step
  tmp[0] = blk[0] * C4 + blk[2] * C2;
  tmp[1] = blk[4] * C4 + blk[6] * C6;
  tmp[2] = blk[0] * C4 + blk[2] * C6;
  tmp[3] = -blk[4] * C4 - blk[6] * C2;
  tmp[4] = blk[0] * C4 - blk[2] * C6;
  tmp[5] = -blk[4] * C4 + blk[6] * C2;
  tmp[6] = blk[0] * C4 - blk[2] * C2;
  tmp[7] = blk[4] * C4 - blk[6] * C6;

  tmp[8] = blk[1] * C7 - blk[3] * C5;
  tmp[9] = blk[5] * C3 - blk[7] * C1;
  tmp[10] = blk[1] * C5 - blk[3] * C1;
  tmp[11] = blk[5] * C7 + blk[7] * C3;
  tmp[12] = blk[1] * C3 - blk[3] * C7;
  tmp[13] = -blk[5] * C1 - blk[7] * C5;
  tmp[14] = blk[1] * C1 + blk[3] * C3;
  tmp[15] = blk[5] * C5 + blk[7] * C7;

  // second step
  tmp[0] = 0.5*(tmp[0] + tmp[1]);
  tmp[1] = 0.5*(tmp[2] + tmp[3]);
  tmp[2] = 0.5*(tmp[4] + tmp[5]);
  tmp[3] = 0.5*(tmp[6] + tmp[7]);
  tmp[4] = 0.5*(tmp[8] + tmp[9]);
  tmp[5] = 0.5*(tmp[10] + tmp[11]);
  tmp[6] = 0.5*(tmp[12] + tmp[13]);
  tmp[7] = 0.5*(tmp[14] + tmp[15]);

  // third step
  blk[0] = tmp[0] + tmp[7];
  blk[1] = tmp[1] + tmp[6];
  blk[2] = tmp[2] + tmp[5];
  blk[3] = tmp[3] + tmp[4];
  blk[4] = tmp[3] - tmp[4];
  blk[5] = tmp[2] - tmp[5];
  blk[6] = tmp[1] - tmp[6];
  blk[7] = tmp[0] - tmp[7];

}

void idctcol(int *blk){
  double tmp[16];

  // first step
  tmp[0] = blk[0 * 8] * C4 + blk[2 * 8] * C2;
  tmp[1] = blk[4 * 8] * C4 + blk[6 * 8] * C6;
  tmp[2] = blk[0 * 8] * C4 + blk[2 * 8] * C6;
  tmp[3] = -blk[4 * 8] * C4 - blk[6 * 8] * C2;
  tmp[4] = blk[0 * 8] * C4 - blk[2 * 8] * C6;
  tmp[5] = -blk[4 * 8] * C4 + blk[6 * 8] * C2;
  tmp[6] = blk[0 * 8] * C4 - blk[2 * 8] * C2;
  tmp[7] = blk[4 * 8] * C4 - blk[6 * 8] * C6;

  tmp[8] = blk[1 * 8] * C7 - blk[3 * 8] * C5;
  tmp[9] = blk[5 * 8] * C3 - blk[7 * 8] * C1;
  tmp[10] = blk[1 * 8] * C5 - blk[3 * 8] * C1;
  tmp[11] = blk[5 * 8] * C7 + blk[7 * 8] * C3;
  tmp[12] = blk[1 * 8] * C3 - blk[3 * 8] * C7;
  tmp[13] = -blk[5 * 8] * C1 - blk[7 * 8] * C5;
  tmp[14] = blk[1 * 8] * C1 + blk[3 * 8] * C3;
  tmp[15] = blk[5 * 8] * C5 + blk[7 * 8] * C7;

  // second step
  tmp[0] = 0.5*(tmp[0] + tmp[1]);
  tmp[1] = 0.5*(tmp[2] + tmp[3]);
  tmp[2] = 0.5*(tmp[4] + tmp[5]);
  tmp[3] = 0.5*(tmp[6] + tmp[7]);
  tmp[4] = 0.5*(tmp[8] + tmp[9]);
  tmp[5] = 0.5*(tmp[10] + tmp[11]);
  tmp[6] = 0.5*(tmp[12] + tmp[13]);
  tmp[7] = 0.5*(tmp[14] + tmp[15]);

  // third step
  blk[0 * 8] = tmp[0] + tmp[7];
  blk[1 * 8] = tmp[1] + tmp[6];
  blk[2 * 8] = tmp[2] + tmp[5];
  blk[3 * 8] = tmp[3] + tmp[4];
  blk[4 * 8] = tmp[3] - tmp[4];
  blk[5 * 8] = tmp[2] - tmp[5];
  blk[6 * 8] = tmp[1] - tmp[6];
  blk[7 * 8] = tmp[0] - tmp[7];
}


void idct(int *dctblock){
  int i;
  for (i = 0; i < 8; i++) idctrow(dctblock + 8 * i);
  for (i = 0; i < 8; i++) idctcol(dctblock + i);
}
