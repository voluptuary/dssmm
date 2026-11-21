# Review of the owner-supplied audit patches

The owner’s `PATCHES.md` and twelve patches (archived intact under
`docs/history/patch-review-2026-11-21/` at repository root) are proposals, not verified corrections.
No patch was applied verbatim. The report does not establish a completed audit or
elimination of technical debt; several referenced functions do not exist.

| Patch | Decision | Reason |
| --- | --- | --- |
| 001 | Leave unchanged | OMEGA comments/debug cleanup, outside SuperFW ownership; no demonstrated functional correction. |
| 002 | Reject | Replacing constant multiplication with shifts offers no demonstrated compiler or timing benefit. |
| 003 | Reject | The proposed legacy helper is absent and the diff is not a usable source change. |
| 004 | Leave unchanged | Cosmetic OMEGA changes outside this lane. |
| 005–008 | Reimplement behavior | Valid SuperFW requests, but proposals target OMEGA files and invented APIs. Use the actual header, settings transaction and clock renderer. |
| 009 | Reimplement with tests | Wrap needs navigation, animation and five-slot cache changes together; index modulo five collides across wrapped boundaries. |
| 010 | Presentation only | Reuse native SuperFW NOR operations/guards. Port suitable display/management behavior, never EZ-FLASH drivers. |
| 011 | Reject patch; retain useful aim | Placeholder APIs/statuses are absent. Existing five-image cache already caches misses and defers reads during motion. Measure before changing I/O policy. |
| 012 | Reject | It removes proof that the ARM store targets WAITCNT and can patch unrelated code. The extra LZ condition adds no protection to the validated parser invariant. |

The interrupted plugin work remains separate: exact recognized reset adaptation,
compressed-plugin support and SuperCard firmware restoration require actual ARM
and mGBA tests. No physical exit compatibility is claimed from those fixtures.
EZ-FLASH native-renderer mGBA harnesses are an additional owner request; implement
each in its owning hardware lane and compare screenshots without mixing drivers.
