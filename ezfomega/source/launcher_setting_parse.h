#ifndef LAUNCHER_SETTING_PARSE_H
#define LAUNCHER_SETTING_PARSE_H

#include <strings.h>

/*
 * Parses an on/off value from SETTINGS.TXT. The English words, the localised word and a leading
 * 1 or 0 are accepted, case-insensitively. The two shapes come from the original readers: with the
 * default off only an on-like value turns the setting on, with the default on only an off-like value
 * turns it off. *migrate is set when the text is not exactly "On" or "Off", so the file is rewritten
 * in canonical form. The localised words are arguments because they depend on the language at run time.
 */
static __inline__ unsigned Launcher_ParseOnOff(const char *text, const char *on_text,
	const char *off_text, unsigned default_on, unsigned *migrate)
{
	unsigned on_like = !strcasecmp(text, "On") || !strcasecmp(text, "Yes") ||
		!strcasecmp(text, on_text) || text[0] == '1';
	unsigned off_like = !strcasecmp(text, "Off") || !strcasecmp(text, "No") ||
		!strcasecmp(text, off_text) || text[0] == '0';

	if(strcasecmp(text, "On") && strcasecmp(text, "Off"))
		*migrate = 1;
	return default_on ? !off_like : on_like;
}

#endif
