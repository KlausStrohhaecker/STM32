/******************************************************************************/
/** @file		nl_coos.c
	@date		2015-01-30
    @author		[2013-07-07 DTZ]
*******************************************************************************/
#include "../application/coos.h"

#define COOS_MAX_TASKS 48  // max number of task the COOS should handle (memory size)

typedef struct
{
  void (*pTask)(void);  // pointer to the task
  int32_t countDown;    // delay (ticks) until the function will (next) be run
  int32_t period;       // interval (ticks) between subsequent run
  int32_t run;          // incremented by the scheduler when task is due to execute
} sTask;

static sTask coos_task_array[COOS_MAX_TASKS];  // array for the tasks

static volatile int32_t task_overflow       = 0;  // signals a task overflow => turn on warning led
static volatile int32_t check_task_overflow = 0;
static volatile uint8_t sleep               = 1;

/******************************************************************************/
/** @brief    	init everything with 0
*******************************************************************************/
void coosInit(void)
{
  uint8_t index;

  for (index = 0; index < COOS_MAX_TASKS; index++)
  {
    coosTaskDelete(index);
  }
}

/******************************************************************************/
/** @brief		Function to add tasks to the task list
				- periodic tasks
				- one time tasks
    @param[in]  taskName - name of the task (function) that should be registered
	@param[in]	phase - one time delay in sysTicks to generate a phase (offset)
	@param[in]	period - intervall in sysTicks between repeated execusions of
				the task
				0: execute only once
    @return		taskId - position in the taskArray
				-1: error
*******************************************************************************/
int32_t coosTaskAdd(void (*task_name)(), uint32_t phase, uint32_t period)
{
  uint8_t index = 0;

  while ((coos_task_array[index].pTask != 0) && (index < COOS_MAX_TASKS))  // check for space in the task array
  {
    index++;
  }

  if (index == COOS_MAX_TASKS)  // is the end of the task list accomplished?
  {
    return -1;  // task list is full: return error
  }

  /* there is a space in the taskArray - add task */
  coos_task_array[index].pTask     = task_name;
  coos_task_array[index].countDown = phase + 1;
  coos_task_array[index].period    = period;
  coos_task_array[index].run       = 0;

  return index;  // so task can be deleted
}

/******************************************************************************/
/** @brief
    @param[in]	taskIndex
    			number of the task (id)
    @return		 0  everything ok
    			-1	error: no task at this location, nothing to delete
*******************************************************************************/
int32_t coosTaskDelete(const uint8_t task_index)
{
  if (coos_task_array[task_index].pTask == 0)
  {
    return -1;  // error: no task at this location, nothing to delete
  }
  else
  {
    /* delete task */
    coos_task_array[task_index].pTask     = 0x0000;
    coos_task_array[task_index].countDown = 0;
    coos_task_array[task_index].period    = 0;
    coos_task_array[task_index].run       = 0;
    return 0;  // everything ok
  }
}

void COOS_GoToArtificialSleep(void)
{
  while (sleep == 1)
    ;
  sleep = 1;
}

/******************************************************************************/
/** @brief    	The dispatcher will run the registered tasks
    @param[]
    @return
*******************************************************************************/
void coosDispatch(void)
{
  //DBG_Pod(POD_2, ON);															// monitor the duration of the dispatch function

  uint8_t index;

  for (index = 0; index < COOS_MAX_TASKS; index++)  // run the next task (if one is ready)
  {
    if (coos_task_array[index].run > 0)
    {
      (*coos_task_array[index].pTask)();  // run the task
      coos_task_array[index].run--;       // decrease the run flag, so postponed tasks will also be handled

      if (coos_task_array[index].period == 0)  // if one shot task: remove from taskArray
      {
        coosTaskDelete(index);
      }
    }
  }

  //DBG_Pod(POD_2, OFF);														// monitor the duration of the dispatch function

  if (task_overflow == 0)  // no task overflow -> everything all right -> goto sleep
  {
    sleep = 1;
    check_task_overflow--;
    //COOS_GoToArtificialSleep();
  }
  else
  {
    task_overflow--;  // task overflow -> try to catch up -> go an other round
  }
}

/******************************************************************************/
/** @brief    	Calculates when a task is due to run and sets the run flag when
 	 	 	 	it is. It will not execute any taks!!!
	@note		This function must be called every sysTick
*******************************************************************************/
void coosUpdate(void)
{
  uint8_t index;
  sleep = 0;

#if 1  // check for task overrun
  if (check_task_overflow > 0)
  {
    task_overflow++;  // error: Dispatch() took longer than one time slot
                      //DBG_Led_Warning_On();
                      //coos_task_add(DBG_Led_Warning_Off, 20000, 0);							// stays on for 2.5 sec
  }
  else
  {
    check_task_overflow++;  // this flag must be reseted by Dispatch() before Dispatch is called again, otherwise: task overflow
  }
#endif

  for (index = 0; index < COOS_MAX_TASKS; index++)  // calculations are made in sysTicks
  {
    if (coos_task_array[index].pTask)  // check for registered task
    {
      coos_task_array[index].countDown--;

      if (coos_task_array[index].countDown <= 0)  // check if task is due to run / <0 for one shot tasks
      {
        coos_task_array[index].run++;  // yes, task is due to run -> increase run-flag
        if (coos_task_array[index].period >= 1)
        {  // schedule periodic task to run again
          coos_task_array[index].countDown = coos_task_array[index].period;
        }
      }
    }
  }
}
