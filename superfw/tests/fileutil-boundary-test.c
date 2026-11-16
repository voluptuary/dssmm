/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "config.h"
#include "fatfs/ff.h"
#include "util.h"
static unsigned calls;
FRESULT f_mkdir(const TCHAR *path) {assert(strlen(path)<MAX_FN_LEN);++calls;return FR_OK;}
int main(void) {
 char name[MAX_FN_LEN+16];
 strcpy(name,"/SYSTEM/");memset(name+8,'a',MAX_FN_LEN-11);strcpy(name+MAX_FN_LEN-3,"/x");
 assert(strlen(name)==MAX_FN_LEN-1);create_basepath(name);assert(calls==2);
 calls=0;memset(name,'a',sizeof(name)-1);name[sizeof(name)-1]=0;create_basepath(name);assert(!calls);
 create_basepath(0);create_basepath("");assert(!calls);
 create_basepath("/game.sav");assert(!calls);
 return 0;
}
