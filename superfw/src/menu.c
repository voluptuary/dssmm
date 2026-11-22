/*
 * Copyright (C) 2024 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "compiler.h"
#include "gbahw.h"
#include "patchengine.h"
#include "fatfs/ff.h"
#include "common.h"
#include "settings.h"
#include "loader_layout.h"
#include "artwork_file.h"
#include "browser_carousel.h"
#include "browser_motion.h"
#include "dssmm_background.h"
#include "update_unlock.h"
#include "util.h"
#include "utf_util.h"
#include "fonts/font_render.h"
#include "nanoprintf.h"
#include "messages.h"
#include "save.h"
#include "cheats.h"
#include "ingame.h"
#include "emu.h"
#include "recent.h"
#include "flash_mgr.h"
#include "flash.h"
#include "sha256.h"
#include "supercard_driver.h"

#include "res/icons.h"
#include "res/icon-gamegear.h"
#include "res/logo.h"
#include "res/dssmm-panel.h"

extern t_card_info sd_info;
extern bool fastew;
extern bool slowsd;

enum {
  MENUTAB_RECENT,        // Browses recently loaded ROMs (can be disabled / hidden)
  MENUTAB_ROMBROWSE,     // Browses ROMs and launches games.
  #ifdef SUPPORT_NORGAMES
  MENUTAB_NORBROWSE,     // Browses Flash games and launches them.
  #endif
  MENUTAB_SETTINGS,      // General settings / defaults
  MENUTAB_UILANG,        // UI / Language settings
  MENUTAB_TOOLS,         // Tools (advaned menu)
  MENUTAB_INFO,          // Info / About / Updater?
  MENUTAB_MAX,
};

#define ANIM_INITIAL_WAIT     128    // Intial wait (in anim cycles)

#define KEY_REPEAT_INITIAL    384    // Initial wait for key repeat
#define KEY_REPEAT_MID        160    // Next key presses
#define KEY_REPEAT_FAST        96    // End speed
#define KEY_REPEAT_CNT1         5

enum {
  POPUP_NONE,
  POPUP_GBA_LOAD,              // Load a GBA ROM
  POPUP_SAVFILE,               // Load/Store a SAV file
  POPUP_FWFLASH,               // Flash a new firmware image
  POPUP_FILE_MGR,              // Write ROM to flash, delete, hide/unhide...
#ifdef SUPPORT_NORGAMES
  POPUP_GBA_NORWRITE,          // Write a GBA ROM to NOR
  POPUP_GBA_NORLOAD,           // Launch a NOR game
#endif
};

#define BROWSER_ROWS                 8
#define RECENT_ROWS                  9
#define NORGAMES_ROWS                8

// First entries reserved for the logo palette.
#define FG_COLOR         16
#define BG_COLOR         17
#define FT_COLOR         18
#define HI_COLOR         19
#define TOP_BG          246
#define TOP_TEXT        247
#define SELECTED_TEXT   236
#define IGM_PAL_FG      240
#define IGM_PAL_BG      241
#define IGM_PAL_HI      242
#define IGM_PAL_SH      243
#define IGM_PAL_BL      244
#define IGM_PAL_FONT    (240+IGM_FONT_PALETTE_SLOT)
#define SEL_COLOR       255

#define FLASH_GO_KEYS          (KEY_BUTTUP|KEY_BUTTL|KEY_BUTTR)

enum {
  UiSetTheme = 0,
  UiSetFont,
  UiSetArtwork,
  UiSetView,
  UiSetLang,
  UiSetRect,
  UiSetASpd,
  UiSetHid,
  UiSetAccent,
  UiSetTopBar,
  UiSetClean,
  UiSetClock,
  UiSetWrap,
  UiSetSave, /* retired sentinel; no selectable Save action */
  UiSetMAX = UiSetWrap,
};

enum {
  ToolsSDRAMTest = 0,
  ToolsSRAMTest,
  ToolsBatteryTest,
  ToolsSDBench,
  ToolsFlashBak,
  #ifdef SUPPORT_NORGAMES
  ToolsFlashClr,
  #endif
  ToolsMAX,
};

enum {
  SettTitle1 = 0,
  SettHotkey,
  SettBootType,
  SettFastSD,
  #ifdef SUPPORT_NORGAMES
  SettVerifyNOR,
  #endif
  SettFastEWRAM,
  SettSaveBkp,
  SettCheatEn,
  SettTitle2,
  DefsPatchEng,
  DefsGamMenu,
  DefsRTCEnb,
  DefsRTCVal,
  DefsRTCSpeed,
  DefsLoadPol,
  DefsSavePol,
  DefsPrefDS,
  SettSave, /* retired sentinel; no selectable Save action */
  SettMAX = SettSave
};

enum {
  DefsSave     = 4,
  DefsMAX      = 4,
};

enum {
  GbaLoadPopInfo  = 0,
  GbaLoadPopLoadS = 1,
  GbaLoadPopPatch = 2,
  GbaLoadCNT      = 3,

  GbaNorWrPatch   = 1,
  GbaNorWrCNT     = 2,

  GbaNorLoad      = 1,
  GbaNorLoadCNT   = 2,
};

enum {
  GBAInfoCNT   = 1,
  GBALoadButt  = 0,

  GBALdSetCNT    = 5,
  GBALdSetLoadP  = 0,
  GBALdSetSaveP  = 1,
  GBALdSetRTC    = 2,
  GBALdSetCheats = 3,
  GBALdRemember  = 4,

  GBAPatchCNT  = 5,
  GBALoadPatch = 0,
  GBASavePatch = 1,
  GBAInGameMen = 2,
  GBARTCPatch  = 3,
  GBAPatchGen  = 4,
};

enum {
  SaveWrite  = 0,
  SavLoad    = 1,
  SavClear   = 2,
  SavQuit    = 3,
  SavMAX     = 3,
};

enum {
  FlashingReady    = 0,
  FlashingLoading  = 1,
  FlashingChecking = 2,
  FlashingErasing  = 3,
  FlashingWriting  = 4,
};

enum {
  FiMgrDelete,
  FiMgrHide,
#ifdef SUPPORT_NORGAMES
  FiMgrWriteNOR,
#endif
  FiMgrCNT
};

typedef struct {
  // ROM information
  char romfn[MAX_FN_LEN];             // File to load/write
  uint32_t romfs;                     // File ROM size
  char gcode[5];                      // ASCII sanitized game code.
  t_rom_header romh;                  // ROM header (for info purposes)
  // Patching info
  t_patch patches_datab;              // Loaded patches (from DB)
  t_patch patches_cache;              // Loaded patches (from patch engine's cache)
  bool patches_datab_found;           // Whether we had a patch match in the database
  bool patches_cache_found;           // Same but for the patch cache
  // Patching configuration
  t_patch_policy patch_type;          // Patching type
  bool use_dsaving;                   // Whether we use direct-saving mode
  bool ingame_menu_enabled;           // Enable the in-game menu.
  bool rtc_patch_enabled;             // Patch for RTC workarounds.
} t_load_gba_info;

typedef struct {
  // Save read/write policies and info
  t_sram_load_policy sram_load_type;  // SRAM loading policy
  t_sram_save_policy sram_save_type;  // SRAM auto-saving policy
  char savefn[MAX_FN_LEN];            // Save file path.
  bool savefile_found;                // Whether there's a .sav file.
  // RTC config
  uint32_t rtcval;                    // Initial RTC value.
  // Cheats policy
  bool use_cheats;                    // Whether we want to load cheats to use them.
  bool cheats_found;                  // Whether there's a cheats file (not parsed tho!)
  unsigned cheats_size;               // Size of the cheat buffer
  char cheatsfn[MAX_FN_LEN];          // Cheats file path.
} t_load_gba_lcfg;

typedef void (*t_mrender_fn)(volatile uint8_t *frame);
typedef void (*t_mkeyupd_fn)(unsigned newkeys);

// Info and state for the menu tab
static struct {
  uint8_t menu_tab;

  unsigned anim_state;            // Animation (text rotation) status.

  // Recent ROMs state
  struct {
    int selector;                 // Pointed file offset
    int seloff;                   // Entry at the top of the list
    int maxentries;               // Total file/dir count in current dir
  } recent;

  // ROM browser state
  struct {
    char cpath[MAX_FN_LEN];       // Current path
    int selector;                 // Pointed file offset
    int seloff;                   // Entry at the top of the list
    int filecount;                // Total file/dir count in current dir (unfiltered)
    int dispentries;              // Maximum number of visible entries (filtered)
    uint16_t selhist[16];         // History of directory offsets
  } browser;

  // Flash ROM browser state
  struct {
    int selector;                 // Pointed file offset
    int seloff;                   // Entry at the top of the list
    uint8_t maxentries;           // Total file/dir count in current dir
    uint8_t usedblks, freeblks;   // NOR usage info
  } fbrowser;

  // UI settings
  struct {
    int selector;                 // Pointed option
    int seloff;                   // First visible option
  } uiset;

  // Main settings
  struct {
    int selector;                 // Pointed option
  } set;

  // Tools menu
  struct {
    int selector;                 // Render panel
  } tools;

  // Info/About menu
  struct {
    int selector;                 // Render panel
    char tstr[64];                // Temp message render
  } info;
} smenu;

/* Start/settings navigation is independent of native tab numbering. */
static uint8_t launcher_page,launcher_selected=1,launcher_category,launcher_settings_group,launcher_browser_panel=1;
static bool launcher_valid[2];
static bool restart_required;
static void launcher_invalidate(void) {launcher_valid[0]=launcher_valid[1]=false;}

// Same but for popups.
static struct {
  const char *alert_msg;          // Extra pop-up message

  uint8_t pop_num;                // Current pop-up in display
  char submenu;                   // Which submenu tab we are in (if any)
  char selector;                  // Option selector (if any)
  unsigned anim;                  // Animation state

  // Pop up message (for whatever action). Allows returning to previous popup.
  struct {
    const char *message;
    const char *default_button;
    const char *confirm_button;
    void (*callback)(bool confirm);       // Function to call on "confirm".
    uint8_t option;                       // Selected button
    bool clear_popup_ok;                  // Whether any pop up must be cleared.
  } qpop;

  // RTC time set pop up, a bit special.
  struct {
    t_dec_date val;
    int selector;
    bool menu_clock;
    void (*callback)();                   // Function to call on "save"
  } rtcpop;

  union {
    // GBA launch ROM pop up menu
    struct {
      t_load_gba_info i;                  // ROM/Patch info and patch policy.
      t_load_gba_lcfg l;                  // ROM loading info and settings;
    } load;

    // Write GBA game to NOR memory
    struct {
      t_load_gba_info i;                  // ROM/Patch info and patch policy.
    } norwr;
    // Launch GBA game from NOR memory
    struct {
      t_load_gba_lcfg l;                  // ROM loading info and settings;
      const t_flash_game_entry *e;        // NOR game entry on RAM
    } norld;

    // Save file menu (.sav files)
    struct {
      char savfn[MAX_FN_LEN];             // SAV file to load/store/mangle
    } savopt;
    // Update menu (for .fw files)
    struct {
      char fn[MAX_FN_LEN];                // FW file to load and flash
      bool issfw;                         // The firmware is a superFW image.
      uint32_t superfw_ver;               // Reported FW version.
      uint32_t fw_size;                   // Size in bytes reported by stat.
      unsigned curr_state;                // Flashing FSM state.
    } update;

    // Not really a pop up, but used as "popup" data for menu questions.
    struct {
      char fn[MAX_FN_LEN];
      unsigned fs;
    } pdb_ld;
  } p;
} spop;

typedef struct {
  uint32_t filesize;
  uint16_t isdir;
  uint16_t attr;
  char fname[MAX_FN_LEN];
  uint16_t sortname[MAX_FN_LEN];       // Pre-decoded and sort-friendly name.
} t_centry;
_Static_assert (sizeof(t_centry) % 4 == 0, "t_centry must be word-friendly");

// Pointer to SDRAM, where we place some data:
//  - Scratch area 2MiB (for FW updates)
//  - File list order (~64KiB)
//  - File list filtered (~64KiB)
//  - Browser file information (~13MB)
//  - Recently played ROMs table (~64KiB)
//  - Font data (placed by the bootloader at the 15..16MB range)
// At the end of the SDRAM, ro-data can be loaded by the loader.
#define scratch_mem_size (2*1024*1024)
typedef struct {
  uint8_t scratch[scratch_mem_size];
  t_centry *fileorder[BROWSER_MAXFN_CNT];
  t_centry *filefilter[BROWSER_MAXFN_CNT];
  t_centry fentries[BROWSER_MAXFN_CNT];
  t_rentry rentries[RECENT_MAXFN_CNT];
  t_reg_entry_max nordata;
  struct {
    uint16_t pixels[ARTWORK_MAX_PIXELS / 2];
  } carousel_art[ARTWORK_CACHE_SLOTS];
  uint16_t artwork_pixels[ARTWORK_MAX_PIXELS / 2];
  uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE] __attribute__((aligned(4)));
} t_sdram_state;

_Static_assert (sizeof(t_sdram_state) <= 14.5*1024*1024, "scratch SDRAM doesn't exceed 14.5MB");

t_sdram_state *sdr_state = (t_sdram_state*)0x08000000;
uint8_t *hiscratch = (uint8_t*)ROM_HISCRATCH_U8;

typedef struct {
  uint16_t x, y;
  unsigned tn;
} t_oamobj;

static bool enable_flashing = false;
static update_unlock_state update_unlock;
static uint16_t fresh_keys;
static unsigned framen = 0;
static unsigned objnum = 0;
static t_oamobj fobjs[64];

#pragma GCC optimize ("Os")

unsigned lang_lookup(uint16_t code) {
  for (unsigned i = 0; i < LANG_COUNT; i++)
    if (lang_codes[i] == code)
      return i;

  return 0;  // Fallback to default (english)
}

static inline uint16_t curr_pressed_keys() {
  return REG_KEYINPUT ^ 0x3FF;
}

uint16_t lang_getcode() {
  return lang_codes[lang_id];
}

static inline bool isascii(char code) {
  // Abuse signed :D
  return code >= 32;
}

// Uses the FAT attribute and also the unix filename convention.
static inline bool ishidden(const t_centry *e) {
  return (e->attr & AM_HID) || e->fname[0] == '.';
}

bool is_superfw(const t_rom_header *h) {
  return !memcmp(&h->data[SUPERFW_COMMENT_DOFFSET], "SUPERFW~DAVIDGF", 16);
}

static int strcmp16(const uint16_t *a, const uint16_t *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a - *b;
}

NOINLINE int filesort(const void *a, const void *b) {
  const t_centry *ca = *(t_centry**)a;
  const t_centry *cb = *(t_centry**)b;

  // Directories some up first.
  if (ca->isdir != cb->isdir)
    return cb->isdir - ca->isdir;

  // Other files are string-ordered
  return strcmp16(ca->sortname, cb->sortname);
}

NOINLINE int romsort(const void *a, const void *b) {
  const t_flash_game_entry *ca = (t_flash_game_entry*)a;
  const t_flash_game_entry *cb = (t_flash_game_entry*)b;

  return strcasecmp(&ca->game_name[ca->bnoffset], &cb->game_name[cb->bnoffset]);
}

static void loadrom_progress(unsigned done, unsigned total) {
  // Draws and flips the buffer, do not care about vsync here
  volatile uint8_t *frame = &MEM_VRAM_U8[0xA000*framen];

  // Render the full background to a solid color
  dma_memset16(&frame[0], dup8(BG_COLOR), SCREEN_WIDTH*SCREEN_HEIGHT/2);

  // Render a simple progress bar
  unsigned prog=total?(unsigned)((uint64_t)MIN(done,total)*200/total):0;
  for (unsigned i = 76; i < 84; i++)
    dma_memset16(&frame[SCREEN_WIDTH * i + 20], dup8(FG_COLOR), prog/2);

  dma_memset16(MEM_OAM, 0, 256);  // Clear icons

  REG_DISPCNT = (REG_DISPCNT & ~0x10) | (framen << 4);
  framen ^= 1;
}

static bool loadrom_progress_abort(unsigned done, unsigned total) {
  loadrom_progress(done, total);

  // Capture A/B buttons to abort the progress
  return ((~REG_KEYINPUT) & KEY_BUTTSTA);
}


bool generate_patches_progress(const char *fn, unsigned fs) {
  FIL fd;
  if (f_open(&fd, fn, FA_READ) != FR_OK) return false;
  if (f_size(&fd) != fs) {f_close(&fd);return false;}

  t_patch_builder pb;
  patchengine_init(&pb, fs);
  const unsigned capacity = 8*1024*1024;
  const unsigned step = capacity - PATCH_SCAN_LOOKBEHIND - PATCH_SCAN_LOOKAHEAD;

  for (unsigned scan = 0; scan < fs; scan += step) {
    unsigned start = scan > PATCH_SCAN_LOOKBEHIND ? scan-PATCH_SCAN_LOOKBEHIND : 0;
    unsigned available = MIN(capacity, fs-start);
    unsigned begin = scan-start, count = MIN(step, fs-scan);
    if (f_lseek(&fd, start) != FR_OK) goto read_failed;
    for (unsigned j = 0; j < available; j += 4096) {
      UINT got;
      uint32_t tmp[4096/4];
      unsigned expected = MIN(sizeof(tmp), available-j);
      if (f_read(&fd, tmp, expected, &got) != FR_OK || got != expected) goto read_failed;
      unsigned padded = (expected+3)&~3u;
      memset((uint8_t*)tmp+expected, 0, padded-expected);
      set_supercard_mode(MAPPED_SDRAM, true, false);
      dma_memcpy32(&hiscratch[j], tmp, padded/4);
      set_supercard_mode(MAPPED_SDRAM, true, true);
      if (!(j & 0xffff)) loadrom_progress((scan*2+j)>>8, MAX(1,fs>>7));
    }
    void upd_pe_prog(unsigned progress) {
      loadrom_progress((scan*2+count+progress*4)>>8, MAX(1,fs>>7));
    }
    pb.rom_offset = start;
    set_supercard_mode(MAPPED_SDRAM, true, false);
    bool ok = patchengine_process_window((uint32_t*)hiscratch, available,
                                         begin, begin+count, &pb, upd_pe_prog);
    set_supercard_mode(MAPPED_SDRAM, true, true);
    if (!ok) goto read_failed;
  }
  if (f_close(&fd) != FR_OK) return false;
  patchengine_finalize(&pb);

  WRITE_LOG("Patch engine done. Found wcnt: %d save: %d (save mode: %d) irqh: %d rtc: %d",
            pb.p.wcnt_ops, pb.p.save_ops, pb.p.save_mode, pb.p.irqh_ops, pb.p.rtc_ops);

  // Proceed to write patches to their cache.
  return write_patches_cache(fn, &pb.p);

read_failed:
  f_close(&fd);
  return false;
}

bool dump_flashmem_backup() {
  f_mkdir(SUPERFW_DIR);

  // Use a different file name to ensure we do not overwrite firmwares by
  // accident. This adds some minimal overhead.
  SHA256_State st;
  sha256_init(&st);

  FIL fd;
  FRESULT res = f_open(&fd, FLASHBACKUPTMP_FILEPATH, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK)
    return false;

  const unsigned fsize = flashinfo.size ? flashinfo.size : FW_MAX_SIZE_KB*1024;
  for (unsigned i = 0; i < fsize; i += 4*1024) {
    const uint8_t *faddr = (uint8_t*)(ROM_FLASHFIRMW_ADDR + i);

    uint32_t tmp[4096/4];
    set_supercard_mode(MAPPED_FIRMWARE, true, false);
    dma_memcpy32(tmp, faddr, 1024);
    set_supercard_mode(MAPPED_SDRAM, true, true);

    sha256_transform(&st, tmp, sizeof(tmp));

    UINT wrbytes;
    if (FR_OK != f_write(&fd, tmp, sizeof(tmp), &wrbytes) || wrbytes != sizeof(tmp)) {
      f_close(&fd);
      return false;
    }

    loadrom_progress(i >> 10, fsize >> 10);
  }

  f_close(&fd);

  // Calculate the final hash, use a hash prefix as the filename.
  uint8_t h256[32];
  sha256_finalize(&st, h256);

  char finalfn[64];
  npf_snprintf(finalfn, sizeof(finalfn), FLASHBACKUP_FILEPTRN,
               h256[0], h256[1], h256[2], h256[3]);
  f_rename(FLASHBACKUPTMP_FILEPATH, finalfn);

  return true;
}

#ifdef SUPPORT_NORGAMES
static bool gba_write_nor;
static bool nor_replace_games;
static void request_nor_write(void);
#endif

void patch_gen_callback(bool confirm);

void sram_battery_test_callback(bool confirm) {
  if (confirm) {
    // Fill SRAM with some pseudorandom data to test later.
    sram_pseudo_fill();
    // Program a check on the next reboot!
    program_sram_check();

    spop.alert_msg = msgs[lang_id][MSG_SRAMTST_RDY];
  }
}


static const t_patch * get_game_patch(const t_load_gba_info *info) {
  return info->patch_type == PatchDatabase && info->patches_datab_found ? &info->patches_datab :
         info->patch_type == PatchEngine   && info->patches_cache_found ? &info->patches_cache : NULL;
}

bool ingame_menu_avail_sdram(const t_load_gba_info *info) {
  const t_patch *p = get_game_patch(info);
  uint32_t required;
  t_loader_layout layout;
  if (!loader_menu_size(ingame_menu_runtime_size,font_block_size(),spop.p.load.l.cheats_size,&required) ||
      !loader_plan(info->romfs,required,0,p?p->hole_addr:0,p?p->hole_size:0,&layout))
    return false;

  // Check if the patches exist and have proper IRQ support.
  return p && p->irqh_ops > 0;
}

bool ingame_menu_avail_flash(const t_load_gba_info *info) {
  const t_patch *p = get_game_patch(info);

  // Checks if the ROM is small enough so the last 4MiB block can be remapped.
  if (info->romfs > MAX_GBA_ROM_SIZE - NOR_BLOCK_SIZE) {
    // Otherwise find a gap to flash on NOR our tiny payload
    if (!p || (p->hole_addr & 3) || p->hole_addr > info->romfs ||
        p->hole_size > info->romfs - p->hole_addr || p->hole_size < DIRSAVE_REQ_SPACE + ingame_trampoline_payload_size)
      return false;   // Too big to fit!
  }

  // Check if the patches exist and have proper IRQ support.
  return p && p->irqh_ops > 0;
}

// Calculates whether DirectSaving can be used given some information.
bool dirsav_avail_sdram(const t_load_gba_info *info) {
  const t_patch *p = get_game_patch(info);

  // Check if there's enough space for it! (Placing it at the end).
  if (info->romfs > MAX_GBA_ROM_SIZE - DIRSAVE_REQ_SPACE) {
    if (!p || p->hole_size < DIRSAVE_REQ_SPACE || p->hole_addr + p->hole_size > info->romfs)
      return false;   // Too big to fit!
  }

  return (p && supports_directsave(p->save_mode));
}

bool dirsav_avail_flash(const t_load_gba_info *info) {
  const t_patch *p = get_game_patch(info);

  // Checks if the ROM is small enough so the last 4MiB block can be remapped.
  if (info->romfs > MAX_GBA_ROM_SIZE - NOR_BLOCK_SIZE) {
    // Otherwise find a gap to flash on NOR our tiny payload
    if (!p || (p->hole_addr & 3) || p->hole_addr > info->romfs ||
        p->hole_size > info->romfs - p->hole_addr || p->hole_size < DIRSAVE_REQ_SPACE)
      return false;   // Too big to fit!
  }

  return (p && supports_directsave(p->save_mode));
}

bool rtcemu_avail(const t_load_gba_info *info) {
  const t_patch *p = get_game_patch(info);
  return (p && p->rtc_ops);
}

