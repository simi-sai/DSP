# Coeficientes de Filtro FIR

pkg load signal;

## Filtro Pasa Bajos

fc = 3600;       # 3600 Hz
A_stop = 30;     # 30 dB
fs = 48000;      # 48 kHz (frecuencia mas alta de muestreo del ADC)
BW = 0.2;        # 20 %

N = round(4/BW);    # Longitud de Filtro

coeff = fir1(N, fc/(fs/2), 'low');  # Coeficientes
freqz(coeff, 1, 1024, fs);          # Respuesta en frecuencia
