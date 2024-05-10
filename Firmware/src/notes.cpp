/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#define  Cm1   0         // 8.18 Hz
#define  Cdm1  1         // 8.66 Hz
#define  Dm1   2         // 9.18 Hz
#define  Ddm1  3         // 9.72 Hz
#define  Em1   4         // 10.30 Hz
#define  Fm1   5         // 10.91 Hz
#define  Fdm1  6         // 11.56 Hz
#define  Gm1   7         // 12.25 Hz
#define  Gdm1  8         // 12.98 Hz
#define  Am1   9         // 13.75 Hz
#define  Adm1  10        // 14.57 Hz
#define  Bm1   11        // 15.43 Hz
#define  C0    12        // 16.35 Hz
#define  Cd0   13        // 17.32 Hz
#define  D0    14        // 18.35 Hz
#define  Dd0   15        // 19.44 Hz
#define  E0    16        // 20.60 Hz
#define  F0    17        // 21.83 Hz
#define  Fd0   18        // 23.13 Hz
#define  G0    19        // 24.50 Hz
#define  Gd0   20        // 25.98 Hz
#define  A0_   21        // 27.50 Hz
#define  Ad0   22        // 29.14 Hz
#define  B0_   23        // 30.87 Hz
#define  C1    24        // 32.70 Hz
#define  Cd1   25        // 34.65 Hz
#define  D1_   26        // 36.71 Hz
#define  Dd1   27        // 38.89 Hz
#define  E1    28        // 41.20 Hz
#define  F1    29        // 43.65 Hz
#define  Fd1   30        // 46.25 Hz
#define  G1    31        // 40.00 Hz
#define  Gd1   32        // 51.91 Hz
#define  A1_   33        // 55.00 Hz
#define  Ad1   34        // 58.27 Hz
#define  B1_   35        // 61.74 Hz
#define  C2    36        // 65.41 Hz
#define  Cd2   37        // 69.30 Hz
#define  D2_   38        // 73.42 Hz
#define  Dd2   39        // 77.78 Hz
#define  E2    40        // 82.41 Hz
#define  F2    41        // 87.31 Hz
#define  Fd2   42        // 92.50 Hz
#define  G2    43        // 98.00 Hz
#define  Gd2   44        // 103.83 Hz
#define  A2_   45        // 110.00 Hz
#define  Ad2   46        // 116.54 Hz
#define  B2    47        // 123.47 Hz
#define  C3    48        // 130.81 Hz
#define  Cd3   49        // 138.59 Hz
#define  D3    50        // 146.83 Hz
#define  Dd3   51        // 155.56 Hz
#define  E3    52        // 164.81 Hz
#define  F3    53        // 174.61 Hz
#define  Fd3   54        // 185.00 Hz
#define  G3    55        // 196.00 Hz
#define  Gd3   56        // 207.65 Hz
#define  A3_   57        // 220.00 Hz
#define  Ad3   58        // 233.08 Hz
#define  B3    59        // 246.94 Hz
#define  C4    60        // 261.63 Hz
#define  Cd4   61        // 277.18 Hz
#define  D4    62        // 293.67 Hz
#define  Dd4   63        // 311.13 Hz
#define  E4    64        // 329.63 Hz
#define  F4    65        // 349.23 Hz
#define  Fd4   66        // 369.99 Hz
#define  G4    67        // 392.00 Hz
#define  Gd4   68        // 415.31 Hz
#define  A4_   69        // 440.00 Hz
#define  Ad4   70        // 466.16 Hz
#define  B4    71        // 493.88 Hz
#define  C5    72        // 523.25 Hz
#define  Cd5   73        // 554.37 Hz
#define  D5    74        // 587.33 Hz
#define  Dd5   75        // 622.25 Hz
#define  E5    76        // 659.26 Hz
#define  F5    77        // 698.46 Hz
#define  Fd5   78        // 739.99 Hz
#define  G5    79        // 784.00
#define  Gd5   80        // 830.61
#define  A5_   81        // 880.00
#define  Ad5   82        // 932.33
#define  B5    83        // 987.77
#define  C6    84        // 1046.50
#define  Cd6   85        // 1108.73
#define  D6    86        // 1174.66
#define  Dd6   87        // 1244.51
#define  E6    88        // 1318.51
#define  F6    89        // 1396.91
#define  Fd6   90        // 1479.98
#define  G6    91        // 1567.98
#define  Gd6   92        // 1661.22
#define  A6_   93        // 1760.00
#define  Ad6   94        // 1864.66
#define  B6    95        // 1975.53
#define  C7    96        // 2093.01
#define  Cd7   97        // 2217.46
#define  D7    98        // 2349.32
#define  Dd7   99        // 2489.02
#define  E7    100       // 2637.02
#define  F7    101       // 2793.83
#define  Fd7   102       // 2959.96
#define  G7    103       // 3135.96
#define  Gd7   104       // 3322.44
#define  A7_   105       // 3520.00
#define  Ad7   106       // 3729.31
#define  B7    107       // 3951.07
#define  C8    108       // 4186.01
#define  Cd8   109       // 4434.92
#define  D8    110       // 4698.64
#define  Dd8   111       // 4978.03
#define  E8    112       // 5274.04
#define  F8    113       // 5587.65
#define  Fd8   114       // 5919.91
#define  G8    115       // 6271.93
#define  Gd8   116       // 6644.88
#define  A8_   117       // 7040.00
#define  Ad8   118       // 7458.62
#define  B8    119       // 7902.13
#define  C9    120       // 8372.02
#define  Cd9   121       // 8869.84
#define  D9    122       // 9397.27
#define  Dd9   123       // 9956.06
#define  E9    124       // 10548.08
#define  F9    125       // 11175.30
#define  Fd9   126       // 11839.82
#define  G9    127       // 12543.85