static bool prepare_gba_info(
  t_load_gba_info *info, const t_rom_load_settings *st,
  const char *fn, uint32_t fs,
  bool load_sdram
) {
  // Pre-load ROM header
  if (preload_gba_rom(fn, fs, &info->romh))
    return false;

  // Fill/copy ROM info.
  if (fn != info->romfn)
    strcpy(info->romfn, fn);
  info->romfs = fs;

  // Sanitize the game code for display
  for (unsigned i = 0; i < 4; i++)
    info->gcode[i] = isascii(info->romh.gcode[i]) ? info->romh.gcode[i] : 0x1A;
  info->gcode[4] = 0;

  // Look up patches, have them handy.
  uint8_t gamecode[5] = {
    info->romh.gcode[0], info->romh.gcode[1],
    info->romh.gcode[2], info->romh.gcode[3],
    info->romh.version
  };
  set_supercard_mode(MAPPED_SDRAM, true, false);
  info->patches_datab_found = patchmem_lookup(gamecode, (uint8_t*)ROM_PATCHDB_U8, &info->patches_datab);
  set_supercard_mode(MAPPED_SDRAM, true, true);

  if (!info->patches_datab_found)
    WRITE_LOG("No patches in PatchDB found for '%s' with gamecode %c%c%c%c-%d",
              fn, gamecode[0], gamecode[1], gamecode[2], gamecode[3], (int)gamecode[4]);

  // Attempt to load any existing patch and check also the PE cache dir.
  info->patches_cache_found = load_rom_patches(fn, &info->patches_cache);
  if (!info->patches_cache_found) {
    WRITE_LOG("No patch file found for '%s'", fn);
    info->patches_cache_found = load_cached_patches(fn, &info->patches_cache);
    if (!info->patches_cache_found)
      WRITE_LOG("No patch file found in patches cache dir for '%s'", fn);
  }

  // If PatchAuto is selected, resolve it. Downgrade if not found.
  if (st->patch_policy == PatchAuto) {
    if (info->patches_cache_found)
      info->patch_type = PatchEngine;      // Try existing patches
    else if (info->patches_datab_found)
      info->patch_type = PatchDatabase;    // Try the database then
    else
      info->patch_type = PatchNone;
  }
  else {
    // Use the specified patch type.
    info->patch_type = st->patch_policy;

    // Downgrade to no patches if the specified was not found.
    if ((info->patch_type == PatchDatabase && !info->patches_datab_found) ||
        (info->patch_type == PatchEngine   && !info->patches_cache_found))
      info->patch_type = PatchNone;
  }

  // Fill defaults as requested if possible.
  bool allowds = load_sdram ? dirsav_avail_sdram(info) : dirsav_avail_flash(info);
  bool allowigm = load_sdram ? ingame_menu_avail_sdram(info) : ingame_menu_avail_flash(info);

  info->rtc_patch_enabled = st->use_rtc && rtcemu_avail(info);
  info->use_dsaving = st->use_dsaving && allowds;
  info->ingame_menu_enabled = st->use_igm && allowigm;

  return true;
}

static void prepare_gba_cheats(const char *gcode, uint8_t ver, t_load_gba_lcfg *data, const char *fn, bool prefer_cheats) {
  // Attempt to find a cheat file if cheats are enabled.
  data->cheats_size = 0;
  data->cheats_found = false;
  if (enable_cheats) {
    strcpy(data->cheatsfn, fn);
    replace_extension(data->cheatsfn, ".cht");
    data->cheats_found = check_file_exists(data->cheatsfn);
    if (!data->cheats_found) {
      WRITE_LOG("No cheat file found at '%s'", data->cheatsfn);
      // Create a path using the game ID and version.
      npf_snprintf(data->cheatsfn, sizeof(data->cheatsfn), CHEATS_PATH "%c%c%c%c-%02x.cht",
                   gcode[0], gcode[1], gcode[2], gcode[3], ver);
      data->cheats_found = check_file_exists(data->cheatsfn);
      WRITE_LOG("No cheat file found at '%s'", data->cheatsfn);

      // Load the cheats into memory if enabled.
      if (data->cheats_found) {
        // Load the cheats to the ROM area, just after the font pack. This is for easier relocation.
        uint8_t *cheat_area = (uint8_t*)(ROM_FONTBASE_U8 + font_block_size());
        unsigned max_area = 1536*1024 - font_block_size();    // 1.5MB is reserved at the end.
        int cheatsz = open_read_cheats(cheat_area, max_area, data->cheatsfn);
        if (cheatsz < 0)
          data->cheats_found = false;
        else
          data->cheats_size = cheatsz;

        WRITE_LOG("Loaded cheats returned %d", cheatsz);
      }
    }
  }
  data->use_cheats = enable_cheats && data->cheats_found && prefer_cheats;
}

static void prepare_gba_settings(t_load_gba_lcfg *data, bool uses_dsaving, uint32_t rtcts, bool game_no_save) {
  // Calculate the .sav file name, and check its existance.
  data->savefile_found = check_file_exists(data->savefn);
  if (data->savefile_found)
    WRITE_LOG("Savefile found at '%s'", data->savefn);
  else
    WRITE_LOG("No savefile found for '%s'", data->savefn);

  // Use default settings (and file existance) to fill in default choice.
  // DirectSaving enabled overrides the other settings.
  if (uses_dsaving) {
    data->sram_load_type = data->savefile_found ? SaveLoadSav : SaveLoadReset;
    data->sram_save_type = SaveDirect;
  }
  else {
    data->sram_load_type = game_no_save         ? SaveLoadDisable :
                           !autoload_default    ? SaveLoadDisable :
                           data->savefile_found ? SaveLoadSav :
                                                  SaveLoadReset;
    data->sram_save_type = autosave_default && !game_no_save ? SaveReboot : SaveDisable;
  }

  data->rtcval = rtcts;
}


static bool insert_recent_flush(const char *fn,unsigned flags);
static void browser_open_gba(const char *fn, uint32_t fs, bool prompt_patchgen) {
  if (fs > MAX_GBA_ROM_SIZE) {
    // The ROM is too big to be loaded!
    spop.alert_msg = msgs[lang_id][MSG_ERR_TOOBIG];
  } else {
    // Default to global settings (in case the file is not found).
    t_rom_load_settings ld_sett = {
      .patch_policy = patcher_default,
      .use_igm = ingamemenu_default,
      .use_rtc = rtcpatch_default,
      .use_dsaving = autosave_prefer_ds
    };
    t_rom_launch_settings lh_sett = {
      .use_cheats = true,              // Defaults to true (just preferred, might be disabled/N/A)
      .rtcts = rtcvalue_default
    };
    // Check for any game-specific config file, so we don't have to guess the config.
    // The config file can be partial, hence the defaults.
    load_rom_settings(fn, &ld_sett, &lh_sett);

    if (!prepare_gba_info(&spop.p.load.i, &ld_sett, fn, fs, true))
      spop.alert_msg = msgs[lang_id][MSG_ERR_READ];
    else {
      const t_rom_header *rmh = &spop.p.load.i.romh;

      // If patch engine is selected but no patches found, prompt for generation.
      // If auto is selected and no patches nor DB entries found, do prompt too.
      bool no_patches = (ld_sett.patch_policy == PatchAuto &&
                         !spop.p.load.i.patches_datab_found && !spop.p.load.i.patches_cache_found);
      bool no_engine  = (ld_sett.patch_policy == PatchEngine && !spop.p.load.i.patches_cache_found);
      bool issfw = is_superfw(rmh);
      if(prompt_patchgen && !issfw && !insert_recent_flush(spop.p.load.i.romfn,FLAG_RECENT_SD)) {
        spop.alert_msg="Could not update recent games.";return;
      }

      if (prompt_patchgen && !issfw && (no_patches || no_engine)) {
        // No patches found, ask the user if they want to generate patches
        // using the patch engine.
        spop.qpop.message = msgs[lang_id][no_patches ? MSG_Q1_NOPATCH : MSG_Q1_PATCHENG];
        spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
        spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
        spop.qpop.option = 0;
        spop.qpop.callback = patch_gen_callback;
        spop.qpop.clear_popup_ok = true;
        return;
      }

      // What if the game doesn't have a save method? Select sane defaults.
      const t_patch *p = get_game_patch(&spop.p.load.i);
      bool game_no_save = (p && p->save_mode == SaveTypeNone) || issfw;

      // Attempt to find a cheat file if cheats are enabled.
      prepare_gba_cheats((char*)&rmh->gcode[0], rmh->version, &spop.p.load.l, fn, lh_sett.use_cheats);

      // Load and set default and sane settings honoring defaults and preferences.
      sram_filename_calc(fn, spop.p.load.l.savefn);
      prepare_gba_settings(&spop.p.load.l, spop.p.load.i.use_dsaving, lh_sett.rtcts, game_no_save);

      // Show load ROM menu.
      spop.pop_num = POPUP_GBA_LOAD;
      spop.anim = 0;
      spop.submenu = GbaLoadPopInfo;
      spop.selector = GBALoadButt;
      #ifdef SUPPORT_NORGAMES
      gba_write_nor = false;
      #endif
    }
  }
}

#ifdef SUPPORT_NORGAMES
static void browser_open_nor(const t_flash_game_entry * e) {
  // Use attributes to determine patched save method.
  const bool game_no_save = GET_GATTR_SAVEM(e->gattrs) <= SaveTypeNone;
  const bool game_uses_dsaving = (e->gattrs & GATTR_SAVEDS);

  t_rom_launch_settings lh_sett = {
    .use_cheats = true,              // Defaults to true (just preferred, might be disabled/N/A)
    .rtcts = rtcvalue_default
  };
  load_rom_settings(e->game_name, NULL, &lh_sett);

  // Attempt to find a cheat file if cheats are enabled.
  prepare_gba_cheats((char*)&e->gamecode, e->gamever, &spop.p.norld.l, e->game_name, lh_sett.use_cheats);

  // Load and set default and sane settings honoring defaults and preferences.
  sram_filename_calc(e->game_name, spop.p.norld.l.savefn);
  WRITE_LOG("loadp: %d savep: %d uses_ds: %d gamens: %d",
            spop.p.norld.l.sram_load_type, spop.p.norld.l.sram_save_type,
            game_uses_dsaving, game_no_save);
  prepare_gba_settings(&spop.p.norld.l, game_uses_dsaving, lh_sett.rtcts, game_no_save);

  if(!insert_recent_flush(e->game_name,FLAG_RECENT_NOR)) {
    spop.alert_msg="Could not update recent games.";return;
  }
  // Save entry pointer
  spop.p.norld.e = e;

  // Show load ROM menu.
  spop.pop_num = POPUP_GBA_NORLOAD;
  spop.submenu = GbaLoadPopInfo;
  spop.selector = 0;
}
#endif

void patch_gen_callback(bool confirm) {
  // Generate patches if confirm was selected
  if (confirm) {
    generate_patches_progress(spop.p.load.i.romfn, spop.p.load.i.romfs);
    spop.alert_msg = msgs[lang_id][MSG_PATCHGEN_OK];
  }

  // Either way, show the popup screen afterwards without prompt
  browser_open_gba(spop.p.load.i.romfn, spop.p.load.i.romfs, false);
}

static void load_patchdb_action(bool confirm) {
  if (confirm) {
    FIL fd;
    FRESULT res = f_open(&fd, spop.p.pdb_ld.fn, FA_READ);
    if (res != FR_OK) {
      spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];
      return;
    } else {
      for (unsigned off = 0; off < spop.p.pdb_ld.fs; off += 1024) {
        UINT rdbytes;
        uint32_t tmp[1024/4];
        if (FR_OK != f_read(&fd, tmp, sizeof(tmp), &rdbytes)) {
          spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];
          return;
        }

        set_supercard_mode(MAPPED_SDRAM, true, false);
        dma_memcpy32(ROM_PATCHDB_U8 + off, tmp, sizeof(tmp)/4);
        set_supercard_mode(MAPPED_SDRAM, true, true);
      }
    }
    spop.alert_msg = msgs[lang_id][MSG_OK_GENERIC];
  }
}

unsigned guess_file_type(const uint8_t *header) {
  const t_rom_header *gbah = (t_rom_header*)header;
  uint32_t sig = *(uint32_t*)header;

  if (gbah->fixed == 0x96 && gbah->unit_code == 0x00 && gbah->devtype == 0x00 &&
      header[3] == 0xEA /* Starts with an unconditional branch */ &&
      validate_gba_header(header))
    return FileTypeGBA;
  else if (validate_gb_header(&header[0x100]))
    return FileTypeGB;
  else if (sig == 0x1A53454E)
    return FileTypeNES;
  else if (sig == 0x31424450)
    return FileTypePatchDB;

  return FileTypeUnknown;
}

static bool insert_recent_flush(const char *fn, unsigned flags) {
  // Insert element.
  smenu.recent.maxentries = insert_recent_fn(sdr_state->rentries, smenu.recent.maxentries, fn, flags);
  launcher_invalidate();
  return recent_flush(sdr_state->rentries, smenu.recent.maxentries);
}

static bool delete_recent_flush(unsigned entry_num) {
  smenu.recent.maxentries = delete_recent(sdr_state->rentries, smenu.recent.maxentries, entry_num);

  smenu.recent.selector = MIN(smenu.recent.maxentries - 1, smenu.recent.selector);
  if (!smenu.recent.maxentries)
    smenu.menu_tab = MENUTAB_ROMBROWSE;

  return recent_flush(sdr_state->rentries, smenu.recent.maxentries);
}

static void recent_reload() {
  smenu.recent.selector = 0;
  smenu.recent.seloff = 0;
  smenu.anim_state = 0;
  smenu.recent.maxentries = recent_load_merged(sdr_state->rentries,(t_rentry*)sdr_state->scratch);
}

static void browser_invalidate_artwork(void);
static void browser_reload(void);
static void flashbrowser_reload(void);
/* Failed loads can already have overwritten directory/cache tables in SDRAM. */
static void recover_emu_browser(void) {
  smenu.browser.filecount=smenu.browser.dispentries=0;
  smenu.browser.selector=smenu.browser.seloff=0;
  browser_invalidate_artwork();
  browser_reload();flashbrowser_reload();recent_reload();
}

void start_emu_game(const t_emu_loader *ldinfo, const char *fn, uint32_t fs) {
  char stable_filename[MAX_FN_LEN];
  strbcpy(stable_filename,fn,sizeof(stable_filename));fn=stable_filename;
  if(!insert_recent_flush(fn,FLAG_RECENT_SD)){spop.alert_msg="Could not update recent games.";return;}
  // Load: Sav/Reset Save: Reboot/Disable
  sram_filename_calc(fn, spop.p.load.l.savefn);
  t_sram_load_policy lp = check_file_exists(spop.p.load.l.savefn) ? SaveLoadSav : SaveLoadReset;
  unsigned errsave = prepare_sram_based_savegame(lp, SaveReboot, spop.p.load.l.savefn);
  if (errsave) {
    unsigned errmsg = (errsave == ERR_SAVE_BADSAVE)   ? MSG_ERR_SAVERD :
                                                        MSG_ERR_SAVEWR;
    spop.alert_msg = msgs[lang_id][errmsg];
  }
  else {
    unsigned plugin_error=load_extemu_rom(fn,fs,&pogo_plugin_loader,loadrom_progress);
    if(plugin_error!=ERR_LOAD_NOEMU) {
      recover_emu_browser();
      spop.alert_msg=msgs[lang_id][MSG_ERR_READ];return;
    }
    // Try to load the emu and ROM, keep trying if there's more than one emulatior option.
    unsigned errcode = ERR_LOAD_NOEMU;
    while (ldinfo->emu_name) {

      errcode = load_extemu_rom(fn, fs, ldinfo, loadrom_progress);
      if (errcode && errcode != ERR_LOAD_NOEMU)
        break;
      ldinfo++;
    }
    recover_emu_browser();
    unsigned errmsg = (errcode == ERR_LOAD_NOEMU) ? MSG_ERR_NOEMU :
                                                    MSG_ERR_READ;
    spop.alert_msg = msgs[lang_id][errmsg];
  }
}

NOINLINE static void browser_open(const char *fn, uint32_t fs) {
  const char *ext = find_extension(fn);
  if (ext && !strcasecmp(ext, "gba"))
    // GBA ROMs (most likely)
    browser_open_gba(fn, fs, true);
  else if (ext && !strcasecmp(ext, "sav")) {
    spop.pop_num = POPUP_SAVFILE;
    spop.selector = SavMAX;
    strbcpy(spop.p.savopt.savfn, fn, sizeof(spop.p.savopt.savfn));
  }
  else if (ext && !strcasecmp(ext, "fw")) {
    // A SuperFW firmware update is selected!
    if (!enable_flashing)
      spop.alert_msg = msgs[lang_id][MSG_FWUP_DISABLED];
    else if (fs > FW_MAX_SIZE_KB*1024 || (flashinfo.size && fs > flashinfo.size))
      spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRSZ];
    else {
      // Read the header and perform some more basic checks!
      FIL fd;
      FRESULT res = f_open(&fd, fn, FA_READ);
      if (res != FR_OK)
        spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRRD];
      else {
        UINT rdbytes;
        uint8_t tmp[512];
        if (FR_OK != f_read(&fd, tmp, sizeof(tmp), &rdbytes) || rdbytes != sizeof(tmp))
          spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRRD];
        else if (!validate_gba_header(tmp))  // Is it a valid GBA ROM header?
          spop.alert_msg = msgs[lang_id][MSG_FWUP_BADHD];
        else {
          spop.p.update.issfw = check_superfw(tmp, &spop.p.update.superfw_ver);
          spop.p.update.fw_size = fs;
          spop.p.update.curr_state = FlashingReady;
          spop.pop_num = POPUP_FWFLASH;
          strbcpy(spop.p.update.fn, fn, sizeof(spop.p.update.fn));
          f_close(&fd);
        }
      }
    }
  }
  else {
    // Any emulator-based console supported
    if (ext) {
      const t_emu_loader *ldinfo = get_emu_info(ext);
      if (ldinfo) {
        start_emu_game(ldinfo, fn, fs);
        return;
      }
    }

    /* Other extension-named plugins can handle formats outside the built-ins. */
    char plugin_path[64];
    if(ext && find_pogo_plugin(fn,plugin_path)) {
      static const t_emu_loader empty_loaders[]={{NULL,NULL}};
      start_emu_game(empty_loaders,fn,fs);
      if(spop.alert_msg!=msgs[lang_id][MSG_ERR_NOEMU])return;
      spop.alert_msg=NULL;
    }

    // Attempt to load the file magic and detect what kind of file this is.
    if (fs >= 512) {
      FIL fi;
      if (FR_OK == f_open(&fi, fn, FA_READ)) {
        uint32_t tmphdr[512 / 4];
        UINT rdbytes;
        if (FR_OK == f_read(&fi, tmphdr, sizeof(tmphdr), &rdbytes) && rdbytes == sizeof(tmphdr)) {
          unsigned guesstype = guess_file_type((uint8_t*)tmphdr);
          switch (guesstype) {
          case FileTypeGBA:
            browser_open_gba(fn, fs, true); break;
          case FileTypeGB:
            start_emu_game(get_emu_info("gbc"), fn, fs);
            break;
          case FileTypePatchDB:
            strbcpy(spop.p.pdb_ld.fn, fn, sizeof(spop.p.pdb_ld.fn));
            spop.p.pdb_ld.fs = fs;
            spop.qpop.message = msgs[lang_id][MSG_Q3_LOADPDB];
            spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
            spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
            spop.qpop.option = 0;
            spop.qpop.callback = load_patchdb_action;
            spop.qpop.clear_popup_ok = false;
            break;
          default:
            spop.alert_msg = msgs[lang_id][MSG_ERR_UNKTYP];
            break;
          };
        }
        f_close(&fi);
      }
    }
  }
}

static void browser_reload_sort() {
  // We take the list of files and sort them using a list of pointers instead of
  // actually copying entries. Faster, less memory and allows us to play with it later.

  // Init pointer table, then sort them.
  for (unsigned i = 0; i < smenu.browser.filecount; i++)
    sdr_state->fileorder[i] = &sdr_state->fentries[i];

  heapsort4(sdr_state->fileorder, smenu.browser.filecount, sizeof(t_centry*) / sizeof(uint32_t), filesort);
}

static void browser_reload_filter() {
  // Process the sorted pointer table and filter out files that we don't want to display.

  unsigned fcount = 0;
  for (unsigned i = 0; i < smenu.browser.filecount; i++) {
    if (hide_hidden && ishidden(sdr_state->fileorder[i]))
      continue;

    sdr_state->filefilter[fcount++] = sdr_state->fileorder[i];
  }

  if (smenu.browser.selector >= fcount)
    smenu.browser.selector = fcount - 1;
  smenu.browser.seloff = MAX(0, smenu.browser.selector - BROWSER_ROWS / 2);
  smenu.browser.dispentries = fcount;
}

// Loads a new directory list in the ROM browser.
// TODO: Implement filtering (.gba/.rom/.bin... etc) using settings
static bool artwork_cached, artwork_valid;
/* Cold filename storage leaves IWRAM for pixel loops and the native stack. */
static char artwork_rom[MAX_FN_LEN] __attribute__((section(".sbss"),aligned(4)));
static unsigned artwork_cached_mode;
static artwork_info preview_info;
/* Byte flags and path strings must live in GBA RAM, not the 16-bit cart bus. */
static struct {
  bool cached, valid;
  uint16_t index; /* Uses former alignment padding; stays off the cartridge bus. */
  unsigned mode;
  char rom[MAX_FN_LEN];
  artwork_info info;
} carousel_meta[ARTWORK_CACHE_SLOTS];
/* Selected artwork is sampled repeatedly during motion. Keep a hot copy in
 * EWRAM; the linker bounds this NOLOAD allocation on every board. */
static uint16_t carousel_hot_pixels[ARTWORK_MAX_PIXELS/2] __attribute__((section(".sbss")));
static bool carousel_hot_valid;
static int carousel_hot_index;
static int browser_motion_offset;
static bool browser_fast_reuse, browser_fast_valid[2];
static int browser_fast_selection[2];
static unsigned browser_fast_view[2], browser_fast_mode[2];

static unsigned carousel_find_slot(int index) {
  for(unsigned slot=0;slot<ARTWORK_CACHE_SLOTS;++slot)
    if(carousel_meta[slot].cached && carousel_meta[slot].index==index)return slot;
  return ARTWORK_CACHE_SLOTS;
}

static void browser_invalidate_artwork(void) {
  browser_fast_valid[0] = browser_fast_valid[1] = false;
  artwork_cached = false;
  carousel_hot_valid = false;
  for (unsigned i = 0; i < ARTWORK_CACHE_SLOTS; ++i) {
    carousel_meta[i].cached = false;
    carousel_meta[i].valid = false;
  }
}

static void browser_reload() {
  browser_invalidate_artwork();
  browser_motion_offset = 0;
  smenu.anim_state = 0;

  unsigned fcount = 0;
  DIR d;
  if (FR_OK != f_opendir(&d, smenu.browser.cpath))
    return;   // FIXME: Implement error reporting!

  while (1) {
    FILINFO info;
    if (f_readdir(&d, &info) != FR_OK || !info.fname[0])
      break;

    if (fcount >= BROWSER_MAXFN_CNT)
      break;

    t_centry *e = &sdr_state->fentries[fcount++];
    e->filesize = (uint32_t) info.fsize;  // TODO: Support 4GB+ files?
    e->isdir = (info.fattrib & AM_DIR) ? 1 : 0;
    e->attr = info.fattrib;
    dma_memcpy16(e->fname, info.fname, MAX_FN_LEN/2);
    sortable_utf8_u16(info.fname, e->sortname);
  }
  f_closedir(&d);

  smenu.browser.filecount = fcount;

  // Sort and filter the new list of files/dirs
  browser_reload_sort();
  browser_reload_filter();
}

// Loads NOR game entries so they can be browsed.
static void flashbrowser_reload() {
  #ifdef SUPPORT_NORGAMES
  smenu.fbrowser.selector = 0;
  smenu.anim_state = 0;

  if (!flashmgr_load(ROM_FLASHMETA_ADDR, FLASH_METADATA_SIZE, (t_reg_entry*)&sdr_state->nordata))
    // No data found, reset the entries
    memset(&sdr_state->nordata, 0, sizeof(sdr_state->nordata));

  // Calculate block usage, free space, etc.
  smenu.fbrowser.usedblks = 0;
  for (unsigned i = 0; i < sdr_state->nordata.gamecnt; i++) {
    const t_flash_game_entry *e = &sdr_state->nordata.games[i];
    for (unsigned j = 0; j < MAX_GAME_BLOCKS; j++)
      if (e->blkmap[j])
        smenu.fbrowser.usedblks++;
  }
  smenu.fbrowser.freeblks = NOR_GAMEBLOCK_COUNT - smenu.fbrowser.usedblks;

  smenu.fbrowser.maxentries = sdr_state->nordata.gamecnt;
  heapsort4(sdr_state->nordata.games, smenu.fbrowser.maxentries, sizeof(t_flash_game_entry) / sizeof(uint32_t), romsort);
  #endif
}

