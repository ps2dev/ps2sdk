/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Simple list support.
*/

#ifndef DRV_LIST_H
#define DRV_LIST_H

typedef struct _list {
	struct _list *next;
	struct _list *prev;
} list_t;

#define LIST_INIT(name) { &(name), &(name) }

static inline int list_empty(void *l)
{
	list_t *list = (list_t *)l;

	return list->next == list && list->prev == list;
}

/* Insert an item after the given list.  */
static inline void list_insert(void *l, void *i)
{
	list_t *list = (list_t *)l, *item = (list_t *)i;

	item->prev = list;
	item->next = list->next;
	list->next->prev = item;
	list->next = item;
}

/* Remove the item from the list and return the item.  */
static inline list_t *list_remove(void *i)
{
	list_t *item = (list_t *)i;

	item->prev->next = item->next;
	item->next->prev = item->prev;
	return item;
}


/* Iterate over a list.  Dir is 'next' to iterate forward and 'prev' to
   iterate in reverse.  */
#define list_for_each(dir, pos, head)	\
	for (pos = (head)->dir; pos != (head); pos = pos->dir)

#endif /* DRV_LIST_H */
