#ifndef LIST_H_
#define LIST_H_

#include <stddef.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) ); })

struct list_head
{
    struct list_head *next, *prev;
};

static inline void list_init(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void list_insert(struct list_head *list, struct list_head *node)
{
    struct list_head *prev = list->prev;

    node->next = list;
    node->prev = list->prev;
    list->prev = node;
    prev->next = node;
}

static inline int list_empty(struct list_head *list)
{
    return list == list->next;
}

static inline void list_join(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_remove(struct list_head *node)
{
    list_join(node->prev, node->next);
    node->next = NULL;
    node->prev = NULL;
}

static inline int list_node_is_last(struct list_head *list)
{
    return list->next == list->prev;
}

static inline int list_same_node(struct list_head *node1, struct list_head *node2)
{
    return node1 == node2;
}

#define list_first_entry(ptr, type, member) \
    container_of((ptr)->next, type, member)

#define list_next_entry(pos, member) \
    container_of((pos)->member.next, typeof(*(pos)), member)

#define list_entry_is_head(pos, head, member) \
    (&(pos)->member == (head))

#define list_for_each_it(var, list)    \
    for ((var) = (list)->next;         \
         !list_same_node((var), list); \
         (var) = (var)->next)

#define list_for_each(var, head, member)                         \
    for ((var) = list_first_entry(head, typeof(*(var)), member); \
         !list_entry_is_head(var, head, member);                 \
         (var) = list_next_entry(var, member))

#define list_for_each_safe_it(var, n, list)       \
    for ((var) = (list)->next, (n) = (var)->next; \
         !list_same_node((var), list);            \
         (var) = (n), (n) = (var)->next)

#define list_for_each_safe(var, head, member)                           \
    typeof((var)) var##__safe;                                          \
    for ((var)        = list_first_entry(head, typeof(*(var)), member), \
        (var##__safe) = list_next_entry(var, member);                   \
         !list_entry_is_head(var, head, member);                        \
         (var) = (var##__safe), (var##__safe) = list_next_entry(var##__safe, member))

static inline int list_length(struct list_head *list)
{
    struct list_head *i;
    int len = 0;

    list_for_each_it(i, list)
    {
        len++;
    }

    return len;
}


#endif // LIST_H_