#pragma GCC optimize ("O2")

static inline void render_icon(unsigned x, unsigned y, unsigned iconn) {
  fobjs[objnum++] = (t_oamobj){x, y, 8*iconn };
}

static inline void render_icon_trans(unsigned x, unsigned y, unsigned iconn) {
  fobjs[objnum++] = (t_oamobj){x, y | 0x0400, 8*iconn };
}

// Guess the file type based on the file name.
static unsigned guessicon(const char *path) {
  const char *ext = find_extension(path);

  if (ext) {
    static const struct {
      const char *ext;
      unsigned icon;
    } exticon[] = {
      {"gba", ICON_GBACART},
      {"gb",  ICON_GBCART},
      {"gbc", ICON_GBCCART},
      {"nes", ICON_NESCART},
      {"sms", ICON_SMSCART},
      {"gg", ICON_GGCART},
      {"fw",  ICON_UPDFILE},
    };
    for (unsigned i = 0; i < sizeof(exticon)/sizeof(exticon[0]); i++)
      if (!strcasecmp(exticon[i].ext, ext))
        return exticon[i].icon;
  }

  return ICON_BINFILE;
}

// Draws text adding some support for overflow.
#define THREEDOTS_WIDTH  9
#pragma GCC optimize ("Os")

static void draw_text_ovf_color(const char *t, volatile uint8_t *frame, unsigned x, unsigned y, unsigned maxw, uint8_t color) {
  if(y<16 && menu_theme>=THEME_DSSMM_LIGHT)color=TOP_TEXT;
  uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x];
  unsigned twidth = font_width(t);
  if (twidth <= maxw)
    draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, color);
  else {
    char tmpbuf[256];
    unsigned numchars = font_width_cap(t, maxw - THREEDOTS_WIDTH);
    memcpy(tmpbuf, t, numchars);
    memcpy(&tmpbuf[numchars], "...", 4);
    draw_text_idx8_bus16(tmpbuf, basept, SCREEN_WIDTH, color);
  }
}
static void draw_text_ovf(const char *t,volatile uint8_t *frame,unsigned x,unsigned y,unsigned maxw) {
  draw_text_ovf_color(t,frame,x,y,maxw,FT_COLOR);
}


static void draw_text_leftovf(const char *t, volatile uint8_t *frame, unsigned x, unsigned y, unsigned maxw) {
  uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x];
  unsigned numchars = font_width_lcap(t, maxw - THREEDOTS_WIDTH);
  if (numchars) {
    draw_text_idx8_bus16("...", basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
    draw_text_idx8_bus16(&t[numchars], basept + THREEDOTS_WIDTH, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  } else {
    draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  }
}

static void draw_text_ovf_rotate(const char *t, volatile uint8_t *frame, unsigned x, unsigned y, unsigned maxw, unsigned *franim) {
  uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x];
  unsigned twidth = font_width(t);
  if (twidth <= maxw)
    draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  else {
    unsigned anim = *franim > ANIM_INITIAL_WAIT ? (*franim - ANIM_INITIAL_WAIT) >> 4 : 0;

    // Wrap around once the text end reaches the mid point aprox.
    char tmpbuf[540];
    strcpy(tmpbuf, t);
    strcat(tmpbuf, "      ");
    unsigned pixw = font_width(tmpbuf);
    if (anim > pixw)
      *franim = ANIM_INITIAL_WAIT + ((anim - pixw) << 4);
    strcat(tmpbuf, t);

    draw_text_idx8_bus16_range(tmpbuf, basept, anim, maxw, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  }
}

static void draw_box_outline(volatile uint8_t *frame, unsigned left, unsigned right, unsigned top, unsigned bottom, uint8_t color) {
  dma_memset16(&frame[SCREEN_WIDTH * top + left], dup8(color), (right - left) / 2);
  dma_memset16(&frame[SCREEN_WIDTH * (top + 1) + left], dup8(color), (right - left) / 2);
  dma_memset16(&frame[SCREEN_WIDTH * (bottom - 1) + left], dup8(color), (right - left) / 2);
  dma_memset16(&frame[SCREEN_WIDTH * (bottom - 2) + left], dup8(color), (right - left) / 2);
  while (top < bottom) {
    *((uint16_t*)&frame[SCREEN_WIDTH * top + left]) = dup8(color);
    *((uint16_t*)&frame[SCREEN_WIDTH * top + right - 2]) = dup8(color);
    top++;
  }
}

static void draw_box_full(
  volatile uint8_t *frame, unsigned left, unsigned right, unsigned top, unsigned bottom,
  uint8_t outlinecolor, uint8_t bgcolor
) {
  draw_box_outline(frame, left, right, top, bottom, outlinecolor);
  for (unsigned i = top + 2; i < bottom - 2; i++)
    dma_memset16(&frame[SCREEN_WIDTH * i + left + 2], dup8(bgcolor), (right - left - 4) / 2);
}

/* Extend the Settings right-hand panel while retaining its original bevel. */
static void draw_lined_panel(volatile uint8_t *frame) {
  bool dark = menu_theme == THEME_DSSMM_DARK;
  dssmm_background_clear(frame, dark, 0, 16, 240, 144);
  for (unsigned y = 21; y < 152; ++y) {
    const uint8_t *p = dssmm_panel_profile[dark][y - 21];
    volatile uint16_t *row = (volatile uint16_t *)(frame + y * 240);
    dma_memset16(row + 1, dup8(p[1]), 118);
    row[1] = (p[0] & 15) | ((p[0] >> 4) << 8);
    row[117] = (p[2] & 15) | ((p[2] >> 4) << 8);
    row[118] = (p[3] & 15) | ((p[3] >> 4) << 8);
  }
}

static void draw_button_box(
  volatile uint8_t *frame, unsigned left, unsigned right, unsigned top, unsigned bottom, bool selected
) {
  if (selected)
    draw_box_full(frame, left, right, top, bottom, FG_COLOR, HI_COLOR);
  else
    draw_box_outline(frame, left, right, top, bottom, FG_COLOR);
}


static void draw_rightj_text(const char *t, volatile uint8_t *frame, unsigned x, unsigned y) {
  unsigned twidth = font_width(t);
  uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x - twidth];
  draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
}

static void draw_central_text(const char *t, volatile uint8_t *frame, unsigned x, unsigned y) {
  unsigned twidth = font_width(t);
  uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x - twidth / 2];
  draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
}

static void draw_central_text_ovf(const char *t, volatile uint8_t *frame, unsigned x, unsigned y, unsigned maxw) {
  unsigned twidth = font_width(t);
  if (twidth <= maxw) {
    uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x - twidth / 2];
    draw_text_idx8_bus16(t, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  } else {
    char tmpbuf[256];
    unsigned numchars = font_width_cap(t, maxw - THREEDOTS_WIDTH);
    memcpy(tmpbuf, t, numchars);
    memcpy(&tmpbuf[numchars], "...", 4);
    uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x - maxw / 2];
    draw_text_idx8_bus16(tmpbuf, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));
  }
}

static void draw_central_text_wrapped(const char *t, volatile uint8_t *frame, unsigned x, unsigned y, unsigned maxw) {
  while (*t) {
    char tmp[128];
    unsigned outw;
    unsigned linechars = font_width_cap_space(t, maxw, &outw);
    unsigned charcnt = linechars ?: utf8_strlen(t);
    uint8_t *basept = (uint8_t*)&frame[y * SCREEN_WIDTH + x - outw / 2];

    memcpy(tmp, t, charcnt);
    tmp[charcnt] = 0;
    draw_text_idx8_bus16(tmp, basept, SCREEN_WIDTH, (y<16 && menu_theme>=THEME_DSSMM_LIGHT?TOP_TEXT:FT_COLOR));

    t += charcnt;      // Advance text
    y += 16;           // Move down in the buffer
  }
}

#pragma GCC optimize ("O2")

void render_recent(volatile uint8_t *frame) {
  // Render the list from memory.
  for (unsigned i = 0; i < RECENT_ROWS; i++) {
    if (smenu.recent.seloff + i >= smenu.recent.maxentries)
      break;

    t_rentry *e = &sdr_state->rentries[smenu.recent.seloff + i];
    const char *fn = &e->fpath[e->fname_offset];
    char display[MAX_FN_LEN];if(clean_list){clean_display_name(display,sizeof(display),fn);fn=display;}
    render_icon(2, (i+1)*16, guessicon(e->fpath+e->fname_offset));

    // Animate the row entries if they are too long!
    if (i == smenu.recent.selector - smenu.recent.seloff)
      draw_text_ovf_rotate(fn, frame, 20, (1 + i) * 16,
                           SCREEN_WIDTH - 24, &smenu.anim_state);
    else
      draw_text_ovf(fn, frame, 20, (1 + i) * 16, SCREEN_WIDTH - 24);
  }

  for (unsigned i = 0; i < 240; i += 16)
    render_icon_trans(i, (smenu.recent.selector - smenu.recent.seloff + 1)*16, 63);
}

#ifdef SUPPORT_NORGAMES
void render_flashbrowser(volatile uint8_t *frame) {
  // NOR currently uses the list browser layout on every theme.
  dma_memset16(&frame[240*144],dup8(FG_COLOR),240*16/2);

  // Render the list from memory.
  if (!smenu.fbrowser.maxentries)
    draw_central_text(msgs[lang_id][MSG_NOR_EMPTY], frame, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-8);
  else {
    for (unsigned i = 0; i < NORGAMES_ROWS; i++) {
      if (smenu.fbrowser.seloff + i >= smenu.fbrowser.maxentries)
        break;

      const t_flash_game_entry *e = &sdr_state->nordata.games[smenu.fbrowser.seloff + i];
      render_icon(2, (i+1)*16, ICON_GBACART);

      // Animate the row entries if they are too long!
      char szstr[16];
      human_size(szstr, sizeof(szstr), e->numblks * NOR_BLOCK_SIZE);
      draw_rightj_text(szstr, frame, SCREEN_WIDTH - 2, (1 + i) * 16);

      // Animate the row entries if they are too long!
      const char *romname = &e->game_name[e->bnoffset];
      char display[MAX_FN_LEN];if(clean_list){clean_display_name(display,sizeof(display),romname);romname=display;}
      if (i == smenu.fbrowser.selector - smenu.fbrowser.seloff)
        draw_text_ovf_rotate(romname, frame, 20, (1 + i) * 16,
                             SCREEN_WIDTH - 26 - font_width(szstr), &smenu.anim_state);
      else
        draw_text_ovf(romname, frame, 20, (1 + i) * 16, SCREEN_WIDTH - 26 - font_width(szstr));
    }

    for (unsigned i = 0; i < 240; i += 16)
      render_icon_trans(i, (smenu.fbrowser.selector - smenu.fbrowser.seloff + 1)*16, 63);
  }

  char tmp[32], tmp1[32], tmp2[32];
  npf_snprintf(tmp, sizeof(tmp), "%u/%d", smenu.fbrowser.selector + 1, smenu.fbrowser.maxentries);
  draw_rightj_text(tmp, frame, SCREEN_WIDTH - 1, 1);

  human_size(tmp1, sizeof(tmp1), smenu.fbrowser.usedblks * NOR_BLOCK_SIZE);
  human_size(tmp2, sizeof(tmp2), NOR_GAMEBLOCK_COUNT * NOR_BLOCK_SIZE);
  npf_snprintf(tmp,sizeof(tmp),"NOR %s/%s",tmp1,tmp2);
  draw_text_ovf(tmp,frame,8,144,140);
  draw_box_outline(frame,156,234,148,158,FT_COLOR);
  unsigned filled=(uint64_t)MIN(smenu.fbrowser.usedblks,NOR_GAMEBLOCK_COUNT)*74/NOR_GAMEBLOCK_COUNT;
  for(unsigned y=150;y<156;++y)
    dma_memset16(frame+y*240+158,dup8(HI_COLOR),(filled&~1u)/2);
}
#endif

static void render_browser_list(volatile uint8_t *frame) {
  unsigned list_width = artwork_mode ? 112 : SCREEN_WIDTH;
  // Render bar below to show path URI
  dma_memset16(&frame[240*144], dup8(FG_COLOR), 240*16/2);

  if (!smenu.browser.dispentries)
    draw_central_text(msgs[lang_id][MSG_BROW_EMPTY], frame, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-8);
  else {
    for (unsigned i = 0; i < BROWSER_ROWS; i++) {
      if (smenu.browser.seloff + i >= smenu.browser.dispentries)
        break;

      char szstr[16] = {0};
      t_centry *e = sdr_state->filefilter[smenu.browser.seloff + i];

      unsigned iconidx = ishidden(e)        ? ((e->attr & AM_DIR) ? ICON_HFOLDER : ICON_HFILE) :
                         (e->attr & AM_DIR) ? ICON_FOLDER :
                         guessicon(e->fname);

      const unsigned row_y = (i + 1) * 16;
      if (smenu.browser.selector == smenu.browser.seloff + i)
        dma_memset16(&frame[SCREEN_WIDTH * row_y], dup8(HI_COLOR), list_width / 2);
      render_icon(2, row_y, iconidx);

      if (!artwork_mode && !(e->attr & AM_DIR)) {
        human_size(szstr, sizeof(szstr), e->filesize);
        draw_rightj_text(szstr, frame, SCREEN_WIDTH - 2, (1 + i) * 16);
      }

      char display[MAX_FN_LEN];const char *shown=e->fname;
      if(clean_list && !(e->attr&AM_DIR)){clean_display_name(display,sizeof(display),shown);shown=display;}
      // Animate the row entries if they are too long!
      if (i == smenu.browser.selector - smenu.browser.seloff)
        draw_text_ovf_rotate(shown, frame, 20, (1 + i) * 16,
                             list_width - 26 - font_width(szstr), &smenu.anim_state);
      else
        draw_text_ovf(shown, frame, 20, (1 + i) * 16, list_width - 26 - font_width(szstr));
    }

    for (unsigned i = 0; i < list_width; i += 16)
      render_icon_trans(i, (smenu.browser.selector - smenu.browser.seloff + 1)*16, 63);
  }

  if (artwork_mode && smenu.browser.dispentries > 0 && smenu.browser.selector >= 0) {
    const t_centry *selected = sdr_state->filefilter[smenu.browser.selector];
    char rom[MAX_FN_LEN];
    int n = npf_snprintf(rom, sizeof(rom), "%s%s", smenu.browser.cpath, selected->fname);
    if (n > 0 && (unsigned)n < sizeof(rom)) {
      if (!artwork_cached || artwork_cached_mode != artwork_mode || strcmp(artwork_rom, rom)) {
        strbcpy(artwork_rom, rom, sizeof(artwork_rom));
        artwork_cached_mode = artwork_mode;
        artwork_valid = (selected->isdir ? artwork_load_directory : artwork_load)(rom, artwork_mode - 1,
          (uint8_t *)sdr_state->artwork_pixels, ARTWORK_MAX_PIXELS,
          sdr_state->artwork_bmp_buffer, sizeof(sdr_state->artwork_bmp_buffer), &preview_info);
        artwork_cached = true;  // Cache misses too; avoid SD probes every frame.
      }
      if (artwork_valid) {
        unsigned xoff = 120 + (120 - preview_info.width) / 2;
        artwork_blit_frame((volatile uint16_t *)frame, sdr_state->artwork_pixels,
                           &preview_info, xoff, 40);
      }
    }
  }

  // Draw path, cut left part if necessary.
  draw_text_leftovf(smenu.browser.cpath, frame, 8, 144, SCREEN_WIDTH - 88);
  draw_rightj_text(menu_theme>=THEME_DSSMM_LIGHT?"Start: Home":"Start: Art", frame, SCREEN_WIDTH - 2, 144);

  char selinfo[16];
  npf_snprintf(selinfo, sizeof(selinfo), "%u/%d", smenu.browser.selector + 1, smenu.browser.dispentries);
  draw_rightj_text(selinfo, frame, SCREEN_WIDTH - 1, 1);
}

static unsigned browser_entry_icon(const t_centry *entry) {
  return ishidden(entry)        ? ((entry->attr & AM_DIR) ? ICON_HFOLDER : ICON_HFILE) :
         (entry->attr & AM_DIR) ? ICON_FOLDER : guessicon(entry->fname);
}

/* One bounded SD read per settled frame; hot drawing stays in the renderer. */
__attribute__((optimize("Os")))
static void browser_cache_prepare(const int candidates[ARTWORK_CACHE_SLOTS]) {
  for (unsigned c = 0; c < ARTWORK_CACHE_SLOTS; ++c) {
    int index = candidates[c];
    if (index < 0 || index >= smenu.browser.dispentries) continue;
    const t_centry *entry = sdr_state->filefilter[index];
    char rom[MAX_FN_LEN];
    int n = npf_snprintf(rom, sizeof(rom), "%s%s", smenu.browser.cpath, entry->fname);
    if (n <= 0 || (unsigned)n >= sizeof(rom)) continue;
    unsigned slot=carousel_find_slot(index);
    if(slot<ARTWORK_CACHE_SLOTS && carousel_meta[slot].mode==artwork_mode &&
        !strcmp(carousel_meta[slot].rom,rom))continue;
    if(slot==ARTWORK_CACHE_SLOTS)for(unsigned s=0;s<ARTWORK_CACHE_SLOTS;++s) {
      bool keep=false;
      if(carousel_meta[s].cached)for(unsigned c=0;c<ARTWORK_CACHE_SLOTS;++c)
        if(candidates[c]==carousel_meta[s].index)keep=true;
      if(!keep){slot=s;break;}
    }
    if(slot==ARTWORK_CACHE_SLOTS)continue;
    if(carousel_hot_valid && carousel_meta[slot].cached &&
        carousel_meta[slot].index==carousel_hot_index)carousel_hot_valid=false;
    carousel_meta[slot].index=index;
    strbcpy(carousel_meta[slot].rom, rom, MAX_FN_LEN);
    carousel_meta[slot].mode = artwork_mode;
    carousel_meta[slot].valid = ((entry->attr & AM_DIR) ? artwork_load_directory : artwork_load)(rom, artwork_mode - 1,
        (uint8_t *)sdr_state->carousel_art[slot].pixels, ARTWORK_MAX_PIXELS,
        sdr_state->artwork_bmp_buffer, sizeof(sdr_state->artwork_bmp_buffer),
        &carousel_meta[slot].info);
    carousel_meta[slot].cached = true;
    if (index==candidates[0] || index==candidates[1] || index==candidates[2]) {
      browser_fast_reuse = false;
      browser_fast_valid[0] = browser_fast_valid[1] = false;
    }
    break;
  }
}

static void render_browser_carousel(volatile uint8_t *frame, bool vertical) {
  if (!browser_fast_reuse) {
    /* DS backgrounds were drawn once by menu_render before the header. */
    if(menu_theme<THEME_DSSMM_LIGHT)
      dma_memset16(&frame[240 * 16],dup8(BG_COLOR),240*(144-16)/2);
  } else {
    unsigned tx = vertical ? 98 : 42, ty = vertical ? 85 : 122;
    unsigned tw = vertical ? 136 : 156;
    if(menu_theme>=THEME_DSSMM_LIGHT)dssmm_background_clear(frame,menu_theme==THEME_DSSMM_DARK,tx,ty,tw,16);
    else for (unsigned row = 0; row < 16; ++row)
      dma_memset16(frame + (ty+row)*240 + tx, dup8(BG_COLOR), tw/2);
    if(menu_theme<THEME_DSSMM_LIGHT)dma_memset16(frame+144*240,dup8(BG_COLOR),240*16/2);
  }

  if (!smenu.browser.dispentries) {
    draw_central_text(msgs[lang_id][MSG_BROW_EMPTY], frame, 120, 72);
  } else {
    int selected = smenu.browser.selector;
    browser_carousel_item items[3];
    unsigned item_count = browser_carousel_layout(vertical, selected,
        smenu.browser.dispentries, browser_motion_offset, carousel_wrap, items);
    /* Populate one missing slot per settled frame, selected entry first. */
    int candidates[ARTWORK_CACHE_SLOTS] = {selected, selected-1, selected+1, selected-2, selected+2};
    for(unsigned c=0;c<ARTWORK_CACHE_SLOTS;++c)
      candidates[c]=browser_carousel_index(candidates[c],smenu.browser.dispentries,carousel_wrap);
    if(artwork_mode && browser_motion_offset==0)browser_cache_prepare(candidates);
    int focus_index = selected;
    for (unsigned i = 0; i < item_count; ++i) {
      const browser_carousel_item *item = &items[i];
      const t_centry *entry = sdr_state->filefilter[item->index];
      if (item->focused) focus_index = item->index;
      unsigned slot=carousel_find_slot(item->index);
      char rom[MAX_FN_LEN];
      int n = npf_snprintf(rom, sizeof(rom), "%s%s", smenu.browser.cpath, entry->fname);
      bool drawn = false;
      if (artwork_mode && n > 0 && (unsigned)n < sizeof(rom) &&
          slot<ARTWORK_CACHE_SLOTS && carousel_meta[slot].cached && carousel_meta[slot].valid &&
          carousel_meta[slot].mode == artwork_mode &&
          !strcmp(carousel_meta[slot].rom, rom)) {
        const artwork_info *info = &carousel_meta[slot].info;
        const uint16_t *pixels = sdr_state->carousel_art[slot].pixels;
        if (item->focused) {
          if (!carousel_hot_valid || carousel_hot_index != item->index) {
            dma_memcpy16(carousel_hot_pixels, pixels, info->width*info->height/2);
            carousel_hot_index = item->index;
            carousel_hot_valid = true;
          }
          pixels = carousel_hot_pixels;
        }
        int x = item->x;
        unsigned width = item->w, height = item->h;
        if (info->width == 80) { width = height; x += (item->w - width) / 2; }

        x &= ~1u;
        width &= ~1u;
        if (browser_fast_reuse) drawn = true;
        else if (width == info->width && height == info->height && x >= 0 && x+(int)width <= 240 && item->y >= 16 && item->y+(int)height <= 160)
          drawn = artwork_blit_frame((volatile uint16_t *)frame,
              pixels, info, x,
              item->y);
        else
          drawn = artwork_blit_scaled_cropped((volatile uint16_t *)frame,
              pixels, info, x, item->y, width, height);
      }
      if (!drawn) {
        unsigned icon_x = MAX(0, MIN(224, item->x + item->w/2 - 8));
        unsigned icon_y = MAX(16, MIN(128, item->y + item->h/2 - 8));

        render_icon(icon_x, icon_y, browser_entry_icon(entry));
      }
    }
    const t_centry *focus = sdr_state->filefilter[focus_index];

    char display[MAX_FN_LEN];const char *shown=focus->fname;
    if(clean_list && !(focus->attr&AM_DIR)){clean_display_name(display,sizeof(display),shown);shown=display;}
    if (vertical)
      draw_text_ovf_rotate(shown, frame, 98, 85,
                           136, &smenu.anim_state);
    else if(font_width(shown)<=154)
      draw_central_text_ovf(shown,frame,120,122,154);
    else
      draw_text_ovf_rotate(shown, frame, 43, 122, 154, &smenu.anim_state);
  }

  if(menu_theme<THEME_DSSMM_LIGHT) {
  draw_text_leftovf(smenu.browser.cpath, frame, 8, 144, SCREEN_WIDTH - 88);
  draw_rightj_text(menu_theme>=THEME_DSSMM_LIGHT?"Start: Home":"Start: Art", frame, SCREEN_WIDTH - 2, 144);
  }
  char selinfo[16];
  npf_snprintf(selinfo, sizeof(selinfo), "%u/%d", smenu.browser.selector + 1, smenu.browser.dispentries);
  draw_rightj_text(selinfo, frame, SCREEN_WIDTH - 1, 1);
}

void render_browser(volatile uint8_t *frame) {
  if (browser_view == 1 || browser_view == 2)
    render_browser_carousel(frame, browser_view == 2);
  else
    render_browser_list(frame);
}

/* Text-heavy dialogs favor size; browser/font pixel loops keep O2. */
#pragma GCC optimize ("Os")

