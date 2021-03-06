//--------------------------------------------------------------------
//Error correction level
const unsigned char ECCLEVEL = 1;   //纠错级别 L 设置-----------------.
//Number of error correction blocks 纠错级别不同而不同.看QR ISO PAGE42.
const unsigned char neccblk1 = 2;
const unsigned char neccblk2 = 0;

//Error correction code per block 纠错级别不同而不同.看QR ISO PAGE42.
//(c, k, r): c = total number of codewords           总容量.
//           k = number of data codewords            数据容量.
//           r = number of error correction capacity 纠错容量.
const unsigned char datablkw = 78;
const unsigned char eccblkwid = 20;

const unsigned char VERSION = 7;    //QR版本---------------------------.
//版本不同，大小会不同，容量也不同.看QR ISO PAGE21..
const unsigned char WD = 45;        //宽度 单位:点数  方形.
const unsigned char WDB = 6;        //宽度 单位:字节. 
unsigned char strinbuf[270]="www.hawkbell.com";  //需要装换的数据缓冲.
unsigned char qrframe[600]; //QR生成BIT图缓冲 列行式取模.
unsigned char rlens[46];    //计算缓冲.
////--------------------------------------------------------------------


#define PROGMEM
#define memcpy_P memcpy
#define __LPM(x) *x

//Structure of a QR Code symbo.
const unsigned char framebase[45*6]  = //行列式取模.
{
0xfe,0x00,0x00,0x00,0x0b,0xf8,
0x82,0x00,0x00,0x00,0x12,0x08,
0xba,0x00,0x00,0x00,0x12,0xe8,
0xba,0x00,0x00,0x00,0x1a,0xe8,
0xba,0x00,0x0f,0x80,0x3a,0xe8,
0x82,0x00,0x08,0x80,0x02,0x08,
0xfe,0xaa,0xaa,0xaa,0xab,0xf8,
0x00,0x00,0x08,0x80,0x00,0x00,
0x02,0x00,0x0f,0x80,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x0f,0x80,0x0f,0x80,0x0f,0x80,
0x08,0x80,0x08,0x80,0x08,0x80,
0x0a,0x80,0x0a,0x80,0x0a,0x80,
0x08,0x80,0x08,0x80,0x08,0x80,
0x0f,0x80,0x0f,0x80,0x0f,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x00,0x00,0x00,0x00,0x00,
0x78,0x00,0x00,0x00,0x00,0x00,
0x9a,0x00,0x0f,0x80,0x0f,0x80,
0x00,0x80,0x08,0x80,0x08,0x80,
0xfe,0x00,0x0a,0x80,0x0a,0x80,
0x82,0x00,0x08,0x80,0x08,0x80,
0xba,0x00,0x0f,0x80,0x0f,0x80,
0xba,0x00,0x00,0x00,0x00,0x00,
0xba,0x00,0x00,0x00,0x00,0x00,
0x82,0x00,0x00,0x00,0x00,0x00,
0xfe,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char framask[]  = {
0xff,0xff,0xff,0xff,0xff,0xf8,
0x10,0x04,0x00,0x80,0x08,0x00,
0x40,0x01,0x00,0x02,0x00,0x02,
0x00,0x01,0x00,0x00,0x40,0x00,
0x08,0x00,0x03,0xe0,0x02,0x1f,
0x00,0x18,0x7c,0x00,0x70,0xf8,
0x00,0xf0,0xf8,0x00,0xf8,0x10,
0x00,0x00,0x04,0x00,0x00,0x00,
0x80,0x00,0x00,0x08,0x00,0x00,
0x00,0x40,0x00,0x00,0x01,0x00,
0x00,0x00,0x02,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x01,0x00,
0x00,0x00,0x1f,0xc0,0x00,0x00,
0x03,0xf8,0x00,0x00,0x00,0x3f,
0x80,0x03,0xe0,0x03,0xff,0x00,
0x1f,0x00,0x1f,0xfc,0x00,0x7c,
0x00,0x7f,0xf8,0x00,0xf8,0x00,
0xff,0xf8,0x00,0xf8,0x00,0xff,
0xfc,0x00,0x00,0x00,0x01,0xff,
0x00,0x00,0x00,0x00,0x3f,0xe0,
0x00,0x00,0x00,0x03,0xfe,0x00,
0x00,0x00,0x00,0x00,
};
