iAqua Lite: A DIY Full Spectrum Cross-fading Ecoxotic E-Series Light Controller

This is a repository for AnotherHobby's iAqua-Lite LED Controller 
based on the Arduino Mega 2560.

This repository is meant to be dropped into (On windows: c:/User Dir/User Name/Documents/Arduino/)
You'll also want to remove the Robot_IR_Remote from program files/Arduino/libraries folder before compiling



Excerpt from original post on PlantedTank.net
http://www.plantedtank.net/forums/showthread.php?t=783426

Here is the quick pitch, and why anybody can make one of these:

No soldering. No code. No drilling. No Cutting. No case fabrication. $50 in parts.

Just order parts, plug them in as outlined below, and upload some free code over USB. When you are done, it's actually an attractive looking device that doesn't look hacked together.

So yes, this is another DIY Arduino controller project, but I started this one with a very simple driving motive: make it insanely easy to build.

A lot of people shy away from these projects because they get very technical. Lots of soldering, case fabrication, diagrams, and code editing. This project has zero of any of that. If you can play legos and use a computer, you can do this. The most technical part in the entire project is reading the pin numbers on some parts. That's it. Everything just plugs together with minimal effort, using parts bought online, and there is no code editing at all. Just plug in the USB cable, and upload. All configuration is done in the menus on the device, even setting the clock.

Quick and dirty software feature list:
100% LCD and button menu configuration (literally ZERO code editing, just upload the sketch via USB)
Easy to use and navigate LCD menu system
Home Screen with Date / Time / Current Mode / Remaining Time
Optional Power On / Off via IR for the light (can be disabled to run 24x7 or use external power)
6 Programmable Full Spectrum Cross Fade Schedules
Set Date / Time in menu (no need to use code to program the clock chip)
Globe friendly: 12 or 24 hour time display setting in menu
Globe friendly: MM/DD/YYYY or DD/MM/YYYY date display setting in menu
Set Screen Brightness in menu
All settings instantly saved to EEPROM, meaning it saves your settings when powered off or rebooted.

What does it actually do?

Essentially it uses the M1, M2, Daylight, and Moon memory buttons as starting and ending points for cross fading. You get to schedule 6 of these fades to start end end when you want, as well as power on/off. Essentially, you get to control moon > dawn > sunrise > mid day sun > sun set > dusk > moon. Power off would typically happen after the transition to moon is done, and power on would be before the transition to dawn starts.

Parts and Cost!
Depending on how fast you want your parts, this controller will cost as cheap as $50 (including shipping), or maybe as much as $10 more if you want the parts fast. 

The parts used in this project were specifically selected because they allow the elimination all wiring and soldering, and they fit together correctly in an exclosure that doesn't need to be modified. You can get most of the parts from the US, which will show up quick. The parts from China will take about 3 weeks normally. No matter what, the clock chip has to come from China. I cannot find a reasonably priced US source, so no matter what you will have to wait anyway, so I’d just get the cheapest parts. You will need one of each of the following (prices may change, evilBay actions my disappear later):

Arduino Mega 2560 (required due to the amount of memory used by the menu system)
- Cheap US Source: evilBay item 201197139954 for $13.85 shipped
- Cheap China Source: evilBay item 321462769303 for $11.17 shipped

ZiGo Clear Enclosure for Arduino Mega & Ethernet shield
- You will definitely want to pay the $4 for expedited shipping. You choose this when viewing your shopping cart. I did not choose this and it took almost a month and a half to get. I’m guessing the $4 gets you the item in half the time, and that’s well worth it.
- The ZiGo case was selected because it's clear, attractive, cheap, pre-cut and pre-drilled, and everything fits. They will also take custom orders (which you will want to do to have it not engraved). I don’t absolutely love the case, but it does work pretty well, and I cannot find another case that fits all of the components correctly and requires zero modification. The only downside is that for final assembly, it should be glued together (except the top!). I really wish it held together on it's own, but it doesn't really do that. A rubber band would hold it together as well. There aren't any other case options I could find that worked out of the box, but if you don’t mind drilling a few holes or fabricating mounting, there are plenty of options. One example is  this box on Amazon for $10 in the small size this box on Amazon for $10 in the small size 
- http://www.amazon.com/gp/product/B00017UT3U/?tag=viglink20386-20
- IMPORTANT: When you check out with paypal, put instructions to NOT engrave the ZiGo logo on the case. Tell them you want a completely clear top.
- Buy it here for $17.99 shipped.

A standard USB power adapter and A-B cable, or a 9v Power Adapter with a standard 2.1mm plug
- There are many sources for these. A USB cable and USB power adapter are cheap and work great. You probably have one on hand.
- Should not cost more than $5.  Here is one for $5 on amazon Here is one for $5 on amazon
- http://www.amazon.com/HTC-Travel-Charger-Adapter-Smartphones/dp/B0047EP8IY/?tag=viglink20386-20

LinkSprite 16X2 LCD Keypad Shield for Arduino Version A
- Make sure you get version A. This screen was selected for specific physical properties that allow it to fit in the case. The only other screen that I could find that fits is the more expensive ADA fruit version that needs to be soldered.
- Cheapest is to buy direct from LinkSprite for $14.58 shipped.
http://store.linksprite.com/linksprite-16x2-lcd-keypad-shield-for-arduino-version-a/