/* No cartridge assets are safe after a failed full-ROM transfer. */
static void render_restart_required(volatile uint8_t *frame) {
  font_set_style(FONT_STYLE_DSSMM);
  MEM_PALETTE[BG_COLOR]=0x7fff;
  MEM_PALETTE[FT_COLOR]=0;
  dma_memset16(frame,dup8(BG_COLOR),240*160/2);
  draw_box_outline(frame,8,232,32,128,FT_COLOR);
  draw_central_text("Game loading failed",frame,120,46);
  draw_central_text("Restart the console",frame,120,76);
  draw_central_text("before continuing.",frame,120,92);
}

void render_fw_flash_popup(volatile uint8_t *frame) {
  // Opaque notification panel hides the browser underneath.
  if(menu_theme>=THEME_DSSMM_LIGHT)draw_lined_panel(frame);
  else draw_box_full(frame, 2, 240-2, 18, 158, FT_COLOR, BG_COLOR);

  draw_central_text(msgs[lang_id][MSG_FWUPD_MENU], frame, 120, 30);

  draw_box_outline(frame, 16, 224, 64, 92, FG_COLOR);
  if (spop.p.update.issfw) {
    char tmp[32];
    npf_snprintf(tmp, sizeof(tmp), "SuperFW (ver %lu.%lu)",
                 spop.p.update.superfw_ver >> 16,
                 spop.p.update.superfw_ver & 0xFFFF);
    draw_central_text(tmp, frame, 120, 70);
  } else {
    draw_central_text(msgs[lang_id][MSG_FWUPD_UNK], frame, 120, 70);
  }

  const char *smsg[] = {
    msgs[lang_id][MSG_FWUPD_GO],
    msgs[lang_id][MSG_FWUPD_LOADING],
    msgs[lang_id][MSG_FWUPD_CHECKING],
    msgs[lang_id][MSG_FWUPD_ERASING],
    msgs[lang_id][MSG_FWUPD_PROGRAM],
  };

  draw_central_text(smsg[spop.p.update.curr_state], frame, 120, 120);
}

void render_sav_menu_popup(volatile uint8_t *frame) {
  // Render a box to give a pop-up feeling
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  for (unsigned i = 0; i < 3; i++) {
    draw_button_box(frame, 20, 220, 32 + 28 * i, 32 + 28 * i + 20, spop.selector == i);
    draw_text_ovf(msgs[lang_id][MSG_SAVOPT_OPT0 + i], frame, 24, 34 + 28 * i, 192);
  }
  draw_button_box(frame, 20, 220, 124, 144, spop.selector == SavQuit);
  draw_central_text(msgs[lang_id][MSG_CANCEL], frame, 120, 126);
}

/* Framebuffer selection leaves text crisp and avoids the native tiled OAM bar. */
static void draw_menu_selection(volatile uint8_t *frame, unsigned y,
                                unsigned left, unsigned right) {
  unsigned height=menu_font==FONT_STYLE_DSSMM?14:16;
  unsigned bottom=menu_theme>=THEME_DSSMM_LIGHT?149:160;
  for(unsigned row=0;row<height && y+row<bottom;++row)
    dma_memset16(frame+(y+row)*SCREEN_WIDTH+left,dup8(HI_COLOR),(right-left)/2);
}

static void render_gbarom_info(volatile uint8_t *frame, const char *dispname,
                               bool issf, const char *gcode, uint8_t ver, int save_type) {
  char tmp[64];
  draw_text_ovf(msgs[lang_id][MSG_GBALOAD_MINFO], frame, 24, 23, 192);

  const char *romname = file_basename(dispname);
  draw_text_ovf_rotate(romname, frame, 12, 52, SCREEN_WIDTH-24, &spop.anim);

  npf_snprintf(tmp, sizeof(tmp), msgs[lang_id][MSG_LOADINFO_GAME], gcode, ver);
  draw_text_ovf(tmp, frame, 12, 82, SCREEN_WIDTH - 24);

  if (save_type < 0)
    draw_text_ovf(msgs[lang_id][MSG_LOADINFO_UNKW], frame, 12, 102, SCREEN_WIDTH - 24);
  else if (issf)
    draw_text_ovf("SuperFW firmware", frame, 12, 102, SCREEN_WIDTH - 24);
  else {
    const char *stype[] = {
      msgs[lang_id][MSG_SAVETYPE_NONE],       // SaveTypeNone
      msgs[lang_id][MSG_SAVETYPE_SRAM],       // SaveTypeSRAM
      msgs[lang_id][MSG_SAVETYPE_EEPROM],     // SaveTypeEEPROM4K
      msgs[lang_id][MSG_SAVETYPE_EEPROM],     // SaveTypeEEPROM64K
      msgs[lang_id][MSG_SAVETYPE_FLASH],      // SaveTypeFlash512K
      msgs[lang_id][MSG_SAVETYPE_FLASH],      // SaveTypeFlash1024K
    };
    const char *ssize[] = {
      "0KB",       // SaveTypeNone
      "32KB",      // SaveTypeSRAM
      "0.5KB",     // SaveTypeEEPROM4K
      "8KB",       // SaveTypeEEPROM64K
      "64KB",      // SaveTypeFlash512K
      "128KB",     // SaveTypeFlash1024K
    };

    npf_snprintf(tmp, sizeof(tmp), msgs[lang_id][MSG_LOADINFO_SAVE], stype[save_type], ssize[save_type]);
    draw_text_ovf(tmp, frame, 12, 102, SCREEN_WIDTH - 24);
  }

  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_full(frame, 20, 220, 132, 152, FG_COLOR, HI_COLOR);
}

static const char *render_gbarom_patching(volatile uint8_t *frame, const t_load_gba_info *info, int selector) {
  if (menu_theme >= THEME_DSSMM_LIGHT)
    draw_menu_selection(frame, 44 + 18*selector, 8, 232);
  draw_text_ovf(msgs[lang_id][MSG_GBALOAD_MPATCH], frame, 24, 23, 192);
  draw_text_ovf(msgs[lang_id][MSG_DEFS_PATCH], frame, 12, 44, 116);
  draw_text_ovf(msgs[lang_id][MSG_PATCH_TYPE0 + info->patch_type], frame, 132, 44, 96);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_SAVET], frame, 12, 62, 116);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_ST0 + (info->use_dsaving ? 0 : 1)], frame, 132, 62, 96);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_MENU], frame, 12, 80, 116);
  draw_text_ovf(msgs[lang_id][info->ingame_menu_enabled ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, 132, 80, 96);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_RTCE], frame, 12, 98, 116);
  draw_text_ovf(msgs[lang_id][info->rtc_patch_enabled ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, 132, 98, 96);

  draw_text_ovf(msgs[lang_id][MSG_LOADER_PTCH], frame, 12, 116, 116);
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 170 - 20, 170 + 20, 115, 133, FG_COLOR);
  draw_central_text("▸", frame, 170, 116);

  return (selector == GBALoadPatch) ? msgs[lang_id][MSG_PATCH_TYPE_I0 + info->patch_type] :
         (selector == GBASavePatch) ? msgs[lang_id][MSG_LOADER_ST_I0 + (info->use_dsaving ? 0 : 1)] :
         (selector == GBAInGameMen) ? msgs[lang_id][MSG_INGAME_I] :
         (selector == GBARTCPatch)  ? msgs[lang_id][MSG_PATCHRTC_I] :
         (selector == GBAPatchGen)  ? msgs[lang_id][MSG_PATCHE_I] : NULL;
}

static const char *render_gbarom_loading(volatile uint8_t *frame, const t_load_gba_lcfg *data, bool rtc_patching, int selector) {
  if (menu_theme >= THEME_DSSMM_LIGHT)
    draw_menu_selection(frame, 44 + 18*selector, 8, 232);
  char tmp[64];
  draw_text_ovf(msgs[lang_id][MSG_GBALOAD_OPTS], frame, 24, 23, 192);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_LOADP], frame, 12, 44, 116);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_LOADP0 + data->sram_load_type], frame, 132, 44, 96);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_SAVEP], frame, 12, 62, 116);
  draw_text_ovf(msgs[lang_id][MSG_LOADER_SAVEP0 + data->sram_save_type], frame, 132, 62, 96);
  draw_text_ovf(msgs[lang_id][MSG_DEF_RTCVAL], frame, 12, 80, 116);
  if (rtc_patching) {
    t_dec_date d;
    timestamp2date(data->rtcval, &d);
    npf_snprintf(tmp, sizeof(tmp), "%02d/%02d/%02d %02d:%02d",
      d.year, d.month, d.day, d.hour, d.min);
    draw_text_ovf(tmp, frame, 112, 80, 120);
  }
  else
    draw_text_ovf("-", frame, 132, 80, 96);
  draw_text_ovf(msgs[lang_id][MSG_SETT_LDCHT], frame, 12, 98, 116);
  draw_text_ovf(msgs[lang_id][data->use_cheats ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, 132, 98, 96);

  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 170 - 20, 170 + 20, 115, 133, FG_COLOR);
  draw_text_ovf(msgs[lang_id][MSG_SETT_REMEMB], frame, 12, 116, 116);
  render_icon(170-8, 116, ICON_DISK);

  return (selector == GBALdSetLoadP) ? msgs[lang_id][MSG_LOADER_LOADP_I0 + data->sram_load_type] :
         (selector == GBALdSetSaveP) ? msgs[lang_id][MSG_LOADER_SAVEP_I0 + data->sram_save_type] :
         (selector == GBALdSetCheats && !enable_cheats) ? msgs[lang_id][MSG_CHEATSDIS_I] :
         (selector == GBALdSetCheats && !data->cheats_found) ? msgs[lang_id][MSG_CHEATSNOA_I] :
         (selector == GBALdRemember) ? msgs[lang_id][MSG_REMEMB_I] : NULL;
}

static void render_gba_action(volatile uint8_t *frame, const char *label, bool selectable) {
  unsigned y=menu_theme>=THEME_DSSMM_LIGHT && menu_font!=FONT_STYLE_DSSMM?132:134;
  if(menu_theme>=THEME_DSSMM_LIGHT)draw_menu_selection(frame,y,8,232);
  draw_central_text_ovf(label, frame, 120, y, 192);
  if (selectable) {
    draw_text_ovf("<", frame, 12, y, 16);
    draw_rightj_text(">", frame, 228, y);
  }
}

static void render_gba_help(volatile uint8_t *frame, const char *text) {
  if (!text)return;
  unsigned y=menu_theme>=THEME_DSSMM_LIGHT?(menu_font==FONT_STYLE_DSSMM?135:133):137;
  if (font_width(text)>SCREEN_WIDTH-20)
    draw_text_ovf_rotate(text,frame,10,y,SCREEN_WIDTH-20,&spop.anim);
  else
    draw_central_text_ovf(text,frame,SCREEN_WIDTH/2,y,SCREEN_WIDTH-20);
}

void render_gba_load_popup(volatile uint8_t *frame) {
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);
  draw_text_ovf("⯇", frame, 10, 23, 64);
  draw_rightj_text("⯈", frame, SCREEN_WIDTH - 10, 23);

  const t_load_gba_info *info = &spop.p.load.i;
  const t_patch *p = get_game_patch(info);
  const char *ht = NULL;
  switch (spop.submenu) {
  case GbaLoadPopInfo:
    render_gbarom_info(frame, info->romfn, is_superfw(&info->romh), info->gcode,
                       info->romh.version, p ? p->save_mode : -1);
    #ifdef SUPPORT_NORGAMES
    render_gba_action(frame, gba_write_nor ? "Write GBA ROM to NOR" : msgs[lang_id][MSG_LOAD_GBA], true);
    #else
    render_gba_action(frame, msgs[lang_id][MSG_LOAD_GBA], false);
    #endif
    break;
  case GbaLoadPopLoadS:
    ht = render_gbarom_loading(frame, &spop.p.load.l, info->rtc_patch_enabled, spop.selector);
    break;
  case GbaLoadPopPatch:
    ht = render_gbarom_patching(frame, &spop.p.load.i, spop.selector);
    break;
  };

  // Show some help if necessary
  render_gba_help(frame,ht);

  if (spop.submenu != GbaLoadPopInfo) {
    const unsigned offy = 43;
    if (menu_theme < THEME_DSSMM_LIGHT)
    for (unsigned i = 8; i < 232; i += 16) {
      render_icon_trans(i, offy + 0 + spop.selector * 18, 63);
      render_icon_trans(i, offy + 2 + spop.selector * 18, 63);
    }
  }
}

void render_filemgr(volatile uint8_t *frame) {
  // Draw the file name and the options available
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  t_centry *e = sdr_state->filefilter[smenu.browser.selector];
  const char *bn = file_basename(e->fname);

  unsigned twidth = font_width(bn);
  if (twidth > SCREEN_WIDTH - 20)
    draw_text_ovf_rotate(bn, frame, 10, 32, SCREEN_WIDTH - 20, &spop.anim);
  else
    draw_central_text_ovf(bn, frame, SCREEN_WIDTH/2, 32, SCREEN_WIDTH - 20);

  for (unsigned i = 0; i < FiMgrCNT; i++)
    draw_button_box(frame, 20, 220, 60 + i*30, 80 + i*30, i == spop.selector);

  draw_text_ovf(msgs[lang_id][MSG_FMGR_DEL], frame, 24, 62 + 30*FiMgrDelete, 192);
  draw_text_ovf(msgs[lang_id][(e->attr & AM_HID) ? MSG_FMGR_UNHIDE : MSG_FMGR_HIDE], frame, 24, 62 + 30*FiMgrHide, 192);

  #ifdef SUPPORT_NORGAMES
  draw_text_ovf(msgs[lang_id][MSG_NOR_WRITE], frame, 24, 62 + 30*FiMgrWriteNOR, 192);
  #endif
}


#ifdef SUPPORT_NORGAMES
void render_gba_norwrite(volatile uint8_t *frame) {
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  draw_text_ovf("⯇", frame, 10, 23, 64);
  draw_rightj_text("⯈", frame, SCREEN_WIDTH - 10, 23);

  if (spop.submenu == GbaLoadPopInfo) {
    const t_load_gba_info *info = &spop.p.norwr.i;
    const t_patch *p = get_game_patch(info);
    render_gbarom_info(frame, info->romfn, is_superfw(&info->romh),
                       info->gcode, info->romh.version, p ? p->save_mode : -1);
    render_gba_action(frame,msgs[lang_id][MSG_NOR_WRITE],false);
  } else {
    const char *ht = render_gbarom_patching(frame, &spop.p.norwr.i, spop.selector);
    render_gba_help(frame,ht);
    const unsigned offy = 43;
    if (menu_theme < THEME_DSSMM_LIGHT)
    for (unsigned i = 8; i < 232; i += 16) {
      render_icon_trans(i, offy + 0 + spop.selector * 18, 63);
      render_icon_trans(i, offy + 2 + spop.selector * 18, 63);
    }
  }
}

void render_gba_norload(volatile uint8_t *frame) {
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  draw_text_ovf("⯇", frame, 10, 23, 64);
  draw_rightj_text("⯈", frame, SCREEN_WIDTH - 10, 23);

  const t_flash_game_entry *e = spop.p.norld.e;
  if (spop.submenu == GbaLoadPopInfo) {
    int save_type = GET_GATTR_SAVEM(e->gattrs);
    char gcode[5] = {
      e->gamecode & 0xFF, (e->gamecode >> 8) & 0xFF, (e->gamecode >> 16) & 0xFF, e->gamecode >> 24, 0
    };
    render_gbarom_info(frame, e->game_name, false, gcode, e->gamever, save_type);
    render_gba_action(frame,msgs[lang_id][MSG_NOR_LAUNCH],false);
  } else {
    bool rtc_patching = e->gattrs & GATTR_RTC;
    const char *ht = render_gbarom_loading(frame, &spop.p.norld.l, rtc_patching, spop.selector);
    render_gba_help(frame,ht);
    const unsigned offy = 43;
    if (menu_theme < THEME_DSSMM_LIGHT)
    for (unsigned i = 8; i < 232; i += 16) {
      render_icon_trans(i, offy + 0 + spop.selector * 18, 63);
      render_icon_trans(i, offy + 2 + spop.selector * 18, 63);
    }
  }
}
#endif

void render_popupq(volatile uint8_t *frame, unsigned fcnt) {
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  // Draw question and two buttons
  draw_central_text_wrapped(spop.qpop.message, frame, SCREEN_WIDTH/2, 32, SCREEN_WIDTH - 20);

  if (spop.qpop.option == 0) {
    draw_box_full(frame, 20, 220, 90, 90 + 20, FG_COLOR, HI_COLOR);
    draw_box_outline(frame, 20, 220, 120, 120 + 20, FG_COLOR);
  } else {
    draw_box_full(frame, 20, 220, 120, 120 + 20, FG_COLOR, HI_COLOR);
    draw_box_outline(frame, 20, 220, 90, 90 + 20, FG_COLOR);
  }

  draw_central_text(spop.qpop.default_button, frame, 120, 92);
  draw_central_text(spop.qpop.confirm_button, frame, 120, 122);
}

void render_rtcpop(volatile uint8_t *frame) {
  if(spop.rtcpop.menu_clock) {
    draw_central_text("Set time",frame,120,30);
    char time[8];npf_snprintf(time,sizeof(time),"%02u:%02u",(unsigned)(uint8_t)spop.rtcpop.val.hour%24,(unsigned)(uint8_t)spop.rtcpop.val.min%60);
    draw_central_text(time,frame,120,70);
    char hours[3]={time[0],time[1],0},minutes[3]={time[3],time[4],0};
    unsigned x=120-font_width(time)/2+(spop.rtcpop.selector==3?font_width(hours)/2:font_width(hours)+font_width(":")+font_width(minutes)/2);
    draw_central_text("⯅",frame,x,52);draw_central_text("⯆",frame,x,88);
    draw_central_text("A: Set   B: Cancel",frame,120,112);
    draw_central_text("Clock runs while menu is open",frame,120,menu_font==FONT_STYLE_DSSMM?134:132);
    return;
  }
  if(menu_theme<THEME_DSSMM_LIGHT)draw_box_outline(frame, 2, 240-2, 18, 158, FG_COLOR);

  draw_central_text(menu_theme>=THEME_DSSMM_LIGHT?"Set time":msgs[lang_id][MSG_DEF_RTCVAL], frame, SCREEN_WIDTH/2, 32);

  const t_dec_date *v = &spop.rtcpop.val;
  char thour[3] = {'0' + v->hour /10, '0' + v->hour  % 10, 0};
  char tmins[3] = {'0' + v->min  /10, '0' + v->min   % 10, 0};
  char tdays[3] = {'0' + v->day  /10, '0' + v->day   % 10, 0};
  char tmont[3] = {'0' + v->month/10, '0' + v->month % 10, 0};
  char tyear[5] = {'2', '0', '0' + v->year/10, '0' + v->year % 10, 0};

  draw_central_text(tyear, frame,  60, 70);
  draw_central_text("-",   frame,  80, 70);
  draw_central_text(tmont, frame,  94, 70);
  draw_central_text("-",   frame, 106, 70);
  draw_central_text(tdays, frame, 120, 70);
  draw_central_text(thour, frame, 154, 70);
  draw_central_text(":",   frame, 166, 70);
  draw_central_text(tmins, frame, 180, 70);

  const uint8_t cox[] = {
    60, 94, 120, 154, 180
  };
  draw_central_text("⯅", frame, cox[spop.rtcpop.selector], 54);
  draw_central_text("⯆", frame, cox[spop.rtcpop.selector], 84);
  draw_central_text("A: Set   B: Cancel",frame,120,112);
}

static void render_dssmm_settings_background(volatile uint8_t *frame,const char *title);
static unsigned settings_visible_rows(void) {
  return menu_theme<THEME_DSSMM_LIGHT?5:menu_font==FONT_STYLE_DSSMM?9:7;
}
static unsigned ui_visible_rows(void) {
  return menu_theme<THEME_DSSMM_LIGHT?6:menu_font==FONT_STYLE_DSSMM?9:7;
}

/* Filter settings by category while retaining original enum/action identities. */
static const uint8_t settings_games[]={SettHotkey,SettSaveBkp,SettCheatEn,DefsPatchEng,DefsGamMenu,DefsRTCEnb,DefsRTCVal,DefsRTCSpeed,DefsLoadPol,DefsSavePol,DefsPrefDS};
static const uint8_t settings_hardware[]={SettBootType,SettFastSD,
#ifdef SUPPORT_NORGAMES
  SettVerifyNOR,
#endif
  SettFastEWRAM};
static unsigned settings_group_total(void) {
  if(menu_theme<THEME_DSSMM_LIGHT)return SettMAX;
  if(!launcher_settings_group)return SettMAX-2;
  return launcher_settings_group==1?sizeof(settings_games):sizeof(settings_hardware);
}
static unsigned settings_option_at(unsigned index) {
  if(menu_theme<THEME_DSSMM_LIGHT)return index;
  if(!launcher_settings_group){unsigned option=index+1;return option>=SettTitle2?option+1:option;}
  return launcher_settings_group==1?settings_games[index]:settings_hardware[index];
}
static unsigned settings_position(unsigned option) {
  for(unsigned i=0;i<settings_group_total();++i)if(settings_option_at(i)==option)return i;
  return settings_group_total();
}
static bool settings_row_visible(unsigned option,unsigned base) {
  unsigned position=settings_position(option);
  return position<settings_group_total() && position>=base && position<base+settings_visible_rows();
}

static void settings_move(unsigned newkeys) {
  unsigned position=settings_position(smenu.set.selector),total=settings_group_total();
  if(position>=total){position=0;smenu.set.selector=settings_option_at(0);}
  if (newkeys & KEY_BUTTUP)
    smenu.set.selector = settings_option_at(position?position-1:0);
  if (newkeys & KEY_BUTTDOWN)
    smenu.set.selector = settings_option_at(MIN(total-1,position+1));
}

