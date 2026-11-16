/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/patchengine.c"
void memmove32(void *dst,void *src,unsigned bytes){memmove(dst,src,bytes);}
static void truncated_prefixes(void) {
 const uint32_t prefixes[]={SRAM_V_WORD0,EEPROM_V_WORD0,FLASH_V_WORD0,RTC_V_WORD0,
  eeprom_v1_read_word0,eeprom_v2_read_word0,eeprom_v4_write_word0,
  flash_v1_read_word0,flash_v23_read_word0,siirtc_probe_reset_sig_word0,
  siirtc_getstatus_sig_word0,siirtc_getdatetime_sig_word0,0};
 for(unsigned size=4;size<=64;++size)for(unsigned p=0;p<sizeof(prefixes)/sizeof(*prefixes);++p) {
  uint32_t *rom=malloc(size);assert(rom);memset(rom,0,size);rom[0]=prefixes[p];
  t_patch_builder builder;patchengine_init(&builder,size);
  assert(patchengine_process_rom(rom,size,&builder,0));free(rom);
 }
}
static void capacity_and_order(void) {
 t_patch_builder builder;patchengine_init(&builder,0);
 assert(push_rtc_handler(&builder.p,RTC_PROBE_HNDLR,12));
 assert(insert_patch(&builder.p,0,0x02000020,&builder.p.irqh_ops));
 assert((builder.p.op[1]>>28)==OPC_RTC_HD && builder.p.rtc_ops==1);
 for(unsigned i=2;i<MAX_PATCH_OPS;++i)assert(push_save_handler(&builder.p,OPC_EEPROM_HD,0,i*4));
 t_patch before=builder.p;assert(!push_save_handler(&builder.p,OPC_EEPROM_HD,0,0));assert(!memcmp(&before,&builder.p,sizeof(before)));
 /* Scanner propagation must reject an otherwise valid129th match. */
 uint32_t data[]={0xe59f0000,0,WAITCNT_VALUE_EXACT};
 assert(!patchengine_process_rom(data,sizeof(data),&builder,0));
}
static void windows_match_whole(void) {
 const unsigned size=16384,step=1024;
 uint8_t *rom=calloc(1,size);assert(rom);
 /* A signature straddles the scan boundary; an IRQ literal has lookbehind. */
 memcpy(rom+1000,siirtc_getdatetime_sig,sizeof(siirtc_getdatetime_sig));
 uint32_t instruction=0xe59f0010,constant=IRQHADDR_VALUE;
 memcpy(rom+2020,&instruction,4);memcpy(rom+2044,&constant,4);
 instruction=0xe59f0ff8;constant=WAITCNT_VALUE_EXACT;
 memcpy(rom+200,&instruction,4);memcpy(rom+4296,&constant,4);
 t_flash_setup_info_v2 setup={.program_byte_fnptr=0x08001001,.program_sector_fnptr=0x08002001,
  .erase_chip_fnptr=0x08003001,.erase_sector_fnptr=0x08004001,.wait_flash_write_fnptr=0x08005001,
  .timeout_lut_ptr=0x08006000,.flash_size=65536,.sector_size=4096,.shift_amount=12,.sector_count=16,.device_id=0x1cc2};
 memcpy(rom+3060,&setup,sizeof(setup));
 t_patch_builder whole,windowed;patchengine_init(&whole,size);patchengine_init(&windowed,size);
 assert(patchengine_process_rom((uint32_t*)rom,size,&whole,0));
 for(unsigned scan=0;scan<size;scan+=step) {
  unsigned start=scan>PATCH_SCAN_LOOKBEHIND?scan-PATCH_SCAN_LOOKBEHIND:0;
  unsigned end=MIN(size,scan+step+PATCH_SCAN_LOOKAHEAD);
  windowed.rom_offset=start;
  assert(patchengine_process_window((uint32_t*)(rom+start),end-start,scan-start,MIN(size,scan+step)-start,&windowed,0));
 }
 assert(whole.p.rtc_ops && whole.p.irqh_ops && whole.p.save_ops && whole.p.wcnt_ops);
 assert(!memcmp(&whole.p,&windowed.p,sizeof(whole.p)));
 assert(whole.save_type_guess==windowed.save_type_guess && whole.flash64cnt==windowed.flash64cnt);
 assert(whole.ldata==windowed.ldata && whole.ldatacnt==windowed.ldatacnt);
 free(rom);
}
int main(void){truncated_prefixes();capacity_and_order();windows_match_whole();puts("Patch scanner: exact allocation bounds, operation limits/group ordering and overlapping-window equivalence passed.");}
