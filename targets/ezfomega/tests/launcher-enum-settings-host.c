/*
 * Differential test for the enum settings readers. The reader code is extracted VERBATIM from two versions
 * of source/ezkernelnew.c (see scripts/test-launcher-enum-settings-host.sh) and run over the same inputs;
 * the printed results must be identical.
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>

typedef unsigned char u8;
typedef unsigned short u16;
#include SOURCE_CONSTANTS            /* the LAUNCHER_VIEW_* and LAUNCHER_SIDE_ALIGN_* values, copied from the source */

enum {
	LTXT_CLOCK_12H, LTXT_VIEW_HORIZONTAL, LTXT_VIEW_VERTICAL, LTXT_VIEW_LIST_ART, LTXT_ALIGN_TOP, LTXT_ALIGN_CENTER,
	LTXT_ALIGN_BOTTOM, LTXT_ALIGN_LEFT, LTXT_ALIGN_RIGHT, LTXT_ALIGN_CUSTOM, LTXT_BORDER_ACCENT, LTXT_BORDER_BLACK,
	LTXT_BORDER_GREY, LTXT_BORDER_WHITE, LTXT_ROUNDED_FULL, LTXT_ROUNDED_NO_START, LTXT_ON, LTXT_COUNT
};
static const char *g_text[LTXT_COUNT];
static const char *Launcher_Text(unsigned id) { return g_text[id]; }
#define DSTEXT_CLOCK_12H Launcher_Text(LTXT_CLOCK_12H)
#define DSTEXT_VIEW_HORIZONTAL Launcher_Text(LTXT_VIEW_HORIZONTAL)
#define DSTEXT_VIEW_VERTICAL Launcher_Text(LTXT_VIEW_VERTICAL)
#define DSTEXT_VIEW_LIST_ART Launcher_Text(LTXT_VIEW_LIST_ART)
#define DSTEXT_ALIGN_TOP Launcher_Text(LTXT_ALIGN_TOP)
#define DSTEXT_ALIGN_CENTER Launcher_Text(LTXT_ALIGN_CENTER)
#define DSTEXT_ALIGN_BOTTOM Launcher_Text(LTXT_ALIGN_BOTTOM)
#define DSTEXT_ALIGN_LEFT Launcher_Text(LTXT_ALIGN_LEFT)
#define DSTEXT_ALIGN_RIGHT Launcher_Text(LTXT_ALIGN_RIGHT)
#define DSTEXT_ALIGN_CUSTOM Launcher_Text(LTXT_ALIGN_CUSTOM)
#define DSTEXT_BORDER_ACCENT Launcher_Text(LTXT_BORDER_ACCENT)
#define DSTEXT_BORDER_BLACK Launcher_Text(LTXT_BORDER_BLACK)
#define DSTEXT_BORDER_GREY Launcher_Text(LTXT_BORDER_GREY)
#define DSTEXT_BORDER_WHITE Launcher_Text(LTXT_BORDER_WHITE)
#define DSTEXT_ROUNDED_FULL Launcher_Text(LTXT_ROUNDED_FULL)
#define DSTEXT_ROUNDED_NO_START Launcher_Text(LTXT_ROUNDED_NO_START)
#define DSTEXT_ON Launcher_Text(LTXT_ON)

static u16 launcher_clock_24_hour, gl_show_Thumbnail, launcher_list_art_position, launcher_art_border_mode;
static u16 launcher_art_rounded_corners, launcher_vertical_side_align, launcher_horizontal_side_align;
static unsigned launcher_settings_migration_pending;
static unsigned g_vcount = 4, g_hcount = 4;
static u16 Launcher_VerticalSideOptionCount(void) { return (u16)g_vcount; }
static u16 Launcher_HorizontalSideOptionCount(void) { return (u16)g_hcount; }

static int g_present;
static const char *g_value;
static int Launcher_SettingsReadValue(const char *key, char *out, size_t out_size)
{
	(void)key;
	if(!g_present)
		return 0;
	strncpy(out, g_value, out_size - 1);
	out[out_size - 1] = '\0';
	return 1;
}

#include READERS_FILE

