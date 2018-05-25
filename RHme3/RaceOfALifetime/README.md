# Race of a lifetime
You are participating in a race around the world. The prize would be a personalized flag, together with a brand new car. Who wouldn't want that? You are given some locations during this race, and you need to get there as quick as possible. The race organisation is monitoring your movements using the GPS embedded in the car. However, your car is so old and could never win against those used by the opposition. Time to figure out another way to win this race.

## Write-up
When we connect to the console we see a map, our position and the instruction: "Head to the Riscure office to get the directions.".

1. When we try to send data, sometimes we receive a message indicating the format is not correct, and sometimes other indicating we went to fast. To avoid the second error we can test sending our location... after a few test we will see that the correct format for sending the coordinates is  
XXX.XXX, YYY.YYY

2. We already know how to send our position, so, we need to know where we have to go. If we search in google or riscure website where is located the head office we will see the address Delftechpark 49 2628 XJ Delft The Netherlands. We search this address in google maps and we see the coordinates
```
Riscure Head Office (Delft): 51.997899, 4.3853194
```

3. If we send this coordinates to the car and we will receive a message that we went to fast. We need to calculate intermediate points to simulate a reasonable speed (for example, around 120km/h). Doing it we can arrive to the Riscure Head Office and we get the message 
```
Reach the destination within the next 49 hours in order to be the first at the site.
Location: 31.78 123.91
You can take a flight if you need one at the airports of Paris (CDG), Shanghai (PVG) and San Francisco (SFO).
```

4. Argg! It is too far... time to do something automatic, because each mistake forces us to restart the challenge again, and doing it manually we have to spent a lot of time calculating intermediate points and is very easy have a mistake.

5. For future use, we can search in google maps where are the airports and Riscure America to obtain their coordinates
```
Paris (CDG): 49.009722, 2.547778
Shanghai (PVG): 31.143333, 121.805278
San Francisco (SFO): 37.618889, -122.375
Riscure America: 37.7932335, -122.4048855
````

Here is the code I prepared quickly to go from one point to another in a number of steps. I did it in C# because I am comfortable with this language.
```cs
void go(Double latFrom, Double lonFrom, Double latTo, Double lonTo, Double hours)
{
	Double difLat = latTo - latFrom;
	Double difLon = lonTo - lonFrom;
	Double deltaLat = difLat / hours;
	Double deltaLon = difLon / hours;

	Double latitud = latFrom;
	Double longitud = lonFrom;

	for (int i = 0; i < hours; i++)
	{
		if (i == hours - 1)
		{
			latitud = latTo;
			longitud = lonTo;
		}
		else
		{
			latitud += deltaLat;
			longitud += deltaLon;
		}

		string la = (latitud > 180 ? latitud - 360: latitud).ToString("0.000").Replace(",", ".");
		string lo = (longitud > 180 ? longitud - 360 : longitud).ToString("0.000").Replace(",", ".");

		send(la + ", " + lo + "\n");
	}
}
```
And I also prepared two functions to read the coordinates received in the messages
```cs
Double[] getStartPosition(string text)
{
	string slat = text.Substring(text.IndexOf("Latitude: ") + 10, 9);
	string slon = text.Substring(text.IndexOf("Longitude: ") + 11, 9);
	
	Double lat = Convert.ToDouble(slat.Replace(".", ",").Trim());
	Double lon = Convert.ToDouble(slon.Replace(".", ",").Trim());
	
	return new Double[] { lat, lon };
}
Double[] getDestination(string text)
{
	string slat = text.Substring(text.IndexOf("Location: ") + 10, 5);
	string slon = text.Substring(text.IndexOf("Location: ") + 16, 6);
	
	Double lat = Convert.ToDouble(slat.Replace(".", ",").Trim());
	Double lon = Convert.ToDouble(slon.Replace(".", ",").Trim());
	
	return new Double[] { lat, lon };
}
```

6. With these functions is easy reach the destinations and receive the instructions. Here is the complete code needed to get the flag and the log of messages received
```cs
send("p\n");

String message  = getOutput();
Double[] sp = getStartPosition(message);

go(sp[0], sp[1], 51.99, 4.38, 40);                //From start point to Riscure Head Office in 40 hours (CAR)

message = getOutput();
Double[] dp = getDestination(message);

