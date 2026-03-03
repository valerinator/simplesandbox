#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
WINBOOL WINAPI SetLayeredWindowAttributes (HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
HDC memdc;
int*surf;
BITMAPINFOHEADER bmi={
 sizeof(BITMAPINFOHEADER),
 0,
 0,
 1,
 32,
 0,
 0,
 0,
 0,
 0,
 0
};
int rndstate=0;
signed char ovl[3*1366*768*3];
void will(signed char*dst){
register signed char*p;
register int i;
register int j;
register int tmp;
register int rnd;
rnd=rndstate;
p=dst;
i=(1366*768*3);
do{
--i;
rnd*=1103515245;
rnd+=12345;
j=((unsigned)rnd)%(1366*768*3);
tmp=p[j];
p[j]=p[i];
p[i]=tmp;
}while(i);
rndstate=rnd;
}
void advance(signed char*dst){
register signed char*p;
register int i;
p=dst;
i=1366*768*3;
do{
*p+=p[1366*768*3];
if(*p<-8)*p=-8;
else if(*p>8)*p=8;
++p;
}while(--i);
}
void advance2(signed char*dst){
register signed char*p;
register int i;
p=dst;
i=1366*768*3;
do{
*p+=p[1366*768*3];
++p;
}while(--i);
}
void render(){
 register int x;
 register unsigned char*out;
 register unsigned char*p;
 x=1366*768;
 out=((unsigned char*)surf);
 p=(unsigned char*)ovl;
 do{
  *out++=*p++;
  *out++=*p++;
  *out++=*p++;
  ++out;
 }while(--x);
}
void draw(){
 will(ovl+(1366*768*3*2));
 advance(ovl+(1366*768*3));
 advance2(ovl);
 render();
}
int bw=1366;
int bh=768;
int bcnt=0;
static LRESULT CALLBACK WindowProc(
 HWND hwnd,
 UINT msg,
 WPARAM wp,
 LPARAM lp
){
 switch(msg){
  case WM_DESTROY:
   PostQuitMessage(0);
   return 0;
  case WM_TIMER:{
   HDC hdc=GetDC(hwnd);
   if(hdc){
    draw();
    SetStretchBltMode(hdc,COLORONCOLOR);
    StretchBlt(hdc,0,0,
GetSystemMetrics(SM_CXSCREEN),
GetSystemMetrics(SM_CYSCREEN),
memdc,0,0,bw,bh,SRCCOPY);
    GdiFlush();
    ReleaseDC(hwnd,hdc);
    /*++bcnt;
    if(bcnt>=512){
     bcnt=0;
     bw=(rand()&63)+65;
     bh=(rand()&63)+65;
    }*/
   }
  }return 0;
 }
 return DefWindowProcA(hwnd,msg,wp,lp);
}
static WNDCLASSA wc={0,WindowProc,0,0,NULL,NULL,NULL,NULL,NULL,"disco"};
int mrproper(){
 short colors[768];
 int i;
 for(i=0;i!=256;++i){
  colors[i+512]=colors[i+256]=colors[i]=i*257;
 }
 return SetDeviceGammaRamp(GetDC(NULL),colors)!=TRUE;
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
 dpi=GetProcAddress(us,"SetProcessDPIAware");
 if(dpi!=NULL)((WINBOOL(WINAPI*)())dpi)();
 mrproper();
 for(i=0;i<(1366*768);++i){
  ovl[1366*768*3*2+i]=-1;
  ovl[1366*768*3*2+i+1366*768]=0;
  ovl[1366*768*3*2+i+1366*768*2]=1;
 }
 bmi.biWidth=1366;
 bmi.biHeight=768;
 rndstate=time(NULL);
 hdc=GetDC(NULL);
 memdc=CreateCompatibleDC(hdc);
 ReleaseDC(NULL,hdc);
 dib=CreateDIBSection(memdc,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,(void**)&surf,NULL,0);
 if(!dib)goto quit;
 SelectObject(memdc,dib);
 wc.hInstance=GetModuleHandleA(NULL);
 if(!RegisterClassA(&wc))goto quit;
 hwnd=CreateWindowExA(WS_EX_TOPMOST|WS_EX_TRANSPARENT|WS_EX_LAYERED,wc.lpszClassName,wc.lpszClassName,
  WS_POPUP|WS_VISIBLE|WS_SYSMENU,
  0,0,1920,1080,
  NULL,NULL,wc.hInstance,NULL
 );
 if(!hwnd)goto quit;
 SetLayeredWindowAttributes(hwnd,0,5*255/8,2);
 SetTimer(hwnd,1,16,NULL);
 br=LoadLibraryA("discodll.dll");
 if(br==NULL)goto nohook;
 reghook=GetProcAddress(br,"reghook");
 if(reghook==NULL)goto nohook;
 ((void(*)(HWND,HMODULE))reghook)(hwnd,br);
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