typedef struct { const char *name; void (*read)(void); u16 *target; } Reader;
static const Reader readers[] = {
	{ "ClockFormat", Launcher_ReadClockFormatSetting, &launcher_clock_24_hour },
	{ "ViewMode", Launcher_ReadViewModeSetting, &gl_show_Thumbnail },
	{ "ListArtPosition", Launcher_ReadListArtPositionSetting, &launcher_list_art_position },
	{ "ArtBorder", Launcher_ReadArtBorderSetting, &launcher_art_border_mode },
	{ "RoundedCorners", Launcher_ReadRoundedCornersSetting, &launcher_art_rounded_corners },
	{ "VerticalSide", Launcher_ReadVerticalSideSetting, &launcher_vertical_side_align },
	{ "HorizontalSide", Launcher_ReadHorizontalSideSetting, &launcher_horizontal_side_align },
};

static const char *words[] = { "12 hour", "12-hour", "12H", "24 hour", "24H", "Horizontal", "Vertical", "List", "List + art",
	"List art", "Top", "Center", "Centre", "Bottom", "Left", "Right", "Custom", "Accent", "Black", "Grey", "Gray", "White",
	"Full", "On", "Yes", "Off", "No", "No Start", "No start screen", "" };

int main(void)
{
	static const char *digits[] = { "0", "1", "2", "3", "4", "5", "9", "10", "1x", "2x", "0x", "-1", "x", "none", "list", "TOP" };
	static const u16 priors[] = { 0, 1, 2, 3, 4, 0xBEEF };
	char variants[6][64];
	const char *values[400];
	unsigned set, r, n, i, p, vc, hc, nwords = sizeof(words) / sizeof(words[0]);

	for(set = 0; set < 4; set++)
	{
		static char store[LTXT_COUNT][24];
		for(i = 0; i < LTXT_COUNT; i++)
		{
			if(set == 0) g_text[i] = words[(i * 3) % (nwords - 1)];                /* localised word collides with English words */
			else if(set == 1) { snprintf(store[i], sizeof(store[i]), "L%u_word", i); g_text[i] = store[i]; }
			else if(set == 2) g_text[i] = words[(i * 7 + 5) % (nwords - 1)];       /* crossed: another choice's English word */
			else { snprintf(store[i], sizeof(store[i]), "%u%s", i % 3, i % 2 ? "x" : ""); g_text[i] = store[i]; }  /* numeric */
		}
		for(r = 0; r < sizeof(readers) / sizeof(readers[0]); r++)
			for(vc = 0; vc < 6; vc++) for(hc = 0; hc < 6; hc += (vc < 5 ? 5 : 1))
			{
				g_vcount = vc; g_hcount = hc;
				for(p = 0; p < sizeof(priors) / sizeof(priors[0]); p++)               /* key absent */
				{
					*readers[r].target = priors[p]; launcher_settings_migration_pending = 0; g_present = 0; g_value = "";
					readers[r].read();
					printf("%s|set%u|%u|%u|absent|%u|%u|%u\n", readers[r].name, set, vc, hc, priors[p], *readers[r].target, launcher_settings_migration_pending);
				}
				n = 0;
				for(i = 0; i < nwords; i++) values[n++] = words[i];
				for(i = 0; i < sizeof(digits) / sizeof(digits[0]); i++) values[n++] = digits[i];
				for(i = 0; i < LTXT_COUNT; i++) values[n++] = g_text[i];
				for(i = 0; i < 6; i++) { snprintf(variants[i], sizeof(variants[i]), "%s", words[(i * 5 + 2) % (nwords - 1)]); variants[i][0] ^= 0x20; values[n++] = variants[i]; }
				for(i = 0; i < n; i++)                                                 /* key present, upper/lower variants too */
				{
					*readers[r].target = 0xBEEF; launcher_settings_migration_pending = 0; g_present = 1; g_value = values[i];
					readers[r].read();
					printf("%s|set%u|%u|%u|%s|%u|%u\n", readers[r].name, set, vc, hc, values[i], *readers[r].target, launcher_settings_migration_pending);
				}
			}
	}
	return 0;
}
