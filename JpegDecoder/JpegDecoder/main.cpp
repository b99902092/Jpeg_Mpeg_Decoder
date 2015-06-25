#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <string>
#include <fstream> 
#include <map>
#include "data_structure.h"

#define PI 3.1415926

using namespace std;

unsigned char *img_data;
int i;
size_t len;
int preDC[3] = {};

int maxh;
int maxv;

int realX;
int realY;

struct DQT dqt[2];
struct DHT dht[2][2];
struct SOF sof;
struct SOS sos;

int **M[3];
unsigned char *RGB;

void readFileBytes(const char *name){
  FILE *fp;
  fp = fopen(name, "rb");
  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  rewind(fp);
  char *ret = new char[len];
  fread(ret, len, 1, fp);
  img_data = (unsigned char*) ret;
}

int parseHeader(){

  //check SOI
  if (img_data[0] != 0xff || img_data[1] != 0xd8){
    fprintf(stderr, "wrong start bytes");
    return -1;
  }

  for (i = 2; i < len;){
    if (img_data[i] != 0xff){
      printf("Not header\n");
      return -1;
    }
    i++;

    //APPn
    if (img_data[i] >= 0xe0 && img_data[i] <= 0xef){
      int len_APPn = img_data[i + 1] * 256 + img_data[i + 2];
      i += len_APPn + 1;

      printf("Unsupport APPn length: %d\n", len_APPn);
    }
    //COM
    else if (img_data[i] == 0xfe)
    {
      int len_COM = img_data[i + 1] * 256 + img_data[i + 2];
      i += len_COM + 1;

      printf("Comment length: %d\n", len_COM);
    }
    //DQT
    else if (img_data[i] == 0xdb){
      int len_DQT = img_data[i + 1] * 256 + img_data[i + 2] - 2;
      i += 3;
      while (len_DQT > 0){
        int Pq = img_data[i] >> 4;
        int Tq = img_data[i] & 0x0f;
        dqt[Tq].Tq = Tq;
        dqt[Tq].Pq = Pq;

        printf("len: %d, DQT.Pq: %d, DQT.Tq: %d\n", len_DQT, dqt[Tq].Pq, dqt[Tq].Tq);

        i++;
        for (int j = 0; j < 64; j++, i++)
          dqt[Tq].Qk[zigZagOrder[j]] = img_data[i];

        for (int j = 0; j < 64; j++){
          if (j % 8 == 0)
            printf("\n");
          printf("%3d ", dqt[Tq].Qk[j]);
        }
        printf("\n");
        len_DQT -= 65;
      }
    }
    //DHT
    else if (img_data[i] == 0xc4){
      int len_DHT = img_data[i + 1] * 256 + img_data[i + 2] - 2;
      i += 3;
      while (len_DHT > 0){
        int Tc = img_data[i] >> 4;
        int Th = img_data[i] & 0x0f;
        dht[Tc][Th].Tc = Tc;
        dht[Tc][Th].Th = Th;

        i++;
        int len_Vij = 0;
        for (int j = 0; j < 16; j++, i++){
          dht[Tc][Th].Li[j] = img_data[i];
          len_Vij += img_data[i];
          if (img_data[i] >= 0)
            dht[Tc][Th].Vij[j] = new int[img_data[i]];
          else
            dht[Tc][Th].Vij[j] = NULL;
        }

        printf("DHT.Tc: %d, DHT.Th: %d\n", dht[Tc][Th].Tc, dht[Tc][Th].Th);

        int value = 0;
        for (int j = 0; j < 16; j++){
          for (int k = 0; k < dht[Tc][Th].Li[j]; k++, i++){
            dht[Tc][Th].Vij[j][k] = img_data[i];
            dht[Tc][Th].codeword_map[j][value] = dht[Tc][Th].Vij[j][k];
            value++;
            char buffer[20];
            printf("%d, %s, %x\n", j + 1, _itoa(value, buffer, 2), dht[Tc][Th].Vij[j][k]);
          }
          value *= 2;
        }
        len_DHT -= len_Vij + 17;
      }
    }
    //SOF0
    else if (img_data[i] == 0xc0){
      int len_SOF0 = img_data[i + 1] * 256 + img_data[i + 2];
      int dest = i + len_SOF0 + 1;
      sof.P = img_data[i + 3];
      sof.Y = img_data[i + 4] * 256 + img_data[i + 5];
      sof.X = img_data[i + 6] * 256 + img_data[i + 7];
      sof.Nf = img_data[i + 8];
      sof.csp_list = new struct SOF_CSP[sof.Nf];

      printf("SOF0.P: %d, SOF0.Y: %d, SOF0.X: %d, SOF0.Nf: %d\n", sof.P, sof.Y, sof.X, sof.Nf);

      i += 9;
      for (int j = 0; j < sof.Nf; j++, i += 3){
        sof.csp_list[j].C = img_data[i];
        sof.csp_list[j].H = img_data[i + 1] >> 4;
        sof.csp_list[j].V = img_data[i + 1] & 0x0f;
        sof.csp_list[j].Tq = img_data[i + 2];
        printf("Component: %d, H: %d, V: %d, Tq: %d\n", sof.csp_list[j].C, sof.csp_list[j].H, sof.csp_list[j].V, sof.csp_list[j].Tq);
      }
      maxh = sof.csp_list[0].H;
      maxv = sof.csp_list[0].V;
      i = dest;
    }
    // unsupport SOF type
    else if (img_data[i] >= 0xc1 && img_data[i] <= 0xcf){
      printf("Unsupport SOF type: 0xff%x", img_data[i]);
      i++;
    }
    //SOS
    else if (img_data[i] == 0xda){
      int len_SOS = img_data[i + 1] * 256 + img_data[i + 2];
      int dest = i + len_SOS + 1;
      sos.Ns = img_data[i + 3];
      sos.csp_list = new struct SOS_CSP[sos.Ns];

      i += 4;
      for (int j = 0; j < sos.Ns; j++, i += 2){
        sos.csp_list[j].Cs = img_data[i];
        sos.csp_list[j].Td = img_data[i + 1] >> 4;
        sos.csp_list[j].Ta = img_data[i + 1] & 0x0f;
      }

      sos.Ss = img_data[i];
      sos.Se = img_data[i + 1];
      sos.Ah = img_data[i + 2] >> 4;
      sos.Al = img_data[i + 2] & 0x0f;

      printf("SOS.Ns: %d, SOS.Ss: %d, SOS.Se: %d, SOS.Ah: %d, SOS.Al: %d\n", sos.Ns, sos.Ss, sos.Se, sos.Ah, sos.Al);
      for (int j = 0; j < sos.Ns; j++){
        printf("Cs: %d, Td: %d, Ta: %d\n", sos.csp_list[j].Cs, sos.csp_list[j].Td, sos.csp_list[j].Ta);
      }

      i = dest;
      return 1;
    }
    //EOI
    else if (img_data[i] == 0xd9){
      printf("End of Image\n");
      break;
    }
    else{
      printf("Unsupport marker 0xff%x\n", img_data[i]);
      i++;
    }

    printf("index: %d hex: %2x value: %d\n", i, img_data[i], img_data[i]);
    //system("pause");
  }
}

