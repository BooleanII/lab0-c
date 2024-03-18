#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty node */
element_t *e_new(char *s)
{
    element_t *node = malloc(sizeof(element_t));
    if (node) {
        node->value = (char *) malloc(sizeof(char) * (strlen(s) + 1));
        strncpy(node->value, s, strlen(s) + 1);
        INIT_LIST_HEAD(&node->list);
        return node;
    }
    return NULL;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head) {
        INIT_LIST_HEAD(head);
        return head;
    }
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *visit, *visit_next;
    list_for_each_entry_safe (visit, visit_next, head, list) {
        free(visit->value);
        free(visit);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = e_new(s);
    if (node) {
        list_add(&node->list, head);
        return true;
    }
    return false;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = e_new(s);
    if (node) {
        list_add_tail(&node->list, head);
        return true;
    }
    return false;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !sp || list_empty(head))
        return NULL;

    struct list_head *node = head->next;
    element_t *element = list_entry(node, element_t, list);
    size_t strsize = strlen(element->value) + 1;
    if (strsize <= bufsize)
        strncpy(sp, element->value, strsize + 1);
    else {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize] = '\0';
    }
    list_del(node);
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !sp || list_empty(head))
        return NULL;

    struct list_head *node = head->prev;
    element_t *element = list_entry(node, element_t, list);
    size_t strsize = strlen(element->value) + 1;
    if (strsize <= bufsize)
        strncpy(sp, element->value, strsize);
    else {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize] = '\0';
    }
    list_del(node);
    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *forward = head->next, *backward = head->prev;
    while (forward->next != backward && forward != backward) {
        forward = forward->next;
        backward = backward->prev;
    }
    element_t *element = list_entry(forward, element_t, list);
    list_del(forward);
    q_release_element(element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *first = head->next, *second = head->next->next;
    while (first != head && second != head) {
        list_move(first, second);

        first = first->next;
        second = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node, *safe, *tmp;
    list_for_each_safe (node, safe, head) {
        tmp = node->next;
        node->next = node->prev;
        node->prev = tmp;
    }
    tmp = head->next;
    head->next = head->prev;
    head->prev = tmp;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head) || k < 2)
        return;

    int i;
    struct list_head *cut_point, group, tmp;
    INIT_LIST_HEAD(&tmp);

    do {
        cut_point = head->next;
        for (i = 1; i < k && cut_point != head; i++)
            cut_point = cut_point->next;

        list_cut_position(&group, head,
                          (cut_point == head) ? cut_point->prev : cut_point);

        if (cut_point != head)
            q_reverse(&group);
        list_splice_tail(&group, &tmp);
    } while (!list_empty(head));
    list_splice(&tmp, head);
}

/*
** compare list_head struct content with descend option
** @descend: 0-ascend, 1-descend
*/
int compare(const struct list_head *a, const struct list_head *b, bool descend)
{
    if (a == b)
        return 0;

    int res = strcmp(list_entry(a, element_t, list)->value,
                     list_entry(b, element_t, list)->value);

    if (descend)
        return -res;

    return res;
}

/*
** Start of implementation of Timsort
*/

struct pair {
    struct list_head *head, *next;
};

typedef int (*list_cmp_func_t)(const struct list_head *,
                               const struct list_head *,
                               bool);

size_t run_size(struct list_head *head)
{
    if (!head)
        return 0;
    if (!head->next)
        return 1;
    return (size_t) (head->next->prev);
}


struct pair find_run(struct list_head *list, list_cmp_func_t cmp, bool descend)
{
    size_t len = 1;
    struct list_head *next = list->next, *head = list;
    struct pair result;

    if (!next) {
        result.head = head, result.next = next;
        return result;
    }

    if (cmp(list, next, descend) > 0) {
        /* decending run, also reverse the list */
        struct list_head *prev = NULL;
        do {
            len++;
            list->next = prev;
            prev = list;
            list = next;
            next = list->next;
            head = list;
        } while (next && cmp(list, next, descend) > 0);
        list->next = prev;
    } else {
        do {
            len++;
            list = next;
            next = list->next;
        } while (next && cmp(list, next, descend) <= 0);
        list->next = NULL;
    }
    head->prev = NULL;
    head->next->prev = (struct list_head *) len;
    result.head = head, result.next = next;
    return result;
}

