#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <sys/random.h>
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
#define rotate_disco(x) _rotate_disco((void*)(x),sizeof(x))
#define init_disco(x) _init_disco((void*)(x),sizeof(x))
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
#define shuffle_int(x) _shuffle_int((x),sizeof(x)/sizeof(int))
#define MAXW 512
#define MAXH 512
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
int widths[MAXW-2];
unsigned widthsn=0;
int heights[MAXH-2];
unsigned heightsn=0;
int next_width(){
if(widthsn>=sizeof(widths)/sizeof(int)){
shuffle_int(widths);
widthsn=0;
}
return widths[widthsn++];
}
int next_height(){
if(heightsn>=sizeof(heights)/sizeof(int)){
shuffle_int(heights);
heightsn=0;
}
return heights[heightsn++];
}
void set_up_scaler(scaler*sc,unsigned char*data,int w,int h){
int i;
if(sc->lines)free(sc->lines);
if(sc->offs)free(sc->offs);
sc->lines=malloc(h*sizeof(void*));
sc->offs=malloc(w*sizeof(int));
if(!sc->lines||!sc->offs)exit(0);
for(i=0;i<h;++i){
sc->lines[i]=data+(((i*sc->h)/h)*sc->pitch);
}
for(i=0;i<w;++i){
sc->offs[i]=((i*sc->w)/w)*3;
}
}
void random_scaler(scaler*sc,unsigned char*data,int w,int h){
sc->w=next_width();
sc->h=next_height();
set_up_scaler(sc,data,w,h);
}
int main(){
int i;
int roll=0;
int w;
int h;
unsigned char*s0;
unsigned char*s1;
unsigned char*d;
int y;
int x;
 SDL_Window*win;
 SDL_Surface*surf2;
 SDL_Event ev;
     SDL_SysWMinfo info;
     Display*xdisp=0;
    Window xwin = 0;
 getrandom(&rndstate,sizeof(rndstate),0);
 srand(rndstate);
 if(SDL_Init(SDL_INIT_VIDEO))return 1;
SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
 win=SDL_CreateWindow("disco",0,0,1366,768,
 SDL_WINDOW_ALWAYS_ON_TOP|
// SDL_WINDOW_FULLSCREEN_DESKTOP|
 SDL_WINDOW_BORDERLESS);
 if(!win)return 2;
 surf2=SDL_GetWindowSurface(win);
 if(!surf2)return 3;
 if(surf2->w*4!=surf2->pitch)return 4;
 SDL_SetWindowOpacity(win,.5);
     SDL_VERSION(&info.version);
 if(!SDL_GetWindowWMInfo(win, &info))return 5;
    if (info.subsystem == SDL_SYSWM_X11) {
        xdisp = info.info.x11.display;
        xwin = info.info.x11.window;
XRectangle rect={0};
XserverRegion region = XFixesCreateRegion(xdisp, &rect, 1);
XFixesSetWindowShapeRegion(xdisp, xwin, ShapeInput, 0, 0, region);
XFixesDestroyRegion(xdisp, region);
    }else return 6;
w=surf2->w;
h=surf2->h;
for(i=2;i<MAXW;++i)widths[i-2]=i+1;
for(i=2;i<MAXH;++i)heights[i-2]=i+1;
shuffle_int(widths);
shuffle_int(heights);
init_disco(disco0);
init_disco(disco1);
sc0.pitch=MAXW*3;
sc1.pitch=MAXW*3;
for(;;){
    while(SDL_PollEvent(&ev)){
     switch(ev.type){
      case SDL_QUIT:
       return 0;
      case SDL_WINDOWEVENT:
       if(ev.window.event==SDL_WINDOWEVENT_CLOSE)return 0;
     }
    }
if(roll<=0){
random_scaler(&sc0,disco0,w,h);
random_scaler(&sc1,disco1,w,h);
roll=512;
}
rotate_disco(disco0);
rotate_disco(disco1);
for(y=0;y<h;++y){
d=surf2->pixels;
d+=surf2->pitch*y;
for(x=0;x<w;++x){
s0=sc0.lines[y]+sc0.offs[x];
s1=sc1.lines[y]+sc1.offs[x];
d[0]=(s0[0]+s1[0])>>1;
d[1]=(s0[1]+s1[1])>>1;
d[2]=(s0[2]+s1[2])>>1;
d+=4;
}
}
--roll;
SDL_UpdateWindowSurface(win);
}
return 0;
}
