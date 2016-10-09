
#include "ap6181wifi.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "fs.h"
#include "fsdata.h"
#include <string.h>
#include <opt.h>
#include "filedevice.h"
#ifdef __WICE_HTTP_SERVER__
//#pragma arm section code = "lwipcode", rodata = "lwipcode", rwdata = "lwipdata", zidata = "lwipdata"

extern void * FileSysHDC;

#ifdef WEB_ENABLE
#ifdef USE_LWIP
#if LWIP_HTTPD
/*-----------------------------------------------------------------------------------*/
/* Define the number of open files that we can support. */
#ifndef LWIP_MAX_OPEN_FILES
#define LWIP_MAX_OPEN_FILES     10
#endif

/* Define the file system memory allocation structure. */
struct fs_table {
  struct fs_file file;
  u8_t inuse;
};

/* Allocate file system memory */
struct fs_table fs_memory[LWIP_MAX_OPEN_FILES];

#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
#endif /* LWIP_HTTPD_CUSTOM_FILES */

/*-----------------------------------------------------------------------------------*/
static struct fs_file *
fs_malloc(void)
{
#if 0
  int i;
  for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
    if(fs_memory[i].inuse == 0) {
      fs_memory[i].inuse = 1;
      return(&fs_memory[i].file);
    }
  }
#endif
  return(NULL);

}

/*-----------------------------------------------------------------------------------*/
static void
fs_free(struct fs_file *file)
{
    if(file == NULL)
        return;

    FileDev_CloseFile(file->hFile);
    //printf("2222222222 normal hfileDev close 222222222222\n");
    //rkos_memory_free(file->data);
    rkos_memory_free(file);
    return;
}
 /*-----------------------------------------------------------------------------------*/
struct fs_file *
fs_open(char *name)
{
    struct fs_file * file;
    FILE_ATTR stFileAttr;
    rk_err_t ret;
    char *p = name;
    uint8 i=0;
    uint8 len = 0;

    stFileAttr.FileSize = 0;

    file  = (struct fs_file *)rkos_memory_malloc(sizeof(struct fs_file));
    if(file == NULL)
    {
        printf("file malloc fail\n");
        return NULL;
    }
    rkos_memset(file, 0, sizeof(struct fs_file));

    stFileAttr.Path = L"C:\\";
    if((name[0] == '/') &&  (name[1] == 0))
    {
        stFileAttr.FileName = L"rkos control.htm";
        //stFileAttr.FileName = L"404.html";
    }
    else
    {
        len = strlen(name+1);
        i = len;
        while(i--)
        {
            if(*(p+i) == '/')
                *(p+i) = '\\';
        }
        printf("name = %s\n", name+1);
        stFileAttr.FileName = (uint16 *)rkos_memory_malloc(len+len + 2);
        if(stFileAttr.FileName == NULL)
        {
            printf("stFileAttr.FileName malloc fail\n");
            goto err;
        }
        for(i=0; i<len; i++)
            *(stFileAttr.FileName+i) = *(name+1+i);

        *(stFileAttr.FileName+i) = 0x00;
    }

    file->hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
    if((rk_err_t)file->hFile <= 0)
    {
        rk_print_string("File Open Failure\n");
        goto err;
    }
    rk_printf("!!!!stFileAttr.FileSize = %ld!!!\n",stFileAttr.FileSize);
    file->len = stFileAttr.FileSize;
    file->pextension = NULL;
    file->index = 0;
    file->http_header_included = 0;
    file->data = NULL;
    if(len > 0)
        rkos_memory_free(stFileAttr.FileName);
#if 0
    file->data = (char *)rkos_memory_malloc(1050);
    if(file->data == NULL)
    {
        printf("file->data malloc fail \n");
        FileDev_CloseFile(file->hFile);
        goto err;

    }
    rkos_memset(file->data, 0, 1024);
#endif
    return file;

err:
    rkos_memory_free(file);
    return NULL;
}

/*-----------------------------------------------------------------------------------*/
void
fs_close(struct fs_file *file)
{
#if LWIP_HTTPD_CUSTOM_FILES
  if (file->is_custom_file) {
    fs_close_custom(file);
  }
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#if LWIP_HTTPD_FILE_STATE
  fs_state_free(file, file->state);
#endif /* #if LWIP_HTTPD_FILE_STATE */
  fs_free(file);
}
/*-----------------------------------------------------------------------------------*/
int
fs_read(struct fs_file *file, char *buffer, int count)
{
    int read;
    uint8 i=0;
    uint8 j=0;
    uint16 len = 0;

    if(count < 1024)
    {
        printf("buf count too small\n");
        return -1;
    }
    //printf("file->index = %d,file->len = %d\n",file->index, file->len);
    if(file->index == file->len)
        return -1;

    read = file->len - file->index;
    if(read > 1024)
    {
        read = 1024;
    }

    if(FileDev_ReadFile(file->hFile, buffer, 1024) != 1024)
    {
          //rkos_memory_free(file->data);
         // printf("filedev read file fail\n");
    }
    //debug_hex(buffer, 1024, 16);
    file->index += read;
    return read;
#if 0
  int read;

  if(file->index == file->len) {
    return -1;
  }

  read = file->len - file->index;
  if(read > count) {
    read = count;
  }

  MEMCPY(buffer, (file->data + file->index), read);
  file->index += read;

  return(read);
#endif
}
/*-----------------------------------------------------------------------------------*/
int fs_bytes_left(struct fs_file *file)
{
  return file->len - file->index;
}
#endif
#endif
#endif
//#pragma arm section code
#endif
