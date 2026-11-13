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
static bool artwork_header_path(char *out, size_t capacity, const char *rom,
                                unsigned style, unsigned candidate,
                                const char *title, size_t title_len) {
  if (!out || !rom || !title || !title_len || title_len > 12 || style > 1) return false;
  /* Keep the ROM's mirrored directory (including letter folders such as P). */
  const char *slash = strrchr(rom, '/');
  if (!slash) return false;
  char stem[13];
  size_t n = 0;
  while (title_len && (title[title_len - 1] == ' ' || title[title_len - 1] == '\0')) --title_len;
  while (n < title_len && (title[n] == ' ' || title[n] == '\0')) ++n;
  size_t start = n;
  for (; n < title_len; ++n) {
    unsigned char c = (unsigned char)title[n];
    if (c < 32 || c > 126 || c == '/' || c == '\\' || c == ':' || c == '*') return false;
  }
  size_t stem_len = title_len - start;
  if (!stem_len || (stem_len == 1 && title[start] == '.') ||
      (stem_len == 2 && title[start] == '.' && title[start + 1] == '.')) return false;
  memcpy(stem, title + start, stem_len); stem[stem_len] = 0;
  const char *prefix = style ? "/SYSTEM/IMGS2" : "/SYSTEM/IMGS";
  size_t pn = strlen(prefix), dn = slash - rom - (slash != rom), cn = candidate ? 7 : 0;
  size_t need = pn + 1 + cn + dn + (dn ? 1 : 0) + stem_len + 5;
  if (need > capacity) return false;
  memcpy(out, prefix, pn);
  size_t pos = pn;
  out[pos++] = '/';
  if (candidate) { memcpy(out + pos, "CUSTOM/", 7); pos += 7; }
  else if (dn) { memcpy(out + pos, rom + 1, dn); pos += dn; out[pos++] = '/'; }
  memcpy(out + pos, stem, stem_len); pos += stem_len;
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
  /* EZ-FLASH GBA title art can use the 12-byte internal ROM title. */
  if (rom && style == 0) {
    const char *dot = strrchr(rom, '.');
    if (dot && strlen(dot) == 4 && dot[0] == '.' && (dot[1] == 'g' || dot[1] == 'G') &&
        (dot[2] == 'b' || dot[2] == 'B') && (dot[3] == 'a' || dot[3] == 'A') && !dot[4]) {
      FIL source;
      if (f_open(&source, rom, FA_READ) == FR_OK) {
        char title[12]; UINT got = 0;
        bool read_ok = f_lseek(&source, 0xA0) == FR_OK &&
                       f_read(&source, title, sizeof(title), &got) == FR_OK && got == sizeof(title);
        (void)f_close(&source);
        if (read_ok) {
          for (int candidate = 1; candidate >= 0; --candidate) {
            if (!artwork_header_path(path, sizeof(path), rom, style, candidate, title, sizeof(title))) continue;
            FIL file;
            if (f_open(&file, path, FA_READ) != FR_OK) continue;
            bool ok = f_size(&file) <= UINT32_MAX && artwork_decode(read_exact, &file, (uint32_t)f_size(&file), pixels, capacity, info);
            FRESULT closed = f_close(&file);
            if (ok && closed == FR_OK && info->width == (style ? 80 : 120)) return true;
          }
        }
      }
    }
  }
  return false;
}
