/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_SYSTEM_LAYOUT_H
#define DSSMM_SYSTEM_LAYOUT_H
#include "fatfs/ff.h"
/* Move legacy resources without replacing files. Interrupted runs resume. */
FRESULT system_layout_migrate(void);
#endif
