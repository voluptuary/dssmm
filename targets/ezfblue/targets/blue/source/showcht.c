#include <gba_systemcalls.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <gba_base.h>
#include <gba_input.h>
#include <gba_dma.h>

#include "ezkernel.h"
#include "showcht.h"
#include "draw.h"


FM_CHT_LINE tmpCHTFS ;

u8 *pCHTbuffer = (u8*)(pReadCache + 0x2000); //patchbuffer


ST_entry pCHEAT[MAX_CHEAT_ENTRIES]EWRAM_BSS;
u32 gl_cheat_count;
u32 gl_cheat_selected_count;

#define CHEAT_SELECTION_CACHE_MAX 128
#define CHEAT_KEY_LEN (2 * MAX_path_len + 16)
#define CHT_GROUP_EXPANDED 0
#define CHT_GROUP_COLLAPSED 2
#define MAX_CHEAT_MENU_ENTRIES ((MAX_pReadCache_size - 0x2000) / sizeof(FM_CHT_LINE))
extern TCHAR currentpath[MAX_path_len];
extern u16 gl_cheat_remember;
static char cheat_selection_key[CHEAT_KEY_LEN] EWRAM_BSS;
static char cheat_active_key[CHEAT_KEY_LEN] EWRAM_BSS;
static u16 cheat_selection_index[CHEAT_SELECTION_CACHE_MAX] EWRAM_BSS;
static u32 cheat_selection_cache_count;

extern u16 gl_select_lang;

//u16 gl_color_chtBG    = RGB(4,8,0xC);
//------------------------------------------------------------------


extern FIL gfile;
extern void Show_error_num(u8 error_num);
char buf[MAX_BUF_LEN]EWRAM_BSS;
char _paramv[MAX_BUF_LEN] EWRAM_BSS;
extern void Draw_select_icon(u32 X,u32 Y,u32 mode);
extern void UIAudio_PlayMove(void);
extern void UIAudio_PlayAcceptExport(void);
extern void UIAudio_PlayBackExport(void);
extern void UIAudio_UpdateExport(void);
extern void UIAudio_WaitForCurrentClipExport(u32 max_frames);
extern void UIAudio_CutOffTrailingClipExport(void);
extern const char *Launcher_OnOffText(u16 value);
extern void Launcher_DrawCheatBackground(const char *title);
extern void Launcher_ClearCheatRegion(u16 x, u16 y, u16 w, u16 h);
extern void Launcher_DrawCheatCounter(u32 totalcount, u32 select);
extern void Launcher_MarkTopbarNameDirty(void);
extern void Launcher_UpdateCheatTitle(void);

static u32 cheat_use_chinese_folder = 0;

/* CHS2 intentionally does not import CHS1's truncated game identities.
 * A bounded snapshot is accepted only for the exact game and cheat contents.
 * Failed reads never delete files; failed writes cannot activate a partial file.
 */
#define CHEATSTATE_DIR "/SYSTEM/CHEATSTATE"
#define CHEATSTATE_MAGIC 0x32534843u
static u32 cheat_active_hash;
static u32 cheat_selection_hash;
static u32 cheat_hash_valid;
typedef struct {
	u32 magic, content_hash, count;
	char key[CHEAT_KEY_LEN];
	u16 indices[CHEAT_SELECTION_CACHE_MAX];
	u32 checksum;
} CheatState;
static CheatState cheat_state EWRAM_BSS;
static FIL cheat_state_file EWRAM_BSS;

static u32 CheatHash(u32 hash, const void *data, u32 size)
{
	const u8 *bytes = data;
	while(size--) { hash ^= *bytes++; hash *= 16777619u; }
	return hash;
}

static void CheatStatePath(char *path, const char *key)
{
	snprintf(path, 64, CHEATSTATE_DIR "/%08lx.cs2",
		(unsigned long)CheatHash(2166136261u, key, strlen(key)));
}

static void CheatContentHash(void)
{
	UINT count;
	u32 remaining;
	cheat_hash_valid = 0;
	cheat_active_hash = 2166136261u;
	/* Bound work on damaged or unexpectedly large inputs. */
	if(f_size(&gfile) > 1024 * 1024 || f_lseek(&gfile, 0) != FR_OK) return;
	remaining = (u32)f_size(&gfile);
	while(remaining)
	{
		UINT size = remaining > 256 ? 256 : remaining;
		if(f_read(&gfile, buf, size, &count) != FR_OK || count != size) return;
		cheat_active_hash = CheatHash(cheat_active_hash, buf, count);
		remaining -= count;
	}
	cheat_hash_valid = f_lseek(&gfile, 0) == FR_OK;
}

static u32 CheatStateApply(u32 total)
{
	char path[64];
	UINT count;
	u32 i, previous = 0;
	FRESULT result, closed;
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;
	if(!cheat_hash_valid || !cheat_active_key[0]) return 0;
	CheatStatePath(path, cheat_active_key);
	if(f_open(&cheat_state_file, path, FA_READ) != FR_OK) return 0;
	result = f_read(&cheat_state_file, &cheat_state, sizeof(cheat_state), &count);
	closed = f_close(&cheat_state_file);
	if(result != FR_OK || closed != FR_OK || count != sizeof(cheat_state) ||
		cheat_state.magic != CHEATSTATE_MAGIC ||
		cheat_state.content_hash != cheat_active_hash ||
		cheat_state.count > CHEAT_SELECTION_CACHE_MAX ||
		memcmp(cheat_state.key, cheat_active_key, sizeof(cheat_state.key)) ||
		cheat_state.checksum != CheatHash(2166136261u, &cheat_state,
			offsetof(CheatState, checksum))) return 0;
	/* Validate every index and section before applying anything. */
	for(i = 0; i < cheat_state.count; i++)
	{
		u32 index = cheat_state.indices[i], section;
		if(index >= total || entries[index].is_section || (i && index <= previous)) return 0;
		section = index;
		while(section && !entries[section].is_section) section--;
		if(!entries[section].is_section || (i && previous > section)) return 0;
		previous = index;
	}
	for(i = 0; i < cheat_state.count; i++) entries[cheat_state.indices[i]].select = 1;
	return cheat_state.count;
}