unsigned int res = 0;
unsigned int resNum = 0;

void fillNBit(int nbit)
{
  while ((int)resNum<nbit){
    const unsigned char c = img_data[i++];
    res <<= 8;
    if (c == 0xff && img_data[i] == 0x00)
      i++;
    res |= c;
    resNum += 8;
  }
}

int getNBit(int nbit){
  fillNBit(nbit);
  short result = res >> (resNum - nbit);
  resNum -= nbit;
  res &= (1U << resNum) - 1;
  return result;
}

int lookNBit(int nbit){
  fillNBit(nbit);
  int result = res >> (resNum - (nbit));
  return result;
}

void skipNBit( int nbit){
  fillNBit(nbit);
  resNum -= nbit;
  res &= (1U << resNum) - 1;
}

int extend(int ssss, int diff){
  int len = pow(2, ssss);
  if (diff < len / 2){
    return diff - len + 1;
  }
  return diff;
}

void decodeDC(int m[], int n_com){
  map<int, int> *map = dht[0][sos.csp_list[n_com].Td].codeword_map;
  for (int j = 1; j <= 16; j++){
    int codeword = lookNBit(j);
    if (map[j - 1].find(codeword) != map[j - 1].end()) {
      int t = map[j - 1][codeword];
      skipNBit(j);
      int diff = getNBit(t);
      diff = extend(t, diff) + preDC[n_com];
      preDC[n_com] = diff;
      m[0] = diff;
      break;
    }
  }

  
}

