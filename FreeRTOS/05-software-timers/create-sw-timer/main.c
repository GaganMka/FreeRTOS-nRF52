/*
  Create a FreeRTOS software timer
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 5: Software Timer Management 
*/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"   // freeRTOS sw timers
#include "nrf_drv_clock.h"

TimerHandle_t repeating_timer;
TickType_t time_now = 0;

// SW Timer callback function return type void
// and accepts only 1 argument of type TimerHandle_t
void sw_timer_callback(TimerHandle_t timer)
{
  // code should be short and non-blocking
  // never call vTaskDelay function in a SW timer callback
  
  time_now = xTaskGetTickCountFromISR();
  // following call is creating a run time fault
  // printf("Ticks = %u\r\n", time_now);
}

// user defined Task function which must return void and take a void pointer parameter
// pv = pointer to void
void task1_function(void* pvParameters)
{
  printf("Task 1 function\r\n");
  // task's infinite loop which must not exit or return
  // if a task is not required, it should be explicitly deleted
  while(true)
  {
    
    printf("Ticks = %u\r\n", time_now);
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
  
  const TickType_t k_timer_period = pdMS_TO_TICKS(1000);
  
  // create a timer and get the timer handle
  // timer created in DORMANT state i.e. inactive state
  // see configTIMER_TASK_PRIORITY define in FreeRTOSConfig.h (default 2)
  repeating_timer = xTimerCreate(
                                  "RPTTMR",         // name for the timer, used for debugging only
                                  k_timer_period,   // timer's period in ticks
                                  pdTRUE,           // pdTRUE = auto reload timer
                                  NULL,             // timer ID a void pointer
                                  sw_timer_callback // pointer to the callback function
                                  );
  // NULL = insufficient heap memory
  if(repeating_timer == NULL)
  {
    printf("Timer create fail\r\n");
    return -1;
  }

  // task creation function
  // starts with 'x' means it returns BaseType_t value
  // which can be either pdPASS or pdFAIL
  task_err = xTaskCreate(
                          task1_function,                 // pointer to the task function
                          "Task1",                        // task name mainly for debugging
                          configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                          NULL,                           // task arguments
                          1,                              // task priority is lower than Timer (Daemon Task) priority
                          NULL
                        );
  
  // pdFAIL = insufficient heap memory
  if(task_err == pdFAIL)
  {
    printf("Task create fail\r\n");
    return -1;
  }

  // issue timer start command but will not run the timer
  // until the scheduler is not started
  if(xTimerStart(
                  repeating_timer,    // timer handle to start 
                  0                   // ticks to wait
                  ) != pdPASS)
  {
    // pdFAIL if the timer's command queue is full
    // and the 'start a timer' command couldn't be written
    // NOTE: Try specifying non-zero ticks to wait parameter
    printf("Timer start fail\r\n");
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