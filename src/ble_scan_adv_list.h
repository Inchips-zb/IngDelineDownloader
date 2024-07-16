#ifndef __BLE_SCAN_ADV_LIST_H__
#define __BLE_SCAN_ADV_LIST_H__

#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include "ingsoc.h"

typedef struct advNode {
  char name[32];
  uint8_t address[6];
  int8_t rssi;
  struct advNode* next;
} advNode;

typedef struct advLinkedList {
  advNode* head;
  advNode* tail;
} advLinkedList;

typedef void (* f_list_printf)(advNode* startNode,uint8_t showNums,uint16_t total);

extern advLinkedList* advListHead ;

extern  advLinkedList* createAdvLinkedList(void);
extern  int addAdvNode(advLinkedList* list, char* name, uint8_t* address,int8_t rssi);
extern  void freeAdvLinkedList(advLinkedList* list);
extern  void scrollAdvLinkedList(advLinkedList* list,uint8_t showNums,int steps,f_list_printf fListPrintf) ;
extern  void extractLastCharacters(const char *src, int numChars, char *result);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif
