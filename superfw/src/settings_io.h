/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_SETTINGS_IO_H
#define DSSMM_SETTINGS_IO_H
#include <stdbool.h>
typedef void (*settings_line_fn)(const char *,const char *);
typedef bool (*settings_owned_fn)(const char *);
bool settings_read_text(const char *path,settings_line_fn callback,bool *exists);
bool settings_write_text(const char *shared,const char *private_text,settings_owned_fn owned);
#endif