void render_settings(volatile uint8_t *frame) {
  char tmp[128];
  bool ds_style=menu_theme>=THEME_DSSMM_LIGHT;
  if(ds_style)render_dssmm_settings_background(frame,launcher_settings_group==1?"Games":launcher_settings_group==2?"Hardware":"Settings");
  unsigned position=settings_position(smenu.set.selector),total=settings_group_total();
  if(position>=total)position=0;
  unsigned visible=settings_visible_rows(),half=visible/2;
  unsigned baseopt=total<=visible || position<=half?0:position>=total-half?total-visible:position-half;

  if(position>half)draw_central_text("⯅",frame,ds_style?230:120,ds_style?20:15);
  if(position+half+1<total)draw_central_text("⯆",frame,ds_style?230:120,ds_style?136:125);

  const unsigned maxrows = visible;
  unsigned optnum = 0, optcnt = 0;
  const unsigned value_x = ds_style?119:128;
  const unsigned offy = ds_style?(menu_font==FONT_STYLE_DSSMM?23:24):29;
  const unsigned rowh = ds_style?(menu_font==FONT_STYLE_DSSMM?14:18):20;
  if (menu_theme >= THEME_DSSMM_LIGHT)
    draw_menu_selection(frame,offy+(position-baseopt)*rowh,112,224);

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows)
    draw_text_ovf(msgs[lang_id][MSG_SET_TITL1], frame, ds_style?23:8, offy + rowh*optcnt++, ds_style?88:224);

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    npf_snprintf(tmp, sizeof(tmp), "< %s >", hotkey_list[hotkey_combo].cname);
    draw_text_ovf(msgs[lang_id][MSG_SETT_HOTK], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(tmp, frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_SETT_BOOT], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][MSG_BOOT_TYPE0 + boot_bios_splash], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_SETT_FASTSD], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][use_slowld ? MSG_KNOB_DISABLED : MSG_KNOB_ENABLED], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  #ifdef SUPPORT_NORGAMES
  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_SETT_VERNOR], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][use_verify_nor ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }
  #endif

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_SETT_FASTEW], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][use_fastew ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    npf_snprintf(tmp, sizeof(tmp), "< %u >", backup_sram_default);
    draw_text_ovf(msgs[lang_id][MSG_SETT_SAVEBK], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(tmp, frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_SETT_CHTEN], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][enable_cheats ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows)
    draw_text_ovf(msgs[lang_id][MSG_SET_TITL2], frame, ds_style?23:8, offy + rowh*optcnt++, ds_style?88:224);

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_DEFS_PATCH], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][MSG_PATCH_TYPE0 + patcher_default], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_LOADER_MENU], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][MSG_KNOB_DISABLED + ingamemenu_default], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_LOADER_RTCE], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][MSG_KNOB_DISABLED + rtcpatch_default], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    t_dec_date d;
    timestamp2date(rtcvalue_default, &d);
    npf_snprintf(tmp, sizeof(tmp), "%02d/%02d/%02d %02d:%02d",
      d.year, d.month, d.day, d.hour, d.min);
    draw_text_ovf(msgs[lang_id][MSG_DEF_RTCVAL], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(tmp, frame, ds_style?119:112, offy + rowh*optcnt, ds_style?104:120, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    unsigned spdmsg = rtcspeed_default ? (MSG_UIS_SPD0 + rtcspeed_default - 1) :
                                          MSG_STILLRTC;
    npf_snprintf(tmp, sizeof(tmp), "< %s >", msgs[lang_id][spdmsg]);
    draw_text_ovf(msgs[lang_id][MSG_DEF_SPEED], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(tmp, frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_LOADER_LOADP], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][MSG_DEF_LOADP0 + (autoload_default ^ 1)], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_LOADER_SAVEP], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][autosave_default ? MSG_DEF_SAVEP0 : MSG_DEF_SAVEP1], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if (settings_row_visible(optnum++,baseopt) && optcnt < maxrows) {
    draw_text_ovf(msgs[lang_id][MSG_LOADER_PREFDS], frame, ds_style?23:8, offy + rowh*optcnt, ds_style?88:116);
    draw_text_ovf_color(msgs[lang_id][autosave_prefer_ds ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED], frame, value_x, offy + rowh*optcnt, 104, ds_style && baseopt+optcnt==position?SELECTED_TEXT:FT_COLOR); ++optcnt;
  }

  if(!ds_style) {
  // Render bar below for help messge
  dma_memset16(&frame[240*140], dup8(FG_COLOR), 240*20/2);

  {
    unsigned help_msg = smenu.set.selector == SettBootType ? MSG_BOOT_TYPE_I0 + boot_bios_splash :
                        smenu.set.selector == SettSaveBkp  ? MSG_BACKUP_I :
                        smenu.set.selector == SettFastSD   ? MSG_FASTSD_I :
                        smenu.set.selector == SettFastEWRAM? MSG_FASTEW_I :
                        smenu.set.selector == DefsPatchEng ? MSG_PATCH_TYPE_I0 + patcher_default :
                        smenu.set.selector == DefsLoadPol  ? MSG_DEF_LOADP_I0 + (autoload_default ^ 1) :
                        smenu.set.selector == DefsSavePol  ? MSG_DEF_SAVEP_I0 + (autosave_default ^ 1) :
                        smenu.set.selector == DefsPrefDS   ? MSG_LOADER_PREFDSI :
                        #ifdef SUPPORT_NORGAMES
                        smenu.set.selector == SettVerifyNOR ? MSG_VERNOR_I :
                        #endif
                        MSG_EMPTY;
    draw_text_ovf_rotate(msgs[lang_id][help_msg], frame, 4, SCREEN_HEIGHT - 18, 232, &smenu.anim_state);
  }

  }
  if (menu_theme < THEME_DSSMM_LIGHT && smenu.set.selector != SettSave)
    for (unsigned i = 0; i < 240; i += 16)
      render_icon_trans(i, offy + (position - baseopt) * 20, 63);
}

void render_ui_settings(volatile uint8_t *frame) {
  static const char * const theme_names[THEME_COUNT] = {
    "White", "Orange", "Blue", "Green", "Purple", "Gray",
    "Light", "Dark"
  };
  static const char * const font_names[] = {"SuperFW", "DSSMM"};
  static const char * const artwork_names[] = { "List", "Title", "Box" };
  static const char * const view_names[] = { "List", "Horizontal", "Vertical" };
  bool ds_style=menu_theme>=THEME_DSSMM_LIGHT;
  if(ds_style)render_dssmm_settings_background(frame,"Interface");
  const unsigned value_x=ds_style?119:128;
  const unsigned rows=ui_visible_rows(),line_h=ds_style?(menu_font==FONT_STYLE_DSSMM?14:18):18;
  char tmpbuf[64];
  const char * const labels[UiSetSave] = {
    msgs[lang_id][MSG_UIS_THEME], "Font", "Artwork", "Browser view", msgs[lang_id][MSG_UIS_LANG],
    msgs[lang_id][MSG_UIS_RECNT], msgs[lang_id][MSG_UIS_ANSPD], msgs[lang_id][MSG_UIS_BHID], "Accent", "Top bar color", "Clean list", "Clock format", "Wrap around"
  };
  for (unsigned row = 0; row < rows; ++row) {
    unsigned option = smenu.uiset.seloff + row;
    if(option>UiSetMAX)break;
    unsigned y=(ds_style?(menu_font==FONT_STYLE_DSSMM?23:24):20)+row*line_h;
    const char *value = "";
    switch (option) {
    case UiSetTheme: value = theme_names[menu_theme < THEME_COUNT ? menu_theme : 0]; break;
    case UiSetFont: value=font_names[menu_font<FONT_STYLE_COUNT ? menu_font : 0];break;
    case UiSetArtwork: value = artwork_names[artwork_mode < 3 ? artwork_mode : 0]; break;
    case UiSetView: value = view_names[browser_view < 3 ? browser_view : 0]; break;
    case UiSetLang: value = msgs[lang_id][MSG_LANG_NAME]; break;
    case UiSetRect: value = msgs[lang_id][recent_menu ? MSG_KNOB_ENABLED : MSG_KNOB_DISABLED]; break;
    case UiSetASpd: value = msgs[lang_id][MSG_UIS_SPD0 + anim_speed]; break;
    case UiSetHid: value = msgs[lang_id][hide_hidden ? MSG_KNOB_DISABLED : MSG_KNOB_ENABLED]; break;
    case UiSetWrap: value=carousel_wrap?"On":"Off";break;
    case UiSetClock: value=menu_clock_24?"24 hour":"12 hour";break;
    case UiSetClean: value=msgs[lang_id][clean_list?MSG_KNOB_ENABLED:MSG_KNOB_DISABLED];break;
    case UiSetTopBar: value=accent_names[menu_topbar<ACCENT_COUNT?menu_topbar:0];break;
    case UiSetAccent: value = accent_names[menu_accent < ACCENT_COUNT ? menu_accent : 0]; break;
    }
    if (ds_style && (int)option == smenu.uiset.selector)
      draw_menu_selection(frame,y,112,224);
    draw_text_ovf(labels[option],frame,ds_style?23:8,y,ds_style?88:116);
    /* Localized native knob labels may already include their arrows. */
    npf_snprintf(tmpbuf, sizeof(tmpbuf), value[0] == '<' ? "%s" : "< %s >", value);
    draw_text_ovf_color(tmpbuf,frame,value_x,y,104,ds_style && (int)option==smenu.uiset.selector?SELECTED_TEXT:FT_COLOR);
  }

  if (menu_theme < THEME_DSSMM_LIGHT && smenu.uiset.selector != UiSetSave)
    for (unsigned i = 0; i < 240; i += 16)
      render_icon_trans(i, 20+(smenu.uiset.selector-smenu.uiset.seloff)*18, 63);


}

void render_info(volatile uint8_t *frame) {
  if(menu_theme>=THEME_DSSMM_LIGHT)draw_lined_panel(frame);
  uint32_t vmaj = VERSION_WORD >> 16;
  uint32_t vmin = VERSION_WORD & 0xFFFF;
  char tmp[64], tmp2[32];

  if(menu_theme<THEME_DSSMM_LIGHT) {
    init_logo_palette(&MEM_PALETTE[1]);
    render_logo((uint16_t*)frame,SCREEN_WIDTH/2,40,4);
  } else {
    init_logo_palette(&MEM_PALETTE[236]);
    render_logo_indexed((uint16_t*)frame,120,46,4,236);
  }

  switch (smenu.info.selector) {
  case 0:
    npf_snprintf(tmp, sizeof(tmp), "SuperFW %lu.%lu (%08lx)", vmaj, vmin,
                 (unsigned long)VERSION_SLUG_WORD);
    draw_central_text_ovf(tmp, frame, 120, 70, 216);
    npf_snprintf(tmp, sizeof(tmp), "DSSMM Build %lu", (unsigned long)DSSMM_BUILD_NUMBER);
    draw_central_text(tmp, frame, 120, 95);
    draw_central_text(FW_FLAVOUR " variant", frame, 120, 114);
    break;
  case 1:
    draw_text_ovf("Flash info", frame, 12, 70, 216);
    npf_snprintf(tmp, sizeof(tmp), "Dev ID: %08lx", flashinfo.deviceid);
    draw_central_text(tmp, frame, 120, 95);
    if (flashinfo.size && flashinfo.blksize && flashinfo.blkcount) {
      human_size_kb(tmp2, sizeof(tmp2), flashinfo.size >> 10);
      npf_snprintf(tmp, sizeof(tmp), "%s [%lu * %lu]", tmp2, flashinfo.blksize, flashinfo.blkcount);
      if (flashinfo.regioncnt != 1)
        strcat(tmp, " !");
      draw_text_ovf(tmp, frame, 12, 115, 216);
    } else {
      npf_snprintf(tmp, sizeof(tmp), "No CFI! (hardwired %dKiB)", FW_MAX_SIZE_KB);
      draw_text_ovf(tmp, frame, 12, 115, 216);
    }
    break;
  case 2:
    draw_text_ovf(msgs[lang_id][MSG_DBPINFO], frame, 12, 70, 216);
    npf_snprintf(tmp, sizeof(tmp), "%s - %s", pdbinfo.version, pdbinfo.date);
    draw_text_ovf(tmp, frame, 12, 90, 216);
    npf_snprintf(tmp, sizeof(tmp), "Game count: %lu", pdbinfo.patch_count);
    draw_text_ovf(tmp, frame, 12, 110, 216);
    break;
  case 3:
    npf_snprintf(tmp, sizeof(tmp), "SD Card ID: %02x | %04x", sd_info.manufacturer, sd_info.oemid);
    draw_text_ovf(tmp, frame, 12, 70, 216);
    human_size_kb(tmp2, sizeof(tmp2), sd_info.block_cnt / 2);
    npf_snprintf(tmp, sizeof(tmp), "Type: %s  Size: %s", sd_info.sdhc ? "SDHC" : "SDSC", tmp2);
    draw_text_ovf(tmp, frame, 12, 90, 216);
    npf_snprintf(tmp, sizeof(tmp), "Info: %c%c%c%c%c (%d/%d)",
      sd_info.prodname[0] ?: '#', sd_info.prodname[1] ?: '#',
      sd_info.prodname[2] ?: '#', sd_info.prodname[3] ?: '#',
      sd_info.prodname[4] ?: '#', 2000 + sd_info.year, sd_info.month);
    draw_text_ovf(tmp, frame, 12, 110, 216);
    break;
  }

  // Flashing info
  if(menu_theme<THEME_DSSMM_LIGHT)dma_memset16(&frame[138*SCREEN_WIDTH],dup8(FG_COLOR),SCREEN_WIDTH*22/2);
  unsigned unlock_y=menu_theme>=THEME_DSSMM_LIGHT?(menu_font==FONT_STYLE_DSSMM?135:133):141;
  draw_text_ovf_rotate(enable_flashing ? msgs[lang_id][MSG_FWUP_ENABLED] : msgs[lang_id][MSG_FWUP_HOTKEY], frame,
                       5, unlock_y, menu_theme>=THEME_DSSMM_LIGHT?228:SCREEN_WIDTH-10, &smenu.anim_state);
}

void render_tools(volatile uint8_t *frame) {
  unsigned pitch=menu_theme<THEME_DSSMM_LIGHT?22:menu_font==FONT_STYLE_DSSMM?14:18;
  if (menu_theme >= THEME_DSSMM_LIGHT) {
    draw_lined_panel(frame);
    draw_menu_selection(frame, 86 - (pitch*(ToolsMAX-1)+16)/2 + pitch*smenu.tools.selector, 18, 232);
  }
  unsigned top = menu_theme>=THEME_DSSMM_LIGHT ? 86 - (pitch*(ToolsMAX-1)+16)/2 : 26;
  for (unsigned i = 0; i < ToolsMAX; i++)
    draw_text_ovf(msgs[lang_id][MSG_TOOLS0_SDRAM + i], frame, 22, top + pitch * i, 206);

  smenu.anim_state = (smenu.anim_state + 1) & 255;
  draw_central_text("▸", frame, menu_theme>=THEME_DSSMM_LIGHT?10:11 + (smenu.anim_state >> 6), top + pitch * smenu.tools.selector);

  if (menu_theme < THEME_DSSMM_LIGHT)
    for (unsigned i = 0; i < 240; i += 16)
      render_icon_trans(i, 26 + smenu.tools.selector * 22, 63);
}

void reload_theme(unsigned thnum) {
  font_set_style(menu_font);
  launcher_invalidate();
  const t_theme_colors colors = menu_theme_colors(thnum, menu_accent);
  for (unsigned i = 0; i < ARTWORK_PALETTE_SIZE; ++i)
    MEM_PALETTE[ARTWORK_PALETTE_BASE + i] = artwork_palette_color(i);
  // Palette 0..15 contains the main menu template colors
  MEM_PALETTE[FG_COLOR] = colors.fg_color;
  MEM_PALETTE[BG_COLOR] = colors.bg_color;
  MEM_PALETTE[FT_COLOR] = colors.ft_color;
  MEM_PALETTE[HI_COLOR] = colors.hi_color;
  MEM_PALETTE[TOP_BG]=menu_topbar?menu_theme_colors(thnum,menu_topbar).hi_color:themes[THEME_DSSMM_LIGHT].fg_color;
  MEM_PALETTE[TOP_TEXT]=menu_topbar?0x7fff:0;
  MEM_PALETTE[SELECTED_TEXT]=0x7fff;
  // In-game menu palette
  MEM_PALETTE[IGM_PAL_FG] = colors.fg_color;
  MEM_PALETTE[IGM_PAL_BG] = colors.bg_color;
  MEM_PALETTE[IGM_PAL_HI] = colors.ft_color;
  MEM_PALETTE[IGM_PAL_SH] = colors.sh_color;
  MEM_PALETTE[IGM_PAL_BL] = colors.hi_blend;
  MEM_PALETTE[IGM_PAL_FONT] = menu_font;

  // Palette entries for icons and other objects
  MEM_PALETTE[256 + SEL_COLOR] = colors.hi_blend;
}

/* Shared header format; persisted time remains24-hour seconds. */
static void format_menu_clock(char *out,unsigned size,uint32_t seconds) {
  if(seconds==UINT32_MAX){strbcpy(out,"--:--:--",size);return;}
  unsigned hour=seconds/3600%24;
  if(menu_clock_24)npf_snprintf(out,size,"%02u:%02u:%02u",hour,(unsigned)(seconds/60%60),(unsigned)(seconds%60));
  else npf_snprintf(out,size,"%02u:%02u %s",hour%12?hour%12:12,(unsigned)(seconds/60%60),hour<12?"AM":"PM");
}

/* Mask partial Mode4 pairs; odd-width strokes never byte-write VRAM. */
static void launcher_corner_span(volatile uint8_t *frame,unsigned x,unsigned y,unsigned width) {
  for(unsigned px=x&~1u;px<x+width;px+=2) {
    volatile uint16_t *dest=(volatile uint16_t *)(frame+y*240+px);
    unsigned mask=(px>=x?255:0)|((px+1<x+width)?0xff00:0);
    *dest=(*dest&~mask)|(dup8(y<16 && menu_topbar?TOP_TEXT:HI_COLOR)&mask);
  }
}
static void launcher_corners(volatile uint8_t *frame,unsigned x,unsigned y,unsigned w,unsigned h) {
  if(w<8 || h<8 || x+w>240 || y+h>160)return;
  unsigned arm=MIN(8,MIN((w-4)/2,(h-4)/2)),stroke=h<20?3:4;
  for(unsigned row=0;row<h;++row) {
    unsigned width=(row<3 || row>=h-3)?arm:stroke;
    if(row<arm || row>=h-arm) {
      launcher_corner_span(frame,x,y+row,width);
      launcher_corner_span(frame,x+w-width,y+row,width);
    }
  }
}

/* SD/Lite substitute Recents for the Chis-only NOR shortcut. */
__attribute__((optimize("Os")))
static void render_start(volatile uint8_t *frame) {
  dssmm_background_draw(frame,menu_theme==THEME_DSSMM_DARK,true);
  char title_buffer[MAX_FN_LEN];
  const char *title="No recent game";
  if(smenu.recent.maxentries) {
    const t_rentry *entry=&sdr_state->rentries[0];
    strbcpy(title_buffer,file_basename(entry->fpath),sizeof(title_buffer));
    if(clean_list)clean_display_name(title_buffer,sizeof(title_buffer),entry->fpath);
    else {char *extension=strrchr(title_buffer,'.');if(extension)*extension=0;}
    title=title_buffer;
    if(artwork_mode && !(entry->flags&FLAG_RECENT_NOR)) {
      if(!artwork_cached || artwork_cached_mode!=artwork_mode || strcmp(artwork_rom,entry->fpath)) {
        strbcpy(artwork_rom,entry->fpath,sizeof(artwork_rom));artwork_cached_mode=artwork_mode;
        artwork_valid=artwork_load(entry->fpath,artwork_mode-1,
          (uint8_t *)sdr_state->artwork_pixels,ARTWORK_MAX_PIXELS,
          sdr_state->artwork_bmp_buffer,sizeof(sdr_state->artwork_bmp_buffer),&preview_info);
        artwork_cached=true;
      }
      if(artwork_valid)artwork_blit_scaled_cropped((volatile uint16_t *)frame,
          sdr_state->artwork_pixels,&preview_info,preview_info.width==80?40:30,preview_info.width==80?46:48,preview_info.width==80?40:56,preview_info.width==80?40:37);
      else draw_central_text("GBA",frame,60,60);
    }
  }
  const unsigned lines=menu_font==FONT_STYLE_DSSMM?3:2;
  unsigned rows=0;
  for(const char *scan=title;rows<lines && *scan;++rows) {
    unsigned width,count=font_width_cap_space(scan,110,&width);
    if(!count)count=font_width_cap(scan,110);
    if(!count)break;
    scan+=count;while(*scan==' ')++scan;
  }
  unsigned top=launcher_title_top(rows,menu_font==FONT_STYLE_DSSMM);
  for(unsigned row=0;row<lines && *title;++row) {
    unsigned width,count=font_width_cap_space(title,110,&width);
    if(!count)count=font_width_cap(title,110);
    if(!count)break;
    char line[128];if(count>=sizeof(line))count=sizeof(line)-1;
    memcpy(line,title,count);line[count]=0;
    draw_central_text_ovf(line,frame,150,top+row*(lines==3?11:16),110);
    title+=count;while(*title==' ')++title;
  }
  draw_central_text("SD Card",frame,72,106);
#ifdef SUPPORT_NORGAMES
  draw_central_text("NOR Flash",frame,168,106);
#else
  draw_central_text("Recents",frame,168,106);
#endif
  const unsigned boxes[4][4]={{24,42,192,48},{24,92,96,46},{120,92,96,46},{109,142,23,16}};
  const unsigned *box=boxes[launcher_selected<4?launcher_selected:0];
  if(launcher_selected<4)launcher_corners(frame,box[0],box[1]+(launcher_selected==0),box[2],box[3]-(launcher_selected==0));
}

static void render_start_bar(volatile uint8_t *frame) {
  dma_memset16(frame,dup8(menu_theme>=THEME_DSSMM_LIGHT?TOP_BG:FG_COLOR),240*16/2);
  draw_text_ovf(menu_username,frame,3,1,132);
  uint32_t seconds=launcher_clock_get();char text[12];
  format_menu_clock(text,sizeof(text),seconds);
  draw_rightj_text(text,frame,234,1);
  if(launcher_selected==4)launcher_corners(frame,178,0,62,16);
}

static void render_dssmm_settings_header(volatile uint8_t *frame,const char *title) {
  dma_memset16(frame,dup8(menu_theme>=THEME_DSSMM_LIGHT?TOP_BG:FG_COLOR),240*16/2);
  draw_text_ovf(menu_username,frame,3,1,84);
  draw_central_text_ovf(title,frame,120,1,96);
  uint32_t seconds=launcher_clock_get();char time[12];
  format_menu_clock(time,sizeof(time),seconds);
  draw_rightj_text(time,frame,234,1);
}

static void render_dssmm_settings_background(volatile uint8_t *frame,const char *title) {
  dssmm_background_palette(MEM_PALETTE,menu_theme==THEME_DSSMM_DARK);
  dssmm_background_draw(frame,menu_theme==THEME_DSSMM_DARK,DSSMM_BG_SETTINGS);
  render_dssmm_settings_header(frame,title);
}

static void render_settings_home(volatile uint8_t *frame) {
  static const char * const labels[]={"Interface","Games","Hardware","System","Info"};
  render_dssmm_settings_background(frame,"Settings");
  unsigned line_h=menu_font==FONT_STYLE_DSSMM?14:18;
  for(unsigned i=0;i<5;++i) {
    unsigned y=24+i*line_h;
    if(i==launcher_category)draw_menu_selection(frame,y,112,224);
    draw_text_ovf(labels[i],frame,23,y,88);
    draw_text_ovf_color(">",frame,119,y,104,i==launcher_category?SELECTED_TEXT:FT_COLOR);
  }

}

static const struct {
  const t_mrender_fn render;
  const int max_submenu;
} popup_windows[] = {
  { render_gba_load_popup, GbaLoadCNT },
  { render_sav_menu_popup, 1 },
  { render_fw_flash_popup, 1 },
  { render_filemgr,        1 },
  #ifdef SUPPORT_NORGAMES
  { render_gba_norwrite,   GbaNorWrCNT },
  { render_gba_norload,    GbaNorLoadCNT },
  #endif
};

static void render_menu_notice(volatile uint8_t *frame) {
  const char *notice=spop.alert_msg;
  if(!notice && settings_save_preparing())notice="Saving settings...";
  if (notice) {
    draw_box_full(frame, 15, 227, SCREEN_HEIGHT / 2 - 20, SCREEN_HEIGHT / 2 + 20, FG_COLOR, HI_COLOR);
    draw_central_text(notice, frame, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 8);
    REG_WIN0H = 226 | (14 << 8);
    REG_WIN0V = (SCREEN_HEIGHT / 2 + 20) | ((SCREEN_HEIGHT / 2 - 20) << 8);
  } else {
    REG_WIN0H = 0;
    REG_WIN0V = 0;
  }
}

// Renders the menu. Arg0 represents the frame count difference with the
// previous rendered frame (for animations and similar stuff).
#pragma GCC optimize ("O2")

