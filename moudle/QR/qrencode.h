
//#define PROGME
//#define memcpy_P memcpy
#define __LPM(x) *x
#define pgm_read_word(x) *x






#define  neccblk1 4
#define  neccblk2  1
#define  datablkw  13
#define  eccblkwid  26
#define  VERSION  7
#define  ECCLEVEL  4
#define  WD 45
#define  WDB  6


//#define  neccblk1 1
//#define  neccblk2  0
//#define  datablkw  55
//#define  eccblkwid  15
//#define  VERSION  3
//#define  ECCLEVEL  1
//#define  WD 29
//#define  WDB  4





extern unsigned char *strinbuf;//[270];
extern unsigned char *qrframe;//[600];
extern unsigned char *rlens;//[46];

extern const unsigned char framebase[];

extern const unsigned char framask[];


#include "qrbits.h"

// strinbuf in, qrframe out
void qrencode(void);


