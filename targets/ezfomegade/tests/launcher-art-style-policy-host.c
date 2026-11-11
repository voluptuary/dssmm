#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;
typedef unsigned short u16;
static u16 launcher_art_border_mode;
static u16 launcher_art_rounded_corners;

#include POLICY_FILE

static void expect(int condition, const char *message)
{
	if(!condition)
	{
		fprintf(stderr, "FAIL: %s\n", message);
		exit(1);
	}
}

int main(void)
{
	launcher_art_border_mode = 2;
	launcher_art_rounded_corners = 1;
	expect(Launcher_ArtBorderEnabled(), "border applies to ordinary art when unrestricted");
	expect(Launcher_RoundedCornersForCustom(0), "rounding applies to ordinary art when unrestricted");

	launcher_art_border_mode = 2;
	expect(Launcher_ArtBorderEnabled(), "border applies to non-CUSTOM artwork");
	expect(Launcher_ArtBorderEnabled(), "border applies to CUSTOM artwork");

	launcher_art_rounded_corners = 3;
	expect(!Launcher_RoundedCornersForCustom(0), "CUSTOM Only rounding skips non-CUSTOM artwork");
	expect(Launcher_RoundedCornersForCustom(1), "CUSTOM Only rounding applies to CUSTOM artwork");

	launcher_art_border_mode = 0;
	launcher_art_rounded_corners = 0;
	expect(!Launcher_ArtBorderEnabled(), "Off disables border even for CUSTOM artwork");
	expect(!Launcher_RoundedCornersForCustom(1), "Off disables rounding even for CUSTOM artwork");

	launcher_art_border_mode = 2;
	expect(Launcher_ArtBorderEnabled(), "border is unaffected by CUSTOM Only corners mode");
	launcher_art_rounded_corners = 2;
	expect(Launcher_RoundedCornersForCustom(0), "No Start mode rounds non-CUSTOM artwork");
	puts("Artwork style policy scenarios passed.");
	return 0;
}
