#include "DList.h"

// 初始化*系统*头结点链表
void DList_Init(DList *list)
{
    list->head = NULL ;	// 初始化,头尾为空
    list->tail = NULL ;
    list->size = 0 ;		// 初始化,数据为NULL
}

// 创建*数据*新节点
DListNode* DList_NewNode(void *data)
{
    DListNode *node = (DListNode*)malloc(sizeof(DListNode));	// 分配内存
    node->data = data;	// 分配数据
    node->prev = NULL;	// 头结点初始为空
    node->next = NULL;	// 尾节点初始为空
    return node;
}

// 头插法
void DList_PushFront(DList *list , void *data)
{
    DListNode *node = DList_NewNode(data);	// 新建数据链表

    node->next = list->head;								// 新结点的尾巴指向系统节点的头一个,也就是第一个节点(头插)

    if(list->head != NULL)
        list->head->prev = node;						// 第一个节点的头指向新节点(与上一句建立双向互联)
    else
        list->tail = node;  	// 如果原来是空链表

    list->head = node;				// 系统的头指向新节点
    list->size++;							// 数据长度+1
}

// 尾插法
void DList_PushBack(DList *list , void *data)
{
    DListNode *node = DList_NewNode(data);	// 新建数据节点

    node->prev = list->tail;								// 新节点的头指向系统的尾巴,也就是最后一个节点

    if(list->tail != NULL)									// 最后一个节点的尾巴指向新节点
        list->tail->next = node;
    else
        list->head = node;  // 空链表

    list->tail = node;			// 系统的尾巴指向新节点
    list->size++;						// 数据长度+1
}

// 删除节点(根据节点指针)
void DList_RemoveNode(DList *list, DListNode *node)
{
    if(node == NULL) return;

    if(node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if(node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    free(node);
    list->size--;
}

// 删除节点(根据索引,从0开始)
void DList_RemoveAt(DList *list , int index)
{
    if(index < 0 || index >= list->size) return;

    DListNode *p = list->head;
    for(int i=0 ; i<index ; i++)
        p = p->next;

    DList_RemoveNode(list , p);
}

// 查找节点(根据索引,从0开始)
DListNode* DList_GetNode(DList *list , int index)
{
    if(index < 0 || index >= list->size) return NULL;

    DListNode *p;
    if(index < list->size / 2)
    {
        p = list->head;  // 前半段从前找
        for(int i=0 ; i<index ; i++) p = p->next;
    }
    else
    {
        p = list->tail;  // 后半段从后找
        for(int i=list->size-1 ; i>index ; i--) p = p->prev;
    }
    return p;
}

// 遍历(正向)
void DList_Traverse(DList *list , void (*func)(void*))
{
    for(DListNode *p = list->head ; p != NULL ; p = p->next)
        func(p->data);
}

// 遍历(反向)
void DList_TraverseReverse(DList *list , void (*func)(void*))
{
    for(DListNode *p = list->tail ; p != NULL ; p = p->prev)
        func(p->data);
}

// 清空链表
void DList_Clear(DList *list)
{
    DListNode *p = list->head;
    while(p)
    {
        DListNode *next = p->next;
        free(p);
        p = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}
