# Coeficientes de Filtro FIR — Lab 2 DSP

clc; clear all; close all;

pkg load signal;

fs = 48000;

## Beta para Kaiser (A_stop = 30 dB) — filtros 1, 2 y 3
A_stop = 30;
beta_30 = 0.5842 * (A_stop - 21)^(0.4) + 0.07886 * (A_stop - 21);

## 1. Filtro Pasa Bajos

fc = 3600;       # 3600 Hz
BW = 1000;       # 1000 Hz (transicion: 3600 -> 4600 Hz)
N = ceil((A_stop - 8) / (2.285 * 2*pi*BW / fs) + 1);

low_pass_coeff = fir1(N, fc/(fs/2), 'low', kaiser(N+1, beta_30));
figure(1);
freqz(low_pass_coeff, 1, 1024, fs);
title("Respuesta en frecuencia - Filtro Pasa Bajos");

## 2. Filtro Pasa Altos

fc = 35;         # 35 Hz
BW = 500;        # 500 Hz (transicion: 0 -> 35 Hz)
N = ceil((A_stop - 8) / (2.285 * 2*pi*BW / fs) + 1);

high_pass_coeff = fir1(N, fc/(fs/2), 'high', kaiser(N+1, beta_30));
figure(2);
freqz(high_pass_coeff, 1, 1024, fs);
title("Respuesta en frecuencia - Filtro Pasa Altos");

## 3. Filtro Pasa Banda

fc = [35 3500];  # 35 Hz - 3500 Hz
BW = 500;        # 500 Hz
N = ceil((A_stop - 8) / (2.285 * 2*pi*BW / fs) + 1);

band_pass_coeff = fir1(N, fc/(fs/2), 'bandpass', kaiser(N+1, beta_30));
figure(3);
freqz(band_pass_coeff, 1, 1024, fs);
title("Respuesta en frecuencia - Filtro Pasa Banda");

## 4. Filtro Rechaza Banda

fr = 50;         # 50 Hz
BW = 15;         # 15 Hz
A_stop_bs = 25;  # 25 dB
fc = [fr-BW/2 fr+BW/2];  # 42.5 Hz - 57.5 Hz
BW_trans = 100;  # 100 Hz (transicion)
beta_bs = 0;     # Beta = 0 (rectangular)
N = ceil((A_stop_bs - 8) / (2.285 * 2*pi*BW_trans / fs) + 1);

stop_filter_coeff = fir1(N, fc/(fs/2), 'stop', kaiser(N+1, beta_bs));
figure(4);
freqz(stop_filter_coeff, 1, 1024, fs);
title("Respuesta en frecuencia - Filtro Rechaza Banda");

## -----------------------------------

## Exportar coeficientes a header C

to_q15 = @(b) round(b * 32768);
LP = to_q15(low_pass_coeff / max(abs(low_pass_coeff)));
HP = to_q15(high_pass_coeff / max(abs(high_pass_coeff)));
BP = to_q15(band_pass_coeff / max(abs(band_pass_coeff)));
BS = to_q15(stop_filter_coeff / max(abs(stop_filter_coeff)));

fid = fopen('~/Desktop/S/DSP/Lab2/src/FIR_MCXN947/source/coeficientes.h', 'w');
fprintf(fid, '#ifndef COEFICIENTES_H\n#define COEFICIENTES_H\n\n');
fprintf(fid, '#include <stdint.h>\n');
fprintf(fid, '#include <arm_math_types.h>\n\n');
fprintf(fid, '#define TAPS_LP %d\n', length(LP));
fprintf(fid, '#define TAPS_HP %d\n', length(HP));
fprintf(fid, '#define TAPS_BP %d\n', length(BP));
fprintf(fid, '#define TAPS_BS %d\n\n', length(BS));
fprintf(fid, '#define FIR_MAX_TAPS %d\n\n', max([length(LP) length(HP) length(BP) length(BS)]));

fprintf(fid, 'static const q15_t coeffs_lp[TAPS_LP] = {\n    ');
for i = 1:length(LP)
    fprintf(fid, '%6d', LP(i));
    if i < length(LP), fprintf(fid, ','); end
    if mod(i, 8) == 0 && i < length(LP), fprintf(fid, '\n    '); end
end
fprintf(fid, '\n};\n\n');

fprintf(fid, 'static const q15_t coeffs_hp[TAPS_HP] = {\n    ');
for i = 1:length(HP)
    fprintf(fid, '%6d', HP(i));
    if i < length(HP), fprintf(fid, ','); end
    if mod(i, 8) == 0 && i < length(HP), fprintf(fid, '\n    '); end
end
fprintf(fid, '\n};\n\n');

fprintf(fid, 'static const q15_t coeffs_bp[TAPS_BP] = {\n    ');
for i = 1:length(BP)
    fprintf(fid, '%6d', BP(i));
    if i < length(BP), fprintf(fid, ','); end
    if mod(i, 8) == 0 && i < length(BP), fprintf(fid, '\n    '); end
end
fprintf(fid, '\n};\n\n');

fprintf(fid, 'static const q15_t coeffs_bs[TAPS_BS] = {\n    ');
for i = 1:length(BS)
    fprintf(fid, '%6d', BS(i));
    if i < length(BS), fprintf(fid, ','); end
    if mod(i, 8) == 0 && i < length(BS), fprintf(fid, '\n    '); end
end
fprintf(fid, '\n};\n\n');

fprintf(fid, '#endif\n');
fclose(fid);

fprintf('coeficientes.h generado en source/\n');
