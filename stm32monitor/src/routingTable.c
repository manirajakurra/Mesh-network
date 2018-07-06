/* my_main.c: main file for monitor */
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "nrf24l01_lib.h"
#include "common.h"
#include "main.h"
#include "malloc.h"



extern routeTable *pHead;
extern routeTable *newNode;

void addToTable(routeTable **pHead, uint8_t NodeID, uint8_t Count, uint8_t pathSourceID)
{
	uint8_t nodeAlreadyPresent = 0;
	routeTable *tempNode;
	tempNode = *pHead;
	
	if(*pHead == NULL)
	{
		// allocate memory to new node
		newNode = (routeTable *)malloc(sizeof(routeTable));
		newNode->NodeID = NodeID;
		newNode->Count = Count;
		newNode->pathSourceID = pathSourceID;
		newNode->ActiveStatus = ACTIVE;
		newNode->NextNode = NULL;
		*pHead = newNode;
	}
	else
	{
		while(tempNode!= NULL)
		{
			if(tempNode->NodeID == NodeID)
			{
				nodeAlreadyPresent = 1;

				if(tempNode->ActiveStatus == ACTIVE)
				{
					if(tempNode->Count > Count)
					{
						//least distant path found, update the info
						tempNode->Count = Count;
						tempNode->pathSourceID = pathSourceID;
						break;
					}
					else
					{
						//already a least distant path exists no need to update
						break;
					}
				}
				else
				{
					tempNode->ActiveStatus = ACTIVE;
					tempNode->Count = Count;
					tempNode->pathSourceID = pathSourceID;
					break;
				}	
			}
			else
			{
				tempNode = tempNode->NextNode;
			}
		}
		if(!nodeAlreadyPresent)
		{
			//Enter new node info
			newNode = (routeTable *)malloc(sizeof(routeTable));
			newNode->NodeID = NodeID;
			newNode->Count = Count;
			newNode->pathSourceID = pathSourceID;
			newNode->ActiveStatus = ACTIVE;
			newNode->NextNode = *pHead;
			*pHead = newNode;
		}
	}

}


void printTable(routeTable *pHead)
{

	routeTable *tempNode;
	tempNode = pHead;

	if(tempNode == NULL)
	{
		printf("\n\n\n\r---No Nodes in Vicinity---\n\n\r");
	}
	else
	{
		printf("\n\n\r--------------------Items in Routing Table are-------------------\n\n\n\r");

		while (tempNode != NULL)
		{
			
			printf("%d  ",tempNode->NodeID);
			printf("%d  ",tempNode->Count);
			printf("%d  ",tempNode->pathSourceID);
			printf("%d  \n\r",tempNode->ActiveStatus);
			tempNode = tempNode->NextNode;
			
		}
		printf("\n\n\r-----------------------------------------------------------------\n\n\n\r");
	}
}

void changeNeighborNodeStatus(routeTable *pHead)
{

  	routeTable *tempNode;
	tempNode = pHead;
	while(tempNode != NULL)
	{
		tempNode->ActiveStatus = INACTIVE;
		tempNode = tempNode->NextNode;
	}

}


void deleteInActiveNode(routeTable *pHead)
{
	routeTable *tempNode;
	routeTable *prevNode;

	prevNode = pHead;
	tempNode = prevNode->NextNode;

        while((pHead->ActiveStatus == INACTIVE) && (pHead != NULL))
	{
		pHead = pHead->NextNode;
		free(prevNode);
		prevNode = pHead;

		if(prevNode->NextNode != NULL)
		{
			tempNode = prevNode->NextNode;
			
		}
		else
		{	
			tempNode = NULL;
		}
	}


	
	while(tempNode != NULL)
	{
		if(tempNode->ActiveStatus == INACTIVE)
		{
			tempNode = tempNode->NextNode;
			free(prevNode->NextNode);
			prevNode->NextNode = NULL;
		}
		else
		{
			prevNode->NextNode = tempNode;
			prevNode = tempNode;
			tempNode = tempNode->NextNode;
		}
	}
}


uint8_t packBeacon(uint8_t *beacon, routeTable *pHead)
{
	uint8_t i = 0;
	routeTable *tempNode;
	
	tempNode = pHead;

	*(beacon) = 0x06;
	*(beacon + 1) = NODE_ID;
	for(i = 2; tempNode != NULL; i+=2)
	{
		printf("Not NULL\n\n\r");
		*(beacon + i) = tempNode->NodeID;
		*(beacon + i + 1) = tempNode->Count;
		tempNode = tempNode->NextNode;
	}
	return(i);
}


void extractNeighborNodeInfo(uint8_t *rxData, routeTable **pHead)
{

 uint8_t i = 0;
 uint8_t sID = 0;

 sID = rxData[1];
 
 addToTable(pHead, rxData[1], 1, sID);

 for(i = 2;i < 4;i+=2)
 {
	if(rxData[i] != 0 && rxData[i] != NODE_ID)
	addToTable(pHead, rxData[i], (rxData[i+1]) + 1, sID);
 }

}

ParserReturnVal_t sendBeacon(int mode)
{
  	//uint32_t val;
	uint8_t beaconLen = 0;
  
  	if(mode != CMD_INTERACTIVE) return CmdReturnOk;

  	uint8_t beaconPayload[32] = {0};
  	beaconLen = packBeacon(beaconPayload, pHead);
	
	//deleteInActiveNode(pHead);
	//changeNeighborNodeStatus(pHead);
	txMode(beaconPayload, beaconLen);  
	return CmdReturnOk;

}

ADD_CMD("BroadcastBeacon",sendBeacon,"                Broadcast Beacon")


ParserReturnVal_t printRouteTable(int mode)
{
  	//uint32_t val;
  
  	if(mode != CMD_INTERACTIVE) return CmdReturnOk;
	printTable(pHead);
	
	return CmdReturnOk;

}

ADD_CMD("showRouteTable",printRouteTable,"                Print the route table")



    
