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
  if(TEST_SCENE>=20) {
    VAR8(NATIVE_launcher_page)=2;
    VAR8(NATIVE_launcher_category)=0;
    if(TEST_SCENE>=21)key(128);
    if(TEST_SCENE==22)key(128);
    if(TEST_SCENE>=21)key(1); /* Traverse the real Settings category handler. */
    return;
  }
#if TEST_NOR_CAROUSEL
  if(TEST_SCENE>=16) {
    for(unsigned i=0;i<sizeof(native_cache_seed)/sizeof(*native_cache_seed);++i)
      REG16(NATIVE_carousel_meta+i*2)=native_cache_seed[i];
    VAR8(NATIVE_launcher_page)=0;VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB-2;
    VAR32(NATIVE_smenu+OFFSET_smenu_fbrowser_maxentries)=5;
    VAR32(NATIVE_smenu+OFFSET_smenu_fbrowser_selector)=0;
    VAR32(NATIVE_smenu+OFFSET_smenu_fbrowser_seloff)=0;
    VAR8(NATIVE_carousel_wrap)=1;VAR8(NATIVE_browser_view)=(TEST_SCENE&1)?2:1;
    VAR8(NATIVE_artwork_mode)=TEST_SCENE>=18?2:1;
    if(TEST_SCENE==17 || TEST_SCENE==18)key(32); /* First -> last through real input. */
    if(TEST_SHOWCASE)key(16); /* Begin on the middle of the three original samples. */
    return;
  }
#endif

  if(TEST_SCENE==10 || TEST_SCENE==11 || TEST_SCENE==15) {
#if TEST_RECENT
    VAR32(NATIVE_smenu+OFFSET_smenu_recent_maxentries)=1;
#if TEST_START_ART
    VAR8(NATIVE_artwork_mode)=TEST_START_ART;
    VAR8(NATIVE_artwork_cached)=VAR8(NATIVE_artwork_valid)=1;
    VAR32(NATIVE_artwork_cached_mode)=TEST_START_ART;
    REG16(NATIVE_preview_info)=TEST_START_ART==2?80:120;
    REG16(NATIVE_preview_info+2)=80;
    VAR8(NATIVE_preview_info+4)=0;
    for(unsigned i=0;i<sizeof(native_recent_path);++i)VAR8(NATIVE_artwork_rom+i)=native_recent_path[i];
#else
    VAR8(NATIVE_artwork_mode)=0; /* Caption-only fixture does not claim art I/O. */
#endif
#endif
    VAR8(NATIVE_launcher_page)=1;VAR8(NATIVE_launcher_selected)=TEST_SCENE==10?1:4;
    if(TEST_SCENE==11)key(1);
    return;
  }
  if(TEST_SCENE==12) {
    VAR8(NATIVE_smenu+OFFSET_menu_tab)=TEST_UI_TAB+2;
    VAR8(NATIVE_menu_accent)=(VAR8(NATIVE_menu_accent)+1)%5;
    ((void(*)(void))(NATIVE_settings_save_request|1u))();
    return;
  }
  if(TEST_SCENE==13 || TEST_SCENE==14) {
    /* No external font/background resource may be consulted during loading. */
    VAR32(NATIVE_font_base_addr)=0x08000000;
    const char name[]="\xe6\xbc\xa2Test";
    for(unsigned i=0;i<sizeof(name);++i)VAR8(NATIVE_menu_username+i)=name[i];
    VAR8(NATIVE_progress_action)=TEST_PROGRESS_ACTION;
    return;
  }
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
      const char *name=TEST_SHOWCASE?"/Game Boy Advance/Moonlit Trail (USA).gba":"/Game Boy Advance/P/Native menu test ROM (USA).gba";
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
  if (TEST_SCENE>=16 || !TEST_SCENE || TEST_SCENE==9 || TEST_SCENE==10 || TEST_SCENE==15)return true;
  volatile uint16_t *back=(volatile uint16_t *)(0x06000000+(VAR8(NATIVE_framen)?0xa000:0));
  for(unsigned y=21;y<152;y++) {
    const uint8_t *p=dssmm_panel_profile[TEST_THEME==7][y-21];
    if(back[y*120+1]!=panel_pair(p[0]) || back[y*120+117]!=panel_pair(p[2]) ||
       back[y*120+118]!=panel_pair(p[3]))return false;
    if(y>=23 && y<=149 && (back[y*120+2]&255)!=(TEST_THEME==7?11:0))return false;
    if(y<23 || y>=149)for(unsigned x=2;x<117;x++) {
      unsigned expected=p[1]|(p[1]<<8);
      if(x==2 && y==149)expected=(expected&0xff00)|(TEST_THEME==7?11:0);
      if(back[y*120+x]!=expected)return false;
    }
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
  VAR8(NATIVE_menu_font)=TEST_FONT;VAR8(NATIVE_menu_theme)=TEST_THEME;
  VAR8(NATIVE_menu_topbar)=TEST_TOPBAR;select_option(2);
  ((void(*)(unsigned))(NATIVE_reload_theme|1u))(VAR8(NATIVE_menu_theme));
#if TEST_REFERENCE
  const char reference_name[]="Test User";
  for(unsigned i=0;i<sizeof(reference_name);++i)VAR8(NATIVE_menu_username+i)=reference_name[i];
#endif
  if(TEST_SHOWCASE)VAR8(NATIVE_clean_list)=1;
  visual_scene();
  unsigned previous=0,seeded_pages=0;
#if TEST_WORKSPACE_SAVE
  bool workspace_requested=false,workspace_seen=false,workspace_checked=false;
  unsigned expected_clock=0;
#endif
  for(;;) {
    unsigned held=(~REG16(0x04000130))&0x3ff,pressed=held&~previous;previous=held;
    if(!TEST_SCENE && pressed)key(pressed & (16|32|64|128|2));
    if(TEST_SHOWCASE && TEST_SCENE>=16 && pressed)key(pressed & (16|32|64|128));
    VAR32(NATIVE_frame_count)++;
#if TEST_WORKSPACE_SAVE
    if(VAR32(NATIVE_frame_count)==30) {
      expected_clock=VAR8(NATIVE_menu_clock_24)^1;
      VAR8(NATIVE_menu_clock_24)=expected_clock;
      ((void(*)(void))(NATIVE_settings_save_request|1u))();
      workspace_requested=true;
    }
    if(workspace_requested && !workspace_checked) {
      ok=ok&&((bool(*)(void))(NATIVE_settings_save_poll|1u))();
      if(VAR8(NATIVE_settings_busy)) {
        workspace_seen=true;
        /* Force art composition while the real snapshot/FIL occupies the lease. */
        VAR8(NATIVE_browser_fast_valid)=VAR8(NATIVE_browser_fast_valid+1)=0;
      }
      if(!((bool(*)(void))(NATIVE_settings_save_pending|1u))()) {
        VAR8(NATIVE_menu_clock_24)^=1;
        ((void(*)(void))(NATIVE_load_settings|1u))();
        ok=ok&&workspace_seen && VAR8(NATIVE_menu_clock_24)==expected_clock;
        VAR8(NATIVE_browser_fast_valid)=VAR8(NATIVE_browser_fast_valid+1)=0;
        workspace_checked=true;
      }
    }
#endif
#if TEST_REFERENCE
    VAR32(NATIVE_clock_seconds)=12*3600+34*60+56;
    VAR32(NATIVE_clock_frame)=VAR32(NATIVE_frame_count);
#endif
    if(!TEST_SHOWCASE && TEST_SCENE && ((TEST_SCENE<16 && TEST_SCENE!=10 && TEST_SCENE!=15) || !(seeded_pages&(1u<<VAR8(NATIVE_framen))))) {
      seeded_pages|=1u<<VAR8(NATIVE_framen);
      REG16(0x050001d6)=0x7c1f; /* Detect stale artwork behind native modal panels. */
      volatile uint16_t *back=(volatile uint16_t *)(0x06000000+(VAR8(NATIVE_framen)?0xa000:0));
      for(unsigned n=16*120;n<160*120;n++)back[n]=0xebeb;
    }
    if(TEST_SCENE==13 || TEST_SCENE==14) {
      /* Exercise the retained callback: its small render helper is inlined. */
      if(TEST_SCENE==13)((void(*)(unsigned,unsigned))(NATIVE_nor_write_progress|1u))(1,2);
      else ((void(*)(unsigned,unsigned))(NATIVE_loadrom_progress|1u))(1,2);
      VAR8(NATIVE_framen)^=1; /* Callback already flipped; inspect that page. */
    }
    else ((void(*)(unsigned))(NATIVE_menu_render|1u))(1);
    ok=ok&&frame_intact();
    wait_frame();((void(*)(void))(NATIVE_menu_flip|1u))();
#if !TEST_REFERENCE
    bool passed=ok;
#if TEST_WORKSPACE_SAVE
    passed=passed&&workspace_checked;
#endif
    REG16(0x050001de)=passed?0x03e0:0x001f;
    REG16(0x050001f0)=passed?0x7c00:0x001f;
    volatile uint16_t *front=(volatile uint16_t *)(0x06000000+((REG16(0x04000000)&16)?0xa000:0));
    for(unsigned y=1;y<7;y++)for(unsigned x=second?112:116;x<120;x++)front[y*120+x]=0xf8ef;
#endif
    if(TEST_SCENE==12)for(;;)wait_frame();
  }
}
