#ifndef _LIST_H_
#define _LIST_H_


typedef struct _CgList {

  /** Marks the beginning of a list */
  //BOOL headFlag;
  /** Pointer to the next list node */
  struct _CgList *prev;
  /** Pointer to the previous list node */
  struct _CgList *next;

} CgList;
typedef void (*CG_LIST_DESTRUCTORFUNC)(void *);

void dms_free(void *str);
void dms_list_remove(CgList *list);

void dms_list_add(CgList *listprev, CgList *list);
void dms_list_clear(CgList *headlist, CG_LIST_DESTRUCTORFUNC);
void dms_string_addvalue (char **str, const char *value);

#endif
