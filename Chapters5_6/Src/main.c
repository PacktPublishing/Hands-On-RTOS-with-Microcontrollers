#include <FreeRTOS.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>

/**
 * 	function prototypes
 */
void Task1(void *argument);
void Task2(void *argument);
void Task3(void *argument);
void lookBusy( void );

int main(void)
{
	// some common variables to use for each task
	// 128 * 4 = 512 bytes
	//(recommended min stack size per task)
	const static uint32_t stackSize = 128;
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	if (xTaskCreate(Task1, "task1", stackSize, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS)
	{
		if (xTaskCreate(Task2, "task2", stackSize, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS)
		{
			if (xTaskCreate(Task3, "task3", stackSize, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS)
			{
				//start the scheduler - shouldn't return unless there's a problem
				vTaskStartScheduler();
			}
		}
	}

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void Task1(void *argument)
{
  while(1)
  {
	  SEGGER_SYSVIEW_PrintfHost("hey there!\n");
	  GreenLed.On();
	  vTaskDelay(105/ portTICK_PERIOD_MS);
	  GreenLed.Off();
	  vTaskDelay(100/ portTICK_PERIOD_MS);
  }
}

void Task2( void* argument )
{
	while(1)
	{
		SEGGER_SYSVIEW_PrintfHost("task 2 says 'Hi!'\n");
		BlueLed.On();
		vTaskDelay(200 / portTICK_PERIOD_MS);
		BlueLed.Off();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void Task3( void* argument )
{
	while(1)
	{
		lookBusy();

		SEGGER_SYSVIEW_PrintfHost("task3\n");
		RedLed.On();
		vTaskDelay(500/ portTICK_PERIOD_MS);
		RedLed.Off();
		vTaskDelay(500/ portTICK_PERIOD_MS);
	}
}

void lookBusy( void )
{
	volatile uint32_t __attribute__((unused)) dontCare = 0;
	for(int i = 0; i < 50E3; i++)
	{
		dontCare = i % 4;
	}
	SEGGER_SYSVIEW_PrintfHost("looking busy\n");
}
