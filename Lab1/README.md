# Lab 1: Analog Digital Converter (ADC)

## Objetivo

- Manejar el modulo ADC de la placa FRDM-MCXN947.

## Consigna

Realizar la lectura de una señal analogica a distintas velocidades de muestreo en KiloSamples/segundo: `8 kS/s`, `16 kS/s`, `22 kS/s`, `44 kS/s` y `48 kS/s`.

Los cambios de las velocidades de muestreo serán realizados con una de las teclas de la placa, en forma de un buffer circular. Cada velocidad de muestreo se indicara a traves de un color RGB del LED integrado. 

Con otra tecla de la placa se habilitara la adquisicion y paro de muestras (**RUN** / **STOP**). 

Los valores adquiridos seran almacenados en memoria en un buffer circular de `512 muestras` del tipo `q15` (fraccional de 15 bits) y a su vez seran enviados a traves del DAC de 12 bits.

## Desarrollo

