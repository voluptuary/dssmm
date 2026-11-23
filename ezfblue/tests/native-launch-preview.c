/* Native SD launch control-flow fixture; storage/flash/transfer are explicit seams. */
#define main dssmm_kernel_main
#include "../build/native-launch/kernel.c"
#undef main
static const volatile char save_marker[]="SRAM_V113";
#define TRACE ((volatile u8*)0x0e000000)
static unsigned directory_cursor;
static const char test_name[]=PREVIEW_TEST_NAME;
static void phase(unsigned bit){TRACE[1]|=bit;}
FRESULT __wrap_f_opendir(DIR *dir,const TCHAR *path){(void)dir;(void)path;directory_cursor=0;phase(1);return FR_OK;}
FRESULT __wrap_f_readdir(DIR *dir,FILINFO *info){(void)dir;memset(info,0,sizeof(*info));if(!directory_cursor++){strcpy(info->fname,test_name);if(!PREVIEW_NO_ALIAS)strcpy(info->altname,"NATIVE~1.GBA");info->fsize=32768;}return FR_OK;}
FRESULT __wrap_f_readdir_with_offset(DIR *dir,FILINFO *info,DWORD *offset){FRESULT result=__wrap_f_readdir(dir,info);if(offset)*offset=info->fname[0]?0x20:0xffffffff;return result;}
FRESULT __wrap_f_closedir(DIR *dir){(void)dir;return FR_OK;}
FRESULT __wrap_f_chdir(const TCHAR *path){(void)path;return FR_OK;}
FRESULT __wrap_f_getcwd(TCHAR *path,UINT size){if(size<2)return FR_INVALID_PARAMETER;strcpy(path,"/");return FR_OK;}
FRESULT __wrap_f_stat(const TCHAR *path,FILINFO *info){(void)path;(void)info;return FR_NO_FILE;}
FRESULT __wrap_f_open(FIL *file,const TCHAR *path,BYTE mode){(void)mode;if(strcmp(path,test_name))return FR_NO_FILE;memset(file,0,sizeof(*file));file->obj.objsize=32768;phase(2);return FR_OK;}
FRESULT __wrap_f_read(FIL *file,void *out,UINT count,UINT *got){memset(out,0,count);if(!file->fptr&&count>=0xc0){memcpy((u8*)out+0xac,"TEST",4);((u8*)out)[0xb2]=0x96;}file->fptr+=count;*got=count;phase(4);return FR_OK;}
FRESULT __wrap_f_close(FIL *file){(void)file;return FR_OK;}
FRESULT __wrap_f_mkdir(const TCHAR *path){(void)path;return FR_EXIST;}
u16 __wrap_Read_SET_info(u32 offset){(void)offset;return 0;}
u16 __wrap_Read_sav_info(u32 offset){(void)offset;return 0;}
u8 __wrap_Check_mde_file(TCHAR *name){(void)name;return 0;}
u32 __wrap_Check_cheat_file(TCHAR *name){(void)name;return 0;}
void __wrap_Set_64MROM_flag(u16 value){TRACE[4]=value;}
void __wrap_Send_FATbuffer(u32 *buffer,u32 mode){(void)buffer;TRACE[5]=mode;phase(32);}
void __wrap_GBApatch_Cleanrom(u32 *address,int size){if(address==(u32*)PSRAMBase_S98&&size==32768)phase(64);}
void __wrap_GBApatch_DDR(u32 *address,int size,u32 page){if(address==(u32*)PSRAMBase_S98&&size==32768&&page==0x400)phase(64);}
void __wrap_rtc_enable(void){}
void __wrap_rtc_disenable(void){}
int __wrap_rtc_gettime(u8 *time){time[0]=0x12;time[1]=0x34;time[2]=0x56;return 0;}
u8 Process_savefile(u32 emu,TCHAR *name,u32 size,BYTE mode){(void)mode;if(!emu&&!strcmp(name,test_name)&&size==32768)phase(8);return 0;}
void init_FAT_table(void){memset(FAT_table_buffer,0,sizeof(FAT_table_buffer));}
u32 Check_game_RTS_FAT(TCHAR *name,u32 kind){if(!strcmp(name,test_name)&&kind==1)phase(16);return 0;}
void Make_recently_play_file(TCHAR *path,TCHAR *name){(void)path;(void)name;}
static void Save_last_launch_mode(u32 mode){(void)mode;}
void __wrap_Blue_BootNative(u16 page,u32 hard_reset){
 if(page==0x400&&hard_reset==0)phase(128);
 TRACE[2]=0xaa;
 for(unsigned i=0;i<240*160;++i)((volatile u16*)VideoBuffer)[i]=TRACE[1]==(PREVIEW_SCENE==2?191:255)?RGB(0,31,0):RGB(31,0,0);
 for(;;)VBlankIntrWait();
}
int main(void){
 (void)save_marker[0];irqInit();irqEnable(IRQ_VBLANK);SetMode(MODE_3|BG2_ON);
 REG_BG2PA=REG_BG2PD=256;REG_BG2PB=REG_BG2PC=0;REG_BG2X=REG_BG2Y=0;
 LoadEnglish();launcher_theme_index=0;Launcher_ApplyThemeColours();
 strcpy(launcher_system_name,"Launch test");strcpy(currentpath,"/");
 folder_total=0;game_total_SD=1;launcher_active_page=SD_list;
 gl_boot_mode_pref=PREVIEW_SCENE;gl_toggle_reset=0;
 for(unsigned i=0;i<8;++i)TRACE[i]=0;
 TRACE[0]=0xd7;
 (void)SD_list_MENU(0,0,0xbb);TRACE[2]=0xee;
 for(;;)VBlankIntrWait();
}
