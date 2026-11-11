#ifndef FATFS_OEM_PATH_H
#define FATFS_OEM_PATH_H

#include <stddef.h>
#ifndef FF_CODE_PAGE
#include "ff15/ffconf.h"
#endif

/* FatFs OEM paths use CP936 here: a valid character can end in byte0x5C.
   Keep component checks and basename extraction consistent with ff.c. */
static inline const char *FatFsPathLastSeparator(const char *path)
{
    const unsigned char *cursor = (const unsigned char *)path;
    const char *last = NULL;
    if(!cursor) return NULL;
    for(; *cursor; cursor++) {
#if FF_CODE_PAGE == 936
        if(*cursor >= 0x81 && *cursor <= 0xFE &&
           cursor[1] >= 0x40 && cursor[1] <= 0xFE && cursor[1] != 0x7F) {
            cursor++;
            continue;
        }
#endif
        if(*cursor == '/' || *cursor == '\\') last = (const char *)cursor;
    }
    return last;
}
#endif
