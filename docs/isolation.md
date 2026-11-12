# Shared layout and hardware isolation

All four branches (`dssmm`, `ezfomegade`, `ezfomega`, `ezfblue`) track the same
combined repository layout. There are three independent projects at root:

| Directory | Hardware | Project-local verifier |
| --- | --- | --- |
| ezfomegade/ | Definitive Edition Rev. A/B | scripts/verify-ezfomegade.sh |
| ezfomega/ | Original OMEGA | scripts/verify-omega-port.sh |
| ezfblue/ | Newer Definitive Edition B / BLUE | scripts/verify-blue-port.sh |

Source, assets, hardware protocols, settings, tools, tests and platform journals
belong to their own project. Root holds only universal infrastructure and shared
records. BLUE retains its private vendor targets/blue linker/resource subproject.
All projects remain visible in GitHub's tree on every branch. A hardware branch
is a development lane; it does not delete the other projects from Git.

## Focus a working tree

In the hardware branch's own worktree:

```sh
git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomega
make ezfomega
make verify-ezfomega
```

Replace ezfomega with the appropriate project. Sparse checkout is local to the
worktree and leaves all hidden files tracked. Root files stay visible; common
CI, environment, scripts, tests and docs are included explicitly. Use
`git sparse-checkout disable` to restore all three projects. The full dssmm
workspace remains unchanged when another worktree is focused.

`make build-all` and `make verify-all` operate on projects present in that working
view; complete checkouts select all three. Layout checks distinguish real sparse
index entries from accidentally deleted projects. Builds never borrow another
project's source or assets. Pages composition requires a complete checkout.

## Merging

Develop inside the hardware branch's own directory. Submit its normal PR into
dssmm or merge it locally with ordinary Git merge. The PR scope check compares
changes since the merge base, includes deletions and both sides of renames, and
rejects hardware PRs that touch another project or root infrastructure. Feature
PRs into a hardware lane have the same directory restriction; direct hardware
pushes are checked for their unintegrated changes relative to origin/dssmm. dssmm PRs
verify all three hardware projects. Merge dssmm back into hardware branches to
synchronize the integrated tree; those integration syncs are allowed.

Shared root changes belong on dssmm or a shared feature branch. Keep routine
platform progress in that project's HANDOFF/PROGRESS files, not the root logs.
This prevents routine project edits from competing for the same shared files.
Direct hardware-to-other-hardware PRs are rejected; integrate through dssmm.
No workflow automatically pushes, merges or publishes a kernel. CI has read-only
contents permission and does not persist checkout credentials. Pages deployment
is manual and restricted to dssmm.

The common tracked baseline and ancestry remove the old layout conflicts: an
ordinary target-only merge preserves other project directories, including in a
sparse checkout. Same-file edits can still conflict, and arbitrary deletion or
cross-platform driver changes are not made safe by directory names. Required
server-side enforcement still depends on repository branch protection settings.

## Transition and evidence

Before conversion, nonjournal project files on the hardware branches were
verified equal to the combined versions. Exact prior branch root/project logs,
instructions, manifests and variant CI records are preserved in each project's
`docs/history/pre-shared-tree/`. The old registry is retained in root
`docs/history/pre-shared-tree/projects.json`. A one-time ancestry bridge records
those already-imported historical heads; it is not an ongoing merge policy.
Future merges use normal Git behavior, without ours/theirs merge drivers.

The common live projects.json registry records kernel-input baselines rather
than freezing project documentation. `python3 scripts/check-snapshots.py` compares
those source/assets/build/test inputs. Live project docs and journals can evolve
normally without requiring an unrelated registry edit for each hardware change.
Earlier snapshot/CI/binary reports remain historical evidence; current transition
results are recorded in HANDOFF/PROGRESS and the shared-tree verification report.

Run `python3 tests/test-project-isolation.py` and
`python3 tests/test-project-merges.py`. The latter uses real repositories and
worktree-local sparse checkout, performs ordinary independent hardware merges
and reverse integration syncs, checks other project trees remain intact and
rejects a cross-project rename. Actual-repository merge rehearsals are recorded
separately from these small fixtures.

Published releases/tags/assets stay unchanged. DE8.3.2 remains stable/latest;
Original OMEGA Test3 and BLUE Test2 remain hardware-untested. Existing flash/SD
atomicity, chip timing, parser and runtime-stack limits are in the private audit
reports. Repository architecture does not add physical hardware qualification.
