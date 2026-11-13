/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "artwork_file.h"
#include "fatfs/ff.h"
#include <string.h>
#include <limits.h>
/* No truncated candidate opens, path traversal or directory scans. */
bool artwork_path(char *out, size_t capacity, const char *rom, unsigned style, unsigned candidate) {
  if (!out || !capacity || !rom || rom[0] != '/' || !rom[1] || style > 1 || candidate > 1) return false;
  const char *base = rom + 1, *component = base;
  for (const char *p = base;; ++p) {
    if (*p == '\\' || *p == ':') return false;
    if (*p == '/' || !*p) {
      size_t n = p - component;
      if (!n || (n == 1 && component[0] == '.') ||
          (n == 2 && component[0] == '.' && component[1] == '.')) return false;
      if (!*p) break;
      component = p + 1;
    }
  }
  const char *dot = strrchr(component, '.');
  /* File previews only; folders and extensionless entries have no candidate. */
  if (!dot || dot == component || !dot[1]) return false;
  const char *name = candidate ? component : base;
  const char *prefix = style ? "/SYSTEM/IMGS2/" : "/SYSTEM/IMGS/";
  size_t pn = strlen(prefix), nn = dot - name, cn = candidate ? 7 : 0;
  if (pn + cn + nn + 5 > capacity) return false;
  memcpy(out, prefix, pn);
  if (candidate) memcpy(out + pn, "CUSTOM/", cn);
  memcpy(out + pn + cn, name, nn);
  memcpy(out + pn + cn + nn, ".bmp", 5);
  return true;
}
static bool artwork_gamecode_path(char *out, size_t capacity, const char gamecode[4]) {
  static const char prefix[] = "/SYSTEM/IMGS/";
  if (!out || !gamecode || capacity < sizeof(prefix) + 12) return false;
  for (unsigned i = 0; i < 4; ++i) {
    unsigned char c = (unsigned char)gamecode[i];
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) return false;
  }
  size_t pos = sizeof(prefix) - 1;
  memcpy(out, prefix, pos);
  out[pos++] = gamecode[0]; out[pos++] = '/';
  out[pos++] = gamecode[1]; out[pos++] = '/';
  memcpy(out + pos, gamecode, 4); pos += 4;
  memcpy(out + pos, ".bmp", 5);
  return true;
}
static bool read_exact(void *context, uint32_t offset, void *data, size_t size) {
  FIL *file = context;
  UINT got = 0;
  return size <= UINT_MAX && f_lseek(file, offset) == FR_OK &&
         f_read(file, data, (UINT)size, &got) == FR_OK && got == size;
}
bool artwork_load(const char *rom, unsigned style, uint8_t *pixels, size_t capacity, artwork_info *info) {
  char path[288];
  if (!artwork_path(path, sizeof(path), rom, style, 0)) return false;
  for (int candidate = 1; candidate >= 0; --candidate) {
    if (!artwork_path(path, sizeof(path), rom, style, candidate)) continue;
    FIL file;
    if (f_open(&file, path, FA_READ) != FR_OK) continue;
    bool ok = f_size(&file) <= UINT32_MAX &&
      artwork_decode(read_exact, &file, (uint32_t)f_size(&file), pixels, capacity, info);
    FRESULT closed = f_close(&file);
    if (ok && closed == FR_OK && info->width == (style ? 80 : 120)) return true;
  }
  /* EZ-FLASH title artwork is indexed by the four-byte GBA game code. */
  if (rom && style == 0) {
    const char *dot = strrchr(rom, '.');
    if (dot && strlen(dot) == 4 && dot[0] == '.' && (dot[1] == 'g' || dot[1] == 'G') &&
        (dot[2] == 'b' || dot[2] == 'B') && (dot[3] == 'a' || dot[3] == 'A') && !dot[4]) {
      FIL source;
      if (f_open(&source, rom, FA_READ) == FR_OK) {
        char gamecode[4]; UINT got = 0;
        bool read_ok = f_lseek(&source, 0xAC) == FR_OK &&
                       f_read(&source, gamecode, sizeof(gamecode), &got) == FR_OK && got == sizeof(gamecode);
        (void)f_close(&source);
        if (read_ok) {
          if (artwork_gamecode_path(path, sizeof(path), gamecode)) {
            FIL file;
            if (f_open(&file, path, FA_READ) == FR_OK) {
              bool ok = f_size(&file) <= UINT32_MAX && artwork_decode(read_exact, &file, (uint32_t)f_size(&file), pixels, capacity, info);
              FRESULT closed = f_close(&file);
              if (ok && closed == FR_OK && info->width == 120) return true;
            }
          }
        }
      }
    }
  }
  return false;
}
