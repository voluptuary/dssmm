#ifndef OMEGA_NOR_ARTWORK_SD_H
#define OMEGA_NOR_ARTWORK_SD_H
#include <stdint.h>
void Omega_SaveNorArtworkPath(uint32_t slot, const char *full_source_path);
const char *Omega_ReadNorArtworkPath(uint32_t slot);
#endif
