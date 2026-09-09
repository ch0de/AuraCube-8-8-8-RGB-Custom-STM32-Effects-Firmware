#ifndef __CUBEFILE_H
#define __CUBEFILE_H 			   
#include "System.h"   
#include "ff.h"
//#include "tunes.h"

extern uint8_t StringPath[80];
extern volatile uint8_t SDinUse;

u8 mf_mount(u8* path, u8 mt);
u8 mf_open(u8*path, u8 mode);
u8 mf_close(void);
u8 mf_read(u16 len);
u8 mf_write(u8*dat, u16 len);
uint8_t mf_getSemaphore(uint8_t mode);
u8 mf_opendir(u8* path);
u8 mf_closedir(void);
u8 mf_readdir(void);
void mf_openText(char* fileName,uint8_t mode);
uint8_t mf_readText(uint8_t mode);
void mf_readConfigFile(char* fullFile);
FRESULT mf_load_path(char* path);
FRESULT mf_load_path_gen(void);
FRESULT mf_load_next_file(FIL *fp, DIR *dp, uint8_t fileNum);
FRESULT mf_read_to_cube(FIL *fp);
void readSDModeZero(void);
u32 mf_showfree(u8 *drv);
u8 mf_lseek(u32 offset);
u32 mf_tell(void);
u32 mf_size(void);
u8 mf_mkdir(u8*pname);
u8 mf_fmkfs(u8* path, u8 mode, u16 au);
u8 mf_unlink(u8 *pname);
u8 mf_rename(u8 *oldname, u8* newname);
void mf_getlabel(u8 *path);
void mf_setlabel(u8 *path);
void mf_gets(u16 size);
u8 mf_putc(u8 c);
u8 mf_puts(u8*c);
void mf_read_html(char* fullFile);
FRESULT mf_read_to_tune(void);
uint8_t ASCII_TO_HEX(char datM,char datS);
FRESULT musicOpenPath(char* path);
FRESULT load_next_music_file(FIL *fp, DIR *dp);
FRESULT load_specific_music_file(FIL *fp, char* fileName);
FRESULT open_wav_file(FIL *fp, char* path, char* fileName);
uint8_t mf_read_wav(FIL *fp, uint16_t area);
FRESULT load_next_wave_file(FIL *fp, DIR *dp);
FRESULT load_specific_wave_file(FIL *fp, char* fileName);
FRESULT load_next_sound_file(FIL *fp, DIR *dp, uint8_t fileNum, uint8_t *type, uint8_t load);
void playRandomSound(char *path);
void playShuffleSound(char *path);

#endif