static u32 CheatStateSave(void)
{
	char path[64];
	UINT written;
	FRESULT result, closed;
	if(!cheat_hash_valid || !cheat_active_key[0]) return 0;
	memset(&cheat_state, 0, sizeof(cheat_state));
	cheat_state.magic = CHEATSTATE_MAGIC;
	cheat_state.content_hash = cheat_active_hash;
	cheat_state.count = cheat_selection_cache_count;
	memcpy(cheat_state.key, cheat_active_key, sizeof(cheat_state.key));
	memcpy(cheat_state.indices, cheat_selection_index,
		cheat_selection_cache_count * sizeof(cheat_selection_index[0]));
	cheat_state.checksum = CheatHash(2166136261u, &cheat_state, offsetof(CheatState, checksum));
	result = f_mkdir(CHEATSTATE_DIR);
	if(result != FR_OK && result != FR_EXIST) return 0;
	CheatStatePath(path, cheat_active_key);
	if(f_open(&cheat_state_file, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return 0;
	result = f_write(&cheat_state_file, &cheat_state, sizeof(cheat_state), &written);
	closed = f_close(&cheat_state_file);
	return result == FR_OK && closed == FR_OK && written == sizeof(cheat_state);
}

static void CheatSelectionMakeKey(char *dst, u32 dst_len, const TCHAR *gamefilename, u32 havecht)
{
	memset(dst, 0, dst_len);
	int n = snprintf(dst, dst_len, "%08lx:%s/%s", (unsigned long)havecht,
		currentpath, gamefilename ? gamefilename : "");
	/* Never use a truncated identity for a different game. */
	if(n < 0 || (u32)n >= dst_len) dst[0] = '\0';
}

u32 CheatSelectionAppliesTo(TCHAR *gamefilename, u32 havecht)
{
	char key[CHEAT_KEY_LEN];
	CheatSelectionMakeKey(key, sizeof(key), gamefilename, havecht);
	return (key[0] && !strcmp(cheat_selection_key, key)) ? 1 : 0;
}

void CheatSelectionForget(void)
{
	cheat_selection_key[0] = '\0';
	cheat_active_key[0] = '\0';
	cheat_selection_cache_count = 0;
	gl_cheat_selected_count = 0;
}

static void CheatSelectionRestore(u32 total)
{
	u32 i;
	if(!cheat_hash_valid || !cheat_active_key[0] || cheat_selection_hash != cheat_active_hash || strcmp(cheat_selection_key, cheat_active_key))
	{
		gl_cheat_selected_count = 0;
		return;
	}

	gl_cheat_selected_count = 0;
	for(i=0;i<cheat_selection_cache_count;i++)
	{
		if(cheat_selection_index[i] < total &&
			!((FM_CHT_LINE*)pCHTbuffer)[cheat_selection_index[i]].is_section)
		{
			((FM_CHT_LINE*)pCHTbuffer)[cheat_selection_index[i]].select = 1;
			gl_cheat_selected_count++;
		}
	}
}

static void CheatSelectionRemember(u32 total)
{
	u32 i;

	cheat_selection_cache_count = 0;
	cheat_selection_hash = cheat_active_hash;
	strncpy(cheat_selection_key, cheat_active_key, sizeof(cheat_selection_key) - 1);
	cheat_selection_key[sizeof(cheat_selection_key) - 1] = '\0';

	for(i=0;i<total;i++)
	{
		if(!((FM_CHT_LINE*)pCHTbuffer)[i].is_section && ((FM_CHT_LINE*)pCHTbuffer)[i].select == 1)
		{
			if(cheat_selection_cache_count < CHEAT_SELECTION_CACHE_MAX)
				cheat_selection_index[cheat_selection_cache_count++] = i;
		}
	}
}

static u32 CheatTextLooksGB2312(const char *str)
{
	u32 i;
	u32 l = strlen(str);

	for(i=0;i+1<l;i++)
	{
		u8 c1 = (u8)str[i];
		u8 c2 = (u8)str[i+1];

		if((c1 >= 0xC2) && (c1 <= 0xDF) && ((c2 & 0xC0) == 0x80))
		{
			i++;
			continue;
		}
		if((c1 >= 0xE0) && (c1 <= 0xEF) && (i+2<l) && ((c2 & 0xC0) == 0x80) && (((u8)str[i+2] & 0xC0) == 0x80))
		{
			i += 2;
			continue;
		}

		if((c1 >= 0xA1) && (c1 <= 0xF7) && (c2 >= 0xA1) && (c2 <= 0xFE))
			return 1;
	}
	return 0;
}

static u32 CheatTextVisibleColumns(const char *str)
{
	u32 i = 0;
	u32 shown = 0;
	u32 l = strlen(str);

	while(i<l)
	{
		u8 c1 = (u8)str[i++];
		if(c1 < 0x80)
			shown++;
		else if((i<l) && (c1 >= 0xA1) && (c1 <= 0xF7) && ((u8)str[i] >= 0xA1) && ((u8)str[i] <= 0xFE))
		{
			i++;
			shown += 2;
		}
		else
			shown++;
	}
	return shown;
}

static void DrawCheatText12(char *str, u16 len, u16 x, u16 y, u16 c, u8 isDrawDirect)
{
	u16 old_lang = gl_select_lang;
	if((old_lang != 0xE2E2) && CheatTextLooksGB2312(str))
		gl_select_lang = 0xE2E2;
	DrawHZText12(str, len, x, y, c, isDrawDirect);
	gl_select_lang = old_lang;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void Trim(char s[])
{
	int n;
	for(n = strlen(s) - 1; n >= 0; n--)
	{
		if(s[n]!=' ' && s[n]!='\t' && s[n]!='\n')
			break;
		s[n] = '\0';
	}
}

static void Clean_cheat_title(char s[])
{
	char *dash;
	char *p;
	char *dot;
	char temp[128];
	u32 r;
	u32 w = 0;
	u32 paren = 0;
	u32 bracket = 0;

	Trim(s);
	dot = strrchr(s, '.');
	if(dot)
		*dot = '\0';

	dash = strchr(s, '-');
	if(dash && (dash - s) <= 16)
	{
		u32 i;
		u32 prefix_ok = 0;
		for(i = 0; s[i] && (&s[i] < dash); i++)
		{
			if((s[i] >= '0') && (s[i] <= '9'))
				prefix_ok = 1;
			else if((s[i] != ' ') && (s[i] != '_') && (s[i] != '.'))
			{
				prefix_ok = 0;
				break;
			}
		}
		if(prefix_ok)
			memmove(s, dash + 1, strlen(dash + 1) + 1);
	}

	p = s;
	while((*p == ' ') || (*p == '-') || (*p == '_'))
		p++;
	if(p != s)
		memmove(s, p, strlen(p) + 1);

	memset(temp, 0, sizeof(temp));
	strncpy(temp, s, sizeof(temp) - 1);
	for(r = 0; temp[r] && w < sizeof(temp) - 1; r++)
	{
		if(temp[r] == '(')
		{
			paren = 1;
			continue;
		}
		if(temp[r] == ')')
		{
			paren = 0;
			continue;
		}
		if(temp[r] == '[')
		{
			bracket = 1;
			continue;
		}
		if(temp[r] == ']')
		{
			bracket = 0;
			continue;
		}
		if(!paren && !bracket)
			s[w++] = temp[r];
	}
	s[w] = '\0';
	Trim(s);
}
//------------------------------------------------------------------
void Get_KEY_val(FIL* file,char*KEY_section,char*KEY_secval,char getbuff[])
{
	int text_comment = 0;

	int in_section=0;
	//int keyval_count=0;

	char section[MAX_KEY_LEN] = {0};

	f_lseek(&gfile, 0x0);
	while(f_gets(buf, MAX_KEY_LEN, &gfile) != NULL)
	{
		Trim(buf);
    // to skip text comment with flags /* ...*/
    if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/'))
    {
			if (strstr(buf, "/*") != NULL)
			{
				text_comment = 1;
				continue;
			}
			else if (strstr(buf, "*/") != NULL)
			{
				text_comment = 0;
				continue;
			}
    }
    if (text_comment == 1)
	  {
			continue;
    }

		int buf_len = strlen(buf);

    // ignore and skip the line with first chracter '#', '=' or '/'
    if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/')
    {
    	in_section =0;
        continue;
    }

		char _paramk[MAX_KEY_LEN] = {0};
		char _paramv[/*MAX_VAL_LEN*/MAX_KEY_LEN] = {0};

    int _kv=0, _klen=0, _vlen=0;
    int i = 0;

    int is_section=0;
    int section_len=0;
    //int _val_len=0;

    for (i=0; i<buf_len; ++i)
    {
      if (buf[i] == ' ')
          continue;

			if (buf[i] == '[')
			{
				is_section = 1;
				in_section = 0;

				memset(section,0,MAX_KEY_LEN);
				section_len = 0;
				continue;
			}

			if(is_section == 1 && buf[i] != ']')
			{
				if(section_len < MAX_KEY_LEN - 1)
					section[section_len++] = buf[i];
				continue;
			}
      else if (buf[i] == ']')
      {
          is_section = 0;
          in_section = 1;
          //_val_len = 0;
          break;
      }

			if(in_section ==1)
			{
				// scan param key name
        if (_kv == 0 && buf[i] != '=')
        {
		    if (_klen >= MAX_KEY_LEN - 1)
                break;
            _paramk[_klen++] = buf[i];
            continue;
        }
        else if (buf[i] == '=')
        {
            _kv = 1;
            continue;
        }

	      // scan param key value
	      if (_vlen >= MAX_KEY_LEN || buf[i] == '#')
					break;

	      _paramv[_vlen++] = buf[i];
	    }

	  }
     //DEBUG_printf("KEY_section %s, section %s",KEY_section,section);
		if( strcmp(KEY_section,section)== 0)
		{
			if( strcmp(KEY_secval,_paramk) == 0)
			{
				strcpy(getbuff,_paramv);

				return ;
			}
		}
		if (strcmp(_paramk, "")==0 || strcmp(_paramv, "")==0)
			continue;

		memset(buf,0,MAX_KEY_LEN) ;
  }
}
//------------------------------------------------------------------
u32 Get_CHT_val(FIL* file,char*KEY_section,char*KEY_secval/*,char getbuff[]*/)
{
	int text_comment = 0;

	int in_section=0;
	//int keyval_count=0;

	char section[MAX_KEY_LEN] = {0};

	f_lseek(&gfile, 0x0);
	while(f_gets(buf, MAX_BUF_LEN, &gfile) != NULL)
	{
		Trim(buf);
    // to skip text comment with flags /* ...*/
    if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/'))
    {
			if (strstr(buf, "/*") != NULL)
			{
				text_comment = 1;
				continue;
			}
			else if (strstr(buf, "*/") != NULL)
			{
				text_comment = 0;
				continue;
			}
    }
    if (text_comment == 1)
	  {
			continue;
    }

		int buf_len = strlen(buf);
		//DEBUG_printf("cht buf_len %x",buf_len);

    // ignore and skip the line with first chracter '#', '=' or '/'
    if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/')
    {
    	in_section =0;
        continue;
    }

		char _paramk[MAX_KEY_LEN] = {0};
		//char _paramv[/*MAX_VAL_LEN*/MAX_KEY_LEN] = {0};

    int _kv=0, _klen=0, _vlen=0;
    int i = 0;

    int is_section=0;
    int section_len=0;
    //int _val_len=0;

    for (i=0; i<buf_len; ++i)
    {
      if (buf[i] == ' ')
          continue;

			if (buf[i] == '[')
			{
				is_section = 1;
				in_section = 0;

				memset(section,0,MAX_KEY_LEN);
				section_len = 0;
				continue;
			}

			if(is_section == 1 && buf[i] != ']')
			{
				if(section_len < MAX_KEY_LEN - 1)
					section[section_len++] = buf[i];
				continue;
			}
      else if (buf[i] == ']')
      {
          is_section = 0;
          in_section = 1;
          //_val_len = 0;
          break;
      }

			if(in_section ==1)
			{
				// scan param key name
        if (_kv == 0 && buf[i] != '=')
        {
		    if (_klen >= MAX_KEY_LEN - 1)
                break;
            _paramk[_klen++] = buf[i];
            continue;
        }
        else if (buf[i] == '=')
        {
            _kv = 1;
            continue;
        }
	      // scan param key value
	      if (_vlen >= MAX_BUF_LEN || buf[i] == '#')
					break;

	      _paramv[_vlen++] = buf[i];
	    }

	  }
	  if(_vlen > 0)
		_vlen--; //remove 0xd

     //DEBUG_printf("KEY_section %s, section %s",KEY_section,section);
		if( strcmp(KEY_section,section)== 0)
		{
			if( strcmp(KEY_secval,_paramk) == 0)
			{
			  //0111 Multi-line
			  while(f_gets(buf, MAX_BUF_LEN, &gfile) != NULL)
				{
					Trim(buf);
			    // to skip text comment with flags /* ...*/
			    if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/'))
			    {
						if (strstr(buf, "/*") != NULL)
						{
							text_comment = 1;
							break;
						}
						else if (strstr(buf, "*/") != NULL)
						{
							text_comment = 0;
							break;
						}
			    }
			    if (text_comment == 1)
				  {
						break;
			    }

					int buf_len = strlen(buf);
					//DEBUG_printf("cht buf_len %x",buf_len);

			    // ignore and skip the line with first chracter '#', '=' or '/'
			    if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/')
			    {
			        break;
			    }
			    for (i=0; i<buf_len; ++i)
		    	{
		     	 	if (buf[i] == '=')
		          goto endcht;
		      }
				if(buf_len > (int)(MAX_BUF_LEN - _vlen))
					return 0;
			    for (i=0; i<buf_len; ++i)
		    	{
						_paramv[_vlen++] = buf[i];
		      }
		       _vlen--; //remove 0xd
		      //DEBUG_printf("%x %x %x %x %x %x %x %x", buf[0],buf[1], buf[2],buf[3], buf[4],buf[5], buf[6],buf[7]);
		      //wait_btn();

			  }
			  endcht:
				return _vlen;
			}
		}
		if (strcmp(_paramk, "")==0 || strcmp(_paramv, "")==0)
			continue;

		memset(buf,0,MAX_KEY_LEN) ;
  }
  return 0;
}
//------------------------------------------------------------------
u32 Get_all_Section_val(FIL* file)
{
	char buf[MAX_sectionVAL_LEN];
	int text_comment = 0;

	int in_section=0;
	//char keyval[10][25];
	//int keyval_count=0;
	u32 Line = 0;


	char section[MAX_KEY_LEN] = {0};

	//fseek(file,0x0,SEEK_SET);
	f_lseek(&gfile, 0x0);
	while(Line < MAX_CHEAT_MENU_ENTRIES && f_gets(buf, MAX_sectionVAL_LEN, &gfile) != NULL)
	{
		memset(tmpCHTFS.LINEname,0x00,MAX_KEY_LEN);
		Trim(buf);
		// to skip text comment with flags /* ...*/
		if(buf[0] == '-'  && buf[1] == '-')
			break;
		if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/'))
		{
		    if (strstr(buf, "/*") != NULL)
		    {
		        text_comment = 1;
		        continue;
		    }
		    else if (strstr(buf, "*/") != NULL)
		    {
		        text_comment = 0;
		        continue;
		    }
		}
		if (text_comment == 1)
		{
		    continue;
		}

		int buf_len = strlen(buf);

		// ignore and skip the line with first chracter '#', '=' or '/'
		if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/')
		{
			in_section =0;
		    continue;
		}
		char _paramk[MAX_KEY_LEN] = {0};
		//char _paramv[MAX_sectionVAL_LEN] = {0};

		int _kv=0, _klen=0, _vlen=0;
		int i = 0;

		int is_section=0;
		int section_len=0;
		//int _val_len=0;

		for (i=0; i<buf_len; ++i)
		{
			if (buf[i] == ' ')
				continue;

			if (buf[i] == '[')
			{
				is_section = 1;
				in_section = 0;

				memset(section,0,MAX_KEY_LEN);
				section_len = 0;
				continue;
			}

			if(is_section == 1 && buf[i] != ']')
			{
				if(section_len < MAX_KEY_LEN - 1)
					section[section_len++] = buf[i];
				continue;
			}
      else if (buf[i] == ']')
      {
	memcpy(tmpCHTFS.LINEname,section,section_len);
		tmpCHTFS.LINEname[section_len] = '\0';
	tmpCHTFS.is_section = 1;
	tmpCHTFS.len = section_len;
	tmpCHTFS.select =0;

	dmaCopy(&tmpCHTFS,&((FM_CHT_LINE*)pCHTbuffer)[Line], sizeof(FM_CHT_LINE));

	Line++;
	//if(Line==10)return;
        is_section = 0;
        in_section = 1;
        //_val_len = 0;
        break;
      }

			if(in_section ==1)
			{
        // scan param key name
        if (_kv == 0 && buf[i] != '=')
        {
		    if (_klen >= MAX_KEY_LEN - 1)
                break;
            _paramk[_klen++] = buf[i];
            continue;
        }
        else if (buf[i] == '=')
        {
		//DEBUG_printf(":%s ",_paramk);
		memcpy(tmpCHTFS.LINEname,_paramk,_klen);
		tmpCHTFS.LINEname[_klen] = '\0';
		tmpCHTFS.is_section = 0;

		tmpCHTFS.select =0;

          _kv = 1;
          continue;
        }
        else{
		      /*
		      // scan param key value
		      if (_vlen >= MAX_VAL_LEN || buf[i] == '#')
		                break;

		      _paramv[_vlen++] = buf[i];
		      */
	    	}
      }

		}
		if(_kv ==1)
		{
			tmpCHTFS.len = _vlen;
			//memcpy(tmpCHTFS.KEY_val,_paramv,_vlen);
			dmaCopy(&tmpCHTFS,&((FM_CHT_LINE*)pCHTbuffer)[Line], sizeof(FM_CHT_LINE));
			Line++;
		}
  }
	return Line;
}
//------------------------------------------------------------------
/* Scan once: no per-row walk back through a potentially large group. */
u32 GetVisibleCheatMenuCount(u32 total_entries)
{
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;
	u32 index, count = 0, expanded = 1;
	for(index = 0; index < total_entries; index++)
	{
		if(entries[index].is_section)
		{
			expanded = entries[index].select == CHT_GROUP_EXPANDED;
			count++;
		}
		else if(expanded) count++;
	}
	return count;
}

u32 GetVisibleCheatMenuEntryIndex(u32 total_entries, u32 visible_index)
{
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;
	u32 index, count = 0, expanded = 1;
	for(index = 0; index < total_entries; index++)
	{
		if(entries[index].is_section)
			expanded = entries[index].select == CHT_GROUP_EXPANDED;
		if(entries[index].is_section || expanded)
		{
			if(count++ == visible_index) return index;
		}
	}
	return total_entries;
}

u32 SetCheatGroupExpanded(u32 entry_index, u32 total_entries, u32 expanded)
{
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;
	u8 new_state;

	if (entry_index >= total_entries || entries[entry_index].is_section != 1)
		return 0;
	new_state = expanded ? CHT_GROUP_EXPANDED : CHT_GROUP_COLLAPSED;
	if (entries[entry_index].select == new_state)
		return 0;
	entries[entry_index].select = new_state;
	return 1;
}

static u32 CountSelectedCheatMenuEntries(u32 total_entries)
{
	u32 index;
	u32 count = 0;
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;

	for (index = 0; index < total_entries; index++)
	{
		if (entries[index].is_section == 0 && entries[index].select == 1)
			count++;
	}
	return count;
}

void ToggleCheatMenuEntry(u32 entry_index, u32 total_entries)
{
	FM_CHT_LINE *entries = (FM_CHT_LINE*)pCHTbuffer;
	u32 first = entry_index, end = entry_index, i;
	u8 selected;
	if(entry_index >= total_entries || entries[entry_index].is_section) return;
	selected = entries[entry_index].select;
	while(first > 0 && !entries[first - 1].is_section) first--;
	while(end + 1 < total_entries && !entries[end + 1].is_section) end++;
	/* Preserve this fork's one-option-per-section semantics. */
	for(i = first; i <= end; i++) entries[i].select = 0;
	if(!selected && CountSelectedCheatMenuEntries(total_entries) < CHEAT_SELECTION_CACHE_MAX)
		entries[entry_index].select = 1;
}
//------------------------------------------------------------------
static void Show_KEY_line(u32 line, u32 Select, u32 showoffset, u32 total_entries)
{
	char msg[256];
	u32 X_offset=15;
	u32 Y_offset=20;
	u32 line_x = 14;
	u16 name_color;
	u32 row_y = Y_offset+line*line_x;
	u32 msg_len;
	u32 highlight_w;
	u32 visible_index = showoffset + line;
	u32 entry_index = GetVisibleCheatMenuEntryIndex(total_entries, visible_index);
	u8 select;
	FM_CHT_LINE *entry;

	Launcher_ClearCheatRegion(X_offset, row_y, 220, 13);
	if (entry_index >= total_entries)
		return;
	entry = &((FM_CHT_LINE*)pCHTbuffer)[entry_index];
	name_color = (line == Select) ? gl_color_selected : gl_color_text;
	select = entry->select;

	if(entry->is_section == 1)
	{
		snprintf(msg, sizeof(msg),"[%c] %s", select == CHT_GROUP_EXPANDED ? '-' : '+',
			entry->LINEname);
		if(line == Select)
		{
			msg_len = CheatTextVisibleColumns(msg);
			if(msg_len > 30) msg_len = 30;
			highlight_w = msg_len * 6 + 8;
			if(highlight_w < 30) highlight_w = 30;
			Clear(X_offset+1, row_y, highlight_w, 13, gl_color_selectBG_sd, 1);
		}
		DrawCheatText12(msg,30,X_offset+4,row_y,name_color,1);
	}
	else
	{
		Draw_select_icon(X_offset+13,row_y,select);
		snprintf(msg, sizeof(msg),"%s",entry->LINEname);
		if(!strcasecmp(msg, "ON"))
			snprintf(msg, sizeof(msg), "%s", Launcher_OnOffText(select));
		if(line == Select)
		{
			msg_len = CheatTextVisibleColumns(msg);
			if(msg_len > 30) msg_len = 30;
			highlight_w = 18 + msg_len * 6 + 8;
			if(highlight_w < 36) highlight_w = 36;
			if(highlight_w > 205) highlight_w = 205;
			Clear(X_offset+10, row_y, highlight_w, 13, gl_color_selectBG_sd, 1);
			Draw_select_icon(X_offset+13,row_y,select);
		}
		DrawCheatText12(msg,30,X_offset+28,row_y,name_color,1);
	}
}

void Show_KEY_val(u32 total_entries, u32 visible_total, u32 Select, u32 showoffset)
{
	u32 line;

	/* Always visit all 10 rows so rows left over from a collapsed group are cleared. */
	(void)visible_total;
	for(line=0; line<10; line++)
		Show_KEY_line(line, Select, showoffset, total_entries);
}

static void ClampCheatViewport(u32 visible_count, u32 *Select, u32 *showoffset)
{
	u32 cursor = *showoffset + *Select;
	u32 max_offset = visible_count > 10 ? visible_count - 10 : 0;

	if (visible_count == 0)
		cursor = 0;
	else if (cursor >= visible_count)
		cursor = visible_count - 1;
	if (*showoffset > max_offset)
		*showoffset = max_offset;
	if (cursor < *showoffset)
		*showoffset = cursor;
	*Select = cursor - *showoffset;
}

static u16 CheatVisibleSelectionMask(u32 total_entries, u32 visible_total,
	u32 showoffset)
{
	u16 mask = 0;
	u32 line;
	u32 visible = (visible_total > showoffset) ? visible_total - showoffset : 0;

	if (visible > 10)
		visible = 10;
	for (line = 0; line < visible; line++)
	{
		u32 entry_index = GetVisibleCheatMenuEntryIndex(total_entries,
			showoffset + line);
		if (entry_index < total_entries)
		{
			FM_CHT_LINE *entry = &((FM_CHT_LINE*)pCHTbuffer)[entry_index];
			if (entry->is_section != 1 && entry->select)
				mask |= (u16)(1u << line);
		}
	}
	return mask;
}

static void RedrawChangedCheatSelections(u16 changed, u32 total_entries,
	u32 visible_total, u32 Select, u32 showoffset)
{
	u32 line;
	u32 visible = (visible_total > showoffset) ? visible_total - showoffset : 0;

	if (visible > 10)
		visible = 10;
	for (line = 0; line < visible; line++)
	{
		if (changed & (u16)(1u << line))
			Show_KEY_line(line, Select, showoffset, total_entries);
	}
}
//------------------------------------------------------------------
unsigned long str2hex( char*str)
{
	unsigned long sum=0;
	unsigned long i;
	size_t len = strlen(str);

	if(len >8) return 0;
	for(i=0;i<len;i++)
	{
		if(str[i] >= '0' && str[i] <= '9')
			sum = sum*16 + str[i]-'0';
		else if(str[i] >= 'a' && str[i] <= 'f')
			sum = sum*16 + str[i]-0x57;
		else if(str[i] >= 'A' && str[i] <= 'F')
			sum = sum*16 + str[i]-0x37;
	}
	return sum;
}
//------------------------------------------------------------------
void Analyze_KEYVAL(FIL* file,u32 total)
{
	u32 tol,i;
	u32 buflen;
	//u32 entry;
	//u32 address;
	u32 address_len;
	u32 val_len;
	char address_buf[9];
	char val_buf[3];
	u32 is_val;
	u32 is_address;
	u32 address_add;
	u32 malformed_code;
	u32 cheat_start_count;
	//char BUF_val[256];

	//DEBUG_printf("total %x  ",total);
	gl_cheat_count=0;
	gl_cheat_selected_count=0;
	CheatSelectionRemember(total);

	if(total)
	{
	}
	for(tol=0;tol<total;tol++)
	{

		if(!((FM_CHT_LINE*)pCHTbuffer)[tol].is_section && ((FM_CHT_LINE*)pCHTbuffer)[tol].select == 1)
		{
			gl_cheat_selected_count++;
			cheat_start_count = gl_cheat_count;
			if(tol == 0)
				continue;
			u32 current_select = tol - 1;
			while(current_select > 0 && ((FM_CHT_LINE*)pCHTbuffer)[current_select].is_section != 1)
			{
				current_select--;
			}
			if(((FM_CHT_LINE*)pCHTbuffer)[current_select].is_section != 1)
				continue;
			//DEBUG_printf("section %s  ", ((FM_CHT_LINE*)pCHTbuffer)[current_select].LINEname);
					buflen= Get_CHT_val(file,((FM_CHT_LINE*)pCHTbuffer)[current_select].LINEname,((FM_CHT_LINE*)pCHTbuffer)[tol].LINEname);
					if(buflen == 0)
						continue;

			address_len=0;
			val_len=0;
			is_val = 0;
			is_address = 1;
			address_add = 0;
			malformed_code = 0;
			memset(address_buf,0x00,sizeof(address_buf));
			memset(val_buf,0x00,sizeof(val_buf));
			for(i=0;i<buflen;i++)
			{
	      if (_paramv[i] == ' '){
	          continue;
				}
				else if (_paramv[i] == ','){
					if(is_address==1)	//first ','
					{
						is_address = 0;
					}
					else{	//next ','
						//DEBUG_printf(",0x%x =%x", str2hex(address_buf)+address_add,str2hex(val_buf));
						ST_entry cheat = { str2hex(address_buf)+address_add,str2hex(val_buf) };
						if(!malformed_code && gl_cheat_count < MAX_CHEAT_ENTRIES)
							pCHEAT[gl_cheat_count++] = cheat;

						address_add++;
					}
					val_len=0;
					memset(val_buf,0x00,3);
					is_val = 1;
					continue;
				}
				else if(_paramv[i] == ';'){
					ST_entry cheat = { str2hex(address_buf)+address_add,str2hex(val_buf) };
					if(!malformed_code && gl_cheat_count < MAX_CHEAT_ENTRIES)
						pCHEAT[gl_cheat_count++] = cheat;


					is_val = 0;
					is_address = 1;
					address_add=0;
					memset(address_buf,0x00,sizeof(address_buf));
					address_len=0;
					continue;
				}

				if(is_val){
					if(val_len < sizeof(val_buf) - 1)
						val_buf[val_len++] = _paramv[i];
					else
						malformed_code = 1;
				}
				else{
					if(address_len < sizeof(address_buf) - 1)
						address_buf[address_len++] = _paramv[i];
					else
						malformed_code = 1;
				}
			}
			ST_entry cheat = { str2hex(address_buf)+address_add,str2hex(val_buf) };
			if(!malformed_code && gl_cheat_count < MAX_CHEAT_ENTRIES)
				pCHEAT[gl_cheat_count++] = cheat;
			if(malformed_code)
				gl_cheat_count = cheat_start_count;

		}
	}
}
//------------------------------------------------------------------
u32 Check_count(u32 all_count)
{
	u32 count=0;
	u32 Line;
	for(Line=0;Line<all_count;Line++)
	{
		if(strcmp( ((FM_CHT_LINE*)pCHTbuffer)[Line].LINEname,"GameInfo")==0)
			break;

		count++;
		//if(count>512)
			//break;
	}
	return count;
}
//------------------------------------------------------------------
unsigned char HexToChar(unsigned char bChar)
{
    if((bChar>=0x30)&&(bChar<=0x39))
    {
        bChar -= 0x30;
    }
    else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
    {
        bChar -= 0x37;
    }
    else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
    {
        bChar -= 0x57;
    }
    else
    {
        bChar = 0xff;
    }
    return bChar;
}
//------------------------------------------------------------------
u32 Change2cht_folder(u32 chtname)
{
	TCHAR chtnamebuf[100];
	u32 res;
	TCHAR* folder_name;
	TCHAR currentpath[256];
	memset(currentpath,00,256);

	memset(chtnamebuf,0x00,100);
	sprintf(chtnamebuf,"%d%d%d%d",HexToChar(((u8*)&chtname)[0]),HexToChar(((u8*)&chtname)[1]),HexToChar(((u8*)&chtname)[2]),HexToChar(  ((u8*)&chtname)[3] )  );
	u32 num=atoi(chtnamebuf);
	//DEBUG_printf("num =%d", num);
	if(num < 200){
		folder_name = (TCHAR*)"0000";
	}
	else if(num < 400){
		folder_name = (TCHAR*)"0200";
	}
	else if(num < 600){
		folder_name = (TCHAR*)"0400";
	}
	else if(num < 800){
		folder_name = (TCHAR*)"0600";
	}
	else if(num < 1000){
		folder_name = (TCHAR*)"0800";
	}
	else if(num < 1200){
		folder_name = (TCHAR*)"1000";
	}
	else if(num < 1400){
		folder_name = (TCHAR*)"1200";
	}
	else if(num < 1600){
		folder_name = (TCHAR*)"1400";
	}
	else if(num < 1800){
		folder_name = (TCHAR*)"1600";
	}
	else if(num < 2000){
		folder_name = (TCHAR*)"1800";
	}
	else if(num < 2200){
		folder_name = (TCHAR*)"2000";
	}
	else if(num < 2400){
		folder_name = (TCHAR*)"2200";
	}
	else if(num < 2600){
		folder_name = (TCHAR*)"2400";
	}
	else if(num < 2800){
		folder_name = (TCHAR*)"2600";
	}
	else {
		folder_name = (TCHAR*)"2800";
	}


	if(!cheat_use_chinese_folder)
	{
		sprintf(currentpath,"/SYSTEM/CHEAT/Eng/%s",folder_name);
	}
	else{
		sprintf(currentpath,"/SYSTEM/CHEAT/Chn/%s",folder_name);
	}
	res=f_chdir(currentpath);
	return res;
}
//------------------------------------------------------------------
u32 Check_cheat_file(TCHAR *gamefilename)
{
	u32 res;
	UINT ret;
	TCHAR chtnamebuf[MAX_path_len];
	u32 filesize;
	u32 GAMEID=0;
	u32 i;
	u32 len;
	cheat_use_chinese_folder = (gl_select_lang == 0xE2E2);
	if(!gamefilename || strlen(gamefilename) >= sizeof(chtnamebuf))
		return 0;

	res = f_open(&gfile, gamefilename, FA_READ);
	if(res == FR_OK)
	{
		f_lseek(&gfile, 0xAC);
		f_read(&gfile, &GAMEID, 4, (UINT *)&ret);
		f_close(&gfile);
		if(GAMEID==0) return 0;
	}

	strncpy(chtnamebuf, gamefilename, sizeof(chtnamebuf) - 1);
	chtnamebuf[sizeof(chtnamebuf) - 1] = '\0';
	len = strlen(chtnamebuf);
	if(len < 3)
		return 0;
	chtnamebuf[len-3] = 'c';
	chtnamebuf[len-2] = 'h';
	chtnamebuf[len-1] = 't';

	res=f_chdir("/SYSTEM/CHEAT");
	if(res != FR_OK){
		return 0;
	}

	res = f_open(&gfile,chtnamebuf, FA_OPEN_EXISTING);
	//f_chdir(currentpath);
	if(res == FR_OK)//have a cht file
	{
		f_close(&gfile);
		return 0x0000FFFF;
	}
	else
	{
		res = f_open(&gfile,"GameID2cht.bin", FA_READ);
		u32* tempbuff = (u32*)(pReadCache);
		if(res == FR_OK)//have a file
		{
			filesize = f_size(&gfile) > 0x10000 ? 0x10000 : (u32)f_size(&gfile);
			f_lseek(&gfile, 0x0);
			res = f_read(&gfile, tempbuff, filesize, &ret);//pReadCache max 0x20000 Byte
			if(res != FR_OK)
			{
				f_close(&gfile);
				return 0;
			}

			/* Each mapping is a pair of 32-bit values. Ignore an incomplete
			   trailing record, including when the read itself was short. */
			for(i=0;i+1<ret/4;i+=2)
			{

				if(tempbuff[i]== GAMEID)
				{
					f_close(&gfile);

					u32 chtname= ((u32*)tempbuff)[i+1];

					cheat_use_chinese_folder = (gl_select_lang == 0xE2E2);
					res=Change2cht_folder(chtname);
					if(res!=0)
					{
						cheat_use_chinese_folder = !cheat_use_chinese_folder;
						res=Change2cht_folder(chtname);
					}
					if(res!=0)return 0;
					memset(chtnamebuf,0x00,100);
					sprintf(chtnamebuf,"%d%d%d%d.cht",HexToChar(((u8*)&chtname)[0]),HexToChar(((u8*)&chtname)[1]),HexToChar(((u8*)&chtname)[2]),HexToChar(  ((u8*)&chtname)[3] )  );
					res = f_open(&gfile,chtnamebuf, FA_OPEN_EXISTING);

					if(res == FR_OK)//have a cht file
					{
						f_close(&gfile);
						return chtname;
					}
					else{
						return 0;
					}
				}
			}
			f_close(&gfile);
		}
		return 0;
	}
}
//---------------------------------------------------------------------------------
void Show_num(u32 totalcount,u32 select)
{
	u32 i;
	u32 total_sections = 0;
	u32 selected_section = 0;
	u32 line = select ? select - 1 : 0;

	if(totalcount == 0)
	{
		Launcher_DrawCheatCounter(0, 0);
		return;
	}
	if(line >= totalcount)
		line = totalcount - 1;
	for(i = 0; i < totalcount; i++)
	{
		if(((FM_CHT_LINE*)pCHTbuffer)[i].is_section == 1)
		{
			total_sections++;
			if(i <= line)
				selected_section = total_sections;
		}
	}
	if(selected_section == 0 && total_sections)
		selected_section = 1;
	Launcher_DrawCheatCounter(total_sections, selected_section);
}
//------------------------------------------------------------------
static u32 CheatOpenFile(TCHAR *gamefilename, u32 havecht)
{
	u32 res;
	TCHAR chtnamebuf[MAX_path_len];

	if(havecht == 0x0000FFFF)
	{
		res=f_chdir("/SYSTEM/CHEAT");
		if(res != FR_OK){
			return FR_NO_PATH;
		}
		u32 len = gamefilename ? strlen(gamefilename) : 0;
		if(len < 3 || len >= sizeof(chtnamebuf)) return FR_INVALID_NAME;
		memcpy(chtnamebuf, gamefilename, len + 1);
		chtnamebuf[len-3] = 'c';
		chtnamebuf[len-2] = 'h';
		chtnamebuf[len-1] = 't';
	}
	else
	{
		if(Change2cht_folder(havecht) != FR_OK) return FR_NO_PATH;
		u8* chtmode;
		chtmode = (u8*)&havecht;
		sprintf(chtnamebuf,"%d%d%d%d.cht",HexToChar(chtmode[0]),HexToChar(chtmode[1]),HexToChar(chtmode[2]),HexToChar(chtmode[3]));
	}
	return f_open(&gfile,chtnamebuf, FA_READ);

}

void CheatStateLoadHeadless(TCHAR *gamefilename, u32 havecht)
{
	u32 total;
	gl_cheat_count = 0;
	gl_cheat_selected_count = 0;
	if(!havecht || CheatOpenFile(gamefilename, havecht) != FR_OK) return;
	CheatContentHash();
	CheatSelectionMakeKey(cheat_active_key, sizeof(cheat_active_key), gamefilename, havecht);
	total = Check_count(Get_all_Section_val(&gfile));
	CheatSelectionRestore(total);
	if(gl_cheat_remember && (cheat_selection_hash != cheat_active_hash ||
		strcmp(cheat_selection_key, cheat_active_key))) CheatStateApply(total);
	Analyze_KEYVAL(&gfile, total);
	f_close(&gfile);
}

void Open_cht_file(TCHAR *gamefilename,u32 havecht)
{
	u32 res;
	char msg[128];

	char buffer[127]={0};

	res = CheatOpenFile(gamefilename, havecht);

	if(res == FR_OK)//have a cht file
	{
		CheatContentHash();
		CheatSelectionMakeKey(cheat_active_key, sizeof(cheat_active_key), gamefilename, havecht);
		if(strcmp(cheat_selection_key, cheat_active_key))
			gl_cheat_selected_count = 0;

		strncpy(buffer, gamefilename, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		Clean_cheat_title(buffer);
		snprintf(msg, sizeof(msg),"%s",buffer);

		Launcher_DrawCheatBackground(msg);

		u32 all_count = Get_all_Section_val(&gfile);
		u32 Select = 0;
		u32 showoffset = 0;
		u32 re_show = 2;
		u32 old_select = 1;
		u32 old_showoffset = 0;
		u32 cheat_scroll_delay = 0;
		u32 visible_count;
		u16 selection_redraw_mask = 0;

		all_count = Check_count(all_count);
		CheatSelectionRestore(all_count);

		if(all_count)
		{
			all_count = Check_count(all_count);//cut from "GameInfo"
			if(gl_cheat_remember && (cheat_selection_hash != cheat_active_hash || strcmp(cheat_selection_key, cheat_active_key)))
				gl_cheat_selected_count = CheatStateApply(all_count);
			visible_count = GetVisibleCheatMenuCount(all_count);
			if (visible_count == 0)
			{
				f_close(&gfile);
				Launcher_MarkTopbarNameDirty();
				return;
			}
			setRepeat(15,1);
			while(1)//3
			{
				VBlankIntrWait();
				VBlankIntrWait();
				UIAudio_UpdateExport();
				if(selection_redraw_mask)
				{
					RedrawChangedCheatSelections(selection_redraw_mask, all_count,
						visible_count, Select, showoffset);
					selection_redraw_mask = 0;
				}
				else if(re_show)
				{
					if(re_show>1)
					{
						/* Each row restores its own exact background before drawing.
						   Avoid clearing the whole body to preserve v7.3's flicker-free redraw. */
						Show_KEY_val(all_count, visible_count, Select, showoffset);
					}
					else if(old_showoffset == showoffset)
					{
						Show_KEY_line(old_select, Select, showoffset, all_count);
						if(old_select != Select)
							Show_KEY_line(Select, Select, showoffset, all_count);
					}
					else
						Show_KEY_val(all_count, visible_count, Select, showoffset);
					{
					u32 selected_entry = GetVisibleCheatMenuEntryIndex(all_count, showoffset + Select);
					Show_num(all_count, selected_entry < all_count ? selected_entry + 1 : 1);
				}
					old_select = Select;
					old_showoffset = showoffset;
					re_show = 0;
				}
				else
				{
					Launcher_UpdateCheatTitle();
				}
				if(cheat_scroll_delay > 0)
					cheat_scroll_delay--;
				scanKeys();
				u16 keysdown  = keysDown();
				u16 keysup = keysUp();
				u16 keysrepeat = keysDownRepeat();

				if ((keysdown & KEY_DOWN) || ((keysrepeat & KEY_DOWN) && cheat_scroll_delay == 0))
				{
					u32 cursor = showoffset + Select;
					u32 first_press = (keysdown & KEY_DOWN) ? 1 : 0;
					if (cursor + 1 < visible_count)
					{
						cursor++;
						if (cursor >= showoffset + 10)
						{
							showoffset = cursor - 9;
							Select = 9;
							re_show = 2;
						}
						else
						{
							Select++;
							re_show = 1;
						}
						UIAudio_PlayMove();
						cheat_scroll_delay = first_press ? 6 : 3;
					}
				}
				else if((keysdown & KEY_UP) || ((keysrepeat & KEY_UP) && cheat_scroll_delay == 0))
				{
					u32 cursor = showoffset + Select;
					u32 first_press = (keysdown & KEY_UP) ? 1 : 0;
					if (cursor > 0)
					{
						cursor--;
						if (cursor < showoffset)
						{
							showoffset = cursor;
							Select = 0;
							re_show = 2;
						}
						else
						{
							Select--;
							re_show = 1;
						}
						UIAudio_PlayMove();
						cheat_scroll_delay = first_press ? 6 : 3;
					}
				}
				else if((keysdown & KEY_LEFT) || ((keysrepeat & KEY_LEFT) && cheat_scroll_delay == 0))
				{
					u32 entry_index = GetVisibleCheatMenuEntryIndex(all_count, showoffset + Select);
					u32 first_press = (keysdown & KEY_LEFT) ? 1 : 0;
					if (entry_index < all_count &&
						((FM_CHT_LINE*)pCHTbuffer)[entry_index].is_section == 1)
					{
						if (SetCheatGroupExpanded(entry_index, all_count, 0))
						{
							visible_count = GetVisibleCheatMenuCount(all_count);
							ClampCheatViewport(visible_count, &Select, &showoffset);
							UIAudio_PlayAcceptExport();
							re_show = 2;
						}
					}
					else if (showoffset > 0)
					{
						u32 cursor = showoffset + Select;
						showoffset = showoffset > 10 ? showoffset - 10 : 0;
						if (cursor < showoffset) cursor = showoffset;
						Select = cursor - showoffset;
						if (Select > 9) Select = 9;
						UIAudio_PlayMove();
						re_show = 2;
						cheat_scroll_delay = first_press ? 6 : 3;
					}
				}
				else if((keysdown & KEY_RIGHT) || ((keysrepeat & KEY_RIGHT) && cheat_scroll_delay == 0))
				{
					u32 entry_index = GetVisibleCheatMenuEntryIndex(all_count, showoffset + Select);
					u32 first_press = (keysdown & KEY_RIGHT) ? 1 : 0;
					if (entry_index < all_count &&
						((FM_CHT_LINE*)pCHTbuffer)[entry_index].is_section == 1)
					{
						if (SetCheatGroupExpanded(entry_index, all_count, 1))
						{
							visible_count = GetVisibleCheatMenuCount(all_count);
							ClampCheatViewport(visible_count, &Select, &showoffset);
							UIAudio_PlayAcceptExport();
							re_show = 2;
						}
					}
					else if (showoffset + 10 < visible_count)
					{
						u32 cursor = showoffset + Select;
						u32 max_offset = visible_count > 10 ? visible_count - 10 : 0;
						showoffset += 10;
						if (showoffset > max_offset) showoffset = max_offset;
						if (cursor < showoffset) cursor = showoffset;
						Select = cursor - showoffset;
						if (Select > 9) Select = 9;
						UIAudio_PlayMove();
						re_show = 2;
						cheat_scroll_delay = first_press ? 6 : 3;
					}
				}
				else if(keysdown & KEY_A)
				{
					u32 entry_index = GetVisibleCheatMenuEntryIndex(all_count, showoffset + Select);
					if (entry_index < all_count &&
						((FM_CHT_LINE*)pCHTbuffer)[entry_index].is_section == 1)
					{
						SetCheatGroupExpanded(entry_index, all_count,
							((FM_CHT_LINE*)pCHTbuffer)[entry_index].select == CHT_GROUP_COLLAPSED);
						visible_count = GetVisibleCheatMenuCount(all_count);
						ClampCheatViewport(visible_count, &Select, &showoffset);
						re_show = 2;
					}
					else if (entry_index < all_count)
					{
						u16 old_selection_mask = CheatVisibleSelectionMask(all_count,
							visible_count, showoffset);
						ToggleCheatMenuEntry(entry_index, all_count);
						selection_redraw_mask = old_selection_mask ^
							CheatVisibleSelectionMask(all_count, visible_count,
							showoffset);
					}
					UIAudio_PlayAcceptExport();
				}
				else if(keysup & KEY_B)
				{
					UIAudio_PlayBackExport();
					UIAudio_WaitForCurrentClipExport(60);
					UIAudio_CutOffTrailingClipExport();
					Analyze_KEYVAL(&gfile,all_count);
					if(gl_cheat_remember && !CheatStateSave())
						Show_error_num(3); /* Existing localized Save error. */
					break;
				}
			}
		}
	}
	f_close(&gfile);
	Launcher_MarkTopbarNameDirty();
}
