/* Synthetic values for the extracted production settings screen.
 * No storage/input/save actions execute in this preview. */
#define SCREEN_HEIGHT 160
static struct { const char *cname; } hotkey_list[] = {{"L + R"}};
static unsigned hotkey_combo, boot_bios_splash, use_slowld, use_fastew=1;
#ifdef SUPPORT_NORGAMES
static unsigned use_verify_nor=1;
#endif
static unsigned backup_sram_default=3;
static unsigned enable_cheats=1, patcher_default, ingamemenu_default=1;
static unsigned rtcpatch_default, rtcspeed_default, rtcvalue_default;
static unsigned autoload_default=1, autosave_default=1, autosave_prefer_ds=1;
typedef struct { int year,month,day,hour,min; } t_dec_date;
static void timestamp2date(unsigned value,t_dec_date *out) {
  (void)value; *out=(t_dec_date){26,11,15,12,0};
}

/* Fields read by the production patch/loading presentation functions only. */
typedef struct {
 unsigned patch_type;
 bool use_dsaving,ingame_menu_enabled,rtc_patch_enabled;
} t_load_gba_info;
typedef struct {
 unsigned sram_load_type,sram_save_type,rtcval;
 bool use_cheats,cheats_found;
} t_load_gba_lcfg;
#define ICON_DISK 0
static void render_icon(unsigned x,unsigned y,unsigned icon) {(void)x;(void)y;(void)icon;}
