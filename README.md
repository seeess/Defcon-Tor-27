# Defcon 27 Tor Badge / SAO Manual
![badge image](https://i.imgur.com/BjB4c4tl.jpg "front and back")
****
[Here's a feature overview in video form](https://www.youtube.com), you probably want to watch this at 1.5x speed
****
I wanted to make another badge, and Tor mentioned they will have a vendor booth this year. I took all the monitary risk, but these turned out pretty well. I'm giving these to Tor to sell as a fund raiser so hopefully I'll break even. This doesn't count the many hours planning, designing, coding, and crimping 500+ JST battery holders. I spent more time on the presentation this time (custom dye sub lanyards, custom boxes, sticker, etc).

## In the box
* Tor badge
* Lanyard
* Two CR123a non-rechargeable batteries 
* Battery holder
* Sticker
* Zip ties
* Manual

### Powering the Tor SAO
This Tor board supports the new [1.69bis “shitty add-on”](https://hackaday.com/2019/03/20/introducing-the-shitty-add-on-v1-69bis-standard/) (SAO) standard, there is one 2x3 male pin header that you can connect to a main badge that supports the SAO standard for power. You can also power this board from older badges that use the 2x2 pin SAO standard. When using the older 2x2 standard verify the correct pins are connected. This board only utilizes the “3v3” and “gnd” pins of the SAO standard (other pins on the SAO header are not connected).

Alternatively, if you don’t have a main badge to plug this Tor board into for power, a battery holder and two CR123a batteries are provided so you can power this Tor board independently. Just stick the battery holder to the back of the Tor board, this was not done for you because it would prevent you from using the SAO header. 

**Do NOT** attempt to power the Tor board from both the provided battery and a “main” badge through the SAO header simultaneously. The green zip ties can be used as a "leash" while using this as a SAO (so it won't fall off and get lost). Alternativly you could trim them and use them artistically as green leaves on the top of the board. 

### Basic Functionality
On power up the Tor SAO enters the last LED blinking mode. Pressing the button will cycle to the next LED blinking mode. 
 
1.	Very slow LED cycle (one LED at a time in order top to bottom)
2.	Slow LED cycle
3.	Fast LED cycle
4.	Very fast LED cycle
5.	Slow cylon
6.	Fast cylon
7.	Strobe
8.	Slow random
9.	Fast random
10.	Binary counter
11.	LEDs off

### Games
Having only 1 button and 5 LEDs is pretty limiting, but there’s two simple games you can play

##### Reaction Game
1. To enter this game press and hold the button for a few seconds. The top and bottom LEDs will start alternatively blinking quickly.
2. After releasing the button there is a random amount of time where all LEDs will be off. Pressing the LED during this timeframe will result in you losing the game and the previous LED blinking mode will restart.
3. When you see the LEDs begin to light up, press the button as quickly as possible. The LEDs will quickly count in binary until the button is pressed. The lower score the better. If you do not press a button quickly enough all of the LEDs will light up, and the previous LED blinking mode will restart
4. If you pressed the button at the correct time a score is shown until you press the button again. At which point the Tor badge will go back to the previous LED blinking mode. 

##### Button Mashing Game
1. Pressing the button around 10 times quickly will enter the button mashing game
2. Continue to press and release the button as quickly possible. The faster the button is pressed the more LEDs will be lit up, and the brighter they will be.
3. To win, completely fill all LEDs until all LEDs flash. Or to quit the game stop pressing the button.

### Unlocks
If you score five or less on the reaction game you will unlock a new “even/odd” LED blinking mode.

If you beat the button mashing game you will unlock a new “heartbeat” LED blinking mode.

Once unlocked these new modes will available after mode 11 "LEDs off"

### EEPROM Storage
The last blinking mode you were using is stored and recovered on power up, along with any unlocks you already earned.

### Troubleshooting and Recovery
A simple reboot should solve most issues by disconnecting and reconnecting power. Be gentle with the white JST power connector if you power the board from the provided battery holder. The JST connector should be disconnected by pulling on the connector itself, and not the wires. It is also recommended that you hold onto the board-side JST connector while connecting or disconnecting the battery holder.

If the button he held while the board is powered up, and released under ~15 seconds, all LEDs will be lit and eeprom reading and writing will be disabled until the next power up. This can be used as a last resort if you have eeprom problems.

When the board is powered up the last mode and unlocks are read from eeprom. If for some reason your board is in a weird state after these values are read you can reset the eeprom values by pressing and holding the button while the board is powered up. Continue holding the button for at least 15 seconds until LEDs 2 and 4 are lit, then release the button. Your board should now act as if it was powered up for the first time. All unlocks will be lost. 

### Review of Seeed Studio
Last time I made boards I used a no-name middleman in India. Since this board required 3 colors I knew I'd need a more professional manufacturer, and I knew that seeedstudio [was offering](https://www.seeedstudio.com/blog/2019/02/01/calling-all-badge-enthusiasts-gather-for-seeeds-badge-sponsorship-2019/) discounts for badge makers. I applied for the 8% discount level which requires an honest review of my experiance, so here it is. 

I actually tried to use two different fabs prior to seeed studio and they both told me that they couldn't do 3 colors on the board. To set some expectations, I'm also very inexperianced with board design. I can design a circuit and write the code, but like last time I had help with the board design files (this makes me more difficult to deal with). 

Applying for the coupon was super easy and it was credited to my account within a day. And creating the order was easy using their online tool. It was nice to be able to see the live quote as I changed options, but their BOM tool quoted a few of my parts more expensive than they should be. And there wasn't an option for the 3 colors that I needed on my boards. It seemed like to make any progress I needed to put in an order first, then I would be contacted by a factory representative to help actually get the thing made. 

There was some questions back and forth relating to the board files (nearly all my fault). I was initially expecting that the three colors would be done by using 1 solder mask + 2 silk screen layers, primarily because I didn't expect them to have that color of purple in a solder mask. They told me they can't do 1 silk screen + 2 solder masks, but they can do the opposite, which is fine by me. 

I initially was planning on making a smaller test order, followed by the larger bulk order. However after I placed the first smaller order they informed me that a small run of badges would cost an additional $500 more than what I already paid for using their online order page. By this time the questions ate a week or two so I decided I'd rather risk it all and make the bulk order and skip the smaller trial run. 

To change the quantity they suggested I create a new order. And they told me that BOM component costs could not be refunded, so I ended up wasting some money here and it doesn't seem like there was anything I could do to prevent this since I couldn't get an accurate quote ahead of time. Because the BOM didn't change it seems like they could at least use these parts as "waste" parts in the bulk order. They also weren't able to just send me these parts because their finance department won't let them (they need a distributor license or something). 

On the positive side, this time they were able to give me an accurate quote for how much my bulk order would cost. I just had to add some extra $1 dummy items to get my order up to the price they quoted me. And I had to request a new coupon code, which again went smoothly. 

Here's where seeed shined. I totally screwed up big time and got the gender of the SAO header wrong. I had both the male and female headers in my personally tracked BOM initially, and there was so much talk about the female keyed headers and how hard they were to find. Plus all my focus went into making sure the pins were mirrored correctly, since I knew many people have got this wrong in the past. I looked up the right part and it was slightly cheaper, and a week into the order they shouldn't be populating the boards anyway, but since I already knew they couldn't send me spare parts I was worried I just threw away some more money. But within a few hours they confirmed that they could swap the part for me.

I asked for a picture of the boards before assembly, since that is so expensive and I wanted to make sure they looked right. The factory rep that was helping me was able to do this for me and he even seemed a little excited about these boards. A few more weeks went by without much of an update, then suddenly everything was done and I had the tracking number. 

The most important part is the boards look really good considering what they had to work with, and everything was populated correctly. 

Final Score of Seeed 6/7

-BOM pricing using their tool was expensive (they made some corrections when I asked)

-Having to place an order, get it partially refunded, then place a second order

-Not being able to ship me the extra parts I was required to order

-QR codes didn't work (I think the silkscreen pooled up or something)

+Board produced looks good and worked (considering I didn't have time to order a sample run)

+They could deal with my board files that I couldn't easily clean up for them

+Communication was good, and they dealt with me asking for things like pictures

+They were able to switch parts out very late in the process

### Bill of Materials
In case something snapped off of your board, or you want to populate a bare board.

TBD
