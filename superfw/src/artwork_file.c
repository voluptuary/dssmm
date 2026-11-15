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
static bool artwork_gamecode_root_path(char *out, size_t capacity, const char gamecode[4]) {
  static const char prefix[] = "/SYSTEM/IMGS/";
  if (!out || !gamecode || capacity < sizeof(prefix) + 8) return false;
  for (unsigned i = 0; i < 4; ++i) {
    unsigned char c = (unsigned char)gamecode[i];
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) return false;
  }
  memcpy(out, prefix, sizeof(prefix) - 1);
  memcpy(out + sizeof(prefix) - 1, gamecode, 4);
  memcpy(out + sizeof(prefix) + 3, ".bmp", 5);
  return true;
}
typedef struct {
  const uint8_t *bytes;
  bool legacy;
} artwork_buffer;
static bool read_buffer(void *context, uint32_t offset, void *data, size_t size) {
  const artwork_buffer *input = context;
  const uint8_t *buffer = input->bytes;
  if (offset > ARTWORK_BMP_MAX_SIZE || size > ARTWORK_BMP_MAX_SIZE - offset) return false;
  memcpy(data, buffer + offset, size);
  /* The original EZ loader ignores header metadata and copies the 54-byte
   * prefix plus 120x80 GBA pixels. Some legacy packs use desktop BMP metadata
   * that does not describe that payload. Normalize only that exact code-art
   * record, in internal RAM; never byte-write the cartridge staging buffer. */
  if (input->legacy && offset == 0 && size == 54) {
    static const uint8_t header[54] = {
      [0]='B',[1]='M',[2]=0x36,[3]=0x4b,[10]=54,[14]=40,
      [18]=120,[22]=0xb0,[23]=0xff,[24]=0xff,[25]=0xff,
      [26]=1,[28]=16,[34]=0x00,[35]=0x4b
    };
    memcpy(data, header, sizeof(header));
  }
  return true;
}
static bool artwork_decode_file(FIL *file, uint8_t *bmp, size_t bmp_capacity,
                                uint8_t *pixels, size_t capacity, artwork_info *info, bool gamecode) {
  FSIZE_t file_size = f_size(file);
  UINT got = 0;
  if (!bmp || bmp_capacity > ARTWORK_BMP_MAX_SIZE || file_size > bmp_capacity ||
      file_size > UINT32_MAX || file_size > UINT_MAX) return false;
  if (f_tell(file) != 0 && f_lseek(file, 0) != FR_OK) return false;
  if (f_read(file, bmp, (UINT)file_size, &got) != FR_OK || got != file_size) return false;
  artwork_buffer input = {bmp, false};
  if (artwork_decode(read_buffer, &input, (uint32_t)file_size, pixels, capacity, info)) return true;
  /* Match native EZ fixed-record semantics only for game-code fallbacks.
   * Filename/DSSCC art keeps strict validation and its own transparency. */
  if (!gamecode || file_size != 54+120*80*2 || bmp[0] != 'B' || bmp[1] != 'M') return false;
  input.legacy = true;
  return artwork_decode(read_buffer, &input, (uint32_t)file_size, pixels, capacity, info);
}
bool artwork_load(const char *rom, unsigned style, uint8_t *pixels, size_t capacity,
                  uint8_t *bmp, size_t bmp_capacity, artwork_info *info) {
  char path[288];
  if (!artwork_path(path, sizeof(path), rom, style, 0)) return false;
  for (int candidate = 1; candidate >= 0; --candidate) {
    if (!artwork_path(path, sizeof(path), rom, style, candidate)) continue;
    FIL file;
    if (f_open(&file, path, FA_READ) != FR_OK) continue;
    bool ok = artwork_decode_file(&file, bmp, bmp_capacity, pixels, capacity, info, false);
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
          char code_paths[2][288];
          bool have_paths[2] = {
            artwork_gamecode_path(code_paths[0], sizeof(code_paths[0]), gamecode),
            artwork_gamecode_root_path(code_paths[1], sizeof(code_paths[1]), gamecode)
          };
          for (unsigned candidate = 0; candidate < 2; ++candidate) {
            if (!have_paths[candidate]) continue;
            memcpy(path, code_paths[candidate], strlen(code_paths[candidate]) + 1);
            FIL file;
            if (f_open(&file, path, FA_READ) == FR_OK) {
              bool ok = artwork_decode_file(&file, bmp, bmp_capacity, pixels, capacity, info, true);
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
