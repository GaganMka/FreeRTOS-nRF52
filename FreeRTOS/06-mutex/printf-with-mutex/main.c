/*
  Use Mutex to send data to printf() from 2 tasks
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 7: 7.3 Mutexes (and Binary Semaphores)
*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" // to use mutex
#include "nrf_drv_clock.h"

// set configUSE_MUTEXES to 1 in FreeRTOSConfig.h
SemaphoreHandle_t printing_mutex;

bool print_with_mutex(const char* str)
{
  static const TickType_t k_wait_delay = pdMS_TO_TICKS(10);
  // return early on pdFAIL otherwise a run time fault will be generated
  if(xSemaphoreTake(printing_mutex, k_wait_delay) != pdTRUE)  return false;
  printf("%s", str);
  xSemaphoreGive(printing_mutex);
  return true;
}

void task1_function(void* pvParameters)
{
  const TickType_t k_wait = pdMS_TO_TICKS(500);
  UBaseType_t task_priority;
  char msg[50] = {0};
  size_t count = 0;
  while(true)
  {
    // no need of task handle
    task_priority = uxTaskPriorityGet(NULL);
    snprintf(msg, 50, "[%u] Printing for Task 1 with proiroty %u\r\n", count, task_priority);
    // increment count only on success or true from printing function 
    if(print_with_mutex(msg))  count++;
    vTaskDelay(k_wait);
  }
}

void task2_function(void* pvParameters)
{
  const TickType_t k_wait = pdMS_TO_TICKS(1000);
  UBaseType_t task_priority;
  char msg[50] = {0};
  size_t count = 0;
  while(true)
  {
    // no need of task handle
    task_priority = uxTaskPriorityGet(NULL);
    snprintf(msg, 50, "[%u] Printing for Task 2 with proiroty %u\r\n", count, task_priority);
    // increment count only on success or true from printing function 
    if(print_with_mutex(msg)) count++;
    vTaskDelay(k_wait);
  }
}

int main(void)
{
  BaseType_t task_err;
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  // set configUSE_MUTEXES to 1 in FreeRTOSConfig.h
  printing_mutex = xSemaphoreCreateMutex();

  if(printing_mutex == NULL)
  {
    printf("Mutex create fail\r\n");
    return -1;
  }
  
  task_err = xTaskCreate(
                          task1_function,                 // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          NULL,                           // task arguments
                          1,                              // task priority lower than task 2
                          NULL
                        );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task 1 create fail\r\n");
    return -1;
  }

  task_err = xTaskCreate(
                          task2_function,                 // pointer to the task function
                          "Task2",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          NULL,                           // task arguments
                          2,                              // task priority higher than task 1
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