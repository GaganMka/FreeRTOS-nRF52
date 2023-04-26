/*
  Use Idle Hook callback function
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 3: 3.8 The Idle Task and the Idle Task Hook
*/

#include "FreeRTOS.h"
#include "task.h"
#include "nrf_drv_clock.h"

// for task reference
TaskHandle_t task1_handle;

// volatile variable to use in idle hook function
volatile uint32_t counter = 0;

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
    printf("Counter = %ul\r\n", counter);
    vTaskDelay(pdMS_TO_TICKS(1000));  // v = void return type and function is defined in task.c
  }

  // if task reached here, it must be deleted before exiting the task function
  // by passing a NULL parameter to task delete function
  // to delete the calling or this task
  vTaskDelete(NULL);  // v = void return type and function is defined in task.c
}

// configUSE_IDLE_HOOK must be set to 1 in FreeRTOS config file
// called automatically by the idle task once per iteration of the idle task loop
void vApplicationIdleHook(void)
{
  // code in this hook must not block or suspend 
  counter++;
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

  // task creation function
  // starts with 'x' means it returns BaseType_t value
  // which can be either pdPASS or pdFAIL
  task_err = xTaskCreate(
                          task1_function,                 // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          (void*)msg,                     // task arguments explicit cast to void pointer
                          1,                              // task priority, 0 = lowest
                          task1_handle
                        );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task 1 create fail\r\n");
    return -1;
  }

  /* Activate deep sleep mode */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  // Start FreeRTOS scheduler
  // Idle Task will be created automatically with lowest priority
  // which can be changed as needed
  vTaskStartScheduler();

  while (true) //---------
  {
      /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
       * in vTaskStartScheduler function. */
  }
}