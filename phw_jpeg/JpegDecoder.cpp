
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<iostream>
using namespace std;

#define DQT      0xDB    // Define Quantization Table
#define SOF      0xC0    // Start of Frame (size information)
#define DHT      0xC4    // Huffman Table
#define SOI      0xD8    // Start of Image
#define SOS      0xDA    // Start of Scan
#define EOI      0xD9    // End of Image, or End of File

#define word(x) (((x)[0]<<8)|(x)[1])

struct HuffmanEntry{
	unsigned char symbol;
	unsigned int length;
	unsigned short int code;
};
struct HuffmanTable{
	struct HuffmanEntry* entry;
};
struct component{
	unsigned int h;
	unsigned int v;
	unsigned int q;
	unsigned int Td;	
	unsigned int Ta;
};
struct jpeg_data{
	int height;
	int width;
	int c_num;
	
	int Qtable[3][64];
	struct component com[3];
	struct HuffmanTable DC_HT[2];
	struct HuffmanTable AC_HT[2];
};
static int ZigZagArray[64] = 
{
    0,   1,   5,  6,   14,  15,  27,  28,
    2,   4,   7,  13,  16,  26,  29,  42,
    3,   8,  12,  17,  25,  30,  41,  43,
    9,   11, 18,  24,  31,  40,  44,  53,
    10,  19, 23,  32,  39,  45,  52,  54,
    20,  22, 33,  38,  46,  51,  55,  60,
    21,  34, 37,  47,  50,  56,  59,  61,
    35,  36, 48,  49,  57,  58,  62,  63,
};
int pre_DC[3]={};
void de_zigzag(int *Qtable)
{
	int i;
	int tmp[64];
	for(i=0;i<64;i++)
		tmp[i]=Qtable[ZigZagArray[i]];
	for(i=0;i<64;i++)
		Qtable[i]=tmp[i];
}
void de_quantize(int *table,int q,struct jpeg_data *Jdata)
{
	for( int i=0; i<64; i++)
    {
        table[i] = table[i] * Jdata->Qtable[q][i];
    }
}
void check(int *a)
{
	if((*a)>255)
		(*a)=255;
	if((*a)<0)
		(*a)=0;
}
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
    bh.bmtype[0]='B';
    bh.bmtype[1]='M';
    bh.iFileSize = (Width*Height*3) + (Height*iNumPaddedBytes) + sizeof(bh);
    bh.iOffsetBits = sizeof(stBMFH);
    bh.iSizeHeader = 40;
    bh.iPlanes = 1;
    bh.iWidth = Width;
    bh.iHeight = Height;
    bh.iBitCount = 24;


    char temp[1024]={0};
    sprintf(temp, "%s", szBmpFileName);
    FILE* fp = fopen(temp, "wb");
    fwrite(&bh, sizeof(bh), 1, fp);
    for (int y=Height-1; y>=0; y--)
    {
        for (int x=0; x<Width; x++)
        {
            int i = (x + (Width)*y) * 3;
            unsigned int rgbpix = (RGB[i]<<16)|(RGB[i+1]<<8)|(RGB[i+2]<<0);
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
////////////////////////////////////////////////////////
double C(int in)
{
    if (in == 0)
         return (1.0/sqrt(2));
    else
         return 1.0;
}
int func(int x, int y,int block[])
{
    const double PI = 3.1415926;
    double sum=0;
    for(int u=0;u<8;u++)
    {
         for(int v=0;v<8;v++)
         {
             sum = sum +(C(u)*C(v))*block[8*v+u]*cosf(((2*x+1)*u*PI)/16)*cosf(((2*y+1)*v*PI)/16);
         }
    }         
    return (int)((1.0/4.0)*sum);
}
void IDCT(int in[])
{
	int outblock[8][8];
    for(int y=0; y<8; y++)
    {
        for(int x=0; x<8; x++)
        {
            outblock[x][y] = func(x,y,in);
        }
    }
	for(int y=0; y<8; y++)
    {
        for(int x=0; x<8; x++)
        {
            in[y*8+x]  =  outblock[x][y] + 128;
        }
    }
}
//////////////////////////////////////////////////////////

void DQTparse(unsigned char *data,struct jpeg_data *Jdata)
{
	unsigned int len;
	unsigned int table;
	
	len=word(data+1)-2;
	data=data+3;
	int i=0;
	while(len>0)
	{
		table=data[i]&0xf;
		for(int j=1;j<=64;j++)
		{
			Jdata->Qtable[table][j-1]=data[i+j];
			//cout << Jdata->Qtable[table][j-1] << endl;
		}
		i=i+65;
		len=len-65;
		de_zigzag(Jdata->Qtable[table]);
	}
	
}
void SOFparse(unsigned char *data,struct jpeg_data *Jdata)
{
	unsigned int len;
	len=word(data+1)-2;
	data=data+3;
	int i=0;
	Jdata->height=word(data+1);
	Jdata->width=word(data+3);
	Jdata->c_num=data[5];
	data=data+6;
	for(i=0;i<Jdata->c_num;i++)
	{
		Jdata->com[i].h=data[1] >> 4;
		Jdata->com[i].v=data[1]&0xf;
		Jdata->com[i].q=data[2];
		data=data+3;
	}
}
void buildHT(struct jpeg_data *Jdata,unsigned char *data,
			 unsigned char Tc,unsigned char Th,int total,unsigned int *len)
{
	int i,j;
	int n=0;
	unsigned short int code=0;
	int tmp=0;
	if(Tc==0)
	{
		for(i=0;i<16;i++)
		{
			if(len[i]!=0)
			{
				code = code << (i-tmp);
				for(j=0;j<len[i];j++)
				{
					Jdata->DC_HT[Th].entry[n].length=i+1;
					Jdata->DC_HT[Th].entry[n].symbol=data[n];
					Jdata->DC_HT[Th].entry[n].code=code;
					n++;
					code++;
					tmp=i;
				}
			}
		}
	}
	else
	{
		for(i=0;i<16;i++)
		{
			if(len[i]!=0)
			{
				code = code << (i-tmp);
				for(j=0;j<len[i];j++)
				{
					Jdata->AC_HT[Th].entry[n].length=i+1;
					Jdata->AC_HT[Th].entry[n].symbol=data[n];
					Jdata->AC_HT[Th].entry[n].code=code;
					n++;
					code++;
					tmp=i;
				}
			}
		}
	}
}
void DHTparse(unsigned char *data,struct jpeg_data *Jdata)
{
	unsigned int length;
	length=word(data+1)-2;
	data=data+3;
	unsigned int len[16];
	unsigned char Tc,Th;
	while(length>0)
	{
	Tc=data[0] >> 4;
	Th=data[0] & 0xf;
	data=data+1;
	int i;
	int total=0;
	for(i=0;i<16;i++)
	{
		len[i]=data[i];
		total += len[i];
	}
	data=data+16;
	if(Tc==0)
	{
		Jdata->DC_HT[Th].entry= new struct HuffmanEntry[total];
		buildHT(Jdata,data,Tc,Th,total,len);
	}
	else
	{
		Jdata->AC_HT[Th].entry= new struct HuffmanEntry[total];
		buildHT(Jdata,data,Tc,Th,total,len);
	}
	data=data+total;
	length=length-17-total;
	}
}
void SOSparse(unsigned char *data,struct jpeg_data *Jdata)
{
	data=data+3;
	int num=data[0];
	data++;
	int i;
	for(i=0;i<num;i++)
	{
		Jdata->com[i].Td=(data[1]>>4);
		Jdata->com[i].Ta=(data[1]&0xf);
		data=data+2;
	}
}
unsigned char* parseheader(unsigned char *data,struct jpeg_data *Jdata)
{
	unsigned int len;
	bool sos_jud=true;
	while(sos_jud)
	{
		if(data[0]!=0xff)
		{
			cout << "header error" << endl;
			printf("%x",data[0]);
			break;
		}
		data++;
		len=word(data+1);
		switch(data[0])
		{
			case DQT:
				DQTparse(data,Jdata);
				break;
			case SOF:
				SOFparse(data,Jdata);
				break;
			case DHT:
				DHTparse(data,Jdata);
				break;
			case SOS:
				SOSparse(data,Jdata);
				sos_jud=false;
				break;
			default:
				break;
		}
		data=data+len+1;
	}
	return data;
}
//////////////////////////////////////////////////////////////////////
unsigned int reserve = 0;
unsigned int reservenum = 0;

void FillNBits(unsigned char** data, int& nbits_wanted)
{
    while ((int)reservenum<nbits_wanted)
    {
        const unsigned char c = *(*data)++;
        reserve <<= 8;
        if (c == 0xff && (**data) == 0x00)
            (*data)++;
        reserve |= c;
        reservenum+=8;
    }
}
short GetNBits(unsigned char** data, int nbits_wanted)
{
    FillNBits(data, nbits_wanted);
    short result = ((reserve)>>(reservenum-(nbits_wanted))); 
    reservenum -= (nbits_wanted); 
    reserve &= ((1U<<reservenum)-1);
    return result;
}
int LookNBits(unsigned char** data, int nbits_wanted)
{
    FillNBits(data, nbits_wanted);
    int result = ((reserve)>>(reservenum-(nbits_wanted)));
    return result;
}
void SkipNBits(unsigned char** data, int& nbits_wanted)
{
    FillNBits(data, nbits_wanted);
    reservenum -= (nbits_wanted); 
    reserve &= ((1U<<reservenum)-1);
}
//////////////////////////////////////////////////////////////////////////
bool inHT(unsigned short int val,struct HuffmanTable* HT,int len,unsigned char* sym)
{
	int i=0;
	while(HT->entry[i].length<len)
		i++;
	
	while(HT->entry[i].length==len)
	{
		if(HT->entry[i].code == val)
		{
			*sym = HT->entry[i].symbol;
			return true;
		}
		i++;
	}
	return false;
}
int extend(int in,int len)
{
	int num=1;
	int i;
	for(i=0;i<len;i++)
		num=num*2;
	int base=num*(-1)+1;
	int ans=base+in;
	if(ans>(-1)*num/2)
		ans=ans+num-1;
	return ans;
}
unsigned char *decode_DC(unsigned char *data,struct jpeg_data *Jdata,int *tmp,int which,int DC)
{
	int i;
	short int val=0;
	unsigned char sym=0;
	int num;
	int ans=0;
	for(i=1;i<=16;i++)
	{
		val=LookNBits(&data,i);
		if(inHT(val,&(Jdata->DC_HT[which]),i,&sym))
		{
			num=sym;
			SkipNBits(&data,i);
			ans=GetNBits(&data,num);
			tmp[0]=extend(ans,num)+pre_DC[DC];
			pre_DC[DC]=tmp[0];
			break;
		}
	}
	return data;
}
unsigned char *decode_AC(unsigned char *data,struct jpeg_data *Jdata,int *tmp,int which)
{
	int i;
	short int val=0;
	unsigned char sym=0;
	int num;
	int ans=0;
	int n=1;
	int R,S;
	while(n<=63)
	{
		for(i=1;i<=16;i++)
		{
			val=LookNBits(&data,i);
			if(inHT(val,&(Jdata->AC_HT[which]),i,&sym))
			{
				SkipNBits(&data,i);
				if(sym == 0)
				{
					break;
				}
				R=sym >> 4;
				S=sym & 0xf;
				ans=GetNBits(&data,S);
				break;
			}
		}
		if(sym == 0)
		{
			while(n<=63)
			{
				tmp[n]=0;
				n++;
			}
			break;
		}
		for(i=0;i<R;i++)
		{
			tmp[n]=0;
			n++;
		}
		tmp[n]=extend(ans,S);
		n++;
	}
	return data;
}
void Convert(int y, int cb, int cr,int* r, int* g, int* b)
{
    float red, green, blue;
	check(&y);
	check(&cb);
	check(&cr);
    red   = y + 1.402*(cr-128);
    green = y-0.34414*(cb-128)-0.71414*(cr-128);
    blue  = y+1.772*(cb-128);
    *r = (int)red;
    *g = (int)green;
    *b = (int)blue;
}
void convertToRGB(unsigned char *tmp,int x,int y,int Y[][64],int Cb[][64],int Cr[][64],struct jpeg_data *Jdata)
{
	int r,g,b;
	int i,j;
	int index;
	int Yindex,Cbindex,Crindex;
	int num,num2;
	for(i=0;i<Jdata->com[0].v*8;i++)
		for(j=0;j<Jdata->com[0].h*8;j++)
		{
			index= j*3 + i*Jdata->width*3;
			Yindex= j + i*(Jdata->com[0].h*8);
			num=(Yindex%(Jdata->com[0].h*8)/8)+(Yindex/(64*Jdata->com[0].h))*2;
			num2=(Yindex%(Jdata->com[0].h*8)%8)+8*(Yindex/(Jdata->com[0].h*8)%8);
			Convert(Y[num][num2],Cb[num][num2],Cr[num][num2],&r,&g,&b);
			check(&r);
			check(&g);
			check(&b);
			tmp[index] = r;
            tmp[index+1] = g;
            tmp[index+2] = b;
		}
}
void subsample(int a[][64],int h,int v,int maxh,int maxv)
{
	int tmp[16*16]={};
	int c;
	if(h*v==4)
		return;
	else if(maxh/h==2)
	{
		if(maxv/v==2)
		{
			for(int j=0;j<64;j++)
			{
				tmp[2*(j%8)+16*(2*(j/8))]=a[0][j];
				tmp[2*(j%8)+1+16*(2*(j/8))]=a[0][j];
				tmp[2*(j%8)+16*(2*(j/8)+1)]=a[0][j];
				tmp[2*(j%8)+1+16*(2*(j/8)+1)]=a[0][j];
			}
		}
		else
		{
			for(int i=0;i<4;i=i+2)
				for(int j=0;j<64;j++)
				{
					tmp[i*8*8+2*(j%8)+16*((j/8))]=a[i][j];
					tmp[i*8*8+2*(j%8)+1+16*((j/8))]=a[i][j];
				}
		}
	}
	else if(maxv/v==2)
	{
		for(int i=0;i<2;i++)
			for(int j=0;j<64;j++)
			{
				tmp[i*8+j%8+16*(2*(j/8))]=a[i][j];
				tmp[i*8+j%8+16*(2*(j/8)+1)]=a[i][j];
			}
	}
	else
		return;
	int lim;
	if(maxh==2)
		lim=1;
	else
		lim=2;
	for(int i=0;i<maxh*maxv;i++)
	{
		for(int j=0;j<8;j++)
			for(int k=0;k<8;k++)
			{
				a[i*lim][8*j+k]=tmp[((i*lim/2)*64*maxh+(i*lim%2)*8)+j*8*maxh+k];
			}
	}
}
unsigned char *decodeMCU(unsigned char *data,struct jpeg_data *Jdata,int x,int y,unsigned char *tmp)
{
	int i,j;
	int Y[4][64]={};
	int Cb[4][64]={};
	int Cr[4][64]={};
	int num=0;
	for(i=0;i<Jdata->com[0].v;i++)
	{
		for(j=0;j<Jdata->com[0].h;j++)
		{
			num=j+2*i;
			data=decode_DC(data,Jdata,Y[num],Jdata->com[0].Td,0);
			data=decode_AC(data,Jdata,Y[num],Jdata->com[0].Ta);
			de_zigzag(Y[num]);
			de_quantize(Y[num],Jdata->com[0].q,Jdata);
			IDCT(Y[num]);
			 num++;
		}
		num=2;
	}
	for(i=0;i<Jdata->com[1].v;i++)
	{
		for(j=0;j<Jdata->com[1].h;j++)
		{
			num=j+2*i;
			data=decode_DC(data,Jdata,Cb[num],Jdata->com[1].Td,1);
			data=decode_AC(data,Jdata,Cb[num],Jdata->com[1].Ta);
			de_zigzag(Cb[num]);
			de_quantize(Cb[num],Jdata->com[1].q,Jdata);
			IDCT(Cb[num]);
		}
		num=2;
	}
	for(i=0;i<Jdata->com[2].v;i++)
	{
		for(j=0;j<Jdata->com[2].h;j++)
		{
			num=j+2*i;
			data=decode_DC(data,Jdata,Cr[num],Jdata->com[2].Td,2);
			data=decode_AC(data,Jdata,Cr[num],Jdata->com[2].Ta);
			de_zigzag(Cr[num]);
			de_quantize(Cr[num],Jdata->com[2].q,Jdata);
			IDCT(Cr[num]);
		}
		num=2;
	}
	subsample(Y,Jdata->com[0].h,Jdata->com[0].v,Jdata->com[0].h,Jdata->com[0].v);
	subsample(Cb,Jdata->com[1].h,Jdata->com[1].v,Jdata->com[0].h,Jdata->com[0].v);
	subsample(Cr,Jdata->com[2].h,Jdata->com[2].v,Jdata->com[0].h,Jdata->com[0].v);
	convertToRGB(tmp,x,y,Y,Cb,Cr,Jdata);
	return data;
}
void decode(unsigned char *data,struct jpeg_data *Jdata,char* outname)
{
	int x,y;
	int tmp[64];
	int w=Jdata->width * 3 ;
	int h=Jdata->height * 3 ;
	w=w+(8*Jdata->com[0].h)-(w%(8*Jdata->com[0].h));
	h=h+(8*Jdata->com[0].v)-(h%(8*Jdata->com[0].v));
	unsigned char* RGBdata;
	unsigned char* RGBtmp;
	RGBdata = new unsigned char[w*h];
	memset(RGBdata,0,w*h);
	for(y=0;y<Jdata->height;y=y+8*Jdata->com[0].v)
	{
		for(x=0;x<Jdata->width;x=x+8*Jdata->com[0].h)
		{
			RGBtmp = RGBdata + x*3 + (y*Jdata->width*3);
			data=decodeMCU(data,Jdata,x,y,RGBtmp);
		}
	}
	WriteBMP(outname,Jdata->width, Jdata->height,RGBdata);
}
int jpegdecode(unsigned char *data,struct jpeg_data *Jdata,char* outname)
{
	if(word(data) != 0xffd8)
	{
		cout << "not jpeg";
		return -1;
	}
	data=data+2;
	data=parseheader(data,Jdata);
	decode(data,Jdata,outname);
}
void jpeg2bmp(char* name,char* outname)
{
	unsigned char* data;
	FILE* fp;
	long size;
	struct jpeg_data Jdata;
	
	fp=fopen(name,"rb");
	fseek(fp,0,SEEK_END);
	size=ftell(fp);
	rewind(fp);
	
	data = new unsigned char[size];
	fread(data,size,1,fp);
    fclose(fp);
	
	jpegdecode(data,&Jdata,outname);
}
int main(int argc,char* argv[])
{
  argv[1] = "C:\\Users\\leisure\\Desktop\\Âø\\phw_jpeg\\phw_jpeg\\JPEG\\teatime.jpg";
  argv[2] = "C:\\Users\\leisure\\Desktop\\jizz.bmp";
	jpeg2bmp(argv[1],argv[2]);
	return 0;
}
