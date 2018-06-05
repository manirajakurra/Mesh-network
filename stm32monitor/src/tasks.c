/*
*  FILE          : tasks.c
*  PROJECT       : stm32monitor
*  PROGRAMMER    : Manu Murali Student
*  FIRST VERSION : 2017-12-12
*  DESCRIPTION   :
*    This file contains task functions required to add new task, 
*    delete existing task, to schedule the task to be executed
*/
#include <stdio.h>
#include <stdint.h>
#include "common.h"
#define MAX_TASKS 10

typedef struct Task_s 
{
  	void (*f)(void *data); /* Task function */
  	void *data;  /* Private data pointer for this task */
} Task_t;

/* Process list */
Task_t tasks[MAX_TASKS];
int currentTask = 0;

// FUNCTION      : TaskAdd
//
// DESCRIPTION   :
//   This function will add new task to the existing tasklist
// PARAMETERS    :
//   void (*fn)(void *data), void *data
//
// RETURNS       :
//   int32_t
int32_t TaskAdd(void (*fn)(void *data), void *data)
{
 	/* Try to find an empty slot */
 	for(int i=0; i<MAX_TASKS; i++) 
	{
    		if(tasks[i].f == NULL) 
		{
      			tasks[i].f    = fn;
      			tasks[i].data = data;
      			return i;
    		}
  	}
  /* No slots available, return -­1 */
 	return -1;
}

// FUNCTION      : TaskKill
//
// DESCRIPTION   :
//   This function will delete task from the existing list
// PARAMETERS    :
//   int32_t id
//
// RETURNS       :
//   int32_t
int32_t TaskKill(int32_t id)
{
	tasks[id].f = NULL;
	tasks[id].data = NULL;
	return 1;
}

// FUNCTION      : TaskSwitcher
//
// DESCRIPTION   :
//   This function will execute each tasks based on the entry in 
//   the task list
// PARAMETERS    :
//   Nothing
//
// RETURNS       :
//   int32_t
int32_t TaskSwitcher(void)
{
	currentTask = TaskNext();
	if(currentTask< 0) 
	{
		return -1;
	}
	printf("task no:%d \n\r",currentTask);
	tasks[currentTask].f(tasks[currentTask].data);
	return 1;
}

// FUNCTION      : TaskSwitcher
//
// DESCRIPTION   :
//   This function will Find the next task to run 
//   
// PARAMETERS    :
//   Nothing
//
// RETURNS       :
//   int32_t
int32_t TaskNext(void)
{
  	int32_t i;
  	uint32_t count=0;
  
  	i = currentTask;
  	do 
	{
    		i = (i + 1) % MAX_TASKS;
    		count++;
  	} while((tasks[i].f == NULL) && (count <= MAX_TASKS));
 
	return (count <= MAX_TASKS) ? i : (-1);
}
 

