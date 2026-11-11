#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
typedef uint32_t u32;
typedef uint16_t u16;
typedef char TCHAR;
typedef int FRESULT;
typedef int PAGE_NUM;
#define EWRAM_BSS
#define MAX_path_len 100
#define FR_OK 0
#define SD_list 0
#define SET_win 2
#define SETTINGS_CATEGORY_SYSTEM 3
#define DSTEXT_ON "On"
#define DSTEXT_OFF "Off"
#define LAUNCHER_WRAP_OFF 0
#define LAUNCHER_WRAP_ON 1
#define LAUNCHER_WRAP_EXCEPT_SYSTEM 2
static u16 launcher_wrap_around;
static unsigned launcher_settings_migration_pending;
static const char *canonical, *legacy;
static int Launcher_SettingsReadValue(const char *key, char *buf, unsigned size) {
    const char *v = !strcmp(key,"Wrap list") ? canonical : legacy;
    if(!v) return 0;
    snprintf(buf,size,"%s",v); return 1;
}
#include "launcher_setting_parse.h"
#include "readers.inc"
#include "settings_navigation.h"
static u16 launcher_auto_theme, launcher_auto_theme_pending, launcher_dark_mode;
static unsigned repaint;
static void Launcher_ApplyThemeColours(void) { repaint++; }
#include "settings_auto_theme.h"
static u32 launcher_open_system_requested, folder_select, launcher_sd_saved_folder_select;
static u32 launcher_sd_restore_pending, recents_view_active, recents_view_favourites;
static u32 recents_return_show_offset, recents_return_file_select, recents_return_folder_select;
static u32 recents_saved_show_offset, recents_saved_file_select;
static u32 p_folder_select_show_offset[MAX_path_len], p_folder_select_file_select[MAX_path_len];
static char currentpath[MAX_path_len], launcher_sd_saved_path[MAX_path_len], recents_return_path[MAX_path_len];
static int launcher_settings_main_selected, launcher_settings_resume_category;
static int launcher_force_full_redraw, launcher_system_name_dirty, gl_clock_dirty;
static const char *missing;
static int f_chdir(const char *p) { return missing && !strcmp(missing,p) ? 1 : FR_OK; }
#include "system_browser.h"
typedef struct { int x[4], y[4], sx[4], sy[4], arm, thickness; } LauncherStartCorners;
typedef struct { int unused; } LauncherStartCornerSave;
static int gl_color_selectBG_sd;
static void Launcher_GetStartCornersForItem(u32 n, LauncherStartCorners *p) {
    memset(p,0,sizeof(*p)); for(int i=0;i<4;i++) {p->x[i]=n*40+i;p->y[i]=n*20+i;} p->arm=6+n;p->thickness=2;
}
static void Launcher_RestoreSavedStartCorners(LauncherStartCornerSave *p) {(void)p;}
static void Launcher_RestoreStartCorners(u32 n) {(void)n;}
static void Launcher_SaveStartCornersUnder(LauncherStartCorners *p,LauncherStartCornerSave *s) {(void)p;(void)s;}
static void Launcher_DrawStartCornersAtPositions(LauncherStartCorners *p,int c) {(void)p;(void)c;}
static void Launcher_DrawStartCorners(u32 a,u32 b) {(void)a;(void)b;}
#include "start_selection_motion.h"
int main(void) {
    const char *values[] = {"Off","0","No","On","1","Yes","invalid",""};
    const int expected[] = {0,0,0,1,1,1,0,0};
    canonical=legacy=NULL; launcher_wrap_around=0; Launcher_ReadWrapAroundSetting(); assert(launcher_wrap_around==1);
    for(unsigned i=0;i<8;i++) {
        canonical=values[i]; legacy="On"; Launcher_ReadWrapAroundSetting(); assert(launcher_wrap_around==expected[i]);
        canonical=NULL; legacy=values[i]; Launcher_ReadWrapAroundSetting(); assert(launcher_wrap_around==expected[i]);
    }
    for(unsigned wrap=0;wrap<2;wrap++) for(unsigned total=0;total<30;total++) for(unsigned sel=0;sel<(total?total:1);sel++) for(int d=-1;d<=1;d+=2) {
        launcher_wrap_around=wrap;
        unsigned next=Launcher_MenuMove(sel,total,d);
        if(total<2) assert(next==sel);
        else if(d>0) assert(next==(sel+1<total?sel+1:wrap?0:sel));
        else assert(next==(sel?sel-1:wrap?total-1:0));
        unsigned top=Launcher_MenuTop(next,0,7); assert(next>=top && next<top+7);
    }
    for(unsigned count=3;count<=4;count++) for(unsigned i=0;i<count;i++) assert(Launcher_MenuNextAlignment(Launcher_MenuNextAlignment(i,count,1),count,-1)==i);
    launcher_auto_theme=1;
    for(unsigned h=0;h<24;h++) for(unsigned old=0;old<2;old++) {
        launcher_dark_mode=old; unsigned want=h<8||h>=20; repaint=0;
        Launcher_QueueAutoTheme(h); assert(launcher_dark_mode==old);
        assert(Launcher_ApplyPendingAutoTheme()==(want!=old)); assert(launcher_dark_mode==want); assert(repaint==(want!=old));
    }
    launcher_auto_theme_pending=0; Launcher_QueueAutoTheme(24); assert(!launcher_auto_theme_pending);
    launcher_auto_theme_pending=2; launcher_auto_theme=0; assert(!Launcher_ApplyPendingAutoTheme());
    for(unsigned recent=0;recent<2;recent++) {
        strcpy(currentpath,"/games/deep"); strcpy(launcher_sd_saved_path,currentpath); strcpy(recents_return_path,"/return");
        folder_select=4; launcher_sd_saved_folder_select=4; recents_view_active=recent; recents_view_favourites=recent;
        p_folder_select_show_offset[4]=20; p_folder_select_file_select[4]=3;
        unsigned offset=20, selected=3;
        missing="/SYSTEM"; assert(Launcher_OpenSystemFolder(&offset,&selected,&folder_select)!=FR_OK); assert(!launcher_system_browser.active); assert(!strcmp(currentpath,"/games/deep"));
        missing=NULL; assert(!Launcher_OpenSystemFolder(&offset,&selected,&folder_select)); assert(folder_select==2 && offset==0 && selected==0 && !recents_view_active);
        strcpy(currentpath,"/SYSTEM/child"); PAGE_NUM page=SD_list; assert(!Launcher_SystemBrowserBack(&page));
        assert(!Launcher_OpenSystemFolder(&offset,&selected,&folder_select)); /* reentry retains original */
        assert(Launcher_SystemBrowserBack(&page)); assert(page==SET_win); assert(!strcmp(currentpath,"/games/deep"));
        assert(folder_select==4 && p_folder_select_show_offset[4]==20 && p_folder_select_file_select[4]==3 && recents_view_active==recent && recents_view_favourites==recent);
        assert(!Launcher_OpenSystemFolder(&offset,&selected,&folder_select)); missing="/games/deep"; Launcher_CloseSystemBrowser(); assert(!strcmp(currentpath,"/") && folder_select==1 && !recents_view_active); missing=NULL;
    }
    launcher_start_motion=&launcher_start_motion_storage;
    for(unsigned interrupt=0;interrupt<10;interrupt++) {
        memset(launcher_start_motion,0,sizeof(*launcher_start_motion)); Launcher_StartMotionBegin(0,3);
        for(unsigned i=0;i<interrupt;i++) Launcher_StartMotionTick();
        LauncherStartCorners before=launcher_start_motion->current;
        Launcher_StartMotionBegin(3,1); assert(!memcmp(&before,&launcher_start_motion->from,sizeof(before)));
        for(int i=0;i<10;i++) Launcher_StartMotionTick();
        assert(!launcher_start_motion->active); assert(!memcmp(&launcher_start_motion->current,&launcher_start_motion->to,sizeof(before)));
        Launcher_StartMotionBegin(1,2); Launcher_StartMotionSettle(); assert(!launcher_start_motion->active);
    }
    puts("PASS: wrap aliases/defaults, navigation bounds/alignment, Auto hours/deferred apply, SYSTEM restore/failure/reentry, Start easing/retarget/settle");
}
