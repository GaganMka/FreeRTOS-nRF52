/*
  Use FreeRTOS Queue
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 4: Queue Management
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "nrf_drv_clock.h"

// for task reference
TaskHandle_t qwr_handle;
TaskHandle_t qrd_handle;

// for accessing the queue
QueueHandle_t queue_handle;
static const size_t q_size = 10;
static const size_t q_data_bytes = sizeof(char);


// user defined Task function which must return void and take a void pointer parameter
// pv = pointer to void
void q_writer_task_function(void* pvParameters)
{
  const TickType_t k_wait = pdMS_TO_TICKS(500);
  const TickType_t k_wait_1sec = pdMS_TO_TICKS(1000);

  BaseType_t status;
  // cast void Pointer to char Pointer
  char *arg = (char*) pvParameters;
  printf(arg);
  const char items[10] = "0123456789";
   
  // task's infinite loop which must not exit or return
  // if a task is not required, it should be explicitly deleted
  while(true)
  {
    UBaseType_t space_available = uxQueueSpacesAvailable(queue_handle);
    // write to queue only when entire queue is empty
    if(space_available == q_size)
    {
      for(size_t i=0; i<q_size; ++i)
      {
        status = xQueueSendToBack(queue_handle, &items[i], k_wait);
        if(status != pdPASS)
        {
          printf("Couldn't write to queue, Err = %d\r\n", status);
        }
      }
      printf("Queue data written\r\n");
    }
    else
    {
      // delay is must to prevent starvation of queue reader task
      // because both tasks have same priority
      vTaskDelay(k_wait_1sec);
    }
  }
}

void q_reader_task_function(void* pvParameters)
{
  BaseType_t status;
  char rd_val;
  const TickType_t k_wait = pdMS_TO_TICKS(500);
  const TickType_t k_wait_1sec = pdMS_TO_TICKS(1000);

  char *arg = (char*) pvParameters;
  printf(arg);

  while(true)
  {
    UBaseType_t count = uxQueueMessagesWaiting(queue_handle);
    if(count)
    {
      printf("%u Queue data available to read\r\n", count);
      for(size_t i=0; i<count; ++i)
      {
        status = xQueueReceive(queue_handle, &rd_val, k_wait);
        if(status != pdPASS)
        {
          printf("Couldn't read from queue, Err = %d\r\n", status);
        }
        else
        {
          printf("Queue Val read = %c\r\n", rd_val);
        }
      }
    }
    else
    {
      printf("Queue read waiting\r\n");
      // delay is must to prevent starvation of queue reader task
      // because both tasks have same priority
      vTaskDelay(k_wait_1sec);
    }
  }
}

int main(void)
{
  BaseType_t task_err;
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  queue_handle = xQueueCreate(q_size, q_data_bytes);
  
  // defined constant to not use task stack
  static const char *msg = "Queue Writer Task\r\n";
  static const char *msg2 = "Queue Reader Task\r\n";

  if(queue_handle != NULL)
  {
    // reset queue to empty state
    xQueueReset(queue_handle);
    
    // task creation function
    // starts with 'x' means it returns BaseType_t value
    // which can be either pdPASS or pdFAIL
    task_err = xTaskCreate(
                            q_writer_task_function,         // pointer to the task function
                            "Task1",                        // task name mainly for debugging
                            configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                            (void*)msg,                     // task arguments explicit cast to void pointer
                            1,                              // task priority same as 2nd task
                            qwr_handle
                          );
  
    // pdFAIL = insufficient heap memory
    if(task_err == pdFAIL)
    {
      printf("Task 1 create fail\r\n");
      return -1;
    }

    task_err = xTaskCreate(
                            q_reader_task_function,
                            "Task2",
                            configMINIMAL_STACK_SIZE + 200,
                            (void*)msg2,
                            1,
                            qrd_handle
                          );
    // pdFAIL = insufficient heap memory
    if(task_err == pdFAIL)
    {
      printf("Task 2 create fail\r\n");
      return -1;
    }

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Start FreeRTOS scheduler
    // Idle Task will be created automatically with lowest priority
    // which can be changed as needed
    vTaskStartScheduler();
  }

  while (true) //---------
  {
      /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
       * in vTaskStartScheduler function. */
  }
}