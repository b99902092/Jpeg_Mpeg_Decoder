using namespace std;

struct DQT{
  int Pq, Tq;
  int Qk[64];
};

struct DHT{
  int Tc, Th;
  int Li[16];
  int* Vij[16];
  map<int, int> codeword_map[16];
};

struct SOF{
  int P, Y, X, Nf;
  struct SOF_CSP* csp_list;
};

//SOF component-specification parameters
struct SOF_CSP{
  int C, H, V, Tq;
};

struct SOS{
  int Ns, Ss, Se, Ah, Al;
  struct SOS_CSP* csp_list;
};

//SOS component-specification parameters
struct SOS_CSP{
  int Cs, Td, Ta;
};

int zigZagOrder[64] =
{
  0, 1, 8, 16, 9, 2, 3, 10,
  17, 24, 32, 25, 18, 11, 4, 5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6, 7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};


void printMCU(int m[]){
  for (int i = 0; i < 64; i++){
    if (i % 8 == 0)
      cout << endl;
    printf("%4d,", m[i]);
  }
  cout << endl;
}