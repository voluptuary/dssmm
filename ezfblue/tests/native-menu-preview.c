/* Actual kernel renderer with deterministic clock/filesystem seams; no card IO. */
#define main dssmm_kernel_main
#define rtc_enable preview_rtc_enable
#define rtc_disenable preview_rtc_disable
#define rtc_gettime preview_rtc_gettime
#define f_open preview_f_open
#define f_stat preview_f_stat
#define f_chdir preview_f_chdir
#define f_getcwd preview_f_getcwd
#include "../targets/blue/source/ezbluekernel.c"
#undef main

void preview_rtc_enable(void) {}
void preview_rtc_disable(void) {}
int preview_rtc_gettime(u8 *time) {time[0]=0x12;time[1]=0x34;time[2]=0x56;return 0;}
FRESULT preview_f_open(FIL *file,const TCHAR *path,BYTE mode) {(void)file;(void)path;(void)mode;return FR_NO_FILE;}
FRESULT preview_f_stat(const TCHAR *path,FILINFO *info) {(void)path;(void)info;return FR_NO_FILE;}
FRESULT preview_f_chdir(const TCHAR *path) {(void)path;return FR_NO_PATH;}
FRESULT preview_f_getcwd(TCHAR *path,UINT size) {if(size>1){path[0]='/';path[1]=0;}return FR_OK;}
#ifndef PREVIEW_SCENE
#define PREVIEW_SCENE 0
#endif
static void preview_artwork(void) {
    folder_total=0;game_total_SD=3;
    launcher_active_page=SD_list;launcher_thumbnail_style=LAUNCHER_THUMB_STYLE_TITLE;
    Launcher_ResetThumbCache();launcher_cache_center_index=1;
    LauncherThumbCache *cache[]={&launcher_cache_prev,&launcher_cache_selected,&launcher_cache_next};
    for(unsigned tile=0;tile<3;++tile) {
        cache[tile]->valid=cache[tile]->has_thumbnail=cache[tile]->custom_thumbnail=1;
        cache[tile]->absolute_index=tile;
        u16 *pixels=(u16*)cache[tile]->thumb_data;
        for(unsigned y=0;y<80;++y)for(unsigned x=0;x<120;++x)
            pixels[y*120+x]=RGB((x/8+tile*7)%32,(y/6+tile*9)%32,(x/12+y/8+tile*4)%32);
    }
}
static void preview_draw(unsigned scene,unsigned selected) {
    Launcher_ApplyThemeColours();
    if(scene==0)Launcher_DrawStartWindow(selected%4);
    else if(scene==1)Launcher_SettingsDrawList("Settings",SETTINGS_CATEGORY_TOTAL,selected%SETTINGS_CATEGORY_TOTAL,0,Launcher_SettingsCategoryGetLine);
    else if(scene<6) {
        const unsigned categories[]={SETTINGS_CATEGORY_INTERFACE,SETTINGS_CATEGORY_GAMES,SETTINGS_CATEGORY_HARDWARE,SETTINGS_CATEGORY_VIEW};
        unsigned category=categories[scene-2];Launcher_SettingsSelectCategory(category);
        Launcher_SettingsDrawList(Launcher_SettingsCategoryTitle(category),launcher_settings_active_total,selected%launcher_settings_active_total,0,Launcher_SettingsActiveGetLine);
    } else if(scene==6) {
        const char *lines[]={"DS Style visual reference","Actual kernel renderer","Deterministic test data","No flashcard IO"};
        Launcher_DrawHelpTextPage("Info",lines,4,0);
    } else {
        gl_show_Thumbnail=scene==7?LAUNCHER_VIEW_HORIZONTAL:LAUNCHER_VIEW_VERTICAL;
        launcher_effective_show_thumbnail=gl_show_Thumbnail;
        preview_artwork();Launcher_DrawThemeBGFull(Launcher_GetBGImage());
        if(scene==7){Launcher_PreScaleHorzCache();Draw_ModernLauncher_SD(0,1,1);}
        else {Launcher_PreScaleVertCache();Draw_ModernLauncher_SD_Vertical_State(0,1);}
        Launcher_DrawTopbarName(SD_list);Launcher_DrawTopbarTitle(SD_list,"SD Card");Launcher_DrawStartClock(1);
    }
    volatile u8 *status=(volatile u8*)0x0e000000;
    status[0]=0xd5;status[1]=scene;status[2]=0xaa;
}

static const volatile char preview_save_marker[]="SRAM_V113";
int main(void) {
    (void)preview_save_marker[0];
    irqInit();irqEnable(IRQ_VBLANK);
    SetMode(MODE_3|BG2_ON);
    REG_BG2PA=REG_BG2PD=256;REG_BG2PB=REG_BG2PC=0;REG_BG2X=REG_BG2Y=0;
    LoadEnglish();
    strcpy(launcher_system_name,"Test User");
    launcher_theme_index=0;
    unsigned scene=PREVIEW_SCENE,selected=0;
    const volatile u8 *request=(const volatile u8*)0x0e000000;
    if(request[0]==0xd6 && request[1]<9 && request[2]==0xaa)scene=request[1];
    preview_draw(scene,selected);
    while(1) {
        VBlankIntrWait();scanKeys();u16 keys=keysDown();
        if(keys&KEY_R)scene=(scene+1)%9,selected=0;
        if(keys&KEY_L)scene=(scene+8)%9,selected=0;
        if(keys&KEY_DOWN)++selected;
        if(keys&KEY_UP)selected=selected?selected-1:0;
        if(keys)preview_draw(scene,selected);
    }
}
