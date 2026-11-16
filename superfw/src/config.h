/*
 * Copyright (C) 2026 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _CONFIG_H__
#define _CONFIG_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Config config maximum values here and there
#define MAX_FN_LEN                 256
#define FLASHG_MAXFN_CNT           32            // No more than 32 games in NOR
#define BROWSER_MAXFN_CNT     (16*1024)
#define RECENT_MAXFN_CNT          (200)

#define MAX_BACKUP_CNT             16

#define SYSTEM_DIR                "/SYSTEM"
#define SAVER_DIR                 "/SYSTEM/SAVER/"
#define LEGACY_SYSTEM_SAVEGAME_DIR "/SYSTEM/SAVEGAME/"
#define SAVES_DIR                 "/SYSTEM/SAVES/"
#define SAVESTATE_DIR             "/SYSTEM/SAVESTATE/"
#define LEGACY_SAVEGAME_DIR       "/SAVEGAME/"
#define LEGACY_SAVES_DIR          "/SAVES/"
#define LEGACY_SAVESTATE_DIR      "/SAVESTATE/"
#define SUPERFW_LOG_FILEPATH      "/SYSTEM/superfwlog.txt"
#define DISK_SAVESTATE_SLOTS      5

#define SUPERFW_DIR               "/SYSTEM"
#define ROMCONFIG_PATH            "/SYSTEM/config/"
#define PATCHDB_PATH              "/SYSTEM/patches/"
#define CHEATS_PATH               "/SYSTEM/cheats/"
#define EMULATORS_PATH            "/SYSTEM/PLUG/"
#define GBC_EMULATOR_PATH         "/SYSTEM/PLUG/gbc-emu.gba"
#define SETTINGS_FILEPATH         "/SYSTEM/SETTINGS.TXT"
#define LEGACY_RECENT_FILEPATH   "/SYSTEM/SUPERFW-RECENT.TXT"
#define RECENT_FILEPATH           "/SYSTEM/RECENT.TXT"
#define FLASHBACKUPTMP_FILEPATH   "/SYSTEM/flash_backup.tmp"
#define FLASHBACKUP_FILEPTRN      "/SYSTEM/flash_backup-%02x%02x%02x%02x.bin"

#define PENDING_SAVE_FILEPATH     "/SYSTEM/pending-save.txt"
#define PENDING_SRAM_TEST         "/SYSTEM/pending-sram-test.txt"

#endif

