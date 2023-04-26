/*
  Use FreeRTOS Queue with multiple sender tasks having same priority
  and a low priority receiver task
  
  Book: Mastering the FreeRTOS
  Refer Chapter 1: 1.5 Data Types and Coding Style
  Refer Chapter 4: 4.4 Receiving Data From Multiple Sources
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "nrf_drv_clock.h"

QueueHandle_t queue_handle;

// Queue data sender enumerated types
typedef enum
{
  SENDER1 = 0,
  SENDER2
}sender_t; 

// Queue data structure
typedef struct
{
  sender_t sender;
  uint32_t data;
}queue_data_t; 

// array of structures
static queue_data_t data_struct[2] = 
{
  {SENDER1, 20000},
  {SENDER2, 41415}
};

const TickType_t k_wait_1sec = pdMS_TO_TICKS(1000);

void queue_sender_1(void* params)
{
  printf("Queue sender 1 task\r\n");

  const TickType_t k_wait = pdMS_TO_TICKS(100);
  
  while(true)
  {
    if(xQueueSendToBack(queue_handle, params, k_wait) != pdPASS)
    {
      //printf("Sender 1 Couldn't sent to Queue\r\n");
    }
    // add delay so that sender 2 which has the same priority 
    // can get execution time
    vTaskDelay(k_wait_1sec);
  }
}

void queue_sender_2(void* params)
{
  printf("Queue sender 2 task\r\n");

  const TickType_t k_wait = pdMS_TO_TICKS(200);

  while(true)
  {
    if(xQueueSendToBack(queue_handle, params, k_wait) != pdPASS)
    {
      //printf("Sender 2 Couldn't sent to Queue\r\n");
    }
    // add delay so that sender 1 which has the same priority 
    // can get execution time
    vTaskDelay(k_wait_1sec);
  }
}

void queue_receiver(void* params)
{
  printf("Queue receiver task\r\n");

  queue_data_t rx_data;

  while(true)
  {
    UBaseType_t count = uxQueueMessagesWaiting(queue_handle);
    if(count)
    {
      if(xQueueReceive(queue_handle, &rx_data, 0) == pdPASS)
      {
        if(rx_data.sender == SENDER1)
        {
          printf("Sender 1 data = %u\r\n", rx_data.data);
        }
        if(rx_data.sender == SENDER2)
        {
          printf("Sender 2 data = %u\r\n", rx_data.data);
        }
      }
      // no delay for this low priority task
      // 2 high priority senders will preempt it
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

  queue_handle = xQueueCreate(3, sizeof(queue_data_t));

  if(queue_handle != NULL)
  {
    // reset queue to empty state
    xQueueReset(queue_handle);
    
    // task creation function
    // starts with 'x' means it returns BaseType_t value
    // which can be either pdPASS or pdFAIL
    task_err = xTaskCreate(
                            queue_sender_1,                 // pointer to the task function
                            "QS1",                          // task name mainly for debugging
                            configMINIMAL_STACK_SIZE + 200, // task stack depth in words, max value contained in uint16_t data type
                            &(data_struct[0]),              // task arguments
                            2,                              // task priority same as sender 2
                            NULL
                          );
  
    // pdFAIL = insufficient heap memory
    if(task_err == pdFAIL)
    {
      printf("Task 1 create fail\r\n");
      return -1;
    }

    task_err = xTaskCreate(
                            queue_sender_2,
                            "QS2",
                            configMINIMAL_STACK_SIZE + 200,
                            &(data_struct[1]),
                            2,                              // task priority same as sender 1
                            NULL
                          );
    // pdFAIL = insufficient heap memory
    if(task_err == pdFAIL)
    {
      printf("Task 2 create fail\r\n");
      return -1;
    }

    task_err = xTaskCreate(
                        queue_receiver,
                        "QR",
                        configMINIMAL_STACK_SIZE + 200,
                        NULL,
                        1,                              // task priority lower than 2 sender tasks
                        NULL
                      );
    // pdFAIL = insufficient heap memory
    if(task_err == pdFAIL)
    {
      printf("Task 3 create fail\r\n");
      return -1;
    }

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Start FreeRTOS scheduler
    // Idle Task will be created automatically with lowest priority
    // which can be changed as needed
    vTaskStartScheduler();
  }
  else
  {
    printf("Failed to create Queue\r\n");
  }

  while (true) //---------
  {
      /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
       * in vTaskStartScheduler function. */
  }
}