void build_prev_link(struct list_head *head,
                     struct list_head *tail,
                     struct list_head *list)
{
    tail->next = list;
    do {
        list->prev = tail;
        tail = list;
        list = list->next;
    } while (list);

    /* The final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

struct list_head *merge(struct list_head *a,
                        struct list_head *b,
                        list_cmp_func_t cmp,
                        bool descend)
{
    struct list_head *head = NULL;
    struct list_head **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b, descend) <= 0) {
            *tail = a;
            tail = &(a->next);
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &(b->next);
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

struct list_head *merge_at(int *stk_size,
                           struct list_head *at,
                           list_cmp_func_t cmp,
                           bool descend)
{
    size_t len = run_size(at) + run_size(at->prev);
    struct list_head *prev = at->prev->prev;
    struct list_head *list = merge(at->prev, at, cmp, descend);
    list->prev = prev;
    list->next->prev = (struct list_head *) len;
    --(*stk_size);
    return list;
}

struct list_head *merge_force_collapse(int *stk_size,
                                       struct list_head *stack,
                                       list_cmp_func_t cmp,
                                       bool descend)
{
    while (*stk_size >= 3) {
        if (run_size(stack->prev->prev) < run_size(stack)) {
            stack->prev = merge_at(stk_size, stack->prev, cmp, descend);
        } else {
            stack = merge_at(stk_size, stack, cmp, descend);
        }
    }
    return stack;
}

struct list_head *merge_collapse(int *stk_size,
                                 struct list_head *stack,
                                 list_cmp_func_t cmp,
                                 bool descend)
{
    int n;
    while ((n = *stk_size) >= 2) {
        if ((n >= 3 && run_size(stack->prev->prev) <=
                           run_size(stack->prev) + run_size(stack)) ||
            (n >= 4 &&
             run_size(stack->prev->prev->prev) <=
                 run_size(stack->prev->prev) + run_size(stack->prev))) {
            if (run_size(stack->prev->prev) < run_size(stack)) {
                stack->prev = merge_at(stk_size, stack->prev, cmp, descend);
            } else {
                stack = merge_at(stk_size, stack, cmp, descend);
            }
        } else if (run_size(stack->prev) <= run_size(stack)) {
            stack = merge_at(stk_size, stack, cmp, descend);
        } else {
            break;
        }
    }

    return stack;
}

void merge_final(struct list_head *head,
                 struct list_head *a,
                 struct list_head *b,
                 list_cmp_func_t cmp,
                 bool descend)
{
    struct list_head *tail = head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b, descend) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    build_prev_link(head, tail, b);
}

void q_timsort(struct list_head *head, list_cmp_func_t cmp, bool descend)
{
    int stk_size = 0;

    struct list_head *list = head->next, *stack = NULL;
    if (head == head->prev)
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        /* Find next run */
        struct pair result = find_run(list, cmp, descend);
        result.head->prev = stack;
        stack = result.head;
        list = result.next;
        stk_size++;
        stack = merge_collapse(&stk_size, stack, cmp, descend);
    } while (list);

    /* End of input; merge together all the runs. */
    stack = merge_force_collapse(&stk_size, stack, cmp, descend);

    /* The final merge; rebuild prev links */
    struct list_head *stk0 = stack, *stk1 = stk0->prev;
    while (stk1 && stk1->prev)
        stk0 = stk0->prev, stk1 = stk1->prev;
    if (stk_size <= 1) {
        build_prev_link(head, head, stk0);
        return;
    }
    merge_final(head, stk1, stk0, cmp, descend);
}

/*
** End of implementation of Timsort
*/

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    q_timsort(head, compare, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    int count = 1;
    struct list_head *safe, *second, *first = head->prev;

    for (second = first->prev, safe = second->prev; second != head;
         second = safe, safe = second->prev) {
        if (strcmp(list_entry(second, element_t, list)->value,
                   list_entry(first, element_t, list)->value) <= 0) {
            count++;
            first = second;
        } else {
            list_del(second);
            q_release_element(list_entry(second, element_t, list));
        }
    }
    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    int count = 1;
    struct list_head *safe, *second, *first = head->prev;

    for (second = first->prev, safe = second->prev; second != head;
         second = safe, safe = second->prev) {
        if (strcmp(list_entry(second, element_t, list)->value,
                   list_entry(first, element_t, list)->value) >= 0) {
            count++;
            first = second;
        } else {
            list_del(second);
            q_release_element(list_entry(second, element_t, list));
        }
    }
    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