void decodeAC(int m[], int n_com){
  map<int, int> *map = dht[1][sos.csp_list[n_com].Ta].codeword_map;
  for (int j = 1; j < 64 ; j++){
    for (int k = 1; k <= 16; k++){
      int codeword = lookNBit(k);
            if (map[k - 1].find(codeword) != map[k - 1].end()) {
        skipNBit(k);
        int t = map[k - 1][codeword];
        int r = t >> 4;
        if (t == 0){
          for (; j < 64; j++)
            m[zigZagOrder[j]] = 0;
          return;
        }
        for (int l = 0; l < r; l++, j++)
          m[zigZagOrder[j]] = 0;
        int s = t & 0x0f;
        int acCoeef = getNBit(s);
        m[zigZagOrder[j]] = extend(s, acCoeef);
        break;
      }
    }
  }
}

void deQuantize(int m[], int n_com){
  for (int j = 0; j < 64; j++){
    m[j] *= dqt[sof.csp_list[n_com].Tq].Qk[j];
  }
}

inline double C(int u){
  if (u == 0)
    return 1 / sqrt(2);
  else
    return 1;
}

void idct(int d[], int d_new[]){
  for (int y = 0; y < 8; y++){
    for (int x = 0; x < 8; x++){
      double sum = 0;

      for (int u= 0; u < 8; u++){
        for (int v = 0; v < 8; v++){
          sum += C(u)*C(v)*d[u * 8 + v] * cos((2 * y + 1)*u*PI / 16.0)*cos((2 * x + 1)*v*PI / 16.0);
        }
      }
      d_new[y * 8 + x] = sum / 4 + 128;
    }
  }
}

void subsampling2(int m[][64], int n_c, int x, int y){
  int tmp[16][16];

  int h = sof.csp_list[n_c].H;
  int v = sof.csp_list[n_c].V;

  if (maxh / h == 1 && maxv / v == 1){
    for (int j = 0; j < v; j++)
      for (int k = 0; k < h; k++){
        int index = 2* j + k;
        int tmpX = k * 8;
        int tmpY = j * 8;
        for (int l = 0; l < 8; l++)
          for (int n = 0; n < 8; n++)
            tmp[tmpY + l][tmpX + n] = m[index][8 * l + n];
      }
  }
  else if (maxh / h == 2 && maxv / v == 1){
    for (int j = 0; j < v; j++)
      for (int k = 0; k < h; k++){
        int index = 2 * j + k;
        int tmpX = k * 8;
        int tmpY = j * 8;
        for (int l = 0; l < 8; l++)
          for (int n = 0; n < 8; n++){
            tmp[tmpY + l][tmpX + 2 * n] = m[index][8 * l + n];
            tmp[tmpY + l][tmpX + 2 * n + 1] = m[index][8 * l + n];
          }
      }
  }
  else if (maxh / h == 1 && maxv / v == 2){
    for (int j = 0; j < v; j++)
      for (int k = 0; k < h; k++){
        int index = 2 * j + k;
        int tmpX = k * 8;
        int tmpY = j * 8;
        for (int l = 0; l < 8; l++)
          for (int n = 0; n < 8; n++){
            tmp[tmpY + 2 * l][tmpX] = m[index][8 * l + n];
            tmp[tmpY + 2 * l + 1][tmpX] = m[index][8 * l + n];
          }
      }
  }
  else if (maxh / h == 2 && maxv / v == 2){
    for (int j = 0; j < v; j++)
      for (int k = 0; k < h; k++){
        int index = 2 * j + k;
        int tmpX = k * 8;
        int tmpY = j * 8;
        for (int l = 0; l < 8; l++)
          for (int n = 0; n < 8; n++){
            tmp[tmpY + 2 * l][tmpX + 2 * n] = m[index][8 * l + n];
            tmp[tmpY + 2 * l + 1][tmpX + 2 * n] = m[index][8 * l + n];
            tmp[tmpY + 2 * l][tmpX + 2 * n + 1] = m[index][8 * l + n];
            tmp[tmpY + 2 * l + 1][tmpX + 2 * n + 1] = m[index][8 * l + n];
          }
      }
  }

  for (int j = 0; j < 8 * maxv; j++)
    for (int k = 0; k < 8 * maxh; k++)
      M[n_c][y + j][x + k] = tmp[j][k];

}

void subsampling(int m[][4][64], int x, int y){
  for (int j = 0; j < 3; j++)
    subsampling2(m[j], j, x, y);
}

