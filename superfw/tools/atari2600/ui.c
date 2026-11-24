/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Classic handheld-emulator menu; no card writes or game save format changes. */
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "vmachine.h"
#include "resource.h"
#include "ui.h"

#define KEYS (*(volatile uint16_t *)0x04000130)
#define VCOUNT (*(volatile uint16_t *)0x04000006)
#define VIDEO ((volatile uint16_t *)0x06000000)
#define LR 0x300u
static uint16_t saved_frame[240 * 160] __attribute__((section(".sbss")));

/* Original compact raster glyphs; lower-case labels follow the familiar menus. */
static const uint8_t capitals[26][7] = {
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
 {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
};
static const uint8_t small[26][7] = {
 {0,0,14,1,15,17,15},{16,16,22,25,17,17,30},{0,0,14,16,16,17,14},
 {1,1,13,19,17,17,15},{0,0,14,17,31,16,14},{6,9,8,28,8,8,8},
 {0,0,15,17,15,1,14},{16,16,22,25,17,17,17},{4,0,12,4,4,4,14},
 {2,0,6,2,2,18,12},{16,16,18,20,24,20,18},{12,4,4,4,4,4,14},
 {0,0,26,21,21,17,17},{0,0,22,25,17,17,17},{0,0,14,17,17,17,14},
 {0,0,30,17,30,16,16},{0,0,15,17,15,1,1},{0,0,22,25,16,16,16},
 {0,0,15,16,14,1,30},{8,8,28,8,8,9,6},{0,0,17,17,17,19,13},
 {0,0,17,17,17,10,4},{0,0,17,17,21,21,10},{0,0,17,10,4,10,17},
 {0,0,17,17,15,1,14},{0,0,31,2,4,8,31}
};
static const uint8_t numbers[10][7] = {
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,14}
};
static uint8_t glyph(unsigned c,unsigned row) {
    if(c>='A' && c<='Z')return capitals[c-'A'][row];
    if(c>='a' && c<='z')return small[c-'a'][row];
    if(c>='0' && c<='9')return numbers[c-'0'][row];
    if(c==':')return row==2||row==5?4:0;
    if(c=='>')return row==1||row==5?8:row==2||row==4?4:row==3?2:0;
    if(c=='-')return row==3?14:0;
    if(c=='+')return row==3?14:row==2||row==4?4:0;
    if(c=='/')return row<5?1u<<row:0;
    if(c=='*')return row==2||row==4?10:row==3?31:0;
    return 0;
}
static void text(const char *s,unsigned x,unsigned y,unsigned color) {
    unsigned row,col;
    while(*s && x+5<=240) {
        for(row=0;row<7;++row)for(col=0;col<5;++col)
            if(glyph((unsigned char)*s,row)&(16u>>col))VIDEO[(y+row)*240+x+col]=color;
        ++s;x+=6;
    }
}
static void blank(void) {
    unsigned i;for(i=0;i<240*160;++i)VIDEO[i]=0;
}
static void wait_frame(void) {
    while(VCOUNT>=160){}while(VCOUNT<160){}
}
static void release_keys(void) {while((KEYS&0x30f)!=0x30f)wait_frame();}
static void footer(void) {
    text("2600 GBA - DSSMM",72,134,0x7fff);
    text("MikeDX / Virtual VCS",60,146,0x4210);
}
int atari_return_ready(void) {
    const volatile uint32_t *image=(const volatile uint32_t *)0x09800000;
    const volatile uint32_t *stub=(const volatile uint32_t *)0x09a00000;
    uint32_t size=image[0xcc/4];
    return image[0xa0/4]==0x45505553 && image[0xa4/4]==0x00574652 &&
        size>=256 && size<=0x200000 && !(size&511) && stub[1]==0x54525344;
}
void atari_exit(void) {
    if(atari_return_ready())((void(*)(void))0x09a00000)();
    blank();text("Return unavailable",48,64,0x7fff);
    text("Power off to return",48,80,0x7fff);
    for(;;)wait_frame();
}
void atari_bad_rom(void) {
    blank();text("Unsupported ROM",48,48,0x7fff);
    text("Use 2K 4K F8 or F6",48,64,0x7fff);
    text("L+R to exit",48,88,0x7fff);
    release_keys();while(KEYS&LR)wait_frame();atari_exit();
}
static void draw_menu(unsigned page,unsigned selected) {
    static const char *main_items[]={"Controller->","Display->","Other Settings->",
        "Help->","Resume","Restart","Exit"};
    unsigned i;char value[]="Frame skip: 0";
    blank();footer();
    if(!page) {
        for(i=0;i<7;++i)text(main_items[i],8,40+i*8,i==selected?0x7fe0:0x7fff);
        text("*",0,40+selected*8,0x7fe0);
    } else if(page==1) {
        text(app_data.swap?"Swap players: On":"Swap players: Off",8,40,0x7fe0);
        text("D-pad: Joystick",8,64,0x7fff);text("A: Fire",8,72,0x7fff);
        text("Start: Reset",8,80,0x7fff);text("Select: Game select",8,88,0x7fff);
    } else if(page==2) {
        value[12]='0'+app_data.rr-1;text(value,8,40,0x7fe0);
        text("Left/Right or A to change",8,64,0x7fff);
    } else if(page==3) {
        text(riotRead[0x282]&8?"Console: Color":"Console: B/W",8,40,0x7fe0);
    } else {
        text("L+R: Menu",8,40,0x7fff);text("A: Select  B: Back",8,48,0x7fff);
        text("ROMs: 2K 4K F8 F6",8,64,0x7fff);
        text("Experimental - no sound",8,80,0x7fff);
        text("Alex Hornby / Daniel Boris",8,96,0x7fff);
    }
}
int atari_menu(void) {
    unsigned i,page=0,selected=0,parent=0,previous,pressed,held;
    for(i=0;i<240*160;++i)saved_frame[i]=VIDEO[i];
    release_keys();previous=0;draw_menu(page,selected);
    for(;;) {
        wait_frame();held=(~KEYS)&0x3ff;pressed=held&~previous;previous=held;
        if(!pressed)continue;
        if(pressed&2) {
            if(!page)break;
            page=0;selected=parent;
        } else if(!page) {
            if((pressed&64) && selected)--selected;
            if((pressed&128) && selected<6)++selected;
            if(pressed&1) {
                if(selected<4)parent=selected,page=selected+1;
                else if(selected==4)break;
                else if(selected==5){
                    for(i=0;i<240*160;++i)VIDEO[i]=saved_frame[i];
                    release_keys();return 1;
                }
                else atari_exit();
            }
        } else if(pressed&(1|16|32)) {
            if(page==1)app_data.swap^=1;
            if(page==2)app_data.rr=pressed&32?(app_data.rr+3)%5+1:app_data.rr%5+1;
            if(page==3)riotRead[0x282]^=8;
        }
        draw_menu(page,selected);
    }
    for(i=0;i<240*160;++i)VIDEO[i]=saved_frame[i];
    release_keys();return 0;
}
