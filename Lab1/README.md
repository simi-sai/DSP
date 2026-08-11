# Lab 1: Analog Digital Converter (ADC)

## Consigna

Realizar la lectura de una señal analogica a distintas velocidades de muestreo en KiloSamples/segundo: `8 kS/s`, `16 kS/s`, `22 kS/s`, `44 kS/s` y `48 kS/s`.

Los cambios de las velocidades de muestreo serán realizados con una de las teclas de la placa, en forma de un buffer circular. Cada velocidad de muestreo se indicara a traves de un color RGB del LED integrado. 

Con otra tecla de la placa se habilitara la adquisicion y paro de muestras (**RUN** / **STOP**). 

Los valores adquiridos seran almacenados en memoria en un buffer circular de 512 muestras del tipo `q15` (fraccional de 15 bits) y a su vez seran enviados a traves del DAC de 12 bits.

## Desarrollo

### Configuracion de Clocks

Principalmente se deben configurar los clocks de la placa, ya que casi todos los modulos de la misma dependen de esta configuracion. Para ello se utilizara dentro de ConfigTools el modulo `CLOCK` del SDK de NXP, el cual permite configurar los distintos clocks de la placa.

<table align="center">
    <tr>
        <th>Clock Component</th>
        <th>Frequency</th>
        <th>Clock Source</th>
        <th>Description</th>
    </tr>
    <tr>
        <td>MAIN_clock</td>
        <td>150 MHz</td>
        <td>PLL0 (ref. FIRC 48 MHz)</td>
        <td>Clock principal de la placa</td>
    </tr>
    <tr>
        <td>ADC0_clock</td>
        <td>48 MHz</td>
        <td>Fast IRC</td>
        <td>Clock del modulo ADC0 (ADC)</td>
    </tr>
    <tr>
        <td>DAC0_clock</td>
        <td>48 MHz</td>
        <td>Fast IRC</td>
        <td>Clock del modulo DAC0 (DAC)</td>
    </tr>
    <tr>
        <td>CTIMER0_clock</td>
        <td>150 MHz</td>
        <td>PLL0</td>
        <td>Clock del modulo CTIMER0 (Timer)</td>
    </tr>
    <tr>
        <td>FLEXXCOM4_clock</td>
        <td>12 MHz</td>
        <td>Slow IRC</td>
        <td>Clock del modulo FLEXXCOM4 (UART)</td>
    </tr>
</table>

El modulo UART se utilizara para enviar mensajes de debug a la PC, por lo que se utilizo una pre configuracion con un baudrate de 115200 bps y un clock de 12 MHz.

### Modulo ADC (LPADC0)

El modulo ADC de la placa FRDM-MCXN947 es un conversor de 12 bits con un rango de voltaje de referencia de 0 a 3.3V. El mismo se configuró desde el PeripheralsTool del ConfigTools con los siguientes parámetros:

#### Configuración General

<table align="center">
    <tr>
        <th> Campo</th>
        <th> Valor</th>
        <th> Descripción</th>
    </tr>
    <tr>
        <td> Pre-enable ADC analog circuits</td>
        <td> Enabled</td>
        <td> Enciende el circuito analógico del ADC</td>
    </tr>
    <tr>
        <td> Autocalibration</td>
        <td> Enabled</td>
        <td> Calibra el ADC al iniciar para compensar desviaciones de fábrica y temperatura</td>
    </tr>
    <tr>
        <td> Doze mode</td>
        <td> Enabled</td>
        <td> Permite que el ADC funcione en modo de bajo consumo</td>
    </tr>
    <tr>
        <td> FIFO 0 watermark</td>
        <td> 1</td>
        <td> Genera una interrupción cuando hay al menos 1 muestra en la FIFO</td>
    </tr>
    <tr>
        <td> Enable interrupt vector</td>
        <td> Enabled</td>
        <td> Habilita la interrupción del ADC</td>
    </tr>
    <tr>
        <td> Interrupt Sources</td>
        <td> FIFO 0 watermark</td>
        <td> Genera una interrupción cuando hay al menos 1 muestra en la FIFO</td>
    </tr>
    <tr>
        <td> Interrupt Priority</td>
        <td> 0</td>
        <td> Prioridad de la interrupción del ADC</td>
    </tr>
</table>

Al generar la interrupción, el CPU ejecuta la rutina de servicio de interrupción (ISR) del ADC, la cual se encarga de leer la muestra de la FIFO y almacenarla en un buffer circular de 512 muestras.

#### Comando de Conversión (Conversion Command)

