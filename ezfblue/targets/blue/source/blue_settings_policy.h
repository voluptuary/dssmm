#ifndef BLUE_SETTINGS_POLICY_H
#define BLUE_SETTINGS_POLICY_H
/* Separate stock Mode B from the launcher preference. Comparison/design credit:
   FrankieT19 BLUE Test4, bc4418c. Independently implemented for DSSMM. */
#define BLUE_BOOT_PREF_TAG 0x4450u
static inline unsigned BlueBootPreference(unsigned value, unsigned legacy)
{
    unsigned choice = (value & 0xfffcu) == BLUE_BOOT_PREF_TAG ? value & 3u : legacy >> 8;
    return choice <= 2 ? choice : 0;
}
#endif
