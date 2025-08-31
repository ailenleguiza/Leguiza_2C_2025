/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/08/2025 | Document creation		                         |
 *
 * @author Florencia Ailen Leguiza Scandizzo (florencia.leguiza@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

/*==================[macros and definitions]=================================*/
#define ON  1
#define OFF  0
#define TOGGLE 2


/*==================[internal data definition]===============================*/
struct leds{
	uint8_t  mode; //ON, OFF, TOGGLE
	uint8_t n_led; //úmero de led a controla 1,2 o 3
	uint8_t n_ciclos; // cantidad de ciclos de encedido/apagado
	uint16_t periodo; //tiempo de cada ciclo
};

/*==================[internal functions declaration]=========================*/
void Control_Leds(struct leds *control){
 uint8_t periodo = control->periodo/100;
 uint8_t i=0;
 uint8_t j=0;
 	printf("modo: %d",control->mode);

	if( control->mode == ON)
	{
		if(control->n_led == LED_1){
			LedOn(LED_1); 
	
		}
		else if(control->n_led == LED_2){
			LedOn(LED_2);
		}
		else if(control->n_led == LED_3){
			LedOn(LED_3);
		}
	}
	
	else if(control->mode == OFF)
	{
		if(control->n_led == LED_1){
			LedOff(LED_1);
		}
		else if(control->n_led == LED_2){
			LedOff(LED_2);
		}
		else if(control->n_led == LED_3){
			LedOff(LED_3);
		}
	}
	else if(control->mode == TOGGLE){
		printf("entrando en toggle");
		while(i < control->n_ciclos)
		{
			if(control->n_led == LED_1){
				LedToggle(LED_1);
			}
			else if(control->n_led == LED_2){
				LedToggle(LED_2);
			}
			else if(control->n_led == LED_3){
				LedToggle(LED_3);
			}
			i++;
			printf("ciclo: %d",i);
			j=0;
		while (j < periodo){
			j++;
			printf("esperando : %d",j);
			vTaskDelay(100 / portTICK_PERIOD_MS);
			}	
		}
	}



}

/*==================[external functions definition]==========================*/

void app_main(void)
{	
	LedsInit();
    
    struct leds control;

    control.mode = TOGGLE;
    control.n_led = LED_1;
    control.n_ciclos = 10;
    control.periodo = 500;

    // 3. Llamada a la función, pasando la dirección de la variable
    Control_Leds(&control);
	printf("termino\n");
    
}

/*==================[end of file]============================================*/