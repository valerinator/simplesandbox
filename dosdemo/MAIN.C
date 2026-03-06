#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
char const*exit_msg="Thank you for playing.";
void fail(char const*msg){
exit_msg=msg;
exit(0);
}
void restore_vga(){
union REGS regs;
regs.x.ax=3;
int86(0x10,&regs,&regs);
puts(exit_msg);
}
void set_vga(){
int b;
int g;
int r;
union REGS regs;
regs.x.ax=0x13;
int86(0x10,&regs,&regs);
outportb(0x3c8,0);
for(r=0;r<512;r+=0x49){
for(g=0;g<512;g+=0x49){
for(b=0;b<256;b+=0x55){
outportb(0x3c9,r>>3);
outportb(0x3c9,g>>3);
outportb(0x3c9,b>>2);
}
}
}
}
void delay(int ms){
long t=clock();
t+=ms/10;
while((clock()-t)<0);
}
unsigned char*tmpbuf;
long far_fread(void far*dst,long len,FILE*f){
long ret=0;
size_t sz;
unsigned char far*p=dst;
while(len>0){
sz=fread(tmpbuf,1,len<4096?len:4096,f);
if(sz<=0)break;
farmemcpy(p,tmpbuf,sz);
p+=sz;
ret+=sz;
len-=sz;
}
return ret;
}
unsigned char far*surf;
void present(){
farmemcpy((void far*)0xa0000000L,surf,64000L);
}
void load_screen(char const*n){
FILE*f=fopen(n,"rb");
if(!f)return;
far_fread(surf,64000,f);
fclose(f);
}
char const*screens[]={
"02.cps",
"04.cps",
"05.cps",
"09.cps",
"12.cps",
"16.cps",
"17.cps",
"18.cps",
"21.cps",
};
void game_loop(){
int i;
for(;;){
for(i=0;i<9;++i){
load_screen(screens[i]);
present();
delay(1000);
}
}
}
int main(){
int i;
int y;
int x;
atexit(restore_vga);
tmpbuf=malloc(4096);
if(!tmpbuf)fail("can't init farstdio");
surf=farcalloc(320,200);
if(!surf)fail("can't allocate double buffer");
set_vga();
game_loop();
return 0;
}
