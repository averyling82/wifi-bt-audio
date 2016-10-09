#ifndef _WLAN_LIST_H_
#define _WLAN_LIST_H_

#include "wlan_base_type.h"

typedef struct list_head
{
    struct list_head *next, *prev;
} LIST_HEAD_T;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define _INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
}while (0)

/*
 * Insert a new entry between two known consecutive entries.
 * This is only for internal list manipulation where we know the prev/next entries already!
 */
static __inline void __list_add(struct list_head * new, struct list_head * prev, struct list_head * next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/*
 * list_add - add a new entry
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static __inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/*
 * list_add_tail - add a new entry
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static __inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries point32 to each other.
 * This is only for internal list manipulation where we know the prev/next entries already!
 */
static __inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

/*
 * list_del - deletes entry from list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static __inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 */
static __inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    _INIT_LIST_HEAD(entry);
}

/*
 * list_empty - tests whether a list is empty
 */
static __inline int32 list_empty(struct list_head *head)
{
    if(head->next == head)
        return 1;
    else
        return 0;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline void list_splice(struct list_head *list, struct list_head *head)
{
    struct list_head *first = list->next;

    if (first != list)
    {
        struct list_head *last = list->prev;
        struct list_head *at = head->next;
        first->prev = head;
        head->next = first;
        last->next = at;
        at->prev = last;
    }
}

/**
 * list_entry - get the struct for this entry
 * @ptr: the &struct list_head pointer.
 * @type: the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
    ((type *)((uint8 *)(ptr)-(uint32)(&((type *)0)->member)))

/**
 * list_for_each - iterate over a list
 * @pos: the &struct list_head to use as a loop counter.
 * @head: the head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)




#endif