go(51.99, 4.38, 49.00, 2.54, 4);                  //From Riscure Head Office to Pris Airport in 4 hours (CAR)
go(49.00, 2.54, 31.14, 121.80, 20);               //From Pris Airport to Shanghai Airport in 20 hours (PLANE)
go(31.14, 121.80, dp[0], dp[1], 12);              //From Shanghai Airport to destination in 12 hours (CAR)
go(dp[0], dp[1], 31.14, 121.80, 12);              //From destination to Shanghai Airport in 12 hours (CAR)
go(31.14, 121.80, 37.62, 360 - 122.37, 20);       //From Shanghai Airport to San Francisco Airport in 20 hours (PLANE)
go(37.62, 360 - 122.37, 37.79, 360 - 122.40, 1);  //From San Francisco Airport to Riscure America in 1 hour (CAR)
go(37.79, 360 - 122.40, 37.62, 360 - 122.37, 1);  //From Riscure America to San Francisco Airport in 1 hour (CAR)
go(37.62, - 122.37, 49.00, 2.54, 16);             //From San Francisco Airport to Paris Airport 16 hours (PLANE)
go(49.00, 2.54, 51.99, 4.38, 4);                  //From Pris Airport to Riscure Head Office in 4 hours (CAR)
```
```
LOG
------------------------------------------------------------------------------------------------
[37;42mX[0m	-	Your location
Latitude: 35.330002	Longitude: 27.420000
[37;41mF[0m	-	Final destination

                `::- .-o`hMM| :hMMMMMMMMMMMMd        `d-|                    `o-                    
                sNhh||hdMyso  `|oNMMMMMMMMMMh                     yh+     :ohMMMM+     .ys+:.       
               syo:` `+yoyy|-    .NMMMMMMMMM-                    hs  . `|NMMMMMMMN+:|- `|dm+s`      
      .-`      shmMmNhhM+sMMMm.   |MMMMMMMMM-            `      :d` `mymNMMMMMMMMMMMMM+|hMMms-``    
`   sMMMNds+hymhooMMMMhMN|MMMMM+   dMMMMMMh|          -hNMNy+` ..-+oyMMMMMMMMMMMMMMMMMMMMMMMMMMNsoys
md+`yMMMMMMMMMMMMMMMMMMMMMM+`yMMo  sMMMm|` `-:       `NMMMMMMNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMChpMMMMM
 -..mMMMMMMMMMMMMMMMMMMMNyhs +mM-  -MMd`   -hh`     :mMosMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMh
   `mMMMNshNMMMMMMMMMMMM.   sMs:-   :y`           [37;41mF[0m|MMM.dMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNmmh|sd++- 
    -oo:`  `+MMMMMMMMMMMms| hMMMN:             `os `yyh|dMMMMMMMMMMMMMMMMMMMMMMMNMMMMMMMh`   |h.    
   `.        -dMMMMMMMMMMMMhMMMMMmo           .y:mssMMMMMMMMMMMMMMMMMMMMMMMMMMMhdMMMMMMMdy.  ::     
              `mMMMMMMMMMMMMMMMMy.o-             sMMMMMMM[37;42mX[0mMMMMMNNMMMMMMMMMMMMMMMMMMMMMMMdoh         
               +MMMMMMMMMMMMMMN+:.             -osdo|ysdMhd:oNMooMMMMMMMMMMMMMMMMMM   my o          
               `mMMMMMMMMMMMM:`                .hh:|| -`:-yyMMMmsMMMMMMMMMMMMMMMMMMd+  |s|          
                .hMMMMMMNMMM+                 :hMMMMMd+|so|yMMMMNMMMMMMMMMMMMMMMMMMm``||`           
                  -:NMM|  -y-`               +MMMMMMMMMMMMMdmMMN+yosdMMMMMMMMMMMMMd-                
                    `+dyyy-`-|+-             sMMMMMMMMMMMMMM+oMMNy.  -dMMm|-NMMNs. -                
                        .+do:so+o:`          :NMMMMMMMMMMMMMMyos`     `ms   `msh.  -y-              
                           `NMMMMMMh|         `|||||dMMMMMMMMMN+        -    ds `os .`              
                           oMMMMMMMMMmo:            -NMMMMMMM:               .d--y+   .ss|`         
                           -NMMMMMMMMMMm`            +MMMMMMM.                 `-.    ---o+-        
                            `oNMMMMMMMM              hMMMMMMs.|d`                  -dmMMsmh-        
                              +MMMMMms:              .NMMMMy  |:                 :mMMMMMMMMM:       
                              sMMMMN-                 :MMNs`                     :MMMNmNMMMMo       
                             `NMMMs`                   --                         |-`  .|dNy`     +:
                             |MMo`                                                        :.     |: 
                             hMd                                                                `:  
                             yM.                                                                    
                              |:                                                                    
                                                                                                    
                                 :`                                                                 
                               |dd                             .os|.    `o+odNddyyyyyyyhy+.         
                             .sdMN`              `. `.-:|oydysmMMMMMNdoomMMMsecret_sauceMMMNhs|`    
                     `::.`   dMMMy           `sdmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMd-  
           `-:+dyso+oNMMMMmymNMMMo         .yNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMm:  
        +hNMMMMMMMMMMMMMMMMMMMMMMd:       yMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM.   
.     yMMMMMMMMMMMMMMMMMMMMMMMMMMMMd+.  |dMM_RSPO_MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMs++:
Head to the Riscure office to get the directions.


Welcome to Riscure!
There is a new prototype Edison Model L sunk in the East China Sea while it was being transferred to a secret research facility.

Reach the destination within the next 49 hours in order to be the first at the site.
Location: 31.78 123.91
You can take a flight if you need one at the airports of Paris (CDG), Shanghai (PVG) and San Francisco (SFO).


Welcome to Paris airport
Take off...


Welcome to Shanghai airport
Landed...


Well done!
Bring the ECU to Riscure North America where it can be analysed.
Try to be there within 44 hours.
Location: 550 Kearny St, San Francisco, CA, United States


Welcome to Shanghai airport
Take off...


Welcome to San Francisco airport
Landed...


Unfortunately, the analyst who would study the ECU did not get the visa to work at the office.
Hurry up and find him in the office in Delft in order to recover the flag.
You have no more than 21 hours before it is too late.


Welcome to San Francisco airport
Take off...


Welcome to Paris airport
Landed...


Congratulations! You managed to deliver the ECU in time.
Your flag is:
dab8833e0a98d2469342efbfd9368ffc
==END==
```