/*
  Pass parameters to a FreeRTOS task
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 3: Task Management 
*/

#include "FreeRTOS.h"
#include "task.h"
#include "nrf_drv_clock.h"

// for task reference
TaskHandle_t task1_handle;

// user defined Task function which must return void and take a void pointer parameter
// pv = pointer to void
void task1_function(void* pvParameters)
{
  // cast void Pointer to char Pointer
  char *arg = (char*) pvParameters;
    
  // task's infinite loop which must not exit or return
  // if a task is not required, it should be explicitly deleted
  while(true)
  {
    printf(arg);
    vTaskDelay(pdMS_TO_TICKS(1000));  // v = void return type and function is defined in task.c
  }

  // if task reached here, it must be deleted before exiting the task function
  // by passing a NULL parameter to task delete function
  // to delete the calling or this task
  vTaskDelete(NULL);  // v = void return type and function is defined in task.c
}

int main(void)
{
  BaseType_t task_err;
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);
  
  // defined constant to not use task stack
  static const char *msg = "Task 1 function\r\n";
  static const char *msg2 = "Task 2 function\r\n";

  // task creation function
  // starts with 'x' means it returns BaseType_t value
  // which can be either pdPASS or pdFAIL
  task_err = xTaskCreate(
                          task1_function,                 // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          (void*)msg,                     // task arguments explicit cast to void pointer
                          1,                              // task priority, 0 = lowest, same as task 2
                          task1_handle
                        );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task 1 create fail\r\n");
    return -1;
  }

  // task 2 utilizing the same task1 function but will have a different instance
  task_err = xTaskCreate(
                          task1_function,                 // pointer to the task function
                          "Task2",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          (void*) msg2,                   // task arguments explicit cast to void pointer
                          1,                              // task priority, 0 = lowest, same as task 1
                          NULL                            // no task handle is needed for now
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