/* Routing favors size; pixel/scaling functions retain their own hot settings. */
__attribute__((optimize("Os")))
void menu_render(unsigned fcnt) {
  objnum = 0;
  volatile uint8_t *frame = &MEM_VRAM_U8[0xA000*framen];
  if (restart_required) {
    REG_DISPCNT=(REG_DISPCNT&0x10)|0x0404;
    REG_WIN0H=REG_WIN0V=0;
    render_restart_required(frame);
    return;
  }
  if (smenu.menu_tab == MENUTAB_ROMBROWSE && browser_view)
    browser_motion_offset = browser_motion_advance(browser_motion_offset, fcnt);
  else
    browser_motion_offset = 0;

  bool ds_style=menu_theme>=THEME_DSSMM_LIGHT;
  bool launcher=ds_style && launcher_page && !spop.pop_num &&
      !spop.qpop.message && !spop.rtcpop.callback && !spop.alert_msg;
  if(ds_style)
    dssmm_background_palette(MEM_PALETTE,menu_theme==THEME_DSSMM_DARK);
  if(!(launcher && launcher_page==1 && launcher_valid[framen]))
    dma_memset16(&frame[0],dup8(menu_theme>=THEME_DSSMM_LIGHT?TOP_BG:FG_COLOR),SCREEN_WIDTH*16/2);

  bool fast_carousel = !launcher && smenu.menu_tab == MENUTAB_ROMBROWSE && browser_view &&
      browser_motion_offset == 0 && !spop.qpop.message && !spop.rtcpop.callback &&
      !spop.pop_num && !spop.alert_msg;
  if (!fast_carousel) browser_fast_valid[0] = browser_fast_valid[1] = false;
  browser_fast_reuse = fast_carousel && browser_fast_valid[framen] &&
      browser_fast_selection[framen] == smenu.browser.selector &&
      browser_fast_view[framen] == browser_view && browser_fast_mode[framen] == artwork_mode;
  // Preserve stable artwork in each Mode 4 page; text is refreshed separately.
  if (!browser_fast_reuse && !(launcher && launcher_valid[framen])) {
    if(ds_style && !launcher) {
      if(smenu.menu_tab==MENUTAB_RECENT || (smenu.menu_tab==MENUTAB_ROMBROWSE && !browser_view)
#ifdef SUPPORT_NORGAMES
          || smenu.menu_tab==MENUTAB_NORBROWSE
#endif
        )
        dssmm_background_clear(frame,menu_theme==THEME_DSSMM_DARK,0,16,240,144);
      else {
        unsigned kind=smenu.menu_tab==MENUTAB_ROMBROWSE?(browser_view==2?DSSMM_BG_VERTICAL:DSSMM_BG_HORIZONTAL):
                      DSSMM_BG_GRID;
        dssmm_background_draw(frame,menu_theme==THEME_DSSMM_DARK,kind);
        dma_memset16(frame,dup8(menu_theme>=THEME_DSSMM_LIGHT?TOP_BG:FG_COLOR),240*16/2);
      }
    } else dma_memset16(&frame[16*SCREEN_WIDTH],dup8(BG_COLOR),SCREEN_WIDTH*(SCREEN_HEIGHT-16)/2);
  }
  if(!ds_style) {
    int mintab=(recent_menu && smenu.recent.maxentries)?MENUTAB_RECENT:MENUTAB_ROMBROWSE;
    for(unsigned i=mintab;i<MENUTAB_MAX;++i)
      if(i==smenu.menu_tab)render_icon((i-mintab)*16,0,i+ICON_RECENT);
      else render_icon_trans((i-mintab)*16,0,i+ICON_RECENT);
  }

  if(launcher) {
    if(!launcher_valid[framen]) {
      if(launcher_page==1)render_start(frame);
      else {dssmm_background_clear(frame,menu_theme==THEME_DSSMM_DARK,0,16,240,144);render_settings_home(frame);}
      launcher_valid[framen]=true;
    }
    if(launcher_page==1)render_start_bar(frame);
    else render_dssmm_settings_header(frame,"Settings");
    render_menu_notice(frame);
    return;
  }
  launcher_invalidate();

  if(ds_style && (spop.pop_num || spop.qpop.message || spop.rtcpop.callback))
    draw_lined_panel(frame);
  if (spop.qpop.message)
    render_popupq(frame, fcnt);
  else if (spop.rtcpop.callback)
    render_rtcpop(frame);
  else {
    if (spop.pop_num) {
      popup_windows[spop.pop_num - 1].render(frame);
      spop.anim += fcnt * animspd_lut[anim_speed];
    } else {
      static const t_mrender_fn renderfns[] = {
        render_recent,
        render_browser,
        #ifdef SUPPORT_NORGAMES
        render_flashbrowser,
        #endif
        render_settings,
        render_ui_settings,
        render_tools,
        render_info,
      };
      renderfns[smenu.menu_tab](frame);
      smenu.anim_state += fcnt * animspd_lut[anim_speed];
    }
  }

  if (fast_carousel) {
    browser_fast_valid[framen] = true;
    browser_fast_selection[framen] = smenu.browser.selector;
    browser_fast_view[framen] = browser_view;
    browser_fast_mode[framen] = artwork_mode;
  }

  if(ds_style) {
    static const char *const titles[]={"Recents","SD Card",
#ifdef SUPPORT_NORGAMES
      "NOR Flash",
#endif
      "Settings","Interface","System","Info"};
    const char *title=titles[smenu.menu_tab];
    if(smenu.menu_tab==MENUTAB_SETTINGS && launcher_settings_group)
      title=launcher_settings_group==1?"Games":"Hardware";
    render_dssmm_settings_header(frame,title);
  }
  render_menu_notice(frame);
}

void menu_flip() {
  for (unsigned i = 0; i < objnum; i++) {
    MEM_OAM[i*4+0] = fobjs[i].y | 0x2000;  // Use 256 entries palette
    MEM_OAM[i*4+1] = fobjs[i].x | 0x4000;  // Size 16x16
    MEM_OAM[i*4+2] = fobjs[i].tn + 512;    // OBJ numbers start at 512 for Mode 4
  }
  dma_memset16(&MEM_OAM[objnum*4], 0, 256 - objnum*2);  // Clear unused objects
  REG_DISPCNT = (REG_DISPCNT & ~0x10) | (framen << 4);
  framen ^= 1;
  /* Show the snapshot notice before a synchronous read or large-file fallback. */
  if(!settings_save_poll())spop.alert_msg=msgs[lang_id][MSG_ERR_SETSAVE];
}

/* Cold setup/action paths favor size; rendering above retains -O2. */
#pragma GCC optimize ("Os")

void menu_init(int sram_testres) {
  restart_required=false;
  update_unlock.position=0;
  browser_invalidate_artwork();
  // Reset to ROM browser and SD card root.
  memset(&smenu, 0, sizeof(smenu));
  memset(&spop, 0, sizeof(spop));

  // Reset the file browser as well.
  strcpy(smenu.browser.cpath, "/");
  browser_reload();
  flashbrowser_reload();

  // Load recent ROMs (we could disable this for speed)
  recent_reload();

  reload_theme(menu_theme);
  launcher_page=menu_theme>=THEME_DSSMM_LIGHT?1:0;launcher_selected=1;
  launcher_clock_set(menu_clock_seed);

  smenu.menu_tab = (recent_menu && smenu.recent.maxentries) ? MENUTAB_RECENT : MENUTAB_ROMBROWSE;

  // Load icons into VRAM
  dma_memcpy16(MEM_VRAM_OBJS, icons_img, sizeof(icons_img) / 2);
  dma_memcpy16(&MEM_PALETTE[256], icons_pal, sizeof(icons_pal) / 2);
  dma_memcpy16(MEM_VRAM_OBJS+sizeof(icons_img),gamegear_pixels,sizeof(gamegear_pixels)/2);
  dma_memcpy16(&MEM_PALETTE[256+sizeof(icons_pal)/2],gamegear_palette,sizeof(gamegear_palette)/2);
  // Generate some icons (selector)
  dma_memset16(&MEM_VRAM_OBJS[63 * 256], dup8(SEL_COLOR), 256 / 2);

  // Further setup initial video regs. BG2 is setup in the bootloader already!
  REG_WININ  = 0x0004;     // Only BG2 is enabled in Win0
  REG_WINOUT = 0x0014;     // BG2 and OBJ enabled outside of Win0
  REG_WIN0H = 0;
  REG_WIN0V = 0;
  REG_DISPCNT |= 0x2000;   // Enable window 0

  // Setup alpha blending for the selector knob
  REG_BLDCNT = 0x1F40;
  REG_BLDALPHA = 0x0808;  // 50% alpha

  // If there's a test result to report, create a popup
  if (sram_testres >= 0)
    spop.alert_msg = sram_testres ? msgs[lang_id][MSG_SRAMTST_FAIL] :
                                    msgs[lang_id][MSG_SRAMTST_OK];
  else if (!settings_loaded_ok())
    spop.alert_msg="Could not read SETTINGS.TXT.";
}

int movedir_up() {
  char *p = smenu.browser.cpath;
  p = &p[strlen(p)-1];

  if (p != smenu.browser.cpath) {
    do {
      p--;
      if (*p == '/') {
        p[1] = 0;   // Shorten the path here
        return 1;
      }
    } while (p != smenu.browser.cpath);
  }
  return 0;
}

void start_flash_update(const char *fn, unsigned fwsize, bool validate_superfw) {
  // We read the file into SDRAM, apply the update from there.
  FIL fd;
  FRESULT res = f_open(&fd, fn, FA_READ);
  if (res != FR_OK)
    spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRRD];
  else {
    // Loading file...
    spop.p.update.curr_state = FlashingLoading;
    menu_render(1); menu_flip();
    for (unsigned i = 0; i < fwsize; i += 4*1024) {
      UINT rdbytes;
      unsigned tord = fwsize >= i + 4*1024 ? 4*1024 : fwsize - i;
      uint32_t tmp[1024];
      if (FR_OK != f_read(&fd, tmp, tord, &rdbytes) || rdbytes != tord) {
        spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRRD];
        return;
      }
      // Copy (ensure aligned copy!)
      dma_memcpy32(&sdr_state->scratch[i], tmp, 1024);
    }
    spop.p.update.curr_state = FlashingChecking;
    menu_render(1); menu_flip();

    // Now proceed to validate the superfw if necessary.
    if (validate_superfw && !validate_superfw_variant(sdr_state->scratch))
      spop.alert_msg = msgs[lang_id][MSG_FWUP_BADFL];
    else if (validate_superfw && !validate_superfw_checksum(sdr_state->scratch, fwsize))
      spop.alert_msg = msgs[lang_id][MSG_FWUPD_BADCHK];
    else {
      // Can start the flashing!
      spop.p.update.curr_state = FlashingErasing;
      menu_render(1); menu_flip();

      bool erased_ok;
      #ifdef SUPPORT_NORGAMES
      if (flashinfo.blksize)
        erased_ok = flash_erase_sectors(ROM_FLASHFIRMW_ADDR, flashinfo.blksize,
                                        (fwsize + flashinfo.blksize - 1) / flashinfo.blksize);
      else
      #endif
        erased_ok = flash_erase_chip();

      if (!erased_ok)
        spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRCL];
      else {
        spop.p.update.curr_state = FlashingWriting;
        menu_render(1); menu_flip();

        bool programmed_ok;
        #ifdef SUPPORT_NORGAMES
        if (flashinfo.size && flashinfo.blksize && flashinfo.blkcount && flashinfo.blkwrite)
          programmed_ok = flash_program_buffered(ROM_FLASHFIRMW_ADDR, sdr_state->scratch, fwsize, flashinfo.blkwrite);
        else
        #endif
          programmed_ok = flash_program(ROM_FLASHFIRMW_ADDR, sdr_state->scratch, fwsize);

        if (!programmed_ok)
          spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRPG];
        else {
          if (!flash_verify(ROM_FLASHFIRMW_ADDR, sdr_state->scratch, fwsize))
            spop.alert_msg = msgs[lang_id][MSG_FWUP_ERRVR];
          else {
            // Done! Show a pop up, also go up with pop ups too.
            spop.alert_msg = msgs[lang_id][MSG_FWUPD_DONE];
          }
        }
      }
    }
    spop.pop_num = 0;
  }
}

static void keypress_popup_loadgba(unsigned newkeys) {
  #ifdef SUPPORT_NORGAMES
  if (spop.submenu == GbaLoadPopInfo &&
      (newkeys & (KEY_BUTTUP | KEY_BUTTDOWN | KEY_BUTTLEFT | KEY_BUTTRIGHT))) {
    gba_write_nor = !gba_write_nor;
    return;
  }
  #endif
  const unsigned maxm[] = {
    GBAInfoCNT,
    GBALdSetCNT,
    GBAPatchCNT,
  };
  unsigned maxsel = maxm[(int)spop.submenu];

  const int psel = spop.selector;
  if (newkeys & KEY_BUTTUP)
    spop.selector += maxsel - 1;
  if (newkeys & KEY_BUTTDOWN)
    spop.selector++;

  // Limit selector to its max value
  spop.selector %= maxsel;

  if (newkeys & KEY_BUTTLEFT) {
    if (spop.submenu == GbaLoadPopLoadS) {
      if (spop.selector == GBALdSetCheats)
        spop.p.load.l.use_cheats = !spop.p.load.l.use_cheats;
      if (spop.p.load.i.use_dsaving) {
        if (spop.selector == GBALdSetLoadP)
          spop.p.load.l.sram_load_type = (spop.p.load.l.sram_load_type + SaveLoadDSCNT - 1) % SaveLoadDSCNT;
      } else {
        if (spop.selector == GBALdSetLoadP)
          spop.p.load.l.sram_load_type = (spop.p.load.l.sram_load_type + SaveLoadCNT - 1) % SaveLoadCNT;
        else if (spop.selector == GBALdSetSaveP)
          spop.p.load.l.sram_save_type = (spop.p.load.l.sram_save_type + SaveCNT - 1) % SaveCNT;
      }
    }
    else if (spop.submenu == GbaLoadPopPatch) {
      if (spop.selector == GBALoadPatch)
        spop.p.load.i.patch_type = (spop.p.load.i.patch_type + PatchOptCNT - 1) % PatchOptCNT;
      else if (spop.selector == GBAInGameMen)
        spop.p.load.i.ingame_menu_enabled = !spop.p.load.i.ingame_menu_enabled;
      else if (spop.selector == GBASavePatch)
        spop.p.load.i.use_dsaving = !spop.p.load.i.use_dsaving;
      else if (spop.selector == GBARTCPatch)
        spop.p.load.i.rtc_patch_enabled = !spop.p.load.i.rtc_patch_enabled;
    }

    // Handle the different cases where the user attempts to select an invalid option.
    if (!spop.p.load.i.patches_cache_found && spop.p.load.i.patch_type == PatchEngine)
      spop.p.load.i.patch_type = PatchDatabase;  // Might be invalid, handled below.
    if (!spop.p.load.i.patches_datab_found && spop.p.load.i.patch_type == PatchDatabase)
      spop.p.load.i.patch_type = PatchNone;

    if (!dirsav_avail_sdram(&spop.p.load.i))
      spop.p.load.i.use_dsaving = false;

    // DirSav forces automatic saving
    if (spop.p.load.i.use_dsaving)
      spop.p.load.l.sram_save_type = SaveDirect;
    else if (spop.p.load.l.sram_save_type == SaveDirect)
      spop.p.load.l.sram_save_type = autosave_default ? SaveReboot : SaveDisable;

    // If DS is selected, do not allow manual mode.
    if (spop.p.load.l.sram_load_type == SaveLoadDisable && spop.p.load.i.use_dsaving)
      spop.p.load.l.sram_load_type = SaveLoadSav;
    // If no .sav is available, do not allow that option!
    if (spop.p.load.l.sram_load_type == SaveLoadSav && !spop.p.load.l.savefile_found)
      spop.p.load.l.sram_load_type = spop.p.load.i.use_dsaving ? SaveLoadReset : SaveLoadDisable;
  }
  if (newkeys & KEY_BUTTRIGHT) {
    if (spop.submenu == GbaLoadPopLoadS) {
      if (spop.selector == GBALdSetCheats)
        spop.p.load.l.use_cheats = !spop.p.load.l.use_cheats;
      if (spop.p.load.i.use_dsaving) {
        if (spop.selector == GBALdSetLoadP)
          spop.p.load.l.sram_load_type = (spop.p.load.l.sram_load_type + 1) % SaveLoadDSCNT;
      } else {
        if (spop.selector == GBALdSetLoadP)
          spop.p.load.l.sram_load_type = (spop.p.load.l.sram_load_type + 1) % SaveLoadCNT;
        else if (spop.selector == GBALdSetSaveP)
          spop.p.load.l.sram_save_type = (spop.p.load.l.sram_save_type + 1) % SaveCNT;
      }
    }
    else if (spop.submenu == GbaLoadPopPatch) {
      if (spop.selector == GBALoadPatch)
        spop.p.load.i.patch_type = (spop.p.load.i.patch_type + 1) % PatchOptCNT;
      else if (spop.selector == GBAInGameMen)
        spop.p.load.i.ingame_menu_enabled = !spop.p.load.i.ingame_menu_enabled;
      else if (spop.selector == GBASavePatch)
        spop.p.load.i.use_dsaving = !spop.p.load.i.use_dsaving;
      else if (spop.selector == GBARTCPatch)
        spop.p.load.i.rtc_patch_enabled = !spop.p.load.i.rtc_patch_enabled;
    }

    // If the database has no entry, then do not let the user select that mode.
    if (!spop.p.load.i.patches_datab_found && spop.p.load.i.patch_type == PatchDatabase)
      spop.p.load.i.patch_type = PatchEngine;  // Might be invalid, handled below.
    if (!spop.p.load.i.patches_cache_found && spop.p.load.i.patch_type == PatchEngine)
      spop.p.load.i.patch_type = PatchNone;

    if (!dirsav_avail_sdram(&spop.p.load.i))
      spop.p.load.i.use_dsaving = false;

    // DirSav forces automatic saving
    if (spop.p.load.i.use_dsaving)
      spop.p.load.l.sram_save_type = SaveDirect;
    else if (spop.p.load.l.sram_save_type == SaveDirect)
      spop.p.load.l.sram_save_type = autosave_default ? SaveReboot : SaveDisable;

    // If DS is selected, do not allow manual mode.
    if (spop.p.load.l.sram_load_type == SaveLoadDisable && spop.p.load.i.use_dsaving)
      spop.p.load.l.sram_load_type = SaveLoadSav;
    // If no .sav is available, do not allow that option!
    if (spop.p.load.l.sram_load_type == SaveLoadSav && !spop.p.load.l.savefile_found)
      spop.p.load.l.sram_load_type = SaveLoadReset;
  }

  // Disable ingame-menu if not available.
  if (!ingame_menu_avail_sdram(&spop.p.load.i))
    spop.p.load.i.ingame_menu_enabled = false;

  // If no RTC patches are available, force them to false.
  if (!rtcemu_avail(&spop.p.load.i))
    spop.p.load.i.rtc_patch_enabled = false;

  // Disable cheat loading if no cheats are avail, or IGM is disabled
  if (!spop.p.load.l.cheats_found || !spop.p.load.i.ingame_menu_enabled)
    spop.p.load.l.use_cheats = false;

  if (newkeys & KEY_BUTTA) {
    if (spop.submenu == GbaLoadPopLoadS && spop.selector == GBALdSetRTC && spop.p.load.i.rtc_patch_enabled) {
      void accept_rtc() {
        spop.p.load.l.rtcval = date2timestamp(&spop.rtcpop.val);
      }
      if (spop.p.load.i.rtc_patch_enabled) {
        timestamp2date(spop.p.load.l.rtcval, &spop.rtcpop.val);
        spop.rtcpop.menu_clock=false;
    spop.rtcpop.callback = accept_rtc;
      }
    }
    else if (spop.submenu == GbaLoadPopPatch && spop.selector == GBAPatchGen) {
      generate_patches_progress(spop.p.load.i.romfn, spop.p.load.i.romfs);
      spop.alert_msg = msgs[lang_id][MSG_PATCHGEN_OK];
      // Try/Load the just-generated patches.
      spop.p.load.i.patches_cache_found = load_cached_patches(spop.p.load.i.romfn, &spop.p.load.i.patches_cache);
    }
    else if (spop.submenu == GbaLoadPopLoadS && spop.selector == GBALdRemember) {
      // Save settings to disk now!
      t_rom_load_settings ld_sett = {
        .patch_policy = spop.p.load.i.patch_type,
        .use_igm = spop.p.load.i.ingame_menu_enabled,
        .use_rtc = spop.p.load.i.rtc_patch_enabled,
        .use_dsaving = spop.p.load.i.use_dsaving
      };
      t_rom_launch_settings lh_sett = {
        .use_cheats = spop.p.load.l.use_cheats,
        .rtcts = spop.p.load.l.rtcval
      };

      save_rom_settings(spop.p.load.i.romfn, &ld_sett, &lh_sett);
      spop.alert_msg = msgs[lang_id][MSG_REMEMB_CFG_OK];
    }
    else if (GbaLoadPopInfo == spop.submenu) {
      #ifdef SUPPORT_NORGAMES
      if (gba_write_nor) {
        request_nor_write();
        return;
      }
      #endif

      // Honor load.patch_type.
      const t_patch *p = get_game_patch(&spop.p.load.i);
      EnumSavetype st = p ? p->save_mode : SaveTypeNone;

      // Prepare the savegame (load and store stuff, directsave...)
      t_dirsave_info dsinfo;
      unsigned errsave = prepare_savegame(
        spop.p.load.l.sram_load_type, spop.p.load.l.sram_save_type,
        st, &dsinfo, spop.p.load.l.savefn);
      if (errsave) {
        unsigned errmsg = (errsave == ERR_SAVE_BADSAVE)   ? MSG_ERR_SAVERD :
                          (errsave == ERR_SAVE_CANTALLOC) ? MSG_ERR_SAVEPR :
                          (errsave == ERR_SAVE_BADARG)    ? MSG_ERR_SAVEIT :
                                                            MSG_ERR_SAVEWR;
        spop.alert_msg = msgs[lang_id][errmsg];
        return;
      }

      t_rtc_info rtci = {
        .timestamp = spop.p.load.l.rtcval,
        .ts_step = rtcspeed_default
      };

      unsigned err = load_gba_rom(
        spop.p.load.i.romfn, spop.p.load.i.romfs,
        spop.p.load.l.sram_save_type == SaveDisable ? NULL : spop.p.load.l.savefn, p,
        spop.p.load.l.sram_save_type == SaveDirect ? &dsinfo : NULL,
        spop.p.load.i.ingame_menu_enabled,
        spop.p.load.i.rtc_patch_enabled ? &rtci : NULL,
        spop.p.load.l.use_cheats ? spop.p.load.l.cheats_size : 0,
        loadrom_progress);
      if (err) {
        if (err==ERR_LOAD_RESTART) restart_required=true;
        else spop.alert_msg=msgs[lang_id][MSG_ERR_READ];
      }
    }
  }

  if (psel != spop.selector)
    spop.anim = 0;
}

static void keypress_popup_savefile(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    spop.selector = MAX(0, spop.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    spop.selector = MIN(SavMAX, spop.selector + 1);

  if (newkeys & KEY_BUTTA) {
    switch (spop.selector) {
    case SaveWrite:
      if (write_save_sram(spop.p.savopt.savfn))
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG0];
      else
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG_WERR];
      break;
    case SavLoad:
      if (load_save_sram(spop.p.savopt.savfn))
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG1];
      else
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG_RERR];
      break;
    case SavClear:
      if (wipe_sav_file(spop.p.savopt.savfn))
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG2];
      else
        spop.alert_msg = msgs[lang_id][MSG_SAVOPT_MSG_WERR];
      break;
    case SavQuit:
      spop.pop_num = 0;
      break;
    };
  }
}

static void keypress_popup_flash(unsigned newkeys) {
  if ((curr_pressed_keys() & FLASH_GO_KEYS) == FLASH_GO_KEYS)
    start_flash_update(spop.p.update.fn, spop.p.update.fw_size, spop.p.update.issfw);
}

#ifdef SUPPORT_NORGAMES
/* The load and NOR-write popups share their leading t_load_gba_info. */
static unsigned nor_write_blocks(void) {
  return spop.p.norwr.i.romfs / NOR_BLOCK_SIZE + (spop.p.norwr.i.romfs % NOR_BLOCK_SIZE != 0);
}

/* Presentation only: native SuperChis allocation/erase/write/verify remain unchanged. */
static void render_nor_progress(volatile uint8_t *frame,unsigned done,unsigned total) {
  dssmm_background_palette(MEM_PALETTE,menu_theme==THEME_DSSMM_DARK);
  draw_lined_panel(frame);
  render_dssmm_settings_header(frame,"NOR Flash");
  draw_central_text("Writing NOR",frame,120,42);
  draw_box_outline(frame,18,222,76,96,FT_COLOR);
  unsigned width=total?(uint64_t)MIN(done,total)*196/total:0;
  for(unsigned y=79;y<94;++y)
    dma_memset16(frame+y*240+22,dup8(HI_COLOR),(width&~1u)/2);
  char text[12];npf_snprintf(text,sizeof(text),"%u%%",total?(unsigned)((uint64_t)MIN(done,total)*100/total):0);
  draw_central_text(text,frame,120,106);
  draw_central_text("Do not power off",frame,120,132);
}
static void nor_write_progress(unsigned done,unsigned total) {
  if(menu_theme<THEME_DSSMM_LIGHT){loadrom_progress(done,total);return;}
  /* Flash helpers return with SDRAM/SD mapped; keep fonts available to the UI. */
  set_supercard_mode(MAPPED_SDRAM,true,true);
  render_nor_progress(&MEM_VRAM_U8[0xa000*framen],done,total);
  dma_memset16(MEM_OAM,0,256);
  REG_DISPCNT=(REG_DISPCNT&~0x10)|(framen<<4);framen^=1;
}

