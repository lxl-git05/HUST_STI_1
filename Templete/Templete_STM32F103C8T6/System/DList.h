#ifndef __DLIST_H
#define __DLIST_H

#include <stdlib.h>

/*
	这里有两个链表的定义,第一个是普遍的,数据链表(存储数据),第二个是唯一的,是整个系统的控制节点,指向数据链的头和尾
*/

// ================== 双向链表定义 ==================
typedef struct DListNode
{
    struct DListNode *prev ;   // 前驱节点
    struct DListNode *next ;   // 后继节点

    void *data ;               // 数据指针，可存放任意类型
}DListNode;

// ================== 系统头结点双向链表定义 ==================
typedef struct
{
    DListNode *head ;          // 指向第一个节点
    DListNode *tail ;          // 指向最后一个节点
    int size ;                 // 节点数量
}DList;

// ================== 函数 ==================
void DList_Init(DList *list);
void DList_PushFront(DList *list , void *data);
void DList_PushBack(DList *list , void *data);
void DList_RemoveNode(DList *list, DListNode *node);
void DList_RemoveAt(DList *list , int index);
DListNode* DList_GetNode(DList *list , int index);
void* DList_Get(DList *list , int index);
void DList_Traverse(DList *list , void (*func)(void*));
void DList_TraverseReverse(DList *list , void (*func)(void*));
void DList_Clear(DList *list);

#endif

