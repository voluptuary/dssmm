# Current source and reproducible builds

A normal clone/fork contains all four kernel projects, build tools and native
mGBA fixture sources. Use the checked-in devcontainer; its feature lock and
Dockerfile pin the shared environment. Read [local-development.md](local-development.md)
for setup. No private BIOS, session scratch, ignored binaries or sibling
worktrees are required for builds. Release artwork inputs are separate from
kernel compilation.

```sh
make build-all
make superfw BOARD=lite
make superfw BOARD=chis
```

`build-all` builds the three OMEGA-family projects and SuperFW SD. The extra two
commands build SuperFW Lite and Chis. Outputs stay within the owning project.
Build numbers on current source follow the complete Git history; use a full
clone, not a shallow checkout, when reproducing SuperFW's recorded build number.

| Project | Validated checkpoint | Evidence |
| --- | --- | --- |
| DE Rev.A/B | 8.4.1, source `a948b936` | `ezfomegade/docs/` release/native-launch records |
| Original OMEGA | 8.4.1 Test5, source `29bbb055` | `ezfomega/docs/` release/native-launch records; hardware untested |
| BLUE | 8.4.1 Test5 release, source `9bc24e04` | `ezfblue/docs/releases/blue-test5-owner-relabel.json`; embedded8.4.2 preserved; hardware untested |
| SuperFW SD/Lite/Chis | Build822 public, source `2f73b53b` | `superfw/docs/build-822-validation.json`; Build827 is the newer local software checkpoint, not published |

To reproduce a checkpoint's exact build stamp, make a detached worktree at the
full source commit recorded in its JSON report and run the matching project
build there. Current default-branch builds include later documentation/harness
fixes and therefore can report a newer SuperFW build number. Baselines in
`projects.json` are release input references, not a requirement to revert current
source. Hardware-specific protocols remain isolated in their project folders.

Atari plugin development now lives at [a26.gba](https://github.com/voluptuary/a26.gba).
Historical DSSMM recipes remain in SuperFW; the owner authorized removing the
obsolete ZIP. They are not the latest emulator. The optional newer card-bus harness is not a
replacement for DSSMM's native launch/render tests or physical qualification.
