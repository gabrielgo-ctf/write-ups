# Car Crash
This ECU firmware dump, or what's left of it, was taken out of a crashed prototype car. We have to extract the logs from it to investigate the crash. Bad luck, we get some strange garbage printed instead.

Attached is a program you can reverse-engineer and a program you can test. Don't mix them up.

## Write-up
This challenge has a console to decrypt and print EDR data, but when we try to see the data, it prints garbage. We have the plain .hex, so we can reverse to see what is happening...

1. We can focus on option for decrypt data, it calls to sub_8F1

2. Reversing this function we will see it uses a static array. Searching this in Google we can find that it is used in the algorithm **Kuznyechik**
```
0x94, 0x20, 0x85, 0x10, 0xC2, 0xC0, 0x01, 0xFB, 0x01, 0xC0, 0xC2, 0x10, 0x85, 0x20, 0x94, 0x01 
```

3. The implementation seems to be standard but it has custom sbox tables kuz_pi and kuz_pi_inv 
```
kuz_pi                                              kuz_pi_inv
-----------------------------------------------     -----------------------------------------------
FC EE 4E 89 CF FD E6 1C 2D C4 FA DA 28 C5 04 90     39 16 5B DB 0E 30 C0 F8 D4 69 88 3B EB AD 52 91
AC AB BE 94 45 F3 01 BB C0 EB FB F5 8D C6 5E 85     7A D2 57 5A A4 FB 36 F0 21 72 AB 64 07 EF 43 7C
F2 18 B3 51 71 5C 39 B9 81 53 72 9E 8B 93 59 D7     8B C2 00 63 48 5D 4A EC 0C EE 6E E0 58 08 FF 82
05 8E 4C 36 E2 F9 16 7D D1 00 74 0B 31 4D D3 50     83 3C 8C DA 65 D5 00 59 47 26 C8 F2 89 A3 75 94
79 A7 68 1E EA C8 42 38 24 F7 26 A8 F6 75 CE CC     C3 AF 46 95 B3 14 BD E9 6D 7E 9F FD 32 3D 02 00
62 7B 0E 56 9B 95 E5 12 2C 37 13 02 9C 25 5D C3     3F 23 E8 29 CF 00 53 7F D9 2E D8 60 25 5E 1E 67
5B 76 96 23 1B 34 D4 5F 6B 09 E1 6E 9D 48 2A B0     7D 97 50 AE 8E A9 E2 F5 42 F6 CA 68 96 F9 6B C7
77 7E 19 E0 BA 3E 8A A9 22 82 10 80 1F 60 49 57     E1 24 2A C9 3A 4D 61 70 90 40 F1 51 AA 37 71 85
D5 F4 2F 30 E9 7F DB 87 0A 3C DD 20 32 B1 64 9A     7B 28 79 B6 F7 1F B8 00 D1 03 76 2C E7 1C 31 C1
78 0F EC DE 3F 43 6C 61 DC E8 FF C9 4F A0 92 4A     0F B2 9E 00 13 55 62 A1 B1 A7 8F 54 5C 6C 2B D3
A6 97 B6 3D 14 D6 F1 99 C7 65 7C 1A B2 0D 63 41     9D FC F4 CE D0 FA A0 41 4B 77 CB 11 10 00 BA E3
E3 98 91 44 B4 CD 83 BC 86 ED AE E4 F0 46 D0 E7     6F 8D AC 22 B4 E4 A2 DF E5 27 74 17 B7 EA 12 FE
06 8F 21 40 C2 AD C1 6F 3A 73 6A AA 55 D9 A3 54     18 C6 C4 5F 09 0D 1D A8 45 9B DC DE 4F B5 4E 04
A4 88 11 9F 08 35 F8 FE 5A 58 33 03 CA D8 CB B7     BE 38 F3 3E 66 80 A5 00 DD CD 0B 86 98 8A 93 E6
2B 70 66 AF B5 B8 DF 8C 52 47 BD 0C 27 EF 29 1D     73 6A 34 B0 BB 56 06 BF 99 84 44 19 92 B9 01 ED
17 7A 3B D2 A2 67 69 84 07 6D A5 15 A1 4B BF 2E     BC A6 20 15 81 1B 00 49 D6 35 0A 00 00 05 D7 9A
```
```
KEY
98 33 B8 ED 7D 22 F1 3F 26 3A 63 28 7A E4 42 87 C2 59 F1 AE C5 F1 C2 27 C6 A1 B9 7C E9 43 FC 02

ENCRYPTED DATA
B5 80 0C 18 E3 E9 24 2A AC C0 C8 56 66 4C 4D 56 22 1B 2F 7A 6E CF 7D 48 5B B5 29 DE 5F E6 87 E2  
61 1C 12 56 EE 41 09 2F D3 FF E2 C3 4A ED E0 4F EF D1 90 82 16 FE 3B 3F 6C AF D9 D4 1D 53 D6 35  
B5 80 0C 18 E3 E9 24 2A AC C0 C8 56 66 4C 4D 56 84 64 DC 64 E1 1A 14 D5 EC B3 DD 0E 48 17 62 2A  
FB 5E 48 71 BF 3E 60 31 02 F0 80 B3 7B 4F D3 EF B5 80 0C 18 E3 E9 24 2A AC C0 C8 56 66 4C 4D 56  
B1 9F FD D8 81 2F 15 43 EE 9A 7D 4B 48 4B 1D 1E A5 03 B4 7E A3 DF 5A C4 55 6F 80 1C BC 70 1F 39  
8C FF 14 19 DD 4E 1E 53 2C F0 7A 54 35 CB 44 B6 9A D3 CE 40 F0 D3 BB 4E 9C 40 1A C0 81 B3 36 AC  
19 F0 06 19 A7 BA 80 FD BE AB 5E 68 EB B3 03 83 FB 51 FB AB ED 61 00 CA C6 F1 2B 96 EF 64 A4 03  
AD DC 6D EE 4E C8 13 A4 77 C7 90 37 68 BE 4B 92
```

