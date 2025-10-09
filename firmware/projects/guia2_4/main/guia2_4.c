/*! @mainpage guia2_4
 *
 * @section genDesc Descripción General
 *
 * Este programa utiliza FreeRTOS para gestionar tareas relacionadas con la conversión de señales analógicas a digitales (A/D) y digitales a analógicas (D/A) en un ESP32. El sistema toma valores de una entrada analógica y los envía a través de UART. Además, se genera una señal de salida en un pin configurado como DAC.
 *
 * @section hardConn Conexión de Hardware
 *
 * | Periférico    | ESP32   |
 * |:-------------:|:-------:|
 * | Entrada A/D   | GPIO_0  |
 * | Salida D/A    | GPIO_1  |
 *
 * @section changelog Historial de Cambios
 *
 * | Fecha        | Descripción                                  |
 * |:------------:|:---------------------------------------------|
 * | 9/10/2025   | Creación  de documentacion ej 4                      |
 *
 * @section author Autor
 *
 * Florencia Ailen Leguiza Scandizzo (florencia.leguiza@ingenieria.uner.edu.ar)
 */

/*==================[inclusiones]============================================*/


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

/**
 * @brief Tamaño del buffer de datos para la señal de ECG simulada.
 */

#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/

/**
 * @brief Constante para las funciones de conversión A/D y D/A.
 */
TaskHandle_t cambio = NULL; /*cambio -> para osciloscopio -> timer A -> Funcion()*/
TaskHandle_t main_task_handle = NULL; /*main_task_handle -> para ECG -> timer B -> mostrarecg()*/

/**
 * @brief Arreglo de valores de señal ECG simulada.
 */

const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función que se ejecuta en la interrupción del temporizador A.
 *
 * Envía una notificación a la tarea encargada de la lectura de entrada analógica. (Para la tarea con Funcion())
 * 
 * @param param Parámetro opcional (no utilizado).
 */

void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR( cambio, pdFALSE); 
}

/**
 * @brief Función que se ejecuta en la interrupción del temporizador B.
 *
 * Envía una notificación a la tarea encargada de mostrar la señal ECG. (Para la tarea con mostrarecg())
 *
 * @param param Parámetro opcional (no utilizado).
 */

void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR( main_task_handle, pdFALSE); 
}

/**
 * @brief Tarea que lee una señal analógica y la envía por UART.
 *
 * Utiliza el canal CH1 para leer una señal A/D y envía el valor por UART.
 *
 * @param param Parámetro opcional (no utilizado).
 */

void Funcion(void* param)
{
	uint16_t guardado;
	while (true)
	{ 
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1,&guardado);
		UartSendString(UART_PC, (char*)UartItoa(guardado, 10));
		UartSendString(UART_PC, "\r"); 
	}
}

/**
 * @brief Tarea que muestra los valores de la señal ECG en la salida D/A.
 *
 * Toma los valores de la señal ECG simulada y los escribe en una salida D/A.
 *
 * @param param Parámetro opcional (no utilizado).
 */

void mostrarecg()
{
	int i=0; /*Se define un contador i que recorre las posiciones del arreglo ecg[]. */
while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(i<231)
			{AnalogOutputWrite(ecg[i]);} /* Esto escribe el valor de ecg[i] en el DAC*/
		else
			{i=0;}
		i++;
	}
}
/*==================[external functions definition]==========================*/

void app_main(void){
	
	// Configuración de entrada analógica
    analog_input_config_t config = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0
    };
    AnalogInputInit(&config);
    
    // Configuración de temporizador para la tarea A/D
    timer_config_t timer_led_1 = {
        .timer = TIMER_A,
        .period = 2000,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_led_1);

    // Configuración de temporizador para la tarea ECG
    timer_config_t timer_led_2 = {
        .timer = TIMER_B,
        .period = 4000, 
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_led_2);

    // Configuración de UART
    serial_config_t pantalla = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&pantalla);

    // Creación de tareas de FreeRTOS
    xTaskCreate(&Funcion, "OSCILOSCOPIO", 2048, NULL, 5, &cambio);
    xTaskCreate(&mostrarecg, "ecg", 2048, NULL, 5, &main_task_handle);
    
    // Iniciar temporizadores
    TimerStart(timer_led_1.timer);
    TimerStart(timer_led_2.timer);
    
    // Inicialización de la salida D/A
    AnalogOutputInit();
}
/*==================[end of file]============================================*/