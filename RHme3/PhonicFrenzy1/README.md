# Phonic Frenzy 1
Your car has a brand new infotainment center. In order to activate the system you have to register the serial number on the company's website. The only problem is, it's not trivial to find the dang thing.

## Write-up
This challenge confused me during lot of time... when I checked it, the first thing I saw were some strange pulses in PINs A0-5 and AREF. I was trying to combine the pulses to get an audio signal (because the name "phonic"), reading information about audio modulations, nothing made sense. While I was fighting with these pulses I was seeing that many people resolved this challenge, so it should be something "easy". Finally it was...

Connecting an speaker or headphones between PINs D7(or D8) and GND we can heard an audio. This audio has very very bad quality and is difficult understand what is saying. The most difficult thing was distinguish some letters like B/D/E or 8/A, so I recorded them in audacity and used the waveform to identify the same digits.

After some tries (I still hate the captchas), I found the correct flag: a623952272bc4123f76940d402cfdbd1

I prepared this "alphabet" of waveforms.
![waveforms.jpg](waveforms.jpg)
- It is recorded putting the headphone near the speaker of the laptop. Using a direct cable to the speaker input the waveforms are different. 
- In this flag I hadn't E and 8, but I could complete the alphabet later with phonic frenzy 2 flag.
