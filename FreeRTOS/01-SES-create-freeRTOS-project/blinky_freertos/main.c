#include "FreeRTOS.h"
#include "app_error.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "sdk_errors.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

// task handle to assign in task create function
TaskHandle_t led_task_handle;

// 4 LEDs on the DevKit, VDD is common, need 0 to turn ON
// use the Macro to get the PIN number of the LED GPIO
#define LED1 NRF_GPIO_PIN_MAP(0, 13)
#define LED2 NRF_GPIO_PIN_MAP(0, 14)
#define LED3 NRF_GPIO_PIN_MAP(0, 15)
#define LED4 NRF_GPIO_PIN_MAP(0, 16)

// the number of Ticks for the delay function
#define LED_ON_TIME  pdMS_TO_TICKS(100)
#define LED_OFF_TIME pdMS_TO_TICKS(500)

/**
 * @brief LED toggle task function, needed in Task Creation function
 * 
 * @param pvParameter - Pointer to void
 */
static void my_led_toggle_task_function(void* pvParameter)
{
    // cast the void poiter to uint16_t as the value passed during the
    // task create is of type uint16_t
    // Index 0 = LED PIN
    // Index 1 = ON Time
    // Index 2 = OFF Time
    uint16_t *on_off_time = (uint16_t*)pvParameter;

    // The macro is used to suppress unused parameter
    // same as (void)(pvParameter);
    //UNUSED_PARAMETER(pvParameter);

    // task should never exit the infinite loop
    while(true)
    {
        nrf_gpio_pin_clear(*(on_off_time + 0));   // LED ON
        // delay function getting Ticks values equals to the
        // desired mSec delay by calling the Macro pdMS_TO_TICKS()   
        vTaskDelay(pdMS_TO_TICKS(*(on_off_time + 1)));

        nrf_gpio_pin_set(*(on_off_time + 0)); // LED OFF
        vTaskDelay(pdMS_TO_TICKS(*(on_off_time + 2)));
    }
}

static void init_leds()
{
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_pin_set(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_pin_set(LED2);
    nrf_gpio_cfg_output(LED3);
    nrf_gpio_pin_set(LED3);
    nrf_gpio_cfg_output(LED4);
    nrf_gpio_pin_set(LED4);
}

int main(void) 
{
    BaseType_t err = pdPASS;
    ret_code_t err_code;

    // static variable will exist before and after the function has executed
    // this STATIC is important when the variable is insode the function
    static const uint16_t led1_on_off_delays[3] = {LED1, 100, 100};
    static const uint16_t led2_on_off_delays[3] = {LED2, 150, 150};
    static const uint16_t led3_on_off_delays[3] = {LED3, 200, 200};
    static const uint16_t led4_on_off_delays[3] = {LED4, 250, 250};

    /* Initialize clock driver for better time accuracy in FREERTOS */
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    init_leds();

    // Task 1 has control Pin and Values for LED 1
    err = xTaskCreate(my_led_toggle_task_function,      // callback function
                        "LED1",                         // Task Name  
                        configMINIMAL_STACK_SIZE + 200, //Stack depth
                        (void*)led1_on_off_delays,      // pvParameter
                        1,                              // Priority
                        NULL);               // Task Handle

    if(err != pdPASS)
    {
        return -1;
    }

    // Task 2 has control Pin and Values for LED 2
    err = xTaskCreate(my_led_toggle_task_function,      // callback function
                        "LED2",                         // Task Name  
                        configMINIMAL_STACK_SIZE + 200, //Stack depth
                        (void*)led2_on_off_delays,      // pvParameter
                        1,                              // Priority
                        NULL);               // Task Handle

    if(err != pdPASS)
    {
        return -1;
    }

    // Task 3 has control Pin and Values for LED 3
    err = xTaskCreate(my_led_toggle_task_function,      // callback function
                        "LED3",                         // Task Name  
                        configMINIMAL_STACK_SIZE + 200, //Stack depth
                        (void*)led3_on_off_delays,      // pvParameter
                        1,                              // Priority
                        NULL);               // Task Handle

    if(err != pdPASS)
    {
        return -1;
    }

    // Task 4 has control Pin and Values for LED 4
    err = xTaskCreate(my_led_toggle_task_function,      // callback function
                        "LED4",                         // Task Name  
                        configMINIMAL_STACK_SIZE + 200, //Stack depth
                        (void*)led4_on_off_delays,      // pvParameter
                        1,                              // Priority
                        NULL);               // Task Handle

    if(err != pdPASS)
    {
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