A very specific DS1302 RTC board (to keep time)
- This exact RTC board was selected because it will fit in the case and it can run on ANY pins you choose. You can use other RTC chips, but you will either have to solder, or it won’t fit in the case. There are several varieties of DS1302 boards, some with slightly different pin layouts, and some with bigger batteries. It has to be the same exact one pictured in this post.
- http://www.miniinthebox.com/ds1302-real-time-clock-module-for-arduino-works-with-official-arduino-boards-2-0-5-5v_p1141513.html?utm_campaign=27795&litb_from=affiliate_gan&utm_source=gan&utm_medium=affiliate
- Good news is that it’s cheap.
- Bad news is that it’s only available from China, check out evilBay item 371208561894 for $3.67
- This site claims faster shipping for only $1.88, but I’ve never ordered from there.

A very specific IR LED breakout board
- This exact IR LED board was selected for size. It’s the only one that will fit in the case. It must look exactly like the pictures below.
- Cheap from China for $2.75 shipped, evilBay item 390893458380
- If you want it fast,  here it is on Amazon Prime for $6.08 shipped here it is on Amazon Prime for $6.08 shipped .
http://www.miniinthebox.com/ds1302-real-time-clock-module-for-arduino-works-with-official-arduino-boards-2-0-5-5v_p1141513.html?utm_campaign=27795&litb_from=affiliate_gan&utm_source=gan&utm_medium=affiliate


Physical Assembly

Screw the Arduino Mega to the bottom part of the case with the screws, nuts, and plastic bushings that are included with the ZiGo case:

Next you will plug the LCD and the IR LED into the Mega. The LCD plugs in so the buttons under the display are on the power jack side of the Mega, as seen below. The left edge of the display board (oriented such that the buttons are considered down) should line up perfectly with the edge of the Mega. Before you plug the IR LED in, you need to bend the legs of the LED itself so that it’s pointed out to the right, but not too far so it still fits in the case (as seen in the pic). I used a small needle nosed pliers to bend mine. The important thing is that the entire LED is below the top edge of the board, and it doesn’t stick out very far or it’ll hit the case. Plug the IR LED board into the Mega so that the “S” pin goes into pin 46 on the Mega and the "-" pin goes into pin 42 on the Mega, with the middle pin going into 44 on the Mega.

Next thing is to plug the DS1302 RTC into the Mega so that the pins span across 27, 29, 31, 33, 35, and 37 (with 35 and 37 being the VCC pins and 27 being the RST pin). The battery will face away from the Mega. You will also need to bend the pins so that the RTC sits at and angle to allow it to fit in the case. I found this easiest to accomplish by plugging it in first, and then bending it. It was also easier with the LCD shield removed, and then put back in to check fitment. The first pic shows how much I bent the pins for a perfect fit, and the second pic shows it in place.

At this point, all electronics are assembled. I'd hold off on putting the case together until you are done configuring the controller, as it is easier to do so without the case. So the next step is to find your USB cable, and upload the software from your computer!

Software Installation

Download the Arduino IDE for free, and install it on your computer: http://arduino.cc/en/main/software

After you have done that, either download and unzip the this repo in the proper location, (usually documents/arduino/)

Launch the Arduino program that you installed, and go to the menu: Sketch > Import Library > Add Library. Navigate to the package that you unzipped, and go into the "Additional Required Libraries" folder, and add each one of the libraries. When you are done with adding all of those, you need to move the iAquaLite folder from the downloaded package into your Arduino projects folder. You'll find it in your documents folder (My Documents on Windows, Documents on Mac).

NOTE: If you already have the IRremote library, you need to either replace it with the one in this project, or edit yours to use pin 46 on the Mega. To edit your existing library, find the IRremoteInt.h file in the IRremote library, and find the lines pertaining to the timers for an Arduino Mega. Comment out whatever timer/pin is set, and uncomment this line: #define IR_USE_TIMER5 // tx = pin 46.

Once you've done that, quit and relaunch the Arduino program and go to the File > Sketchbook menu and choose iAquaLite. Plug the Arduino into your computer via USB. In the Arduino program go to the menu Tools > Board and make sure Arduino Mega 2560 is selected. In your sketch window, click upload. 

iAquaLite Configuration

To keep this post from becoming insanely long, I have put the configuration into a PDF file for download. You can get it here: iAqua_Lite_User_Guide.pdf

Closing Notes

The final case assembly is not tight, and won't hold together on it's own. I have not glued mine yet, but I think that's the way to go. A rubber band would work, and I'm sure there are other possibilities. If anybody comes up with something, post it. 

I hereby release all of the software and information contained here for public use and modification, so long as it is not for profit. If it's for profit, contact me. Otherwise, have fun with it. 

On a final side note, the hardware is not specific to the Ecoxotic light and could be used to control any IR controllable device. The only part of the software that is absolutely unique to the Ecoxotic is the IR codes. You could conceivably use the same hardware and software to control other IR controllable light or devices with a little modification (such as the Current Satellite Plus).
