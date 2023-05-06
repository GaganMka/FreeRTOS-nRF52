/*
  Create an Event Group
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 8: Event Groups 
*/

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "nrf_drv_clock.h"

#define EVT_GROUP_BIT_0 (1UL << 0UL)
#define EVT_GROUP_BIT_1 (1UL << 1UL)

EventGroupHandle_t evt_group;

void evt_group_setting_task(void* pvParameters)
{
  printf("%s", (char*)pvParameters);  
  const TickType_t k_wait = pdMS_TO_TICKS(200);
  while(true)
  {
    xEventGroupSetBits(evt_group, EVT_GROUP_BIT_0);
    vTaskDelay(k_wait);

    xEventGroupSetBits(evt_group, EVT_GROUP_BIT_1);
    vTaskDelay(k_wait);
  }
}

void evt_group_reading_task(void* pvParameters)
{
  printf("%s", (char*)pvParameters); 
  
  EventBits_t evt_bits;
  const EventBits_t bits_to_wait = EVT_GROUP_BIT_0 | EVT_GROUP_BIT_1;
  while(true)
  {
    evt_bits = xEventGroupWaitBits(evt_group,       // event group handle
                                   bits_to_wait,    // bits to test for task unblock
                                   pdTRUE,          // clear event bits on exit if unblock condition is met
                                   pdFALSE,         // don't wait for all the bits
                                   portMAX_DELAY    // don't time out
                                   );
                                  
    if((evt_bits & EVT_GROUP_BIT_0) != 0)
    {
      printf("Event Group Bit 0 set\r\n");
    }

    if((evt_bits & EVT_GROUP_BIT_1) != 0)
    {
      printf("Event Group Bit 1 set\r\n");
    }
    // no need to add any task delay because the event bits will be cleared
    // as of 3rd argument in xEventGroupWaitBits and the task will be moved to
    // blocked state
  }
}

int main(void)
{
  BaseType_t task_err;
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  // function returns the handle to event group if created
  evt_group = xEventGroupCreate();

  if(evt_group == NULL)
  {
    printf("EG create fail\r\n");
    return -1;
  }
  
  task_err = xTaskCreate(
                          evt_group_setting_task,         // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          "Evt Group Setting Task\r\n",   // task arguments
                          1,                              // task priority
                          NULL
                        );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task 1 create fail\r\n");
    return -1;
  }

  task_err = xTaskCreate(
                        evt_group_reading_task,         // pointer to the task function
                        "Task1",                        // task name mainly for debugging
                        configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                        "Evt Group Reading Task\r\n",   // task arguments
                        1,                              // task priority
                        NULL
                      );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task 2 create fail\r\n");
    return -1;
  }

  /* Activate deep sleep mode */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Start FreeRTOS scheduler. */
  vTaskStartScheduler();

  while (true) //---------
  {
      /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
       * in vTaskStartScheduler function. */
  }
}