void decodeMCU(int x, int y){
  int m[3][4][64] = {}, m_new[3][4][64] = {};
  for (int l = 0; l < 3; l++){
    for (int j = 0; j < sof.csp_list[l].V; j++){
      for (int k = 0; k < sof.csp_list[l].H; k++){
        int index = 2 * j + k;
        decodeDC(m[l][index], l);
        decodeAC(m[l][index], l);
        deQuantize(m[l][index], l);
        idct(m[l][index], m_new[l][index]);
      }
    }
  }
  subsampling(m_new, x, y);
}

void decode(){
  //allocate Y, Cb, Cr matrix
  int blockWidth = sof.csp_list[0].H * 8;
  int blockHeight = sof.csp_list[0].V * 8;

  realX = (sof.X / blockWidth + 1)* blockWidth;
  realY = (sof.Y / blockHeight + 1)* blockHeight;
  
  for (int k = 0; k < 3; k++){
    M[k] = new int*[realY];
    for (int j = 0; j < realY; j++){
      M[k][j] = new int[realX];
    }
  }

  for (int y = 0; y < sof.Y; y += blockHeight){
    for (int x = 0; x < sof.X; x += blockWidth){
      decodeMCU(x, y);
    }
  }
}

int bound(double v){
  if (v>255)
    return 255;
  else if (v < 0)
    return 0;
  else
    return v;
}

void convertRGB(){
  RGB = new unsigned char[sof.X * sof.Y * 3];
  for (int j = 0; j < sof.Y; j++)
    for (int k = 0; k < sof.X; k++){
      RGB[3 * (j*sof.X + k)] = bound(M[0][j][k] + 1.402*(M[2][j][k] - 128));
      RGB[3 * (j*sof.X + k) + 1] = bound(M[0][j][k] - 0.34414*(M[1][j][k] - 128) - 0.71414*(M[2][j][k] - 128));
      RGB[3 * (j*sof.X + k) + 2] = bound(M[0][j][k] + 1.772*(M[1][j][k] - 128));
    }
}

//======================================

void WriteBMP(const char* szBmpFileName, int Width, int Height, unsigned char* RGB)
{
#pragma pack(1)
  struct stBMFH
  {
    char         bmtype[2];
    unsigned int iFileSize;
    short int    reserved1;
    short int    reserved2;
    unsigned int iOffsetBits;

    unsigned int iSizeHeader;
    unsigned int iWidth;
    unsigned int iHeight;
    short int    iPlanes;
    short int    iBitCount;
    unsigned int Compression;
    unsigned int iSizeImage;
    unsigned int iXPelsPerMeter;
    unsigned int iYPelsPerMeter;
    unsigned int iClrUsed;
    unsigned int iClrImportant;
  };
#pragma pack()

  int iNumPaddedBytes = 4 - (Width * 3) % 4;
  iNumPaddedBytes = iNumPaddedBytes % 4;

  stBMFH bh;
  memset(&bh, 0, sizeof(bh));
  bh.bmtype[0] = 'B';
  bh.bmtype[1] = 'M';
  bh.iFileSize = (Width*Height * 3) + (Height*iNumPaddedBytes) + sizeof(bh);
  bh.iOffsetBits = sizeof(stBMFH);
  bh.iSizeHeader = 40;
  bh.iPlanes = 1;
  bh.iWidth = Width;
  bh.iHeight = Height;
  bh.iBitCount = 24;


  char temp[1024] = { 0 };
  sprintf(temp, "%s", szBmpFileName);
  FILE* fp = fopen(temp, "wb");
  fwrite(&bh, sizeof(bh), 1, fp);
  for (int y = Height - 1; y >= 0; y--)
  {
    for (int x = 0; x<Width; x++)
    {
      int i = (x + (Width)*y) * 3;
      unsigned int rgbpix = (RGB[i] << 16) | (RGB[i + 1] << 8) | (RGB[i + 2] << 0);
      fwrite(&rgbpix, 3, 1, fp);
    }
    if (iNumPaddedBytes>0)
    {
      unsigned char pad = 0;
      fwrite(&pad, iNumPaddedBytes, 1, fp);
    }
  }
  fclose(fp);
}
//====================================

int main(int argc, char **argv){


  if (argc != 3){
    printf("<usage>./JpegDecoder <inputfile> <outputfile>");
    return -1;
  }

  cout << "Input: " << argv[1] << "Output:" << argv[2] << endl;
  readFileBytes(argv[1]);

  if (!parseHeader())
    return -1;

  decode();
  convertRGB();
  WriteBMP(argv[2], sof.X, sof.Y, RGB);
 
  return 0;
}