static void confirm_nor_write(bool confirm) {
  if (!confirm) return;
  unsigned blkcnt = nor_write_blocks();
  if (spop.p.norwr.i.romfs < 192 || spop.p.norwr.i.romfs > MAX_GBA_ROM_SIZE ||
      blkcnt > NOR_GAMEBLOCK_COUNT) {
    spop.alert_msg = "GBA ROM is too large to fit in NOR.";
    return;
  }
  /* Validate the source before discarding any existing NOR entries. */
  FIL fd;
  if (f_open(&fd, spop.p.norwr.i.romfn, FA_READ) != FR_OK) {
    spop.alert_msg = msgs[lang_id][MSG_ERR_READ];
    return;
  }
  bool source_ok = f_size(&fd) == spop.p.norwr.i.romfs;
  if (f_close(&fd) != FR_OK) source_ok = false;
  if (!source_ok) {
    spop.alert_msg = msgs[lang_id][MSG_ERR_READ];
    return;
  }
  if (nor_replace_games) {
    if (!flashmgr_wipe(ROM_FLASHMETA_ADDR, FLASH_METADATA_SIZE)) {
      spop.alert_msg = msgs[lang_id][MSG_ERR_NORUPD];
      return;
    }
  }
  flashbrowser_reload();
  if (smenu.fbrowser.freeblks < blkcnt || smenu.fbrowser.maxentries >= FLASHG_MAXFN_CNT) {
    spop.alert_msg = msgs[lang_id][MSG_ERR_NORSPC];
    return;
  }
  const t_load_gba_info *info = &spop.p.norwr.i;
  const t_patch *p = get_game_patch(info);
  const bool dirsaving = info->use_dsaving && dirsav_avail_flash(info);
  const bool ingame_menu = info->ingame_menu_enabled && ingame_menu_avail_flash(info);

  // Allocate the last entry for the new game.
  t_flash_game_entry ne = {
    .gamecode = *(uint32_t*)info->romh.gcode,
    .gamever = info->romh.version,
    .numblks = blkcnt,
    .gattrs = (dirsaving                 ? GATTR_SAVEDS : 0) |
              (ingame_menu               ? GATTR_IGM    : 0) |
              (info->rtc_patch_enabled   ? GATTR_RTC    : 0) |
              GATTR_SAVEM(p),
    .bnoffset = (uint8_t)(file_basename(info->romfn) - info->romfn),
    .entry_addr = ROM_ENTRYPOINT(info->romh)
  };
  memset(&ne.blkmap, 0, sizeof(ne.blkmap));
  strbcpy(ne.game_name, info->romfn, sizeof(ne.game_name));

  if (!flashmgr_allocate_blocks(ne.blkmap, blkcnt, (t_reg_entry*)&sdr_state->nordata)) {
    flashbrowser_reload();
    spop.alert_msg = msgs[lang_id][MSG_ERR_NORSPC];
    return;
  }

  // Go ahead and start the flasher-loader with patching support.
  unsigned errc = flash_gba_nor(info->romfn, info->romfs, &info->romh, p,
                                dirsaving, ingame_menu,
                                info->rtc_patch_enabled,
                                ne.blkmap, nor_write_progress,
                                sdr_state->scratch, scratch_mem_size);
  if (errc)
    spop.alert_msg = msgs[lang_id][errc == ERR_LOAD_BADROM ? MSG_ERR_READ : MSG_ERR_NORUPD];
  else {
    // Now we can just write the metadata entry!
    memcpy32(&sdr_state->nordata.games[smenu.fbrowser.maxentries], &ne, sizeof(ne));
    sdr_state->nordata.gamecnt++;
    if (!flashmgr_store(ROM_FLASHMETA_ADDR, FLASH_METADATA_SIZE, (t_reg_entry*)&sdr_state->nordata))
      spop.alert_msg = msgs[lang_id][MSG_ERR_NORUPD];
    else {
      spop.alert_msg = msgs[lang_id][MSG_NOR_WROK];
      spop.pop_num = POPUP_NONE;
    }
  }

  flashbrowser_reload();
}

static void request_nor_write(void) {
  flashbrowser_reload();
  unsigned blkcnt = nor_write_blocks();
  if (spop.p.norwr.i.romfs < 192 || spop.p.norwr.i.romfs > MAX_GBA_ROM_SIZE ||
      blkcnt > NOR_GAMEBLOCK_COUNT) {
    spop.alert_msg = "GBA ROM is too large to fit in NOR.";
    return;
  }
  nor_replace_games = smenu.fbrowser.freeblks < blkcnt ||
                      smenu.fbrowser.maxentries >= FLASHG_MAXFN_CNT;
  spop.qpop.message = nor_replace_games
    ? "Erase existing NOR games and write this ROM?"
    : "Write this GBA ROM to NOR?";
  spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
  spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
  spop.qpop.option = 0;
  spop.qpop.callback = confirm_nor_write;
  spop.qpop.clear_popup_ok = false;
}

