#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;
    INIT_LIST_HEAD(&ele->list);
    ele->value = strdup(s);
    if (!ele->value) {
        free(ele);
        return false;
    }
    list_add(&ele->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;
    ele->value = strdup(s);
    if (!ele->value) {
        free(ele);
        return false;
    }
    list_add_tail(&ele->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_first_entry(head, element_t, list);
    list_del(&target->list);

    if (sp) {
        size_t copy_size = strlen(target->value) < (bufsize - 1)
                               ? strlen(target->value)
                               : (bufsize - 1);
        strncpy(sp, target->value, copy_size);
        sp[copy_size] = '\0';
    }

    return target;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_last_entry(head, element_t, list);
    list_del(&target->list);

    if (sp) {
        size_t copy_size = strlen(target->value) < (bufsize - 1)
                               ? strlen(target->value)
                               : (bufsize - 1);
        strncpy(sp, target->value, copy_size);
        sp[copy_size] = '\0';
    }

    return target;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int size = 0;
    struct list_head *node;
    list_for_each (node, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    // 可以嘗試用快慢指標的形式做
    if (!head || list_empty(head))
        return false;

    struct list_head *first = head->next;
    struct list_head *second = head->prev;
    while ((first != second) && (first->next != second)) {
        first = first->next;
        second = second->prev;
    }
    element_t *node = list_entry(first, element_t, list);
    //將此中間節點轉換為element_t結構，以方便後續釋放內存
    list_del(first);
    free(node->value);
    free(node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // 可以嘗試用快慢指標的形式做
    if (!head || list_empty(head))
        return false;

    bool dup = false;
    element_t *cur, *tmp;
    list_for_each_entry_safe (cur, tmp, head, list) {
        if (&tmp->list != head && !strcmp(cur->value, tmp->value)) {
            list_del(&cur->list);
            free(cur->value);
            free(cur);
            dup = true;
        } else if (dup) {
            list_del(&cur->list);
            free(cur->value);
            free(cur);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *first, *second;
    list_for_each_safe (first, second, head) {
        //遍歷節點，每一次以兩個為一組交換
        if (second == head)
            break;
        first->prev->next = second;
        second->prev = first->prev;
        first->next = second->next;
        first->prev = second;
        second->next->prev = first;
        second->next = first;

        second = first->next;
        //更新下一對要處理的節點
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        node->next = node->prev;
        node->prev = safe;
    }
    node->next = node->prev;
    node->prev = safe;
    //最後一個節點的運作方式不太確定？doubly linked
    // list的最後一個節點的next應該要指向頭部
    //所以,倒過來應該指向倒數第二個節點，
    //此時的safe代表的第一個節點（頭部）？
    return;
}
/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head)
        return;
    int times = q_size(head) / k;
    struct list_head *tail;
    LIST_HEAD(tmp);
    LIST_HEAD(new_head);

    for (int i = 0; i < times; i++) {
        int j = 0;
        list_for_each (tail, head) {
            if (j >= k)
                break;
            j++;
        }
        list_cut_position(&tmp, head, tail->prev);
        q_reverse(&tmp);
        list_splice_tail_init(&tmp, &new_head);
    }
    list_splice_init(&new_head, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    struct list_head list_less, list_greater;
    element_t *pivot;
    element_t *item = NULL, *tmp = NULL;

    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, tmp, head, list) {
        if (descend == false) {
            if (strcmp(item->value, pivot->value) < 0)
                list_move_tail(&item->list, &list_less);
            else
                list_move_tail(&item->list, &list_greater);
        } else {
            if (strcmp(item->value, pivot->value) > 0)
                list_move_tail(&item->list, &list_less);
            else
                list_move_tail(&item->list, &list_greater);
        }
    }

    q_sort(&list_less, descend);
    q_sort(&list_greater, descend);

    list_add(&pivot->list, head);
    list_splice(&list_less, head);
    list_splice_tail(&list_greater, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *right = list_entry(head->prev, element_t, list);
    element_t *left = list_entry(head->prev->prev, element_t, list);
    while (&left->list != head) {
        if (strcmp(right->value, left->value) > 0) {
            left = list_entry(left->list.prev, element_t, list);
            right = list_entry(right->list.prev, element_t, list);
        } else {
            list_del(&left->list);
            free(left->value);
            free(left);
            left = list_entry(right->list.prev, element_t, list);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *right = list_entry(head->prev, element_t, list);
    element_t *left = list_entry(head->prev->prev, element_t, list);
    while (&left->list != head) {
        if (strcmp(right->value, left->value) < 0) {
            left = list_entry(left->list.prev, element_t, list);
            right = list_entry(right->list.prev, element_t, list);
        } else {
            list_del(&left->list);
            free(left->value);
            free(left);
            left = list_entry(right->list.prev, element_t, list);
        }
    }
    return q_size(head);
}

/* Helper function for q_merge(), merge two lists together */
int support_merge(struct list_head *l1, struct list_head *l2)
{
    if (!l1 || !l2)
        return 0;
    LIST_HEAD(tmp_head);
    while (!list_empty(l1) && !list_empty(l2)) {
        element_t *ele_1 = list_first_entry(l1, element_t, list);
        element_t *ele_2 = list_first_entry(l2, element_t, list);
        element_t *ele_min =
            strcmp(ele_1->value, ele_2->value) < 0 ? ele_1 : ele_2;
        list_move_tail(&ele_min->list, &tmp_head);
    }
    list_splice_tail_init(l1, &tmp_head);
    list_splice_tail_init(l2, &tmp_head);
    list_splice(&tmp_head, l1);
    return q_size(l1);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    else if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);
    int size = q_size(head);
    int count = (size % 2) ? size / 2 + 1 : size / 2;
    int queue_size = 0;
    for (int i = 0; i < count; i++) {
        queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
        queue_contex_t *second =
            list_entry(first->chain.next, queue_contex_t, chain);
        while (!list_empty(first->q) && !list_empty(second->q)) {
            queue_size = support_merge(first->q, second->q);
            list_move_tail(&second->chain, head);
            first = list_entry(first->chain.next, queue_contex_t, chain);
            second = list_entry(first->chain.next, queue_contex_t, chain);
        }
    }
    return queue_size;
}
