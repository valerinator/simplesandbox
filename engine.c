#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#define STBI_ONLY_JPEG
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define VWIDTH 1280
#define VHEIGHT 720
int rndstate=0;
//trance entropy
void rotate_disco_step1(
short*data,int sz){
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
short*data,int sz){
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
short*data,int sz){
int i;
for(i=0;i<sz;++i){
data[i]+=data[i+sz];
}
}
void _rotate_disco(
short*data,int sz){
sz/=3;
rotate_disco_step1(data+(sz*2),sz);
rotate_disco_step2(data+sz,sz);
rotate_disco_step3(data,sz);
}
void _init_disco(
short*data,int sz){
int i;
sz/=3;
data+=sz*2;
for(i=0;i<sz;++i){
data[i]=(i%3)-1;
}
}
#define rotate_disco(x) \
_rotate_disco((void*)(x),sizeof(x)/sizeof(short))
#define init_disco(x) \
_init_disco((void*)(x),sizeof(x)/sizeof(short))
//todo maybe increase later
#define DSIZE (128*9)
unsigned short disco0[DSIZE];
unsigned short disco1[4096*9];
int prepare_sprite(unsigned char*pixels,int w,int h,unsigned char*out){
int ret=sizeof(void*)*3;
int c;
int y;
int x0;
int x1;
int x;
int has_c;
int tmp;
int fails;
int sz;
unsigned char*s;
unsigned char*p=NULL;
if(out)p=out+ret;
for(c=0;c<3;++c){
if(out)((void**)out)[c]=p;
has_c=0;
for(y=0;y<h;++y){
s=pixels+(y*w*3+2-c);
x0=0;
while(x0<w){
 tmp=s[x0*3];
 if(tmp>=16){
  fails=0;
  for(x1=x0+1;x1<w;++x1){
   tmp=s[x1*3];
   if(tmp<16){
    if(fails==3)break;
    ++fails;
   }else fails=0;
  }
  x1-=fails;
  sz=x1-x0;
  if(sz>2){
   if(out){
    *(int*)p=sz;
    p+=sizeof(int);
    *(int*)p=((y+512)*(VWIDTH+1024)+x0+512-(w>>1));
    p+=sizeof(int);
    for(x=x0;x<x1;++x){
     tmp=s[x*3];
     if(tmp<16)tmp=0;
     else if(tmp<32)tmp=(tmp-16)*2;
     *p++=tmp;
    }
   }
   ret+=sizeof(int)*2+sz;
   has_c=1;
  }
  x0=x1;
 }else ++x0;
}
}
if(has_c){
if(out){
*(int*)p=0;
p+=sizeof(int);
}
ret+=sizeof(int);
}else if(out)((void**)out)[c]=NULL;
}
return ret;
}
unsigned char**load_sprite(char const*fname,int force){
void*ret=NULL;
int x,y,n;
unsigned char*pixels=stbi_load(fname, &x, &y, &n, 3);
if(!pixels)return NULL;
if(n!=3||(!force&&(x>512||y>512)))goto err;
n=prepare_sprite(pixels,x,y,NULL);
if(n<=0)goto err;
ret=malloc(n);
if(!ret)goto err;
if(prepare_sprite(pixels,x,y,ret)!=n)abort();
err:
stbi_image_free(pixels);
return ret;
}
unsigned char surf[6*(VHEIGHT+512)*(VWIDTH+1024)+512*(VWIDTH+1024)];
void draw_sprite(unsigned char**sprite,int x,int y,int co){//co 0 or 3
int c;
unsigned char*s;
unsigned char*d;
int sz;
int off;
int base;
if(x<0||y<-511||x>=VWIDTH||y>=VHEIGHT)return;/*adjusted limits to make x centering safe*/
base=y*(VWIDTH+1024)+x;
for(c=0;c<3;++c){
s=sprite[c];
if(s==NULL)continue;
d=surf+((co+c)*(VWIDTH+1024)*(VHEIGHT+512));
for(;;){
sz=*(int*)s;
if(!sz)break;
off=((int*)s)[1]+base;
s+=sizeof(int)*2;
memcpy(d+off,s,sz);
s+=sz;
}
}
}
short gaussian[511];//bss is free
short cosine[511];
int calc_light(short*hramp,short*vramp,int w,int h,int brightness,unsigned char*out){
int ret=sizeof(void*)*3;
unsigned char*p=NULL;
int y;
int x;
int x1;
int sz;
int yval;
int tmp;
if(w<=0||h<=0||w>511||h>511)return 0;
if(brightness<=0)brightness=0;
else if(brightness>256)brightness=256;
if(out){
p=out+ret;
*(void**)out=p;
((void**)out)[1]=NULL;
((void**)out)[2]=NULL;
}
for(y=0;y<h;++y){
 yval=(vramp[y*511/h]*brightness)>>8;
 for(x=0;x<w;++x){
  tmp=(yval*hramp[x*511/w])>>22;
  if(tmp>0){
   x1=w-x;//assume x symmetry
   sz=x1-x;
   if(out){
    *(int*)p=sz;
    p+=sizeof(int);
    *(int*)p=((y+512)*(VWIDTH+1024)+x+512-(w>>1));                
    p+=sizeof(int);
    for(;x<x1;++x){
     *p++=(yval*hramp[x*511/w])>>22;
    }
   }
   ret+=sizeof(int)*2+sz;
   break;
  }
 }
}
if(ret!=(sizeof(void*)*3)){
if(out){
*(int*)p=0;
p+=sizeof(int);
}
ret+=sizeof(int);
}else if(out)*(void**)out=NULL;
return ret;
}
unsigned char**create_light(short*vramp,short*hramp,int h,int w,int brightness){
void*ret=NULL;
int n;
n=calc_light(hramp,vramp,w,h,brightness,NULL);
if(n<=0)goto err;
ret=malloc(n);
if(!ret)goto err;
if(calc_light(hramp,vramp,w,h,brightness,ret)!=n)abort();
err:
return ret;
}
void init_ramps(){
int i;
int k;
double f=log(0x7fff)/-0x20000;
double g=3.14159265/512;
for(i=0;i<511;++i){
k=i-255;
gaussian[i]=exp((k*k)*f)*0x7fff;
cosine[i]=cos(k*g)*0x7fff;
}
}
int main(){
int i;
int w;
int h;
unsigned char*d;
unsigned char*s;
int y;
int x;
int tmp;
 SDL_Window*win;
 SDL_Surface*surf2;
 SDL_Event ev;
unsigned char**bg;
unsigned char**mysprite;
unsigned char**round_light;
unsigned char**flashlight;
unsigned char**disco_light[256];
 if(SDL_Init(SDL_INIT_VIDEO))return 1;
 win=SDL_CreateWindow("disco",0,0,VWIDTH,VHEIGHT,SDL_WINDOW_BORDERLESS);
 if(!win)return 2;
 surf2=SDL_GetWindowSurface(win);
 if(!surf2)return 3;
 if(surf2->w*4!=surf2->pitch)return 4;
w=surf2->w;
h=surf2->h;
//that should be a nearest neighbor upscaler. not really needed now
if(w>VWIDTH)w=VWIDTH;
if(h>VHEIGHT)h=VHEIGHT;
if(w>2048)return 5;
if(h>2048)return 6;
init_ramps();
init_disco(disco0);
init_disco(disco1);
bg=load_sprite("bg.jpg",1);
mysprite=load_sprite("images.jpeg",0);
if(!mysprite)return 7;
flashlight=create_light(cosine,cosine,128,128,256);
if(!flashlight)return 8;
round_light=create_light(gaussian,gaussian,128,128,256);
if(!round_light)return 9;
for(i=0;i<256;++i){
disco_light[i]=create_light(gaussian,gaussian,128,128,i+1);
if(!disco_light[i])return 10;
}
for(;;){
    while(SDL_PollEvent(&ev)){
     switch(ev.type){
      case SDL_QUIT:
       return 0;
      case SDL_WINDOWEVENT:
       if(ev.window.event==SDL_WINDOWEVENT_CLOSE)return 0;
      break;
     }
    }
rotate_disco(disco0);
rotate_disco(disco1);
for(y=0;y<h;++y){
d=surf+((y+512)*(VWIDTH+1024)+512);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)),0,VWIDTH);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)*1),0,VWIDTH);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)*2),0,VWIDTH);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)*3),0x22,VWIDTH);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)*4),0x22,VWIDTH);
memset(d+((VWIDTH+1024)*(VHEIGHT+512)*5),0x22,VWIDTH);
}
draw_sprite(bg,VWIDTH>>1,0,0);
draw_sprite(mysprite,((unsigned short*)disco1)[0]%VWIDTH,((unsigned short*)disco1)[1]%VHEIGHT,0);
for(i=0;(i+9)<=((sizeof(disco0)/sizeof(short))/3);i+=9){
x=(((unsigned short*)disco0)[i+3]>>2)%VWIDTH;
y=(((unsigned short*)disco0)[i+4]>>2)%VHEIGHT;
draw_sprite(disco_light[disco0[i]&255],x+((disco0[i+5]>>2)&63)-32,y+((disco0[i+6]>>2)&63)-32,3);
draw_sprite(disco_light[disco0[i+1]&255],x,y,4);
draw_sprite(disco_light[disco0[i+2]&255],x+((disco0[i+7]>>2)&63)-32,y+((disco0[i+8]>>2)&63)-32,5);
}
for(y=0;y<h;++y){
d=surf2->pixels;
d+=surf2->pitch*y;
s=surf+((y+512)*(VWIDTH+1024)+512);
for(x=0;x<w;++x){
tmp=(s[0]*(s[(VWIDTH+1024)*(VHEIGHT+512)*3]+1))>>7;
if(tmp>255)tmp=255;
*d++=tmp;
tmp=(s[(VWIDTH+1024)*(VHEIGHT+512)]*(s[(VWIDTH+1024)*(VHEIGHT+512)*4]+1))>>7;
if(tmp>255)tmp=255;
*d++=tmp;
tmp=(s[(VWIDTH+1024)*(VHEIGHT+512)*2]*(s[(VWIDTH+1024)*(VHEIGHT+512)*5]+1))>>7;
if(tmp>255)tmp=255;
*d=tmp;
d+=2;
++s;
}
}
SDL_UpdateWindowSurface(win);
SDL_Delay(10);
}
return 0;
}
