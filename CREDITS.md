# Credits and provenance

## Original Omega development branch

The ezfomega backport builds on FrankieT19’s Original Omega DS Style v8.0 adaptation (ef30b8b), which builds on EZ-FLASH and Sterophonick’s work. Merkin Mix artwork and safety improvements are being selectively adapted for this hardware. See docs/porting/OMEGA_STATUS.md for provenance and validation status.

- **EZ-FLASH** — original OMEGA Definitive Edition kernel source.
- **Sterophonick** — SimpleLight custom kernel, part of DS Style's foundation.
- **[FrankieT19](https://github.com/FrankieT19/omega-de-ds-style-kernel)** — DS Style v7.5 foundation ([commit](https://github.com/FrankieT19/omega-de-ds-style-kernel/commit/df346b5ee173ab47bb0c40305a682ce4f7f190e1)), including animated carousels and the launch pulse. His separately authored [v8.0 commit](https://github.com/FrankieT19/omega-de-ds-style-kernel/commit/6d6084e01d9410ddaae8ac69bb93a245c5edac28) supplied the Start-screen/UI refinements, Auto theme, View grouping, Open SYSTEM, wrap defaults, Boot to, double-tap favourites and save-presentation artwork integrated into Merkin Mix 8.3.0.
- **Copilot** — co-authored portions of the 8.3.0 UI integration, regression tests and release documentation.
- **[Merkin / voluptuary](https://github.com/voluptuary/ezfode-dssmm-kernel)** — Merkin Mix maintenance, refreshed splash screens and system icons in the v8.0 lineage, artwork and branding, mirrored/transparent thumbnail integration, audit fixes and kernel improvements. Changes and fixes have been developed with AI assistance, as recorded in the changelog.
- **[exploitz86](https://github.com/exploitz86/omega-de-ds-style-kernel)** — source fork for the remembered per-game cheat selection integration. [Enhanced r1](https://github.com/exploitz86/omega-de-ds-style-kernel/commit/98063b4a70ad3a1eed975bfe39e4c6e88662418a) supplies optional cheat-state persistence, headless loading, ten-row clearing and viewport clamping after collapsing groups; [fc6348b](https://github.com/exploitz86/omega-de-ds-style-kernel/commit/fc6348b000a94ef782910cf389c75b9fb94169d7) localizes the Remember cheats setting.
- **[SkillerCMP](https://github.com/SkillerCMP/omega-de-ds-style-kernel)** — related cheat-engine, RTS and patch-system contributions in the kernel lineage used by exploitz86. Skiller-authored commits include [expanded code support](https://github.com/SkillerCMP/omega-de-ds-style-kernel/commit/d3f078e19d0517db62ddd9859737bb3f0abacb3b) and [v7.4C save-state/patch-system work](https://github.com/SkillerCMP/omega-de-ds-style-kernel/commit/2ad491325bedc6fdd48eeb4192d3a1b22c0124d6). exploitz86's [merge of SkillerCMP's branch](https://github.com/exploitz86/omega-de-ds-style-kernel/commit/919a1a7434896cbca9b28a728374590f8ffc323e) records that ancestry.
- **aidiadayo** — Thai language support.
- **Kekun** — case-sensitive `HZK12.h` include fix.
- **ChaN** — FatFs, as used by the upstream kernel; its source retains its own notices.

The remembered-cheat and collapsed-group integration was checked against exploitz86 main at `88e5ab137f72bdcc61be11cc0bd6b54b50cabdb4`. The collapsible-menu foundation appears in Skiller-authored `d3f078e19d0517db62ddd9859737bb3f0abacb3b`; the stale-row and viewport fixes are specifically exploitz86’s `98063b4a70ad3a1eed975bfe39e4c6e88662418a`.

Merkin Mix adapts that work to its existing parser and one-option-per-section behavior. Its bounded, full-path/content-validated `CHS2` snapshot replaces the source fork’s `CHS1` name-based format; these formats are not interchangeable. The motion guard/reuse corrections are local fork changes. The experimental local silhouette border was rolled back after owner-reported hardware slowdown; 8.2.0 retains the prior rectangular/rounded border. Crediting the source forks does not imply import of their entire cheat engine, RTS format or external patch system. The full Skiller engine remains a separate read-only assessment, outside this release integration.

[DS Style Card Companion (`voluptuary/dsscc`)](https://github.com/voluptuary/dsscc) is a separate companion application, forked from FrankieT19's DS Style Thumbnail Scraper. Its `feature/mirrored-artwork-retry-cache` branch contains Merkin's companion changes. It is not the source of the kernel cheat work.

This document supplements existing source notices; it does not replace component licenses or claim authorship of unimported changes.