<table align="center">
    <tr>
        <th> Campo</th>
        <th> Valor</th>
        <th> Descripción</th>
    </tr>
    <tr>
        <td> Command ID</td>
        <td> start_conversion</td>
        <td> Identificador del comando</td>
    </tr>
    <tr>
        <td> Command number</td>
        <td> 1</td>
        <td> Número de comando asociado al comando</td>
    </tr>
    <tr>
        <td> Channel sample mode</td>
        <td> Single end mode, using A</td>
        <td> Mide la tensión del pin respecto a GND</td>
    </tr>
    <tr>
        <td> Channel number</td>
        <td> A, 0 >> [P3] ADC0_A0/ARD_A0/J4[2]</td>
        <td> Canal de entrada analógica seleccionado</td>
    </tr>
    <tr>
        <td> Conversion resolution mode</td>
        <td> Standard resolution (12-bit)</td>
        <td> Resolución del conversor</td>
    </tr>
    <tr>
        <td> Loop count</td>
        <td> 0</td>
        <td> Ejecuta el comando n+1 vez por cada trigger (<code>n=0</code>)</td>
    </tr>
    <tr>
        <td> Sample time mode</td>
        <td> 3 ADCK cycles total sample time</td>
        <td> Tiempo de muestreo estándar</td>
    </tr>
</table>

#### Trigger de Conversión (Conversion Trigger)

<table align="center">
    <tr>
        <th> Campo</th>
        <th> Valor</th>
        <th> Descripción</th>
    </tr>
    <tr>
        <td> Trigger ID</td>
        <td> timer0_trigger</td>
        <td> Identificador del trigger</td>
    </tr>
    <tr>
        <td> Trigger number</td>
        <td> 0</td>
        <td> Número de trigger asociado al trigger</td>
    </tr>
    <tr>
        <td> Target command number</td>
        <td> 1</td>
        <td> Asocia este trigger al Command ID 1</td>
    </tr>
    <tr>
        <td> High priority</td>
        <td> Enabled</td>
        <td> La conversión tiene prioridad alta</td>
    </tr>
    <tr>
        <td> Select channel A FIFO</td>
        <td> FIFO 0</td>
        <td> Los resultados se almacenan en FIFO 0</td>
    </tr>
    <tr>
        <td> Delay value</td>
        <td> 0</td>
        <td> Conversión inmediata al recibir el trigger</td>
    </tr>
</table>

### Modulo Timer (CTIMER0)

El modulo CTIMER0 es un timer de 32 bits que se utilizará para generar el trigger de conversión del ADC. Se configuró desde el PeripheralsTool del ConfigTools con los siguientes parámetros:

<table align="center">
    <tr>
        <th> Campo</th>
        <th> Valor</th>
        <th> Descripción</th>
    </tr>
    <tr>
        <td> Timer mode</td>
        <td> Timer (bus clock source)</td>
        <td> Modo de operación del timer</td>
    </tr>
    <tr>
        <td> Prescale value</td>
        <td> 1</td>
        <td> El timer corre a la frecuencia del clock del modulo (150 MHz)</td>
    </tr>
    <tr>
        <td> Match channel</td>
        <td> 0</td>
        <td> Se utiliza el canal 0 para generar un trigger al ADC</td>
    </tr>
    <tr>
        <td> Match value</td>
        <td> 18749 (≈ 8 kHz)</td>
        <td> 150 MHz / 18749 ≈ 8000.4 Hz — tasa inicial de muestreo</td>
    </tr>
    <tr>
        <td> Enable counter reset on match</td>
        <td> Enabled</td>
        <td> El timer se reinicia al llegar al valor de match</td>
    </tr>
    <tr>
        <td> Enable match interrupt request</td>
        <td> Enabled</td>
        <td> Se genera una interrupción al llegar al valor de match</td>
    </tr>
    <tr>
        <td> Interrupt Priority</td>
        <td> 1</td>
        <td> Prioridad de la interrupción del timer</td>
    </tr>
</table>

### Modulo DAC (LPDAC0)

El modulo DAC de la placa FRDM-MCXN947 es un conversor digital a analógico de 12 bits con un rango de voltaje de salida de 0 a 3.3V. El mismo se configuró desde el PeripheralsTool del ConfigTools con los siguientes parámetros:

