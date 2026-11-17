/* Actual linked firmware menu/settings; physical SD only is a test fixture. */
#include <stdint.h>
#include <stdbool.h>
#include "native-settings-image.h"
#include "config.h"
#include "flash_mgr.h"
#include "res/dssmm-panel.h"
#define REG16(a) (*(volatile uint16_t *)(a))
#define VAR8(a) (*(volatile uint8_t *)(a))
#define VAR32(a) (*(volatile uint32_t *)(a))
#define SRAM ((volatile uint8_t *)0x0e000000)
static const volatile char save_marker[]="SRAM_V113";
static unsigned sd_read(uint8_t *out,uint32_t sector,unsigned count) {
  if(sector>=128 || count>128-sector)return 1;
  for(unsigned i=0;i<count*512;i++)out[i]=sector*512+i<32768?SRAM[sector*512+i]:0;
  return 0;
}
static unsigned sd_write(const uint8_t *in,uint32_t sector,unsigned count) {
  if(sector>=128 || count>128-sector)return 1;
  for(unsigned i=0;i<count*512;i++) {unsigned off=sector*512+i;if(off<32768)SRAM[off]=in[i];else if(in[i])return 1;}
  return 0;
}
static void disk16(unsigned off,unsigned value) {SRAM[off]=value;SRAM[off+1]=value>>8;}
static void format_disk(void) {
  if(SRAM[0]==0xeb && SRAM[1]==0x3c && SRAM[54]=='F' && SRAM[55]=='A' && SRAM[19]==128)return;
  for(unsigned i=0;i<32768;i++)SRAM[i]=0;
  SRAM[0]=0xeb;SRAM[1]=0x3c;SRAM[2]=0x90;
  const char *oem="DSSMM   ";for(unsigned i=0;i<8;i++)SRAM[3+i]=oem[i];
  disk16(11,512);SRAM[13]=1;disk16(14,1);SRAM[16]=2;disk16(17,32);
  disk16(19,128);SRAM[21]=0xf8;disk16(22,1);disk16(510,0xaa55);
  const char *kind="FAT12   ";for(unsigned i=0;i<8;i++)SRAM[54+i]=kind[i];
  for(unsigned n=0;n<2;n++){unsigned off=(1+n)*512;SRAM[off]=0xf8;SRAM[off+1]=SRAM[off+2]=0xff;}
}
static void patch_sd(unsigned address,void *handler) {
  address&=~1u;unsigned literal=(address+7)&~3u;
  REG16(address)=0x4b00|((literal-((address+4)&~3u))/4);REG16(address+2)=0x4718;
  VAR32(literal)=(uintptr_t)handler|1u;
}
static void bootstrap(void) {
  REG16(0x04000208)=0;REG16(0x04000200)=0;
  for(unsigned s=0;s<sizeof(native_segments)/sizeof(*native_segments);s++) {
    volatile uint8_t *out=(volatile uint8_t *)native_segments[s].address;
    for(unsigned i=0;i<native_segments[s].memsz;i++)out[i]=i<native_segments[s].filesz?native_segments[s].source[i]:0;
  }
  VAR32(NATIVE_FatFs)=NATIVE_sdfs;
  patch_sd(NATIVE_sdcard_read_blocks,sd_read);patch_sd(NATIVE_sdcard_write_blocks,sd_write);
  ((void(*)(void))(NATIVE_load_settings|1u))();
}
static void select_option(unsigned option) {
  VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB;
  VAR32(NATIVE_smenu+OFFSET_uiset_selector)=option;
  VAR32(NATIVE_smenu+OFFSET_uiset_seloff)=option>6?option-6:0;
}
static void key(unsigned keys) {((void(*)(unsigned))(NATIVE_menu_keypress|1u))(keys);}
static bool restored(void) {
  return VAR8(NATIVE_menu_theme)==6 && VAR8(NATIVE_menu_accent)==2 &&
         VAR8(NATIVE_browser_view)==1 && VAR8(NATIVE_artwork_mode)==2;
}
static const t_flash_game_entry sample_nor = {
  .gamecode=0x54534554,.gattrs=7,
  .game_name="/Game Boy Advance/P/Native menu test ROM (USA).gba"
};
static void visual_scene(void) {
  if (!TEST_SCENE)return;
  if (TEST_SCENE==9) {
    select_option(8);VAR32(NATIVE_smenu+OFFSET_uiset_seloff)=TEST_FONT?0:2;
    VAR8(NATIVE_menu_accent)=3;
    ((void(*)(unsigned))(NATIVE_reload_theme|1u))(VAR8(NATIVE_menu_theme));
    return;
  }
  if (TEST_SCENE==1) {
    VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB+2;
    VAR32(NATIVE_smenu+OFFSET_smenu_info_selector)=0;
  } else if(TEST_SCENE==2) {
    VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB+1;
    VAR32(NATIVE_smenu+OFFSET_smenu_tools_selector)=TEST_UI_TAB==4?5:4;
  } else if(TEST_SCENE==6) {
    VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB+1;
    VAR32(NATIVE_spop+OFFSET_spop_qpop_message)=(uintptr_t)"Erase existing NOR games?";
    VAR32(NATIVE_spop+OFFSET_spop_qpop_default_button)=(uintptr_t)"No";
    VAR32(NATIVE_spop+OFFSET_spop_qpop_confirm_button)=(uintptr_t)"Yes";
  } else {
    VAR8(NATIVE_smenu+OFFSET_menu_tab)=1;
    VAR8(NATIVE_browser_view)=1;
    if(TEST_SCENE==4 && TEST_UI_TAB==4) {
      VAR8(NATIVE_spop+OFFSET_spop_pop_num)=6;
      VAR32(NATIVE_spop+OFFSET_spop_p_norld_e)=(uintptr_t)&sample_nor;
    } else if(TEST_SCENE==5) {
      VAR8(NATIVE_spop+OFFSET_spop_pop_num)=3;
      VAR8(NATIVE_spop+OFFSET_spop_p_update_issfw)=1;
      VAR32(NATIVE_spop+OFFSET_spop_p_update_superfw_ver)=22;
    } else {
      VAR8(NATIVE_spop+OFFSET_spop_pop_num)=1;
      const char *name="/Game Boy Advance/P/Native menu test ROM (USA).gba";
      volatile uint8_t *out=(volatile uint8_t *)(NATIVE_spop+OFFSET_spop_p_load_i_romfn);
      for(unsigned i=0;;i++){out[i]=name[i];if(!name[i])break;}
      VAR32(NATIVE_spop+OFFSET_spop_p_load_i_gcode)=0x54534554;
      if(TEST_SCENE>=7)VAR8(NATIVE_spop+OFFSET_spop_submenu)=TEST_SCENE-6;
    }
  }
}
static uint16_t panel_pair(uint8_t packed) {
  return (packed&15)|((packed>>4)<<8);
}
static bool frame_intact(void) {
  if (!TEST_SCENE || TEST_SCENE==9)return true;
  volatile uint16_t *back=(volatile uint16_t *)(0x06000000+(VAR8(NATIVE_framen)?0xa000:0));
  for(unsigned y=21;y<152;y++) {
    const uint8_t *p=dssmm_panel_profile[0][y-21];
    if(back[y*120+1]!=panel_pair(p[0]) || back[y*120+117]!=panel_pair(p[2]) ||
       back[y*120+118]!=panel_pair(p[3]))return false;
    if(y<23 || y>=149)for(unsigned x=2;x<117;x++)
      if(back[y*120+x]!=(p[1]|(p[1]<<8)))return false;
  }
  return true;
}
static void wait_frame(void) {
  while(REG16(0x04000006)>=160){}
  while(REG16(0x04000006)<160){}
}
int main(void) {
  (void)save_marker[0];REG16(0x04000000)=0x0404;
  REG16(0x05000000)=0x001f; /* Remains red if native code crashes before validation. */
  format_disk();bootstrap();
  bool second=((unsigned(*)(const char *,void *))(NATIVE_f_stat|1u))("/SYSTEM/SETTINGS.TXT",0)==0;
  bool ok;
  if(!second) {
    ((unsigned(*)(const char *))(NATIVE_f_mkdir|1u))("/SYSTEM");
    VAR8(NATIVE_menu_theme)=6;VAR8(NATIVE_menu_font)=1;
    VAR8(NATIVE_menu_accent)=0;VAR8(NATIVE_browser_view)=0;VAR8(NATIVE_artwork_mode)=1;
    select_option(0);key(16);key(32); /* Theme Dark then Light. */
    select_option(2);key(16); /* Title to Box. */
    select_option(8);key(16);key(16); /* Accent Green. */
    select_option(3);key(16); /* Horizontal browser. */
    key(2); /* Real menu exit invokes the real settings save transaction. */
    ok=((bool(*)(void))(NATIVE_settings_save_poll|1u))();
    ok=ok&&((bool(*)(void))(NATIVE_settings_save_pending|1u))();
    VAR8(NATIVE_launcher_page)=0;select_option(2);key(128);
    ok=ok&&VAR32(NATIVE_smenu+OFFSET_uiset_selector)==3;
    ok=ok&&((bool(*)(void))(NATIVE_settings_save_pending|1u))();
    ok=ok&&((bool(*)(void))(NATIVE_settings_save_flush|1u))() && restored();bootstrap();ok=ok&&restored();
  } else ok=restored();
  VAR8(NATIVE_menu_font)=TEST_FONT;select_option(2);
  ((void(*)(unsigned))(NATIVE_reload_theme|1u))(VAR8(NATIVE_menu_theme));
  visual_scene();
  unsigned previous=0;
  for(;;) {
    unsigned held=(~REG16(0x04000130))&0x3ff,pressed=held&~previous;previous=held;
    if(!TEST_SCENE && pressed)key(pressed & (16|32|64|128|2));
    VAR32(NATIVE_frame_count)++;
    if(TEST_SCENE) {
      REG16(0x050001d6)=0x7c1f; /* Detect stale artwork behind native modal panels. */
      volatile uint16_t *back=(volatile uint16_t *)(0x06000000+(VAR8(NATIVE_framen)?0xa000:0));
      for(unsigned n=16*120;n<160*120;n++)back[n]=0xebeb;
    }
    ((void(*)(unsigned))(NATIVE_menu_render|1u))(1);
    ok=ok&&frame_intact();
    wait_frame();((void(*)(void))(NATIVE_menu_flip|1u))();
    REG16(0x050001de)=ok?0x03e0:0x001f;
    REG16(0x050001f0)=ok?0x7c00:0x001f;
    volatile uint16_t *front=(volatile uint16_t *)(0x06000000+((REG16(0x04000000)&16)?0xa000:0));
    for(unsigned y=1;y<7;y++)for(unsigned x=second?112:116;x<120;x++)front[y*120+x]=0xf8ef;
  }
}
