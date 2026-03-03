#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
short human[168]={16384, 0, 0, 0, 0, 0, 11104, 2353, 222, 1462, 2891, -1648, -808, 9996, 797, 4639, 1964, -204, 25547, 11569, -3399, -13759, -1934, -1640, 8875, 14932, -1212, -4956, -711, -544, 0, 16384, 0, 0, 0, 0, 10424, 372, 2535, 1993, -1745, 2805, -2245, 1319, 10005, 5547, -94, 1852, 26112, -2972, 10804, -14050, -1478, -2032, 9130, -1080, 14639, -5042, -511, -752, 0, 0, 16384, 0, 0, 0, 13420, -161, -240, 3079, 167, 119, 9079, -76, -257, 6895, 392, 351, 7125, -176, -384, 9427, 289, 103, 4776, -225, -394, 12301, 58, -132, 0, 0, 0, 16384, 0, 0, 8753, 1273, -2310, 7993, 5175, -4500, 16975, -470, -3343, -3829, 10431, -3380, 4593, -246, -766, -1227, 14752, -722, 1977, -92, -342, -488, 15691, -362, 0, 0, 0, 0, 16384, 0, 1461, -96, -112, -234, 15240, 125, 7292, -1757, 1292, 8821, -4356, 5092, 15496, -2891, -524, -3274, -2934, 10511, 2754, -482, -57, -375, -199, 14743, 1144, -214, -2, -95, -125, 15676, 0, 0, 0, 0, 0, 16384, 744, -9, -42, 6, 327, 15358};
unsigned char humanj[54]={0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 0, 6, 6, 7, 7, 8, 8, 9, 9, 10, 0, 11, 11, 12, 12, 13, 13, 14, 14, 15, 12, 16, 16, 17, 17, 18, 18, 19, 19, 20, 18, 21, 12, 22, 22, 23, 23, 24, 24, 25, 25, 26, 24, 27};
unsigned char humank[6]={0,5,10,15,20,26};
unsigned char past[40*180];
unsigned char present[40*180];
unsigned char future[40*180];
unsigned char ovl[40*180];
unsigned char ctrl[40*180];
int changed=0;
unsigned char*setpixel_target=present;
void setpixel(int x,int y,int col){
if(x<0||y<0||x>=320||y>=200)return;
if(setpixel_target==present)changed=1;
if(col)setpixel_target[(y*320+x)>>3]&=~(128>>(x&7));
else setpixel_target[(y*320+x)>>3]|=128>>(x&7);
}
void setline(int x,int y,int px,int py,int col){
int tmp;
int dx=x-px;
int dy=y-py;
if(dx<0)dx=-dx;
if(dy<0)dy=-dy;
if(dx>dy){
if(px<x){
tmp=x;
x=px;
px=tmp;
tmp=y;
y=py;
py=tmp;
}
for(tmp=x;tmp<=px;++tmp){
setpixel(tmp,(tmp-x)*(py-y)/dx+y,col);
}
}else{
if(dy==0)dy=1;
if(py<y){
tmp=x;
x=px;
px=tmp;
tmp=y;
y=py;
py=tmp;
}
for(tmp=y;tmp<=py;++tmp){
setpixel((tmp-y)*(px-x)/dy+x,tmp,col);
}
}
}
int step=1;
int frame=0;
void save(){
FILE*f;
char buf[256];
if(!changed)return;
sprintf(buf,"%07d.bmp",frame);
f=fopen(buf,"wb");
if(!f)return;
if(fwrite("BM^\x1c\0\0\0\0\0\0"
">\0\0\0(\0\0\0@\1\0\0"
"\xb4\0\0\0\1\0\1\0\0\0"
"\0\0 \x1c\0\0\0\0\0\0\0"
"\0\0\0\0\0\0\0\0\0\0"
"\0\xff\xff\xff\0\0\0\0\0",62,1,f)!=1){
fclose(f);
return;
}
if(fwrite(present,1,7200,f)!=7200){
fclose(f);
return;
}
fclose(f);
changed=0;
}
void tryload(unsigned char*p,int n){
FILE*f;
char buf[256];
if(n<0||n>0xffffff)return;
sprintf(buf,"%07d.bmp",n);
f=fopen(buf,"rb");
if(!f)return;
fseek(f,62,SEEK_SET);
fread(p,1,7200,f);
fclose(f);
}
void load(){
save();
if(changed)return;
memset(past,0,7200);
memset(present,0,7200);
memset(future,0,7200);
tryload(present,frame);
tryload(past,frame-step);
tryload(future,frame+step);
}
int hi[2048];
int wi[2048];
short human_pts[12]={
128,100,
136,33,
118,31,
129,136,
140,123,
113,83
};
int human_ptr=-1;
int human_pred[56];
void redraw_human(){
int a;
int i;
int j;
int k;
int m;
memset(ovl,0,sizeof(ovl));
memset(ctrl,0,sizeof(ctrl));
setpixel_target=ovl;
for(i=0;i<56;++i){
j=(i>>1)*6;
k=i&1;
a=0;
for(m=0;m<6;++m){
a+=human[j+m]*human_pts[k+m*2];
}
human_pred[i]=a>>14;
}
for(i=0;i<54;){
j=humanj[i++]*2;
k=humanj[i++]*2;
setline(human_pred[j],human_pred[j+1],human_pred[k],human_pred[k+1],0);
}
setpixel_target=ctrl;
for(i=0;i<6;){
j=humank[i++]*2;
setline(human_pred[j]-4,human_pred[j+1]-4,human_pred[j]+4,human_pred[j+1]-4,0);
setline(human_pred[j]-4,human_pred[j+1]+4,human_pred[j]+4,human_pred[j+1]+4,0);
setline(human_pred[j]-4,human_pred[j+1]-4,human_pred[j]-4,human_pred[j+1]+4,0);
setline(human_pred[j]+4,human_pred[j+1]-4,human_pred[j]+4,human_pred[j+1]+4,0);
}
setpixel_target=present;
}
int main(){
int w;
int h;
int tmp;
int tmp2;
int tmp3;
int mask;
unsigned char*s_1;
unsigned char*s0;
unsigned char*s1;
unsigned char*s2;
unsigned char*s3;
unsigned char*d;
int y;
int x;
int py;
int px;
int oy;
int ox;
int col=-1;
int human_on=0;
 SDL_Window*win;
 SDL_Surface*surf2;
 SDL_Event ev;
 if(SDL_Init(SDL_INIT_VIDEO))return 1;
 win=SDL_CreateWindow("disco",0,0,1280,730,
 SDL_WINDOW_BORDERLESS);
 if(!win)return 2;
 surf2=SDL_GetWindowSurface(win);
 if(!surf2)return 3;
 if(surf2->w*4!=surf2->pitch)return 4;
w=surf2->w;
h=surf2->h-10;
if(w>2048)return 5;
if(h>2048)return 6;
for(y=0;y<h;++y){
hi[y]=179-y*180/h;
if(y&&hi[y]!=(179-(y-1)*180/h))hi[y]=-1;
}
for(x=0;x<w;++x){
wi[x]=x*320/w;
if(x&&wi[x]!=((x-1)*320/w))wi[x]=-1;
}
redraw_human();
load();
for(;;){
    while(SDL_PollEvent(&ev)){
     switch(ev.type){
      case SDL_QUIT:
       return 0;
      case SDL_WINDOWEVENT:
       if(ev.window.event==SDL_WINDOWEVENT_CLOSE)return 0;
      break;
      case SDL_KEYUP:
       switch(ev.key.keysym.sym){
        case SDLK_h:
         human_on=!human_on;
         col=-1;
         human_ptr=-1;
        break;
        case SDLK_COMMA:
         step>>=1;
         if(!step)step=1;
         load();
        break;
        case SDLK_PERIOD:
         step<<=1;
         if(step>1024)step=1024;
         load();
        break;
        case SDLK_LEFT:
         save();
         if(!changed&&frame>0){
          frame-=step;
          load();
         }
        break;
        case SDLK_RIGHT:
         save();
         if(!changed&&frame<0xffffff){
          frame+=step;
          load();
         }
        break;
        default:
         save();
        break;
       }
      break;
      case SDL_MOUSEBUTTONDOWN:
if(!human_on&&!(col==-1||col==-2))col=-2;
      break;
      case SDL_MOUSEBUTTONUP:
if(human_on){
if(human_ptr==-1){
for(y=0;y<12;y+=2){
x=human_pts[y]-(ev.button.x*320/w);
if(x<0)x=-x;
if(x>4)continue;
x=human_pts[y+1]-(179-ev.button.y*180/h);
if(x<0)x=-x;
if(x>4)continue;
human_ptr=y;
break;
}
}else human_ptr=-1;
}else{
      if(ev.button.y>=h)break;
      if(col==-1){
       x=ev.button.x*320/w;
       y=179-ev.button.y*180/h;
       col=present[(y*320+x)>>3]&(128>>(x&7));
       setpixel(x,y,col);
       ox=x;
       oy=y;
       px=x;
       py=y;
      }else col=-1;
}

      break;
      case SDL_MOUSEMOTION:
      if(ev.button.y>=h)break;
if(human_on){
if(human_ptr!=-1){
human_pts[human_ptr]=ev.button.x*320/w;
human_pts[human_ptr+1]=179-ev.button.y*180/h;
redraw_human();
}
}else{
      if((col!=-1&&col!=-2)){
       x=ev.motion.x*320/w;
       y=179-ev.motion.y*180/h;
       x=(x-ox)/4+ox;
       y=(y-oy)/4+oy;
       setpixel(x,y,col);
       setline(x,y,px,py,col);
       px=x;
       py=y;
      }
}
      break;
     }
    }
for(y=0;y<h;++y){
d=surf2->pixels;
d+=surf2->pitch*y;
tmp2=tmp=hi[y];
if(tmp==-1){
for(x=0;x<w;++x){
d[0]=rand();
d[1]=rand();
d[2]=rand();
d+=4;
}
}else{
tmp*=40;
s_1=past+tmp;
s0=present+tmp;
s1=future+tmp;
s2=ovl+tmp;
s3=ctrl+tmp;
for(x=0;x<w;++x){
tmp3=tmp=wi[x];
if(tmp==-1){
d[0]=rand();
d[1]=rand();
d[2]=rand();
}else{
mask=(~tmp)&7;
tmp>>=3;
if((s0[tmp]>>mask)&1){
d[0]=0;
d[1]=0;
d[2]=0;
}else if(col!=-1&&col!=-2&&tmp2==py&&tmp3==px){
d[0]=0;
d[1]=128;
d[2]=255;
}else if((s3[tmp]>>mask)&1){
if(!human_on)goto gray;
d[0]=0;
d[1]=255;
d[2]=0;
}else if((s2[tmp]>>mask)&1){
gray:
d[0]=0x55;
d[1]=0x55;
d[2]=0x55;
}else{
d[0]=((~(s_1[tmp]>>mask))&1)*255;
d[2]=((~(s1[tmp]>>mask))&1)*255;
d[1]=d[0]&d[2];
if(!(d[0]|d[2]))d[0]=d[2]=255;
}
}
d+=4;
}
}
}
d=surf2->pixels;
d+=surf2->pitch*y;
s0=d;
memset(d,0,surf2->pitch);
for(x=0;x<w;++x){
tmp=(x-(w>>1))>>3;
if(!(tmp&1)){
tmp>>=1;
if(tmp==0){
d[0]=255;
d[1]=255;
}else if(tmp==step||tmp==-step){
d[2]=255;
}else{
d[0]=255;
d[1]=255;
d[2]=255;
}
}
d+=4;
}
for(++y;y<h+10;++y){
d=surf2->pixels;
d+=surf2->pitch*y;
memcpy(d,s0,surf2->pitch);
}
for(y=h;y<h+10;++y){
d=surf2->pixels;
d+=surf2->pitch*y;
for(x=0;x<40;++x){
if(changed){
d[0]=0;
d[1]=255;
d[2]=255;
tmp=(y-h-4)*(y-h-4)+(x-20)*(x-20);
if(tmp<=16||(tmp<=32&&(rand()&0x40000000))){
d[1]=0;
}
}else{
d[0]=255;
d[1]=0;
d[2]=0;
}
d+=4;
}
}
SDL_UpdateWindowSurface(win);
SDL_Delay(10);
}
return 0;
}
