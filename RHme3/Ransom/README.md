# Ransom
In theory, this firmware mod was supposed to give you 30% extra horsepower and torque. In reality, it's something different.

## Write-up
This challenge shows us a user ID and ask an unlock code, we need to reverse to see if we can find the this unlock code.

1. Testing this challenge in the simulator I saw in RAM an ASCII string similar to the user ID.

2. After the loop (from 0354 to 0387) there is a 16 bytes hex string in ram... maybe it is the unlock code?

3. Reset... break at 0354... replace our user ID in RAM... continue to 0388... copy the unlock code :grin:

4. We test the unlock code and we get the flag
```
Your car is taken hostage by REVENANTTOAD ransomware version DEBUG_a4fae86c.
To get your car back, send your user ID:
3835320707001500

and $1337 to the following rhme3coin address: 
[CENSORED].

Already paid? Then enter the received unlock code here:

267EDED9BE5402DA79E745ABEEDEBD14

It was a pleasure doing business with you.
Your car is now unlocked.
Here is a bonus:
b724f43058ca3cc7fcd276ddeb46ef2f
Have a nice day!
```