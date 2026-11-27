# Harness Improvement Report: Adapting `voluptuary/pogo.gba` Patterns for DSSMM

*Author: Gemini CLI Agent*
*Target Audience: Successor Engineering Agent / Implementer*
*Reference Source:* <https://github.com/voluptuary/pogo.gba>

## 2026-11-27 source check

The current `pogo.gba` harness at `3e3ac393e6f0ed6edd4de933a57560958cd1daa7`
uses C card models and mGBA memory callbacks. Its six-profile runner restores
the same pristine SD image before each card profile and records separate
results. The Lua implementation described below is a proposal from the
original investigation, not a feature found in that source. DSSMM's
`scripts/verify-shared-sd-image.sh` adapts the shared-image workflow using
the upstream runner executables. See `docs/shared-sd-harness.md`.

---

## 1. Executive Summary & Purpose

This report documents architectural findings and suggestions derived from the advanced test harness and simulation structure developed in `voluptuary/pogo.gba`. Its purpose is to provide a comprehensive reference for a successor engineering agent evaluating how to narrow the gap between virtual CI validation and real hardware execution across the DSSMM firmware family (`ezfomegade`, `ezfomega`, `ezfblue`, and `superfw`).

Per project instructions, **this document is strictly advisory**. The ultimate architectural design, prioritization, and implementation methodology remain entirely in the successor agent's hands.

---

## 2. Comparative Analysis: `pogo.gba` vs. DSSMM Harness Architecture

| Dimension | DSSMM Current Approach | `voluptuary/pogo.gba` Advanced Pattern |
| --- | --- | --- |
| **Execution Validation** | Host C unit tests, FatFs RAM-disk shims, and static linker layout guards. | Cycle-conscious simulation environment with direct integration into emulator loops. |
| **mGBA Automation** | SDL headless headless preview runners (`test-browser-preview-mgba.sh`) driven by shell/Xvfb and `xdotool` screen captures. | Event-driven Lua scripting inside mGBA for instant breakpoint detection and memory inspection. |
| **State Inspection** | Post-execution framebuffer hashes and log file parsing. | Programmatic in-memory assertions (checking active file handles, stack pointers, and register states during execution). |
| **Fault & Bus Simulation** | Static error return codes in host shims. | Dynamic fault injection mimicking card-bus timing constraints and transfer anomalies. |

---

## 3. Recommended Harness Improvements

The following architectural improvements from `pogo.gba` can be adapted to DSSMM's multi-project test framework without compromising kernel memory budgets:

### A. Lua-Driven mGBA Automation
- **Concept:** Replace or augment shell-scripted Xvfb/xdotool injection with mGBA's built-in Lua scripting engine.
- **Benefit:** Allows deterministic triggering of menu navigation, button holds, and screen transitions based on internal emulator hooks rather than brittle wall-clock delays.
- **Application:** Enhances `superfw/tests/` and project-level native preview runners.

### B. Cycle-Accurate Boot & Handoff Assertions
- **Concept:** Implement emulator-side validation hooks that assert CPU register states, IWRAM stack pointer alignment (`IWRAM SP`), and interrupt mask status at the exact instruction boundary prior to kernel-to-game or kernel-to-plugin handoffs.
- **Benefit:** Catches register clobbers and stack corruption bugs (such as the GCC Thumb-1 register allocation issues encountered in previous build cycles) during pre-commit virtual validation.

### C. Expanded Storage & Media Fault Injection
- **Concept:** Extend the existing FatFs host RAM-disk fixtures to dynamically simulate mid-transfer sector timeouts, CRC failures, and partial write aborts.
- **Benefit:** Validates transaction recovery paths and staging structures (like those in F-036 save/settings handlers) under hostile I/O conditions prior to hardware deployment.

---

## 4. Implementation Guidance for the Successor Agent

1. **Non-Invasive Integration:** Ensure any new harness components remain strictly outside production kernel compilation paths. Test fixtures, Lua scripts, and host runners should reside exclusively within `tests/` or owning project `tests/` directories.
2. **Hardware Reality Check:** Remember that virtual harnesses—even with advanced bus and script automation—cannot fully replace physical qualification (e.g., hardware owner Chis testing). Treat harness passes as necessary software gates, not definitive hardware proofs.
3. **Memory Budget Preservation:** Avoid adding heavy runtime profiling code into production kernel binaries where EWRAM/IWRAM margins are tight.