4. I found [this implementation](https://github.com/mjosaarinen/kuznechik) of Kuznyechik algo. Replacing the sbox tables and trying to decrypt data we obtain the same garbage returned by the board. It confirms the C code is correct.

5. kuz_pi_inv table seems corrupt... it has a lot of zero values. So we reconstruct this table using the kuz_pi as reference. The sbox tables have to satisfy this rule: if kuz_pi[XX]=YY then kuz_pi_inv[YY]=XX
```
Fixed kuz_pi_inv
-----------------------------------------------
39 16 5B DB 0E 30 C0 F8 D4 69 88 3B EB AD 52 91
7A D2 57 5A A4 FB 36 F0 21 72 AB 64 07 EF 43 7C
8B C2 78 63 48 5D 4A EC 0C EE 6E E0 58 08 FF 82
83 3C 8C DA 65 D5 33 59 47 26 C8 F2 89 A3 75 94
C3 AF 46 95 B3 14 BD E9 6D 7E 9F FD 32 3D 02 9C
3F 23 E8 29 CF CC 53 7F D9 2E D8 60 25 5E 1E 67
7D 97 50 AE 8E A9 E2 F5 42 F6 CA 68 96 F9 6B C7
E1 24 2A C9 3A 4D 61 70 90 40 F1 51 AA 37 71 85
7B 28 79 B6 F7 1F B8 87 D1 03 76 2C E7 1C 31 C1
0F B2 9E 2D 13 55 62 A1 B1 A7 8F 54 5C 6C 2B D3
9D FC F4 CE D0 FA A0 41 4B 77 CB 11 10 C5 BA E3
6F 8D AC 22 B4 E4 A2 DF E5 27 74 17 B7 EA 12 FE
18 C6 C4 5F 09 0D 1D A8 45 9B DC DE 4F B5 4E 04
BE 38 F3 3E 66 80 A5 2F DD CD 0B 86 98 8A 93 E6
73 6A 34 B0 BB 56 06 BF 99 84 44 19 92 B9 01 ED
BC A6 20 15 81 1B 4C 49 D6 35 0A 1A 00 05 D7 9A
```

6. With the fixed table we decrypt again the data and we obtain the flag
```
32 30 31 38 2D 30 31 2D 31 32 20 31 38 3A 31 32
3A 35 32 2E 30 32 34 09 45 43 55 09 45 52 52 09
43 41 4E 20 44 45 43 4F 44 45 52 20 45 52 52 4F
52 2C 20 53 4B 49 50 50 49 4E 47 20 4D 53 47 0A
32 30 31 38 2D 30 31 2D 31 32 20 31 38 3A 31 32
3A 35 32 2E 33 35 33 09 45 43 55 09 57 52 4E 09
43 41 4E 20 42 55 46 46 45 52 20 46 55 4C 4C 0A
32 30 31 38 2D 30 31 2D 31 32 20 31 38 3A 31 32
3A 35 32 2E 34 39 34 09 41 42 53 09 45 52 52 09
41 42 53 20 4F 46 46 4C 49 4E 45 0A 32 30 31 38
2D 30 31 2D 31 32 20 31 38 3A 31 32 3A 35 34 2E
39 35 30 09 45 43 55 09 44 42 47 09 44 55 4D 50
20 53 45 52 56 49 43 45 20 46 4C 41 47 20 62 62
39 34 62 32 61 35 66 61 65 65 64 36 34 35 37 32
30 30 31 66 65 35 37 37 32 61 64 31 33 64 0A 00

In ASCII:
2018-01-12 18:12:52.024	ECU	ERR	CAN DECODER ERROR, SKIPPING MSG
2018-01-12 18:12:52.353	ECU	WRN	CAN BUFFER FULL
2018-01-12 18:12:52.494	ABS	ERR	ABS OFFLINE
2018-01-12 18:12:54.950	ECU	DBG	DUMP SERVICE FLAG bb94b2a5faeed64572001fe5772ad13d
```

[Here](kuznechik_mod.zip) is my modification of Kuznyechik implementation to get the flag of this challenge