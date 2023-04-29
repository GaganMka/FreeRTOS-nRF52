/*
  Use FreeRTOS Queue to send and receive data with pointers
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 4: 4.5 Working with Large or Variable Sized Data
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "nrf_drv_clock.h"

// for accessing the queue
QueueHandle_t pointer_q;

static const size_t q_size = 5;
static const size_t q_data_bytes = sizeof(char*);


void q_writer_task_function(void* pvParameters)
{
  const size_t str_len = 50;
  char array[str_len];
  char* string_to_send = &array[0];
  BaseType_t str_num = 0;
  printf("%s", (char*)pvParameters);

  while(true)
  {
    if(string_to_send != NULL)
    {
      snprintf(string_to_send, str_len, "Sending string number %d\r\n", str_num);
      
      // same as send to back
      xQueueSend(pointer_q, &string_to_send, 0);
      vTaskDelay(200);
      str_num++;
    }
    else
    {
      printf("Task 1 failed to create buffer\r\n");
    }
  }
}

void q_reader_task_function(void* pvParameters)
{
  char* rec_string;
  printf("%s", (char*)pvParameters);

  while(true)
  {
    xQueueReceive(pointer_q,  &rec_string, 200);
    while(*rec_string)
    {
      printf("%c", *rec_string);
      rec_string++;
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

  pointer_q = xQueueCreate(q_size, q_data_bytes);
  
  // defined constant to not use task stack
  static const char *msg = "Queue Writer Task\r\n";
  static const char *msg2 = "Queue Reader Task\r\n";

  if(pointer_q != NULL)
  {
    // reset queue to empty state
    xQueueReset(pointer_q);
    
    // task creation function
    // starts with 'x' means it returns BaseType_t value
    // which can be either pdPASS or pdFAIL
    task_err = xTaskCreate(
                            q_writer_task_function,         // pointer to the task function
                            "Task1",                        // task name mainly for debugging
                            configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                            (void*)msg,                     // task arguments explicit cast to void pointer
                            1,                              // task priority same as 2nd task
                            NULL
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