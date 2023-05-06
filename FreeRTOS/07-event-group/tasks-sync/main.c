/*
  Use Event Group for Tasks Synchronization
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 8: 8.4 Task Synchronization Using an Event Group
*/

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "nrf_drv_clock.h"
#include "stdlib.h"

#define TASK1_EVT_GROUP_BIT   (1UL << 0UL)
#define TASK2_EVT_GROUP_BIT   (1UL << 1UL)
#define TASK3_EVT_GROUP_BIT   (1UL << 2UL)

static const TickType_t k_max_delay = pdMS_TO_TICKS(2000);
static const TickType_t k_min_delay = pdMS_TO_TICKS(200);

EventGroupHandle_t evt_group;
static const EventBits_t all_sync_bits = TASK1_EVT_GROUP_BIT | TASK2_EVT_GROUP_BIT | TASK3_EVT_GROUP_BIT;

void task1_function(void* pvParameters)
{
  printf("Task 1 started\r\n");  
  TickType_t wait;

  EventBits_t sync_bit = (EventBits_t)pvParameters;
  
  while(true)
  {
    wait = (rand() % k_max_delay) + k_min_delay;
    vTaskDelay(wait);
    printf("Task 1 reached SYNC point\r\n");
    xEventGroupSync(evt_group,        // event group handle
                    sync_bit,         // the event bit used by this task to show SYNC
                    all_sync_bits,    // the bits to wait for 
                    portMAX_DELAY);   // wait indefinitely
    printf("Task 1 exited SYNC\r\n");
  }
}

void task2_function(void* pvParameters)
{
  printf("Task 2 started\r\n");  
  TickType_t wait;

  EventBits_t sync_bit = (EventBits_t)pvParameters;
  
  while(true)
  {
    wait = (rand() % k_max_delay) + k_min_delay;
    vTaskDelay(wait);
    printf("Task 2 reached SYNC point\r\n");
    xEventGroupSync(evt_group,        // event group handle
                    sync_bit,         // the event bit used by this task to show SYNC
                    all_sync_bits,    // the bits to wait for 
                    portMAX_DELAY);   // wait indefinitely
    printf("Task 2 exited SYNC\r\n");
  }
}

void task3_function(void* pvParameters)
{
  printf("Task 3 started\r\n");  
  TickType_t wait;

  EventBits_t sync_bit = (EventBits_t)pvParameters;
  
  while(true)
  {
    wait = (rand() % k_max_delay) + k_min_delay;
    vTaskDelay(wait);
    printf("Task 3 reached SYNC point\r\n");
    xEventGroupSync(evt_group,        // event group handle
                    sync_bit,         // the event bit used by this task to show SYNC
                    all_sync_bits,    // the bits to wait for 
                    portMAX_DELAY);   // wait indefinitely
    printf("Task 3 exited SYNC\r\n");
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
                          task1_function,                 // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          (void*)TASK1_EVT_GROUP_BIT,            // task arguments carrying event group bit
                          1,                              // task priority
                          NULL
                        );

  task_err = xTaskCreate(
                        task2_function,                 // pointer to the task function
                        "Task2",                        // task name mainly for debugging
                        configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                        (void*)TASK2_EVT_GROUP_BIT,            // task arguments carrying event group bit
                        1,                              // task priority
                        NULL
                      );

  task_err = xTaskCreate(
                        task3_function,                 // pointer to the task function
                        "Task3",                        // task name mainly for debugging
                        configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                        (void*)TASK3_EVT_GROUP_BIT,            // task arguments carrying event group bit
                        1,                              // task priority
                        NULL
                        );

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