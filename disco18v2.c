#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#define MYWIDTH 1366
#define MYHEIGHT 768
WINBOOL WINAPI
SetLayeredWindowAttributes(
HWND hwnd, COLORREF crKey,
BYTE bAlpha, DWORD dwFlags);
HDC memdc;
int*surf;
BITMAPINFOHEADER bmi={
sizeof(BITMAPINFOHEADER),
MYWIDTH,-MYHEIGHT,1,32,0};
int rndstate=0;
void rotate_disco_step1(
signed char*data,int sz){
register int rnd;
rnd=rndstate;
int i;
int j;
int t;
for(i=0;i<sz;++i){
rnd*=1103515245;
rnd+=12345;
j=((unsigned)rnd)%sz;
t=data[j];
data[j]=data[i];
data[i]=t;
}
rndstate=rnd;
}
void rotate_disco_step2(
signed char*data,int sz){
int i;
int t;
for(i=0;i<sz;++i){
t=data[i]+data[i+sz];
if(t<=-8)t=-8;
else if(t>8)t=8;
data[i]=t;
}
}
void rotate_disco_step3(
signed char*data,int sz){
int i;
for(i=0;i<sz;++i){
data[i]+=data[i+sz];
}
}
void _rotate_disco(
signed char*data,int sz){
sz/=3;
rotate_disco_step1(data+(sz*2),sz);
rotate_disco_step2(data+sz,sz);
rotate_disco_step3(data,sz);
}
void _init_disco(
signed char*data,int sz){
int i;
sz/=3;
data+=sz*2;
for(i=0;i<sz;++i){
data[i]=(i%3)-1;
}
}
#define rotate_disco(x) \
_rotate_disco((void*)(x),sizeof(x))
#define init_disco(x) \
_init_disco((void*)(x),sizeof(x))
void _shuffle_int(int*data,int sz){
int i;
int j;
int t;
for(i=0;i<sz;++i){
j=rand()%sz;
t=data[j];
data[j]=data[i];
data[i]=t;
}
}
#define shuffle_int(x) \
_shuffle_int((x), \
sizeof(x)/sizeof(int))
#define MAXW 256
#define MAXH 256
#define DSIZE (MAXW*MAXH*9)
unsigned char disco0[DSIZE];
unsigned char disco1[DSIZE];
typedef struct{
int pitch;
int w;
int h;
unsigned char**lines;
int*offs;
}scaler;
scaler sc0={0};
scaler sc1={0};
int widths[MAXW-64];
unsigned widthsn=0;
int heights[MAXH-64];
unsigned heightsn=0;
int next_width(){
if(widthsn>=sizeof(widths)/
sizeof(int)){
shuffle_int(widths);
widthsn=0;
}
return widths[widthsn++];
}
int next_height(){
if(heightsn>=sizeof(heights)/
sizeof(int)){
shuffle_int(heights);
heightsn=0;
}
return heights[heightsn++];
}
void set_up_scaler(
scaler*sc,unsigned char*data,
int w,int h){
int i;
if(sc->lines)free(sc->lines);
if(sc->offs)free(sc->offs);
sc->lines=malloc(h*sizeof(void*));
sc->offs=malloc(w*sizeof(int));
if(!sc->lines||!sc->offs)exit(0);
for(i=0;i<h;++i){
sc->lines[i]=data+
(((i*sc->h)/h)*sc->pitch);
}
for(i=0;i<w;++i){
sc->offs[i]=((i*sc->w)/w)*3;
}
}
void random_scaler(
scaler*sc,unsigned char*data,
int w,int h){
sc->w=next_width();
sc->h=next_height();
set_up_scaler(sc,data,w,h);
}
int roll=0;
void draw(){
unsigned char*s0;
unsigned char*s1;
unsigned char*d;
int y;
int x;
if(roll<=0){
random_scaler(&sc0,disco0,
MYWIDTH,MYHEIGHT);
random_scaler(&sc1,disco1,
MYWIDTH,MYHEIGHT);
roll=512;
}
rotate_disco(disco0);
rotate_disco(disco1);
d=(void*)surf;
for(y=0;y<MYHEIGHT;++y){
for(x=0;x<MYWIDTH;++x){
s0=sc0.lines[y]+sc0.offs[x];
s1=sc1.lines[y]+sc1.offs[x];
d[0]=(s0[0]+s1[0])>>1;
d[1]=(s0[1]+s1[1])>>1;
d[2]=(s0[2]+s1[2])>>1;
d+=4;
}
}
--roll;
}
static LRESULT CALLBACK WindowProc(
 HWND hwnd, UINT msg,
 WPARAM wp, LPARAM lp){
 switch(msg){
  case WM_DESTROY:
   PostQuitMessage(0);
   return 0;
  case WM_TIMER:{
   HDC hdc=GetDC(hwnd);
   if(hdc){
    draw();
    BitBlt(hdc,0,0,
MYWIDTH,
MYHEIGHT,
memdc,0,0,SRCCOPY);
    ReleaseDC(hwnd,hdc);
   }
  }return 0;
 }
 return DefWindowProcA(
  hwnd,msg,wp,lp);
}
static WNDCLASSA wc={
0,WindowProc,0,0,
NULL,NULL,NULL,NULL,NULL,"disco"};
int mrproper(){
 short c[768];
 int i;
 for(i=0;i!=256;++i){
  c[i+512]=c[i+256]=c[i]=i*257;
 }
 return SetDeviceGammaRamp(
  GetDC(NULL),c)!=TRUE;
}
int main(){
 HWND hwnd;
 MSG msg;
 HDC hdc;
 HBITMAP dib;
 int i;
 HMODULE br;
 HMODULE us;
 void*reghook;
 void*dpi;
 us=LoadLibraryA("user32.dll");
 if(us==NULL)goto quit;
 dpi=GetProcAddress(us,
  "SetProcessDPIAware");
 if(dpi!=NULL)
  ((WINBOOL(WINAPI*)())dpi)();
 mrproper();
 srand((rndstate=time(NULL)));
for(i=64;i<MAXW;++i)widths[i-64]=i+1;
for(i=64;i<MAXH;++i)heights[i-64]=i+1;
shuffle_int(widths);
shuffle_int(heights);
init_disco(disco0);
init_disco(disco1);
sc0.pitch=MAXW*3;
sc1.pitch=MAXW*3;
 hdc=GetDC(NULL);
 memdc=CreateCompatibleDC(hdc);
 ReleaseDC(NULL,hdc);
 dib=CreateDIBSection(memdc,
 (BITMAPINFO*)&bmi,DIB_RGB_COLORS,
 (void**)&surf,NULL,0);
 if(!dib)goto quit;
 SelectObject(memdc,dib);
 wc.hInstance=GetModuleHandleA(NULL);
 if(!RegisterClassA(&wc))goto quit;
 hwnd=CreateWindowExA(WS_EX_TOPMOST|
 WS_EX_TRANSPARENT|WS_EX_LAYERED,
 wc.lpszClassName,wc.lpszClassName,
  WS_POPUP|WS_VISIBLE|WS_SYSMENU,
  0,0,MYWIDTH,MYHEIGHT,
  NULL,NULL,wc.hInstance,NULL
 );
 if(!hwnd)goto quit;
 SetLayeredWindowAttributes(
  hwnd,0,5*255/8,2);
 SetTimer(hwnd,1,16,NULL);
 br=LoadLibraryA("discodll.dll");
 if(br==NULL)goto nohook;
 reghook=GetProcAddress(br,"reghook");
 if(reghook==NULL)goto nohook;
 ((void(*)(HWND,HMODULE))
  reghook)(hwnd,br);
nohook:
 for(;;){
  switch(GetMessageA(&msg,NULL,0,0)){
   case 0:
   case -1:
    goto quit;
   default:
    if(msg.message==WM_QUIT)goto quit;
    DispatchMessageA(&msg);
  }
 }
 quit:
 ExitProcess(0);
 return 0;
}
