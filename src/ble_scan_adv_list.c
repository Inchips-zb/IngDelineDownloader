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
	if(NULL == list)
		return NULL;
	list->head = NULL;
	list->tail = NULL;
	totalNodes = 0;
	currentStartIndex = 0;
	scrollSteps = 0;
	return list;
}

int addAdvNode(advLinkedList* list, char* name, uint8_t* address,int8_t rssi) {
	advNode* currentNode = list->head;
	  // 创建并添加新节点
	advNode* newNode = (advNode*)malloc(sizeof(advNode));
	if(!list || !address || !newNode) return 1;
	
	while (currentNode != NULL) {
		if (memcmp(currentNode->address, address, 6) == 0) {
		  // 广播地址已存在，不将新节点添加到链表中
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
    free(currentNode);
    currentNode = nextNode;
  }
  totalNodes = 0;
  free(list);
}


// 获取特定索引位置的节点
advNode* getNodeAtIndex(advLinkedList* list,int index) {
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
  for (int i = 0; i < 4; i++) {
    if (currentNode != NULL) {
      printf("Name: %s, Address: ", currentNode->name);
      for (int j = 0; j < 6; j++) {
        printf("%02X ", currentNode->address[j]);
      }
      printf("\n");

      currentNode = currentNode->next;
    } else {
      printf("No more nodes\n");
      break;
    }
  }
}

// 滚动蓝牙列表的节点
void scrollAdvLinkedList(advLinkedList* list,uint8_t showNums, int steps,f_list_printf fListPrintf) {
  currentStartIndex += steps;
  // 边界检查，确保起始索引不超出蓝牙列表的范围
  if (currentStartIndex < 0) {
    currentStartIndex = 0;
  } else if (currentStartIndex > (totalNodes - 1)) {
    currentStartIndex = totalNodes - 1;
  }
  // 获取新的起始节点
  advNode* newStartNode = getNodeAtIndex(list,currentStartIndex);

  // 打印滚动后的蓝牙列表节点信息
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