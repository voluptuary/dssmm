# Current port priorities

Owner hardware report: Chis Build585 fixes header Title artwork and the
five-image lookahead solves the reported adjacent-art loading problem. This
qualifies those observations on that card, not all gameplay/save/card variants.

1. **CUSTOM directory artwork (implemented; hardware test pending).** For a root folder `Game Boy Advance`, Title uses
   `/SYSTEM/IMGS/CUSTOM/Game Boy Advance.bmp` and Box uses
   `/SYSTEM/IMGS2/CUSTOM/Game Boy Advance.bmp`. Match the complete directory name,
   including dots. Apply it to selected and adjacent carousel art and List+Art
   preview. Missing/invalid images keep the normal folder icon; parent entries
   must not trigger ROM-header reads. Test both sizes and DST1 transparency.
2. **Optional DE DSSMM font (implemented; hardware test pending).** Audit/credit the actual DE system-font assets and
   adapt them to SuperFW's existing halfword-safe renderer. Start with a saved
   Native/DSSMM choice, preserving native Unicode fallback and in-game behavior.
   Keep measurement, clipping and scrolling metrics consistent. Validate all
   three memory layouts before distributing a candidate; retain native default
   while the option is tested. Record any initial coverage/layout limitations.
3. **Further performance/parity.** Directory-open indexing is an investigation,
   not an implemented feature. Revisit after the two requested priorities;
   preserve native filesystem/card backends and test FAT/exFAT before changing
   lookup internals. Continue other UI surfaces without requiring owner input.

Keep numbered hardware-test builds, build manifests, tests, journals and cleanup
current. Keep the latest candidate frozen while subsequent work proceeds.
