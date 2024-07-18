#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "ble_scan_adv_list.h"
#include "bsp_msg.h"

advLinkedList* advListHead = NULL;
int currentStartIndex = 0;
int scrollSteps = 0;
int totalNodes = 0;

advLinkedList* createAdvLinkedList(void) {
	advLinkedList* list = (advLinkedList*)malloc(sizeof(advLinkedList));
	if(NULL == list) return NULL;
	list->head = NULL;
	list->tail = NULL;
	totalNodes = 0;
	currentStartIndex = 0;
	scrollSteps = 0;
	advListHead = NULL;
	return list;
}
extern size_t xPortGetFreeHeapSize( void );//define at heap_4.c
int addAdvNode(advLinkedList* list, char* name, uint8_t* address,int8_t rssi) {
	advNode* currentNode = list->head;
	  // 创建并添加新节点
	if(4096 >= xPortGetFreeHeapSize()) return 1;

	if(!list || !address) return -1;
	advNode* newNode = (advNode*)malloc(sizeof(advNode));
	if( !newNode) return -2;
	
	while (currentNode != NULL) {
		if (memcmp(currentNode->address, address, 6) == 0) {
		  // 广播地址已存在，不将新节点添加到链表中
			free(newNode);
			return 2;
		}
		currentNode = currentNode->next;
	}

	strcpy(newNode->name, name);
	memcpy(newNode->address, address, 6);
	newNode->rssi = rssi;
	newNode->next = NULL;
    totalNodes += 1;
	//printf("scan:%d\n",totalNodes);
	if (list->head == NULL) {
		list->head = newNode;
		list->tail = newNode;

	} else {
		list->tail->next = newNode;
		list->tail = newNode;

	}
	UserQue_SendMsg(USER_MSG_BLE_REFRESH,NULL,0);
	return 0;
}

void freeAdvLinkedList(advLinkedList* list) {
    advNode* currentNode = list->head;
    advNode* nextNode;

    while (currentNode != NULL) {
        nextNode = currentNode->next;
		currentNode->next = NULL;
        free(currentNode);
        currentNode = nextNode;
    }

    list->head = NULL; 
    list->tail = NULL;
    totalNodes = 0;
    free(list);

	list = NULL;
}


// 获取特定索引位置的节点
advNode* getNodeAtIndex(advLinkedList* list,int index) {
	if(!list) return (advNode*)NULL;
	advNode* currentNode = list->head;
	int count = 0;

	// 移动到指定索引位置
	while (currentNode != NULL && count < index) {
		currentNode = currentNode->next;
		count++;
	}

	return currentNode;
}

// 打印蓝牙列表节点信息
static void printAdvLinkedList(advNode* startNode) {
    advNode* currentNode = startNode;
	if(!currentNode) return;
	for (int i = 0; i < 4; i++) {
	if (currentNode != NULL) {
	  printf("Name: %s, Address: ", currentNode->name);
	  for (int j = 0; j < 6; j++) {
		printf("%02X ", currentNode->address[j]);
	  }
	  printf("\n");

	  currentNode = currentNode->next;
	} else 
	{
	  printf("No more nodes\n");
	  break;
	}
	}
}


void scrollAdvLinkedList(advLinkedList* list,uint8_t showNums, int steps,f_list_printf fListPrintf) {
	if(!list) return;
	currentStartIndex += steps;
	if (currentStartIndex < 0) {
	currentStartIndex = 0;
	} else if (currentStartIndex > (totalNodes - 1)) {
	currentStartIndex = totalNodes - 1;
	}

	advNode* newStartNode = getNodeAtIndex(list,currentStartIndex);


	fListPrintf(newStartNode,showNums,totalNodes);
}

void extractLastCharacters(const char *src, int numChars, char *result) {
    int length = strlen(src);
    if (length <= numChars) {
        strcpy(result+4, src);
    } else {
        strcpy(result+4, src + (length - numChars));
    }
}