# Boot resource compression

SD and Lite need compressed firmware/resources to fit their original flash
limits. Chis retains raw resources. All three retain the same supported menu
artwork and languages; board guards omit Chis-only NOR data/icons and extended
fonts on SD/Lite. Lite has no bundled emulators; SD bundles only GB/GBC.

The pending change packs SD/Lite fonts/backgrounds with the existing UPKR codec
at level4. A distinct `.pack.upkr` filename prevents incremental builds from
mistaking older apultra data for the new format. The decoded FO font prefix,
DBR2 backgrounds and ICN1 icon bytes remain identical. One cartridge-safe
halfword decoder handles compressed boot resources; Chis omits unused boot
decoders entirely. Main-menu rendering never decodes these compressed streams.

The native ARM replay checks the actual linked font callsite, each packaged
resource, exact output/guards, halfword stores and preserved source register.
It also verifies each board's bundled-media directory and unchanged boot limits.

Offline measurements for the734296-byte standard resource pack:

| Method | Packed bytes | ARM decoder instructions |
| --- | ---: | ---: |
| Previous apultra | 308252 | 10144522 |
| UPKR4, existing compact halfword decoder | 277868 | 71707618 |
| UPKR4, inline arithmetic/halfword decoder | 277868 | 52105327 |
| UPKR9 | 274047 | Not measured |

These are instruction counts, not GBA cycles or physical boot timing. UPKR
saves about30KiB but spends more CPU on SD/Lite startup. The inline decoder
reduces that extra work; carousel rendering is unaffected. Chis raw-font startup
does not incur this font-decoding cost. Level9 took over8minutes for another
3821bytes, so level4 remains the default. The initial SD prototype fits493056
of524288bytes (31232free); clean final builds must recheck the exact margin.

Byte-plane/delta/XOR preprocessing and smaller apultra windows produced larger
packs and were rejected. Run memory-heavy compressor experiments serially:
four parallel apultra processes caused three local benchmark OOM kills. No
firmware check was running then, and no source or owner data was lost.

Keep upstream decoder/tool sources and authorship. The old apultra tool remains
available for reproducible comparisons; production compressed resources use
UPKR. No BIOS, emulator patch or new dependency is required.
