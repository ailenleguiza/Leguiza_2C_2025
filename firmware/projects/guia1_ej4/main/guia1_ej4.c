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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Florencia Ailen Leguiza Scandizzo (florencia.leguiza@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief Estructura que representa la configuración de un pin GPIO
 * 
 * Esta estructura define los parámetros necesarios para configurar
 * un pin GPIO, incluyendo el número de pin y su dirección.
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t; 

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número de 32 bits a un arreglo BCD
 * 
 * Esta función toma un número entero de 32 bits y lo convierte a formato BCD,
 * almacenando cada dígito decimal en posiciones separadas del arreglo.
 * El dígito más significativo se almacena en bcd_number[0] y el menos 
 * significativo en bcd_number[digits-1].
 * 
 * @param data Número de 32 bits a convertir
 * @param digits Cantidad de dígitos de salida deseados
 * @param bcd_number Puntero al arreglo donde se almacenarán los dígitos BCD
 * @return int8_t Código de retorno (0 = éxito)
 * 
 * @note bcd_number[0] será el dígito más significativo y bcd_number[digits-1] el menos significativo
 * @note El arreglo bcd_number debe tener al menos 'digits' posiciones
 * 
 * @example
 * uint32_t numero = 123;
 * uint8_t digitos = 3;
 * uint8_t bcd_array[3];
 * convertToBcdArray(numero, digitos, bcd_array);
 * // Resultado: bcd_array[0]=1, bcd_array[1]=2, bcd_array[2]=3
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t * bcd_number){

	for(int i = 0; i < digits; i++)
	{
		/* data % 10 obtiene el último dígito decimal de data (el dígito menos significativo) */
		bcd_number[digits-i-1] = (data % 10);
		data /= 10; 
		/* para desplazar de derecha a izquierda */
	}
	return 0;
}

/**
 * @brief Configura los pines GPIO según un dígito BCD
 * 
 * Esta función recibe un dígito BCD (0-9) y configura 4 pines GPIO
 * según la representación binaria de ese dígito. Cada bit del dígito
 * controla el estado de un GPIO específico mediante el mapeo definido.
 * 
 * @param gpio Vector de estructuras gpioConf_t que mapea los 4 bits BCD:
 *             - gpio[0] corresponde a b0 (bit menos significativo) -> GPIO_20
 *             - gpio[1] corresponde a b1 -> GPIO_21  
 *             - gpio[2] corresponde a b2 -> GPIO_22
 *             - gpio[3] corresponde a b3 (bit más significativo) -> GPIO_23
 * @param bcd_number Dígito BCD (0-9) a mostrar en los GPIOs
 * 
 * @note Si un bit está en '1', el GPIO correspondiente se setea (GPIOOn)
 * @note Si un bit está en '0', el GPIO correspondiente se limpia (GPIOOff)
 * @note Los GPIOs deben estar previamente inicializados como OUTPUT
 * 
 * @warning Esta función no verifica si bcd_number está en el rango válido (0-9)
 * 
 * @example
 * // Para mostrar el dígito 5 (binario: 0101)
 * configurar_pines(gpio_bcd, 5);
 * // Resultado: GPIO_20=ON, GPIO_21=OFF, GPIO_22=ON, GPIO_23=OFF
 */
void configurar_pines(gpioConf_t *gpio, uint8_t bcd_number){
	for (int i = 0; i < 4; i++)
	{
		if ((1 << i) & bcd_number)
		{
			GPIOOn(gpio[i].pin);
		}
		else
		{
			GPIOOff(gpio[i].pin);
		}
	}
}

/**
 * @brief Muestra un número de 32 bits en un display BCD multiplexado
 * 
 * Esta función convierte un número a BCD y lo muestra en un display
 * multiplexado de múltiples dígitos. Utiliza dos vectores de GPIO: 
 * uno para los datos BCD de cada dígito y otro para seleccionar 
 * qué dígito del display está activo en cada momento.
 * 
 * @param data Número de 32 bits a mostrar en el display
 * @param digits Cantidad de dígitos a mostrar (máximo 3 en esta implementación)
 * @param gpio_bcd Vector de 4 estructuras gpioConf_t para los datos BCD:
 *                 - gpio_bcd[0] -> b0 (GPIO_20)
 *                 - gpio_bcd[1] -> b1 (GPIO_21)
 *                 - gpio_bcd[2] -> b2 (GPIO_22) 
 *                 - gpio_bcd[3] -> b3 (GPIO_23)
 * @param gpio_display Vector de 3 estructuras gpioConf_t para selección de dígito:
 *                     - gpio_display[0] -> Dígito 1 (GPIO_19)
 *                     - gpio_display[1] -> Dígito 2 (GPIO_18)
 *                     - gpio_display[2] -> Dígito 3 (GPIO_9)
 * 
 * @note Esta función reutiliza convertToBcdArray() para la conversión BCD
 * @note Esta función reutiliza configurar_pines() para controlar los GPIOs BCD
 * @note Implementa multiplexado: activa un dígito, configura su valor BCD, 
 *       luego pasa al siguiente dígito
 * @note En aplicación real se necesitaría un delay entre dígitos para visualización
 * @note Todos los GPIOs deben estar previamente inicializados
 * 
 * @warning Si 'digits' es mayor que 3, puede acceder a posiciones no válidas del arreglo gpio_display
 * 
 * @example
 * mostrar_numero_display(123, 3, gpio_bcd, gpio_display);
 * // Muestra "123" en el display multiplexado
 */
