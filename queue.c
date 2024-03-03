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

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
