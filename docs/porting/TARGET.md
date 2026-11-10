# original EZ-FLASH OMEGA exploration

Branch: `ezfomega`

Framework baseline: `691dc5ec0391cd30c537082a8428978b29c5500a`

Reference: https://github.com/FrankieT19/omega-ds-style-kernel

Pinned commit: `ef30b8ba78147943d95653dfa82f721b52096f67`

Status: active Original Omega port, authorized by owner. Executable source/assets/Makefile now use the pinned Original Omega reference with selective Merkin Mix changes. No hardware validation or release yet. See OMEGA_STATUS.md.

## First investigation when resumed

Start with FrankieT19’s existing Omega adaptation: identify features already ported, then compare cartridge operations, save/export timing, reset, PSRAM geometry and memory reservations against Merkin Mix. Decide whether to use the Omega implementation as the eventual hardware baseline after the comparison.

Follow [the shared preparation checklist](README.md). Produce a feature/provenance matrix and hardware/build inventory before changing code. Owner resumed implementation; keep all changes on ezfomega. Never merge/push these changes into ezfode.