<table align="center">
    <tr>
        <th> Campo</th>
        <th> Valor</th>
        <th> Descripción</th>
    </tr>
    <tr>
        <td> Enable DAC</td>
        <td> Enabled</td>
        <td> Habilita el modulo DAC</td>
    </tr>
    <tr>
        <td> Reference voltage source</td>
        <td> VREF_INT/VDD_ANA</td>
        <td> Fuente de voltaje de referencia</td>
    </tr>
    <tr>
        <td> Opamp as buffer</td>
        <td> Enabled</td>
        <td> Habilita el buffer de salida del DAC</td>
    </tr>
    <tr>
        <td> Enable LPDAC</td>
        <td> Enabled</td>
        <td> Habilita el DAC de bajo consumo</td>
    </tr>
    <tr>
        <td> Set LPDAC level after initialization</td>
        <td> Enabled</td>
        <td> Habilita la salida del DAC al encender</td>
    </tr>
    <tr>
        <td> LPDAC level</td>
        <td> 0</td>
        <td> Nivel de salida del DAC al encender (0V)</td>
    </tr>
</table>

### Programa Principal

El programa inicializa los módulos (pines, clocks, periféricos y consola de debug) y habilita la interrupción del CTIMER0. Luego ejecuta un bucle infinito que solo actualiza el color del LED RGB cuando cambia el estado (RUN/STOP) o la tasa de muestreo.

#### Handlers de Interrupción

<table align="center">
    <tr>
        <th> ISR</th>
        <th> Evento</th>
        <th> Acción</th>
    </tr>
    <tr>
        <td> CTIMER0_IRQHandler</td>
        <td> Match del timer</td>
        <td> Limpia el flag y dispara la conversión con `LPADC_DoSoftwareTrigger()`</td>
    </tr>
    <tr>
        <td> ADC0_IRQHandler</td>
        <td> Watermark FIFO0</td>
        <td> Lee la muestra (`LPADC_GetConvResult`), la guarda en el buffer q15 y la escribe al DAC</td>
    </tr>
    <tr>
        <td> GPIO00_IRQHandler</td>
        <td> SW3 presionado</td>
        <td> Inicia/Detiene el muestreo (RUN / STOP)</td>
    </tr>
    <tr>
        <td> GPIO01_IRQHandler</td>
        <td> SW2 presionado</td>
        <td> Cambia la tasa de muestreo en forma circular</td>
    </tr>
</table>

#### Buffer Circular q15

El ADC entrega 12 bits *left-justified* en los bits `[14:3]` del word de 16 bits, es decir, ya en formato q15 (`0x0000` = `0 V`, `0x7FF8` $\approx$ `VREF`). La muestra se almacena sin procesar en un buffer circular de 512 muestras.

#### Salida al DAC

El DAC de 12 bits recibe la misma muestra desplazada 3 bits a la derecha para ajustarla al rango del DAC, como el ADC deja el dato en los bits `[14:3]` y el DAC los espera en `[11:0]`, el shift de 3 reconstruye exactamente el mismo código de 12 bits.

#### Velocidades de muestreo y colores del LED RGB

<table align="center">
    <tr>
        <th> Tasa [kS/s]</th>
        <th> Ticks (150 MHz)</th>
        <th> Color LED</th>
    </tr>
    <tr>
        <td> 8</td>
        <td> 18750</td>
        <td> Verde</td>
    </tr>
    <tr>
        <td> 16</td>
        <td> 9375</td>
        <td> Azul</td>
    </tr>
    <tr>
        <td> 22</td>
        <td> 6818</td>
        <td> Amarillo (R+G)</td>
    </tr>
    <tr>
        <td> 44</td>
        <td> 3409</td>
        <td> Rojo</td>
    </tr>
    <tr>
        <td> 48</td>
        <td> 3125</td>
        <td> Violeta (R+B)</td>
    </tr>
</table>

#### Botones

Los botones SW2 y SW3 son activos en bajo con resistencia pull-up. La interrupción se configura por flanco descendente.

<table align="center">
    <tr>
        <th> Botón</th>
        <th> Pin</th>
        <th> ISR</th>
        <th> Función</th>
    </tr>
    <tr>
        <td> SW3</td>
        <td> P0_6</td>
        <td> GPIO00_IRQHandler</td>
        <td> RUN / STOP</td>
    </tr>
    <tr>
        <td> SW2</td>
        <td> P0_23</td>
        <td> GPIO01_IRQHandler</td>
        <td> Cambio de tasa</td>
    </tr>
</table>

#### LEDs RGB

Los LEDs son activos en bajo y las macros `LED_RED_*`, `LED_GREEN_*` y `LED_BLUE_*` manejan la polaridad internamente.

## Screenshots

*(Pendiente — capturas de la placa funcionando)*