static void keypress_popup_norwrite(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    spop.selector = MAX(0, spop.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    spop.selector = MIN(GBAPatchCNT - 1, spop.selector + 1);

  if (spop.submenu == GbaNorWrPatch) {
    if (newkeys & (KEY_BUTTLEFT|KEY_BUTTRIGHT)) {
      if (spop.selector == GBALoadPatch)
        spop.p.norwr.i.patch_type = (spop.p.norwr.i.patch_type +
                                     ((newkeys & KEY_BUTTRIGHT) ? 1 : PatchOptCNT - 1)) % PatchOptCNT;
      else if (spop.selector == GBAInGameMen)
        spop.p.norwr.i.ingame_menu_enabled = !spop.p.norwr.i.ingame_menu_enabled;
      else if (spop.selector == GBASavePatch)
        spop.p.norwr.i.use_dsaving = !spop.p.norwr.i.use_dsaving;
      else if (spop.selector == GBARTCPatch)
        spop.p.norwr.i.rtc_patch_enabled = !spop.p.norwr.i.rtc_patch_enabled;
    }

    if (newkeys & KEY_BUTTLEFT) {
      // Handle the different cases where the user attempts to select an invalid option.
      if (!spop.p.norwr.i.patches_cache_found && spop.p.norwr.i.patch_type == PatchEngine)
        spop.p.norwr.i.patch_type = PatchDatabase;  // Might be invalid, handled below.
      if (!spop.p.norwr.i.patches_datab_found && spop.p.norwr.i.patch_type == PatchDatabase)
        spop.p.norwr.i.patch_type = PatchNone;
    }
    if (newkeys & KEY_BUTTRIGHT) {
      // If the database has no entry, then do not let the user select that mode.
      if (!spop.p.norwr.i.patches_datab_found && spop.p.norwr.i.patch_type == PatchDatabase)
        spop.p.norwr.i.patch_type = PatchEngine;  // Might be invalid, handled below.
      if (!spop.p.norwr.i.patches_cache_found && spop.p.norwr.i.patch_type == PatchEngine)
        spop.p.norwr.i.patch_type = PatchNone;
    }

    // Disable certain features (depends on patch types)
    if (!dirsav_avail_flash(&spop.p.norwr.i))
      spop.p.norwr.i.use_dsaving = false;
    if (!ingame_menu_avail_flash(&spop.p.norwr.i))
      spop.p.norwr.i.ingame_menu_enabled = false;
    if (!rtcemu_avail(&spop.p.norwr.i))
      spop.p.norwr.i.rtc_patch_enabled = false;

    if ((newkeys & KEY_BUTTA) && spop.selector == GBAPatchGen) {
      generate_patches_progress(spop.p.norwr.i.romfn, spop.p.norwr.i.romfs);
      spop.alert_msg = msgs[lang_id][MSG_PATCHGEN_OK];
      // Try/Load the just-generated patches.
      spop.p.norwr.i.patches_cache_found = load_cached_patches(spop.p.norwr.i.romfn, &spop.p.norwr.i.patches_cache);
    }
  } else {
    if (newkeys & KEY_BUTTA) {
      request_nor_write();
    }
  }
}

static void keypress_popup_norload(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    spop.selector = MAX(0, spop.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    spop.selector = MIN(GBALdSetCNT - 1, spop.selector + 1);

  const t_flash_game_entry *e = spop.p.norld.e;
  bool uses_dsave = e->gattrs & GATTR_SAVEDS;
  bool uses_igm   = e->gattrs & GATTR_IGM;
  bool uses_rtc   = e->gattrs & GATTR_RTC;

  if (newkeys & KEY_BUTTLEFT) {
    if (spop.submenu == GbaNorLoad) {
      if (spop.selector == GBALdSetCheats)
        spop.p.norld.l.use_cheats = !spop.p.norld.l.use_cheats;
      if (uses_dsave) {
        if (spop.selector == GBALdSetLoadP)
          spop.p.norld.l.sram_load_type = (spop.p.norld.l.sram_load_type + SaveLoadDSCNT - 1) % SaveLoadDSCNT;
      } else {
        if (spop.selector == GBALdSetLoadP)
          spop.p.norld.l.sram_load_type = (spop.p.norld.l.sram_load_type + SaveLoadCNT - 1) % SaveLoadCNT;
        else if (spop.selector == GBALdSetSaveP)
          spop.p.norld.l.sram_save_type = (spop.p.norld.l.sram_save_type + SaveCNT - 1) % SaveCNT;
      }
    }

    // DirSav forces automatic saving
    if (uses_dsave)
      spop.p.norld.l.sram_save_type = SaveDirect;
    else if (spop.p.norld.l.sram_save_type == SaveDirect)
      spop.p.norld.l.sram_save_type = autosave_default ? SaveReboot : SaveDisable;

    // If DS is selected, do not allow manual mode.
    if (spop.p.norld.l.sram_load_type == SaveLoadDisable && uses_dsave)
      spop.p.norld.l.sram_load_type = SaveLoadSav;
    // If no .sav is available, do not allow that option!
    if (spop.p.norld.l.sram_load_type == SaveLoadSav && !spop.p.norld.l.savefile_found)
      spop.p.norld.l.sram_load_type = uses_dsave ? SaveLoadReset : SaveLoadDisable;
  }
  if (newkeys & KEY_BUTTRIGHT) {
    if (spop.submenu == GbaNorLoad) {
      if (spop.selector == GBALdSetCheats)
        spop.p.norld.l.use_cheats = !spop.p.norld.l.use_cheats;
      if (uses_dsave) {
        if (spop.selector == GBALdSetLoadP)
          spop.p.norld.l.sram_load_type = (spop.p.norld.l.sram_load_type + 1) % SaveLoadDSCNT;
      } else {
        if (spop.selector == GBALdSetLoadP)
          spop.p.norld.l.sram_load_type = (spop.p.norld.l.sram_load_type + 1) % SaveLoadCNT;
        else if (spop.selector == GBALdSetSaveP)
          spop.p.norld.l.sram_save_type = (spop.p.norld.l.sram_save_type + 1) % SaveCNT;
      }
    }

    // DirSav forces automatic saving
    if (uses_dsave)
      spop.p.norld.l.sram_save_type = SaveDirect;
    else if (spop.p.norld.l.sram_save_type == SaveDirect)
      spop.p.norld.l.sram_save_type = autosave_default ? SaveReboot : SaveDisable;

    // If DS is selected, do not allow manual mode.
    if (spop.p.norld.l.sram_load_type == SaveLoadDisable && uses_dsave)
      spop.p.norld.l.sram_load_type = SaveLoadSav;
    // If no .sav is available, do not allow that option!
    if (spop.p.norld.l.sram_load_type == SaveLoadSav && !spop.p.norld.l.savefile_found)
      spop.p.norld.l.sram_load_type = SaveLoadReset;
  }

  // Disable cheat loading if no cheats are avail, or IGM is disabled
  if (!spop.p.norld.l.cheats_found || !uses_igm)
    spop.p.norld.l.use_cheats = false;

  if (newkeys & KEY_BUTTA) {
    if (spop.submenu == GbaLoadPopInfo) {
      const int stype = GET_GATTR_SAVEM(e->gattrs);
      const EnumSavetype st = stype < 0 ? SaveTypeNone : stype;
      bool uses_dsave = e->gattrs & GATTR_SAVEDS;
      bool uses_igm   = e->gattrs & GATTR_IGM;
      bool uses_rtc   = e->gattrs & GATTR_RTC;

      t_dirsave_info dsinfo;
      unsigned errsave = prepare_savegame(
        spop.p.norld.l.sram_load_type, spop.p.norld.l.sram_save_type,
        st, &dsinfo, spop.p.norld.l.savefn);
      if (errsave) {
        unsigned errmsg = (errsave == ERR_SAVE_BADSAVE)   ? MSG_ERR_SAVERD :
                          (errsave == ERR_SAVE_CANTALLOC) ? MSG_ERR_SAVEPR :
                          (errsave == ERR_SAVE_BADARG)    ? MSG_ERR_SAVEIT :
                                                            MSG_ERR_SAVEWR;
        spop.alert_msg = msgs[lang_id][errmsg];
        return;
      }
      t_rtc_info rtci = {
        .timestamp = spop.p.norld.l.rtcval,
        .ts_step = rtcspeed_default
      };


      unsigned err = launch_gba_nor(
        e->game_name,
        spop.p.norld.l.sram_save_type == SaveDisable ? NULL : spop.p.norld.l.savefn,
        e->blkmap, e->numblks,
        uses_dsave ? &dsinfo : NULL,
        uses_rtc ? &rtci : NULL,
        uses_igm,
        spop.p.norld.l.use_cheats ? spop.p.norld.l.cheats_size : 0);
      if (err) spop.alert_msg=msgs[lang_id][MSG_ERR_READ];
    }
    else if (spop.selector == GBALdRemember) {
      // Save settings to disk now!
      t_rom_load_settings ld_sett = {  // Use defaults in case it doesn't really exist
        .patch_policy = patcher_default,
        .use_igm = ingamemenu_default,
        .use_rtc = rtcpatch_default,
        .use_dsaving = autosave_prefer_ds
      };
      t_rom_launch_settings lh_sett = {
        .use_cheats = spop.p.norld.l.use_cheats,
        .rtcts = spop.p.norld.l.rtcval
      };

      // We load the loading settings to ensure we do not overwrite them.
      load_rom_settings(e->game_name, &ld_sett, NULL);
      save_rom_settings(e->game_name, &ld_sett, &lh_sett);
      spop.alert_msg = msgs[lang_id][MSG_REMEMB_CFG_OK];
    }
    else if (spop.selector == GBALdSetRTC) {
      void accept_rtc() {
        spop.p.norld.l.rtcval = date2timestamp(&spop.rtcpop.val);
      }
      if (uses_rtc) {
        timestamp2date(spop.p.norld.l.rtcval, &spop.rtcpop.val);
        spop.rtcpop.menu_clock=false;
    spop.rtcpop.callback = accept_rtc;
      }
    }
  }
}
#endif

static void keypress_popup_filemgr(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    spop.selector = MAX(0, spop.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    spop.selector = MIN(FiMgrCNT - 1, spop.selector + 1);

  if (newkeys & KEY_BUTTA) {
    t_centry *e = sdr_state->filefilter[smenu.browser.selector];
    switch (spop.selector) {
    case FiMgrDelete:
      {
        void remove_file_action(bool confirm) {
          char tmpfn[MAX_FN_LEN];
          strcpy(tmpfn, smenu.browser.cpath);
          strcat(tmpfn, sdr_state->filefilter[smenu.browser.selector]->fname);

          if (confirm) {
            if (FR_OK != f_unlink(tmpfn))
              spop.alert_msg = msgs[lang_id][MSG_ERR_DELFILE];
            else
              spop.alert_msg = msgs[lang_id][MSG_OK_DELFILE];

            browser_reload();   // Force reload so the file disappears!
          }
        }
        spop.qpop.message = msgs[lang_id][MSG_Q0_DELFILE];
        spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
        spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
        spop.qpop.option = 0;
        spop.qpop.callback = remove_file_action;
        spop.qpop.clear_popup_ok = true;
      }
      break;
    case FiMgrHide:
      {
        char tmpfn[MAX_FN_LEN];
        strcpy(tmpfn, smenu.browser.cpath);
        strcat(tmpfn, sdr_state->filefilter[smenu.browser.selector]->fname);

        if (FR_OK == f_chmod(tmpfn, e->attr ^ AM_HID, AM_HID))
          e->attr ^= AM_HID;
        else
          spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];
      }
      spop.pop_num = POPUP_NONE;
      break;

    #ifdef SUPPORT_NORGAMES
    case FiMgrWriteNOR:
      if (e->filesize > MAX_GBA_ROM_SIZE)
        spop.alert_msg = msgs[lang_id][MSG_ERR_TOOBIG];
      else {
        char path[MAX_FN_LEN];
        strcpy(path, smenu.browser.cpath);
        strcat(path, e->fname);

        // Load default loading settings if any.
        t_rom_load_settings ld_sett = {
          .patch_policy = patcher_default,
          .use_igm = ingamemenu_default,
          .use_rtc = rtcpatch_default,
          .use_dsaving = autosave_prefer_ds
        };
        load_rom_settings(path, &ld_sett, NULL);

        if (!prepare_gba_info(&spop.p.norwr.i, &ld_sett, path, e->filesize, false))
          spop.alert_msg = msgs[lang_id][MSG_ERR_READ];
        else {
          spop.pop_num = POPUP_GBA_NORWRITE;
          spop.submenu = GbaLoadPopInfo;
          spop.selector = 0;
        }
      }
      break;
    #endif
    };
  }
}

static void keypress_menu_recent(unsigned newkeys) {
  if (smenu.recent.maxentries) {
    if (newkeys & KEY_BUTTUP)
      smenu.recent.selector = MAX(0, smenu.recent.selector - 1);
    else if (newkeys & KEY_BUTTDOWN)
      smenu.recent.selector = MIN(smenu.recent.maxentries - 1, smenu.recent.selector + 1);
    if (newkeys & KEY_BUTTLEFT) {
      smenu.recent.selector = MAX(0, smenu.recent.selector - RECENT_ROWS);
      smenu.recent.seloff   = MAX(0, smenu.recent.seloff - RECENT_ROWS);
    }
    else if (newkeys & KEY_BUTTRIGHT) {
      smenu.recent.selector = MIN(smenu.recent.maxentries - 1, smenu.recent.selector + RECENT_ROWS);
      smenu.recent.seloff   = MIN(smenu.recent.maxentries - 1, smenu.recent.seloff   + RECENT_ROWS);
    }
    if (newkeys & KEY_BUTTA) {
      t_rentry *e = &sdr_state->rentries[smenu.recent.selector];
      #ifdef SUPPORT_NORGAMES
      if (e->flags & FLAG_RECENT_NOR) {
        // Try to find the ROM in the current flash metadata.
        for (unsigned i = 0; i < sdr_state->nordata.gamecnt; i++) {
          const t_flash_game_entry *fe = &sdr_state->nordata.games[i];
          if (!strcmp(e->fpath, fe->game_name)) {
            browser_open_nor(fe);
            return;
          }
        }
        spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];
      }
      else
      #endif
      {
        // stat() the file since we need the size, and validate that it exists!
        FILINFO info;
        FRESULT res = f_stat(e->fpath, &info);
        if (res == FR_OK)
          browser_open(e->fpath, info.fsize);
        else
          spop.alert_msg = msgs[lang_id][MSG_ERR_READ];
      }
    }
    else if (newkeys & KEY_BUTTSEL) {
      void recent_del_cb(bool confirm) {
        if (confirm)
          delete_recent_flush(smenu.recent.selector);
      }
      spop.qpop.message = msgs[lang_id][MSG_Q4_DELREC];
      spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
      spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
      spop.qpop.option = 0;
      spop.qpop.callback = recent_del_cb;
      spop.qpop.clear_popup_ok = false;
    }
  }

  if (smenu.recent.selector < smenu.recent.seloff)
    smenu.recent.seloff = smenu.recent.selector;
  else if (smenu.recent.selector >= smenu.recent.seloff + RECENT_ROWS)
    smenu.recent.seloff = smenu.recent.selector - RECENT_ROWS + 1;
}

static void keypress_menu_browse(unsigned newkeys) {
  int old_selector = smenu.browser.selector;
  if (menu_theme<THEME_DSSMM_LIGHT && (newkeys & KEY_BUTTSTA)) {
    artwork_mode = (artwork_mode + 1) % 3;
    browser_invalidate_artwork();
  }
  if (smenu.browser.dispentries) {
    // Move menu up and down
    if (newkeys & KEY_BUTTUP)
      smenu.browser.selector=browser_carousel_step(smenu.browser.selector,smenu.browser.dispentries,-1,browser_view && carousel_wrap);
    if (newkeys & KEY_BUTTDOWN)
      smenu.browser.selector=browser_carousel_step(smenu.browser.selector,smenu.browser.dispentries,1,browser_view && carousel_wrap);
    if (newkeys & KEY_BUTTLEFT) {
      int step = browser_view ? 1 : BROWSER_ROWS;
      smenu.browser.selector=browser_view?browser_carousel_step(smenu.browser.selector,smenu.browser.dispentries,-1,carousel_wrap):MAX(0,smenu.browser.selector-step);
      smenu.browser.seloff   = MAX(0, smenu.browser.seloff - step);
    }
    if (newkeys & KEY_BUTTRIGHT) {
      int step = browser_view ? 1 : BROWSER_ROWS;
      smenu.browser.selector=browser_view?browser_carousel_step(smenu.browser.selector,smenu.browser.dispentries,1,carousel_wrap):MIN(smenu.browser.dispentries-1,smenu.browser.selector+step);
      smenu.browser.seloff   = MIN(smenu.browser.dispentries - 1, smenu.browser.seloff   + step);
    }
    // Move into a new dir and/or open a file
    if (newkeys & KEY_BUTTA) {
      t_centry *e = sdr_state->filefilter[smenu.browser.selector];
      if (e->isdir) {
        strcat(smenu.browser.cpath, e->fname);
        strcat(smenu.browser.cpath, "/");
        // Push selector history and reset it in the new dir
        memmove(&smenu.browser.selhist[1], &smenu.browser.selhist[0],
                sizeof(smenu.browser.selhist) - sizeof(smenu.browser.selhist[0]));
        smenu.browser.selhist[0] = smenu.browser.selector;
        smenu.browser.selector = 0;
        browser_reload();
      } else {
        char path[MAX_FN_LEN];
        strcpy(path, smenu.browser.cpath);
        strcat(path, e->fname);
        browser_open(path, e->filesize);
      }
    }
    else if (newkeys & KEY_BUTTSEL) {
      // Shows a file management menu.
      spop.pop_num = POPUP_FILE_MGR;
      spop.anim = 0;
      spop.selector = 0;
    }
  }
  if (newkeys & KEY_BUTTB) {
    // Try to go up in the dir structure
    if (movedir_up()) {
      smenu.browser.selector = smenu.browser.selhist[0];
      memmove(&smenu.browser.selhist[0], &smenu.browser.selhist[1],
              sizeof(smenu.browser.selhist) - sizeof(smenu.browser.selhist[0]));
      browser_reload();
    }
  }

  // Selector was updated, figure out how we update the menu params so it
  // can be rendered properly.
  if (smenu.browser.selector < smenu.browser.seloff)
    smenu.browser.seloff = smenu.browser.selector;
  else if (smenu.browser.selector >= smenu.browser.seloff + BROWSER_ROWS)
    smenu.browser.seloff = smenu.browser.selector - BROWSER_ROWS + 1;

  if (browser_view && smenu.browser.selector != old_selector &&
      (newkeys & (KEY_BUTTUP | KEY_BUTTDOWN | KEY_BUTTLEFT | KEY_BUTTRIGHT))) {
    int direction=(newkeys&(KEY_BUTTDOWN|KEY_BUTTRIGHT))?1:-1;
    browser_motion_offset=browser_motion_retarget(browser_motion_offset,direction);
    artwork_cached = false;
  carousel_hot_valid = false;
  }
}

#ifdef SUPPORT_NORGAMES
static void keypress_menu_norbrowse(unsigned newkeys) {
  if (smenu.fbrowser.maxentries) {
    if (newkeys & KEY_BUTTUP)
      smenu.fbrowser.selector = MAX(0, smenu.fbrowser.selector - 1);
    if (newkeys & KEY_BUTTDOWN)
      smenu.fbrowser.selector = MIN(smenu.fbrowser.maxentries - 1, smenu.fbrowser.selector + 1);
    if (newkeys & KEY_BUTTLEFT) {
      smenu.fbrowser.selector = MAX(0, smenu.fbrowser.selector - NORGAMES_ROWS);
      smenu.fbrowser.seloff   = MAX(0, smenu.fbrowser.seloff - NORGAMES_ROWS);
    }
    if (newkeys & KEY_BUTTRIGHT) {
      smenu.fbrowser.selector = MIN(smenu.fbrowser.maxentries - 1, smenu.fbrowser.selector + NORGAMES_ROWS);
      smenu.fbrowser.seloff   = MIN(smenu.fbrowser.maxentries - 1, smenu.fbrowser.seloff   + NORGAMES_ROWS);
    }

    if (newkeys & KEY_BUTTA)
      browser_open_nor(&sdr_state->nordata.games[smenu.fbrowser.selector]);
    else if (newkeys & KEY_BUTTSEL) {
      // Prompt NOR entry deletion.
      void remove_nor_action(bool confirm) {
        if (!confirm)
          return;

        // Remove game entry, just memmove the other games on top.
        sdr_state->nordata.gamecnt--;
        memmove32(&sdr_state->nordata.games[smenu.fbrowser.selector],
                  &sdr_state->nordata.games[smenu.fbrowser.selector + 1],
                  (sdr_state->nordata.gamecnt - smenu.fbrowser.selector) * sizeof(t_flash_game_entry));

        // Go ahead and write a new metadata entry;
        if (!flashmgr_store(ROM_FLASHMETA_ADDR, FLASH_METADATA_SIZE, (t_reg_entry*)&sdr_state->nordata))
          spop.alert_msg = msgs[lang_id][MSG_ERR_NORUPD];
        flashbrowser_reload();   // Force list reload, free block calculation, etc.
      }
      spop.qpop.message = msgs[lang_id][MSG_Q5_DELNORG];
      spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
      spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
      spop.qpop.option = 0;
      spop.qpop.callback = remove_nor_action;
      spop.qpop.clear_popup_ok = true;
    }

    if (smenu.fbrowser.selector < smenu.fbrowser.seloff)
      smenu.fbrowser.seloff = smenu.fbrowser.selector;
    else if (smenu.fbrowser.selector >= smenu.fbrowser.seloff + NORGAMES_ROWS)
      smenu.fbrowser.seloff = smenu.fbrowser.selector - NORGAMES_ROWS + 1;
  }
}
#endif

static void keypress_menu_settings(unsigned newkeys) {
  settings_move(newkeys);
  if (newkeys & KEY_BUTTLEFT) {
    if (smenu.set.selector == SettHotkey)
      hotkey_combo = (hotkey_combo + hotkey_listcnt - 1) % hotkey_listcnt;
    else if (smenu.set.selector == SettSaveBkp)
      backup_sram_default = backup_sram_default ? backup_sram_default - 1 : 0;
    else if (smenu.set.selector == DefsPatchEng)
      patcher_default = (patcher_default + PatchTotalCNT - 1) % PatchTotalCNT;
    else if (smenu.set.selector == DefsRTCSpeed)
      rtcspeed_default = (rtcspeed_default + RTC_SPEED_CNT - 1) % RTC_SPEED_CNT;
  }
  if (newkeys & KEY_BUTTRIGHT) {
    if (smenu.set.selector == SettHotkey)
      hotkey_combo = (hotkey_combo + 1) % hotkey_listcnt;
    else if (smenu.set.selector == SettSaveBkp)
      backup_sram_default = MIN(MAX_BACKUP_CNT, backup_sram_default + 1);
    else if (smenu.set.selector == DefsPatchEng)
      patcher_default = (patcher_default + 1) % PatchTotalCNT;
    else if (smenu.set.selector == DefsRTCSpeed)
      rtcspeed_default = (rtcspeed_default + 1) % RTC_SPEED_CNT;
  }
  if (newkeys & (KEY_BUTTLEFT | KEY_BUTTRIGHT)) {
    if (smenu.set.selector == SettBootType)
      boot_bios_splash ^= 1;
    else if (smenu.set.selector == SettCheatEn)
      enable_cheats ^= 1;
    else if (smenu.set.selector == DefsGamMenu)
      ingamemenu_default ^= 1;
    else if (smenu.set.selector == DefsRTCEnb)
      rtcpatch_default ^= 1;
    else if (smenu.set.selector == DefsLoadPol)
      autoload_default ^= 1;
    else if (smenu.set.selector == DefsSavePol)
      autosave_default ^= 1;
    else if (smenu.set.selector == DefsPrefDS)
      autosave_prefer_ds ^= 1;
    else if (smenu.set.selector == SettFastSD)
      use_slowld ^= 1;
    else if (smenu.set.selector == SettFastEWRAM)
      use_fastew = fastew ? (use_fastew ^ 1) : 0;
    #ifdef SUPPORT_NORGAMES
    else if (smenu.set.selector == SettVerifyNOR)
      use_verify_nor ^= 1;
    #endif
  }

  if (newkeys & KEY_BUTTA && smenu.set.selector == DefsRTCVal) {
    void accept_rtc() {
      rtcvalue_default = date2timestamp(&spop.rtcpop.val);
    }
    timestamp2date(rtcvalue_default, &spop.rtcpop.val);
    spop.rtcpop.menu_clock=false;
    spop.rtcpop.callback = accept_rtc;
  }

}

static void keypress_menu_uisettings(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    smenu.uiset.selector = MAX(0, smenu.uiset.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    smenu.uiset.selector = MIN(UiSetMAX, smenu.uiset.selector + 1);
  if (newkeys & KEY_BUTTLEFT) {
    if (smenu.uiset.selector == UiSetTheme)
      menu_theme = menu_theme ? menu_theme - 1 : 0;
    else if (smenu.uiset.selector == UiSetFont)
      menu_font ^= 1;
    else if (smenu.uiset.selector == UiSetArtwork)
      artwork_mode = (artwork_mode + 2) % 3;
    else if (smenu.uiset.selector == UiSetView) {
      browser_view = (browser_view + 2) % 3;
      browser_motion_offset = 0;
      browser_invalidate_artwork();
    }

    else if (smenu.uiset.selector == UiSetTopBar)
      menu_topbar = menu_topbar ? menu_topbar-1 : 0;
    else if (smenu.uiset.selector == UiSetAccent)
      menu_accent = menu_accent ? menu_accent - 1 : 0;
    else if (smenu.uiset.selector == UiSetASpd)
      anim_speed = anim_speed ? anim_speed - 1 : 0;
    else if (smenu.uiset.selector == UiSetWrap)
      carousel_wrap ^= 1, browser_invalidate_artwork();
    else if (smenu.uiset.selector == UiSetClock)
      menu_clock_24 ^= 1;
    else if (smenu.uiset.selector == UiSetClean)
      clean_list ^= 1;
    else if (smenu.uiset.selector == UiSetHid)
      hide_hidden ^= 1;
    else if (smenu.uiset.selector == UiSetRect)
      recent_menu ^= 1;
    else if (smenu.uiset.selector == UiSetLang)
      lang_id = (lang_id + LANG_COUNT - 1) % LANG_COUNT;
  }
  if (newkeys & KEY_BUTTRIGHT) {
    if (smenu.uiset.selector == UiSetTheme)
      menu_theme = MIN(THEME_COUNT - 1, menu_theme + 1);
    else if (smenu.uiset.selector == UiSetFont)
      menu_font ^= 1;
    else if (smenu.uiset.selector == UiSetArtwork)
      artwork_mode = (artwork_mode + 1) % 3;
    else if (smenu.uiset.selector == UiSetView) {
      browser_view = (browser_view + 1) % 3;
      browser_motion_offset = 0;
      browser_invalidate_artwork();
    }

    else if (smenu.uiset.selector == UiSetTopBar)
      menu_topbar = MIN(ACCENT_COUNT-1,menu_topbar+1);
    else if (smenu.uiset.selector == UiSetAccent)
      menu_accent = MIN(ACCENT_COUNT - 1, menu_accent + 1);
    else if (smenu.uiset.selector == UiSetASpd)
      anim_speed = MIN(animspd_cnt - 1, anim_speed + 1);
    else if (smenu.uiset.selector == UiSetWrap)
      carousel_wrap ^= 1, browser_invalidate_artwork();
    else if (smenu.uiset.selector == UiSetClock)
      menu_clock_24 ^= 1;
    else if (smenu.uiset.selector == UiSetClean)
      clean_list ^= 1;
    else if (smenu.uiset.selector == UiSetHid)
      hide_hidden ^= 1;
    else if (smenu.uiset.selector == UiSetRect)
      recent_menu ^= 1;
    else if (smenu.uiset.selector == UiSetLang)
      lang_id = (lang_id + 1) % LANG_COUNT;
  }

  unsigned rows=ui_visible_rows();
  if (smenu.uiset.selector < smenu.uiset.seloff)
    smenu.uiset.seloff = smenu.uiset.selector;
  else if (smenu.uiset.selector >= smenu.uiset.seloff + (int)rows)
    smenu.uiset.seloff = smenu.uiset.selector-(int)rows+1;

  // Apply settings immediately
  reload_theme(menu_theme);

  // This is relatively cheap, we can trigger it here already (no sorting)
  if ((newkeys & (KEY_BUTTLEFT | KEY_BUTTRIGHT)) && smenu.uiset.selector == UiSetHid)
    browser_reload_filter();
}

static void keypress_menu_tools(unsigned newkeys) {
  if (newkeys & KEY_BUTTUP)
    smenu.tools.selector = MAX(0, smenu.tools.selector - 1);
  if (newkeys & KEY_BUTTDOWN)
    smenu.tools.selector = MIN(ToolsMAX - 1, smenu.tools.selector + 1);

  if (newkeys & KEY_BUTTA) {
    if (smenu.tools.selector == ToolsSDRAMTest) {
      // Performs a test on the SRAM/SDRAM, ensure they are fine.
      set_supercard_mode(MAPPED_SDRAM, true, false);

      if (sdram_test(loadrom_progress_abort))
        spop.alert_msg = msgs[lang_id][MSG_BAD_SDRAM];
      else
        spop.alert_msg = msgs[lang_id][MSG_GOOD_RAM];

      set_supercard_mode(MAPPED_SDRAM, true, true);
    }
    if (smenu.tools.selector == ToolsSRAMTest) {
      if (sram_test())
        spop.alert_msg = msgs[lang_id][MSG_BAD_SRAM];
      else
        spop.alert_msg = msgs[lang_id][MSG_GOOD_RAM];
    }
    else if (smenu.tools.selector == ToolsBatteryTest) {
      // Go ahead and fill in SRAM with a pattern.
      spop.qpop.message = msgs[lang_id][MSG_Q2_SRAMTST];
      spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
      spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
      spop.qpop.option = 0;
      spop.qpop.callback = sram_battery_test_callback;
      spop.qpop.clear_popup_ok = true;
    }
    else if (smenu.tools.selector == ToolsSDBench) {
      slowsd = use_slowld;
      int ret = sdbench_read(loadrom_progress_abort);
      slowsd = true;
      if (ret <= 0)
        spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];
      else {
        unsigned speed = 8*1024*1024 / (unsigned)ret;
        npf_snprintf(smenu.info.tstr, sizeof(smenu.info.tstr), msgs[lang_id][MSG_BENCHSPD], speed);
        spop.alert_msg = smenu.info.tstr;
      }
    }
    else if (smenu.tools.selector == ToolsFlashBak) {
      // Backup the flash contents to a file.
      if (dump_flashmem_backup())
        spop.alert_msg = msgs[lang_id][MSG_FLASH_READOK];
      else
        spop.alert_msg = msgs[lang_id][MSG_ERR_GENERIC];

      browser_reload();
    }
    #ifdef SUPPORT_NORGAMES
    else if (smenu.tools.selector == ToolsFlashClr) {
      void flash_clear_callback(bool confirm) {
        if (confirm) {
          // Delete all metadata (data is not really wiped, takes too long)
          if (flashmgr_wipe(ROM_FLASHMETA_ADDR, FLASH_METADATA_SIZE))
            spop.alert_msg = msgs[lang_id][MSG_NOR_CLOK];
          else
            spop.alert_msg = msgs[lang_id][MSG_ERR_NORUPD];

          flashbrowser_reload();     // Ensure we clear the NOR entries from RAM.
        }
      }
      // Prompt the user for clearing the memory.
      spop.qpop.message = msgs[lang_id][MSG_Q6_CLRNOR];
      spop.qpop.default_button = msgs[lang_id][MSG_Q_NO];
      spop.qpop.confirm_button = msgs[lang_id][MSG_Q_YES];
      spop.qpop.option = 0;
      spop.qpop.callback = flash_clear_callback;
      spop.qpop.clear_popup_ok = true;
    }
    #endif
  }
}

static void keypress_menu_info(unsigned newkeys) {
  if(update_unlock_feed(&update_unlock,newkeys&fresh_keys,frame_count)){enable_flashing=true;return;}
  if (newkeys & KEY_BUTTA)
    smenu.info.selector = (smenu.info.selector + 1) % 4;
}


static void accept_menu_clock(void) {
  menu_clock_seed=(unsigned)spop.rtcpop.val.hour*3600+(unsigned)spop.rtcpop.val.min*60;
  launcher_clock_set(menu_clock_seed);launcher_invalidate();
  settings_save_request();
}
static void open_menu_clock(void) {
  uint32_t seconds=launcher_clock_get();if(seconds==UINT32_MAX)seconds=12*3600;
  spop.rtcpop.val=(t_dec_date){.year=0,.month=1,.day=1,.hour=seconds/3600,.min=seconds/60%60};
  spop.rtcpop.selector=3;spop.rtcpop.menu_clock=true;spop.rtcpop.callback=accept_menu_clock;
}
static void launcher_move(unsigned newkeys) {
  if(launcher_selected==1 || launcher_selected==2)launcher_browser_panel=launcher_selected;
  if(newkeys&KEY_BUTTDOWN)launcher_selected=launcher_selected==0?launcher_browser_panel:launcher_selected<=2?3:launcher_selected==3?4:0;
  else if(newkeys&KEY_BUTTUP)launcher_selected=launcher_selected==0?4:launcher_selected==4?3:launcher_selected==3?launcher_browser_panel:0;
  if(newkeys&KEY_BUTTLEFT && launcher_selected==2)launcher_selected=1;
  if(newkeys&KEY_BUTTRIGHT && launcher_selected==1)launcher_selected=2;
  if(launcher_selected==1 || launcher_selected==2)launcher_browser_panel=launcher_selected;
}

void menu_keypress(unsigned newkeys) {
  /* Finish the pending transaction before actions can remap cartridge memory. */
  if((newkeys&KEY_BUTTA) && settings_save_pending() &&
      (spop.pop_num || (!launcher_page &&
       (smenu.menu_tab==MENUTAB_ROMBROWSE || smenu.menu_tab==MENUTAB_RECENT)))) {
    if(!settings_save_flush()){spop.alert_msg=msgs[lang_id][MSG_ERR_SETSAVE];return;}
  }

  if (restart_required) return;
  if(smenu.menu_tab!=MENUTAB_INFO || launcher_page || spop.pop_num || spop.alert_msg || spop.qpop.message || spop.rtcpop.callback || (newkeys&(KEY_BUTTB|KEY_BUTTSEL|KEY_BUTTSTA|KEY_BUTTL|KEY_BUTTR)))update_unlock.position=0;
  /* Actions/tools may reuse cartridge RAM; never reuse the preview afterward. */
  if (newkeys & (KEY_BUTTA | KEY_BUTTSEL | KEY_BUTTL | KEY_BUTTR))
    browser_invalidate_artwork();
  if (spop.alert_msg) {
    // Modal message pop up!
    if (newkeys & (KEY_BUTTA | KEY_BUTTB))
      spop.alert_msg = NULL;
  }
  else if (spop.qpop.message) {
    // Modal confirm/question dialog
    if (newkeys & (KEY_BUTTUP | KEY_BUTTDOWN))
      spop.qpop.option ^= 1;
    else if (newkeys & KEY_BUTTB)
      spop.qpop.message = NULL;   // Exit the modal dialog.
    else if (newkeys & KEY_BUTTA) {
      if (spop.qpop.callback) {
        if (spop.qpop.option && spop.qpop.clear_popup_ok)
          spop.pop_num = POPUP_NONE;
        spop.qpop.callback(spop.qpop.option);
      }
      spop.qpop.message = NULL;   // Exit the modal dialog.
    }
  }
  else if (spop.rtcpop.callback) {
    if (newkeys & KEY_BUTTLEFT)
      spop.rtcpop.selector = MAX(spop.rtcpop.menu_clock?3:0, spop.rtcpop.selector - 1);
    if (newkeys & KEY_BUTTRIGHT)
      spop.rtcpop.selector = MIN(4, spop.rtcpop.selector + 1);

    if (newkeys & KEY_BUTTUP)
      ((uint8_t*)&spop.rtcpop.val)[spop.rtcpop.selector]++;
    if (newkeys & KEY_BUTTDOWN)
      ((uint8_t*)&spop.rtcpop.val)[spop.rtcpop.selector]--;

    if (newkeys & (KEY_BUTTUP|KEY_BUTTDOWN))
      fixdate(&spop.rtcpop.val);

    if (newkeys & KEY_BUTTB) {
      spop.rtcpop.selector = 0;
      spop.rtcpop.callback = NULL;
    }
    else if (newkeys & KEY_BUTTA) {
      spop.rtcpop.selector = 0;
      spop.rtcpop.callback();
      spop.rtcpop.callback = NULL;
    }
  }
  else if (spop.pop_num) {
    const int subcnt = popup_windows[spop.pop_num - 1].max_submenu;
    if (newkeys & KEY_BUTTL)
      spop.submenu = (spop.submenu + subcnt - 1) % subcnt;
    if (newkeys & KEY_BUTTR)
      spop.submenu = (spop.submenu + 1) % subcnt;

    // Close pop-up on B button
    if (newkeys & KEY_BUTTB)
      spop.pop_num = 0;
    else {
      const t_mkeyupd_fn keyfns[] = {
        NULL,
        keypress_popup_loadgba,
        keypress_popup_savefile,
        keypress_popup_flash,
        keypress_popup_filemgr,
        #ifdef SUPPORT_NORGAMES
        keypress_popup_norwrite,
        keypress_popup_norload,
        #endif
      };
      keyfns[spop.pop_num](newkeys);
    }
  } else {
    /* Queue a snapshot on exit; filesystem steps run between menu frames. */
    bool settings_tab=!launcher_page && (smenu.menu_tab==MENUTAB_SETTINGS || smenu.menu_tab==MENUTAB_UILANG);
    if(settings_tab && (newkeys&(KEY_BUTTB|KEY_BUTTSTA|KEY_BUTTL|KEY_BUTTR))) {
      settings_save_request();
      if(menu_theme<THEME_DSSMM_LIGHT && (newkeys&KEY_BUTTB)){smenu.menu_tab=MENUTAB_ROMBROWSE;return;}
    }
    if(menu_theme>=THEME_DSSMM_LIGHT) {
      if(newkeys)launcher_invalidate();
      if((newkeys&KEY_BUTTSTA) && launcher_page!=1) {launcher_page=1;browser_invalidate_artwork();return;}
      if((newkeys&KEY_BUTTSTA) && launcher_page==1)newkeys=(newkeys&~KEY_BUTTSTA)|KEY_BUTTA;
      if(launcher_page==1) {
        launcher_move(newkeys);
        if(newkeys&KEY_BUTTB) {launcher_page=0;smenu.menu_tab=MENUTAB_ROMBROWSE;}
        if(newkeys&KEY_BUTTA) {
          browser_invalidate_artwork();
          if(launcher_selected==0) {
            if(smenu.recent.maxentries){smenu.recent.selector=smenu.recent.seloff=0;keypress_menu_recent(KEY_BUTTA);}
          } else if(launcher_selected==1){launcher_page=0;smenu.menu_tab=MENUTAB_ROMBROWSE;}
          else if(launcher_selected==2){launcher_page=0;
#ifdef SUPPORT_NORGAMES
            smenu.menu_tab=MENUTAB_NORBROWSE;
#else
            smenu.menu_tab=MENUTAB_RECENT;
#endif
          } else if(launcher_selected==3)launcher_page=2;
          else open_menu_clock();
        }
        return;
      }
      if(launcher_page==2) {
        if(newkeys&KEY_BUTTUP)launcher_category=launcher_category?launcher_category-1:0;
        if(newkeys&KEY_BUTTDOWN)launcher_category=MIN(4,launcher_category+1);
        if(newkeys&KEY_BUTTB)launcher_page=1;
        if(newkeys&KEY_BUTTA) {
          static const uint8_t tabs[]={MENUTAB_UILANG,MENUTAB_SETTINGS,MENUTAB_SETTINGS,MENUTAB_TOOLS,MENUTAB_INFO};
          launcher_page=0;smenu.menu_tab=tabs[launcher_category];
          if(launcher_category==1 || launcher_category==2) {
            launcher_settings_group=launcher_category;smenu.set.selector=settings_option_at(0);
          }
        }
        return;
      }
#ifdef SUPPORT_NORGAMES
      if((newkeys&KEY_BUTTB) && smenu.menu_tab==MENUTAB_NORBROWSE){launcher_page=1;launcher_selected=launcher_browser_panel=2;return;}
#endif
      if((newkeys&KEY_BUTTB) && smenu.menu_tab==MENUTAB_ROMBROWSE && !strcmp(smenu.browser.cpath,"/")){launcher_page=1;launcher_selected=launcher_browser_panel=1;return;}
      if((newkeys&KEY_BUTTB) && smenu.menu_tab>=MENUTAB_SETTINGS){launcher_page=2;return;}
    }
    // Menu change via trigger buttons
    int mintab = (recent_menu && smenu.recent.maxentries) ? MENUTAB_RECENT : MENUTAB_ROMBROWSE;
    if (newkeys & KEY_BUTTL)
      smenu.menu_tab = MAX((int)smenu.menu_tab - 1, mintab);
    else if (newkeys & KEY_BUTTR)
      smenu.menu_tab = MIN(smenu.menu_tab + 1, MENUTAB_MAX - 1);

    if((newkeys&(KEY_BUTTL|KEY_BUTTR)) && smenu.menu_tab==MENUTAB_SETTINGS)
      launcher_settings_group=0;
    if (newkeys & (KEY_BUTTL | KEY_BUTTR | KEY_BUTTUP | KEY_BUTTDOWN))
      smenu.anim_state = 0;

    const t_mkeyupd_fn keyfns[] = {
      keypress_menu_recent,
      keypress_menu_browse,
      #ifdef SUPPORT_NORGAMES
      keypress_menu_norbrowse,
      #endif
      keypress_menu_settings,
      keypress_menu_uisettings,
      keypress_menu_tools,
      keypress_menu_info,
    };
    keyfns[smenu.menu_tab](newkeys);
  }
}

// Only repeat keys A/B and Dir
const uint16_t keyrep = 0x0F3;
static uint32_t keyreptmr[10] = {0};
static uint8_t  keyrepcnt[10] = {0};
static uint32_t prev_keys = 0;

// Handle button input. Supports key re-press whenever a button is held for a while.
// This key repeat pattern can be tuned for speed and what not.
uint16_t get_keypress() {
  uint32_t ckeys = curr_pressed_keys();
  fresh_keys=ckeys&~prev_keys;
  uint32_t mkeys = 0;
  for (unsigned i = 0; i < 10; i++) {
    if (ckeys & (1 << i)) {
      if (!(prev_keys & (1 << i))) {
        keyreptmr[i] = systime() + KEY_REPEAT_INITIAL;
        mkeys |= (1 << i);
        keyrepcnt[i] = 0;
      }
      else if (((1 << i) & keyrep) && systime() > keyreptmr[i]) {
        if (keyrepcnt[i] > KEY_REPEAT_CNT1)
          keyreptmr[i] = systime() + KEY_REPEAT_FAST;
        else {
          keyreptmr[i] = systime() + KEY_REPEAT_MID;
          keyrepcnt[i]++;
        }
        mkeys |= (1 << i);
      }
    }
    else
      keyreptmr[i] = 0;
  }

  prev_keys = ckeys;
  return mkeys;
}

