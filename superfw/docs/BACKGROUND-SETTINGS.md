# Cooperative settings saves

Leaving settings queues a save. `menu_render` performs one bounded transaction
step per frame; keys and rendering run between steps. Changes made during an
active save queue a second snapshot. Unchanged snapshots do not write the card.
Game/plugin handoff drains pending saves before cartridge memory can be remapped.

The existing 8 KiB settings buffer owns the immutable output and the aligned
FatFs file object in its unused tail. Snapshot construction uses temporary stack
storage; no second permanent buffer or cartridge byte-write storage is added.
Near-capacity files without room for the file object use the synchronous fallback.
The original file is preserved until replacement, then the result is verified by
size/hash. Failure rolls back; interrupted replacement is recovered on next boot.
Unknown settings and canonical `SYSTEM/SETTINGS.TXT` behavior are unchanged.

This is cooperative work on a single CPU, not a second thread: individual card
operations and snapshot reads can still briefly delay a frame. Do not power off
while saving. Native ARM replay measures stack against linked live data; mGBA
checks navigation while pending and persistence across processes. Neither models
physical card latency, power-loss timing or SD/Lite hardware behavior.