void mostrar_numero_display(uint32_t data, uint8_t digits, gpioConf_t *gpio_bcd, gpioConf_t *gpio_display){
	
	/* Crear arreglo para almacenar los dígitos BCD */
	uint8_t bcd_number[digits];
	
	/* Convertir el número a BCD usando la función del punto 4 */
	if (convertToBcdArray(data, digits, bcd_number) != 0) {
		printf("Error en conversión BCD\n");
		return;
	}
	
	printf("Mostrando número %" PRIu32 " en display:\n", data);
	
	/* Recorrer cada dígito para mostrarlo en el display */
	for (int d = 0; d < digits; d++) {
		
		/* Paso 1: Apagar todos los dígitos del display */
		for (int i = 0; i < digits; i++) {
			GPIOOff(gpio_display[i].pin);
		}
		
		/* Paso 2: Configurar los pines BCD con el valor del dígito actual */
		configurar_pines(gpio_bcd, bcd_number[d]);
		
		/* Paso 3: Activar solo el dígito actual del display */
		GPIOOn(gpio_display[d].pin);
		
		/* Información de debug para seguimiento */
		printf("  Dígito %d (posición %d): valor %u -> GPIO_%u activado\n", 
		       d+1, d, bcd_number[d], gpio_display[d].pin);
		
		/* Para simulación, apagamos el dígito actual antes del siguiente */
		GPIOOff(gpio_display[d].pin);
	}
	
	printf("Display actualizado completamente\n\n");
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación
 * 
 * Esta función implementa las actividades 4, 5 y 6 del ejercicio:
 * - Actividad 4: Conversión de número a BCD
 * - Actividad 5: Configuración individual de dígitos en GPIOs BCD  
 * - Actividad 6: Visualización completa en display multiplexado
 * 
 * La función inicializa los GPIOs necesarios, prueba la conversión BCD,
 * muestra cada dígito individualmente y finalmente presenta el número
 * completo en un display multiplexado.
 * 
 * @note Utiliza el número 138 como ejemplo de prueba
 * @note Configura 7 GPIOs en total: 4 para datos BCD + 3 para selección de dígitos
 * @note Los GPIOs deben estar disponibles en la plataforma de hardware utilizada
 * 
 * Configuración de hardware requerida:
 * - GPIO_20, GPIO_21, GPIO_22, GPIO_23: Datos BCD (b0-b3)
 * - GPIO_19, GPIO_18, GPIO_9: Selección de dígitos del display
 */
void app_main(void){
	
	uint32_t data = 138;
	uint8_t digits = 3;
	uint8_t bcd_number[digits];

	/* Vector de configuración para los 4 bits BCD */
	gpioConf_t gpio_bcd[4] = {
		{GPIO_20, GPIO_OUTPUT},  // b0 - bit menos significativo
		{GPIO_21, GPIO_OUTPUT},  // b1
		{GPIO_22, GPIO_OUTPUT},  // b2  
		{GPIO_23, GPIO_OUTPUT}   // b3 - bit más significativo
	};
	
	/* Vector de configuración para selección de dígitos del display */
	gpioConf_t gpio_display[3] = {
		{GPIO_19, GPIO_OUTPUT},  // Dígito 1 (más significativo)
		{GPIO_18, GPIO_OUTPUT},  // Dígito 2 (medio)
		{GPIO_9,  GPIO_OUTPUT}   // Dígito 3 (menos significativo)
	};

	/* Inicializar todos los GPIOs BCD como salidas */ 
	for (int i = 0; i < 4; i++) {
		GPIOInit(gpio_bcd[i].pin, gpio_bcd[i].dir);
	}
	
	/* Inicializar todos los GPIOs del display como salidas */
	for (int i = 0; i < 3; i++) {
		GPIOInit(gpio_display[i].pin, gpio_display[i].dir);
	}
	
	printf("=== ACTIVIDAD 4: Conversión a BCD ===\n");
	if (convertToBcdArray(data, digits, bcd_number) != 0) {
		printf("convertToBcdArray: error\n");
	}
	printf("Número: %" PRIu32 " convertido a BCD:\n", data);

	printf("=== ACTIVIDAD 5: Configuración individual de dígitos BCD ===\n");
	for (int i = 0; i < digits; i++) {
		printf("%u ", bcd_number[i]);
	}
	printf("\n");
	
	printf("=== ACTIVIDAD 6: Display completo con multiplexado ===\n");
	/* Mostrar el número completo en el display usando ambos vectores de GPIO */
	mostrar_numero_display(data, digits, gpio_bcd, gpio_display);
	
	/* Pruebas adicionales comentadas - descomentar para probar otros números */
	/*printf("Probando con número 456:\n");
	mostrar_numero_display(456, 3, gpio_bcd, gpio_display);
	
	printf("Probando con número 789:\n");
	mostrar_numero_display(789, 3, gpio_bcd, gpio_display);
	*/
}

/*==================[end of file]============================================*/