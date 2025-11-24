# Lab 1: Modulo ADC

## Objetivos

Manejar el modulo del Conversor Analogo-Digital (ADC) integrado en el MCU MK64FN1M0VLL12 de la familia Kinetis K64.

## Descripción

El laboratorio consiste en la lectura de una señal analogica a distintas velocidades de muestreo, siendo estas `8K/S`, `16K/S`, `22K/S`, `44K/S` y `48K/S`. Los cambios de las velocidades de muestreo serán realizados con una de las teclas de la placa, en forma de un buffer circular. Cada velocidad de muestreo se indicara a traves de un color RGB del LED integrado. Con otra tecla de la placa se habilitara la adquisicion o se parara la misma (`RUN`/`STOP`). Los valores adquiridos seran almacenados en memoria en un buffer circular de `512` muestras del tipo `q15` (fraccional 15bits) y a su vez seran enviados a traves del DAC (de 12 bits).

## Desarrollo
