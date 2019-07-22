/*
 * File:    main.c
 * Author:  twitter @see_ess
 * Using the attiny402
 * 
 * License:
 * -----
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                  Version 2, December 2004
 * 
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 * 
 *           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 * 0. You just DO WHAT THE FUCK YOU WANT TO.
 * 
 * -----
 * 
 * chip pinout (fron datasheet):
 * 1 - vdd (this is the bottom right pin
 * 2 - PA6 - LED 5 (bottom right LED)
 * 3 - PA7 - LED 4
 * 4 - PA1 - LED 3
 * 5 - PA2 - LED 2
 * 6 - PA0/rst/udpi - button NOTE: this pin can get stuck in udpi/programming mode which requires 12v zap to undo the fuse
 * 7 - PA3 - LED 1
 * 8 - gnd
 * 
 * built using mplab x IDE 5.15 + XC8 2.05 (free vers)
 * 
 * Pickit4 pinout (v4 is probably required for UDPI)
 * Pin 1 - NC
 * Pin 2 - VDD+
 * Pin 3 - GND
 * Pin 4 - Data
 */
#define CODE_VERSION 31 //version 1-31, used during power on test mode
#define F_CPU 3500000UL
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
FUSES = {
    .OSCCFG = 0b00000010,//calibration unlocked, 20mhz
    .SYSCFG0 = CRCSRC_NOCRC_gc | RSTPINCFG_UPDI_gc,
    .SYSCFG1 = SUT_64MS_gc,
    .APPEND = 0x00,
    .BOOTEND = 0x02
};
//TODO brown out detect BOD
//TODO turn off other stuff like timers and WD if not in use
//TODO support led dimming

//blink back modes:
//power up show version?
//0  = boot?
//1  = slow cycle
//2  = kilo cyclee
//3  = mega cycl
//4  = giga cycle
//5  = terminator slow
//6  = terminator fast
//7  = strobe
//8  = random slow
//9  = random fast
//10 = counter
//11 = off
//12 = even odd mode - unlocked from beating reaction game
//13 = heartbeat mode - unlocked from beating button mash game
//100 = reaction game
//101 = button mash game

//eeprom
//address 0 = 0x1400 = last stored mode
//address 1 = 0x1401 = if unlocked even/odd mode from beating reaction game
//address 2 = 0x1402 = if unlocked heartbet mode from beating button press game

char mode = 0; //current mode of operation (boot)
char counter = 0; //used for counter mode
char randled = 0; //used for random mode
char loop = 0; //increment after each loop, used for various counters
char loop2 = 0; //increment after loop hits max
char lastled = 100; //used to prevent two random leds in a row, set to invalid value initially
char debounce = 0; //used to debounce the button, 1=button pressed previously
char buttonheld = 0; //counter of how long the button was held for
char terminator = 0; //used for terminator mode
char reactheld = 0; //used to detect if the button is held down
char reactlastmode = 0; //when exiting react game, go back to last mode
char reactblackouttime = 0; //the amount of time to blackout
char reacttimer = 0; //amount of time it took to react; 0 not started, 1-15 timer started after blackout, 254 freeze counter
char term0 = 0;//used to precalculate ranges for cycle modes
char term1 = 0;//used to precalculate ranges for cycle modes
char term2 = 0;//used to precalculate ranges for cycle modes
char term3 = 0;//used to precalculate ranges for cycle modes
char term4 = 0;//used to precalculate ranges for cycle modes
char buttonmash = 0; //for button mash game, increments on button press, decrements automatically
char buttonmashdisplay = 0; //for button mash game used to simplify display calc
char eepromreaction = 0; //set to reaction game not beat
char eeprombuttonmash = 0; //set to button mash game not beat
char flashenabled = 1; //disable flash if button held on bootup
char reactiongamejustwon = 0;//used to know if we should switch to the new unlocked mode, or if they've beat this game previously

void switchMode(char newmode){
    PORTA.OUT = 0b11111111; //set LED sink pins to high to turn off LEDs
    mode = newmode;
    if(mode < 14 && mode > 0){//not in a game or boot mode
        if(flashenabled){
            if(eeprom_is_ready()){
                eeprom_write_byte(0,mode);//address,value
                asm("NOP");//almost certainly not needed but whatever
                asm("NOP");
            }
        }
    }
    loop = 255;
    loop2 = 255;
    counter = 0; //reset the counter value
    lastled = 100; //initial invalid value for random modes
    if(mode < 5){
        //precalculate loop ranges to save space:
        term0 = 51 / (mode*mode);
        term1 = 102 / (mode*mode);
        term2 = 153 / (mode*mode);
        term3 = 204 / (mode*mode);
        term4 = 255 / (mode*mode);
    }
    //react game
    reactblackouttime = 0x3F | rand();//rand time between 63-255, this is roughly 2-7 seconds
    reacttimer = 0;//reset the amount of time it took to react (score), before entering game
}

int main(void) {
    //set pullups and stuff
    CLKCTRL.MCLKCTRLA = 0b00000001; //set clock source to slow 32k reference, 0=16mhz-20mhz ref
    CLKCTRL.MCLKCTRLB = 0b00000000; //turn off prescaler
    VREF.CTRLB = 0; //disable adc0 reference until called, and set to lowest ref
    VREF.CTRLB = 0; //disable ac0 reference until called, and set to lowest ref
    PORTA.DIR = 0b11001110; //set PA 1,2,3,6,7 to output, 0 to input
    PORTA.OUT = 0b11111111; //set LED sink pins to high to turn off LEDs
    PORTA.PIN3CTRL = 0b00000001;//verify that pullups are turned on on pin 0
    
    //startup mode if button is held on power up
    char displaymode0 = CODE_VERSION;
    while(!(PORTA.IN & 0b00000001)){//as long as the button is pressed, show the code version
        flashenabled = 0;//disable flash during this power cycle
        mode = 1;//we normally set this during the flash check but we can't now
        PORTA.OUT = 0b11111111; //all LEDs off since we calculate each one to turn on
        if(displaymode0 & 0b00010000){ PORTA.OUT &= 0b11110111; }//if the 16 bit is set light LED 1 on, PA3
        if(displaymode0 & 0b00001000){ PORTA.OUT &= 0b11111011; }//if the 8 bit is set light LED 2 on, PA2
        if(displaymode0 & 0b00000100){ PORTA.OUT &= 0b11111101; }//if the 4 bit is set light LED 3 on, PA1
        if(displaymode0 & 0b00000010){ PORTA.OUT &= 0b01111111; }//if the 2 bit is set light LED 4 on, PA7                
        if(displaymode0 & 0b00000001){ PORTA.OUT &= 0b10111111; }//if the 1 bit is set light LED 5 on, PA6
        loop2++;//if it is held down for a long time, erase the flash to defaults
        if(loop2 == 254){
            if(eeprom_is_ready()){
                eeprom_write_byte((uint8_t*)0, 0xFF);
                asm("NOP");//almost certainly not needed but whatever
                asm("NOP");
            }
            if(eeprom_is_ready()){
                eeprom_write_byte((uint8_t*)1, 0xFF);
                asm("NOP");//almost certainly not needed but whatever
                asm("NOP");
            }
            if(eeprom_is_ready()){
                eeprom_write_byte((uint8_t*)2, 0xFF);
                asm("NOP");//almost certainly not needed but whatever
                asm("NOP");
            }
            flashenabled = 1;//enable flash now that we erased it
            displaymode0 = 10;//light led 2 and 4 to show flash erased
        }
        _delay_ms(55);//make it take ~15 seconds to reset flash
    }//while button press check
    if(flashenabled){
        //https://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html
        if(eeprom_is_ready()){//load stored mode from eeprom
            mode = eeprom_read_byte((uint8_t*)0);
            asm("NOP");//almost certainly not needed but whatever
            asm("NOP");
            if(mode < 1 || mode > 13){//invalid stored mode, could be first boot
                mode = 1;//reset to first mode
            }
        }
        if(eeprom_is_ready()){
            eepromreaction = eeprom_read_byte((uint8_t*)1); //is the evenodd mode unlocked?
            asm("NOP");//almost certainly not needed but whatever
            asm("NOP");
        }

        if(eepromreaction > 1){//bounds check
            eepromreaction = 0;//something went wrong, lock this mode again
                if(eeprom_is_ready()){
                    eeprom_write_byte((uint8_t*)1, eepromreaction);
                    asm("NOP");//almost certainly not needed but whatever
                    asm("NOP");
                }
        }
        if(eeprom_is_ready()){
            eeprombuttonmash = eeprom_read_byte((uint8_t*)2); //is the heardbeat mode unlocked?
            asm("NOP");//almost certainly not needed but whatever
            asm("NOP");
        }
        if(eeprombuttonmash > 1){//bounds check
            eeprombuttonmash = 0;//something went wrong, lock this mode again
            if(eeprom_is_ready()){
                eeprom_write_byte((uint8_t*)2, eeprombuttonmash);
                asm("NOP");//almost certainly not needed but whatever
                asm("NOP");
            }
        }
    }//flashmemoryenabled check
    
    switchMode(mode);//move to normal ops
    while(1){ //do stuff
        //crc scan on boot for flash, then present an error if broken
        if(mode==101){//butonmash game
            PORTA.OUT = 0b11111111; //all LEDs off since we calculate each one to turn on
            //0-3 = first led + 8
            //4-7 = second led + 8
            //8-11 = third led + 8
            //12-15 = fourth led + 8
            //16-19 = fifth led + 8
            if(buttonmash < 12){//if loop2%4=0
                if(loop%5 < buttonmash-7)//magic
                    PORTA.OUT &= 0b10111111;//light LED 5 on, PA6
            }else if(buttonmash < 16){
                PORTA.OUT &= 0b10111111;//light LED 5 on, PA6
                if(loop%5 < buttonmash-11)//magic
                    PORTA.OUT &= 0b00111111;//light LED 4+5 on, PA7
            }else if(buttonmash < 20){
                PORTA.OUT &= 0b00111111;//light LED 4+5 on, PA6
                if(loop%5 < buttonmash-15)//magic
                    PORTA.OUT &= 0b00111101;//light LED 4+5+3 on, PA1
            }else if(buttonmash < 24){
                PORTA.OUT &= 0b00111101;//light LED 3+4+5 on, PA6
                if(loop%5 < buttonmash-19)//magic
                    PORTA.OUT &= 0b00111001;//light LED 3+4+5+2 on, PA2  
            }else if(buttonmash < 27){//buttonmashdisplay 16-19 //can't just to else() so it maxes out
                PORTA.OUT &= 0b00111001;//light LED 2+3+4+5 on, PA6
                if(loop%5 < buttonmash-22)//magic
                    PORTA.OUT &= 0b00110001;//light LED 1+2+3+4+5 on, PA3
            }else if(buttonmash == 27){//game won
                char gamewon = 0;//used to blink game won leds
                eeprombuttonmash = 1;//unlock new blink mode
                if(flashenabled){//can we write to flash?
                    if(eeprom_is_ready()){//store unlock to eeprom
                        eeprom_write_byte((uint8_t*)2, eeprombuttonmash);
                        asm("NOP");//almost certainly not needed but whatever
                        asm("NOP");
                    }
                }
                do{
                    PORTA.OUT = 0b00110001;//light all leds
                    _delay_ms(120);
                    PORTA.OUT = 0b11111111;//all off
                    _delay_ms(150);
                    gamewon++;
                }while(gamewon != 20);//while the button is held
                buttonmash = 0;
                switchMode(13);//go to the new heartbeat mode
            }
        }else if (mode <= 4){ //cycle modes 1-4, higher mode == faster
            int modemod = loop2 % term4;//used so we don't have to reset loop2's counter
            if(modemod <= term0){//first LED's chance to shine   
                PORTA.OUT = 0b11110111; //LED 1 on, PA3
            }else if(modemod <= term1){//first LED's chance to shine   
                PORTA.OUT = 0b11111011; //LED 2 on, PA2
            }else if(modemod <= term2){//second LED
                PORTA.OUT = 0b11111101; //LED 3 on, PA1
            }else if(modemod <= term3){//third LED
                PORTA.OUT = 0b01111111; //LED 4 on, PA7
            }else{//forth LED
                PORTA.OUT = 0b10111111; //LED 5 on, PA6
            }
        }else if(mode <= 6){//terminator mode 5/6 
            if(mode == 5){ //slow terminator
                //terminator = (loop2 % 128) / 12; //128/12~=10 steps and 256/2~=128
                terminator = loop2 / 25;
            }else{ //fast terminator
                terminator = (loop2 % 51) / 5; //51/5 ~= 10 steps
            }
            if(terminator == 0){//only light 1, no trail
                PORTA.OUT = 0b11110111; //LED 1 on, PA3
            }else if(terminator == 1){
                PORTA.OUT = 0b11111011; //LED 2 on, PA2
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b11110011; //LED 1+2 on, PA3
                }
            }else if(terminator == 2 ){
                PORTA.OUT = 0b11111101; //LED 3 on, PA1
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b11111001; //LED 2+3 on, PA2
                }
            }else if(terminator == 3){
                PORTA.OUT = 0b01111111; //LED 4 on, PA7
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b01111101; //LED 3+4 on, PA1
                }
            }else if(terminator == 4){
                PORTA.OUT = 0b10111111; //LED 5 on, PA6
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b00111111; //LED 4+5 on, PA7
                }
            }else if(terminator == 5){ //no trail
                PORTA.OUT = 0b10111111; //LED 5 on, PA6 
            }else if(terminator == 6){
                PORTA.OUT = 0b01111111; //LED 4 on, PA7
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b00111111; //LED 4+5 on, PA6 
                }
            }else if(terminator == 7){
                PORTA.OUT = 0b11111101; //LED 3 on, PA1
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b01111101; //LED 3+4 on, PA7
                }
            }else if(terminator == 8){
                PORTA.OUT = 0b11111011; //LED 2 on, PA2
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b11111001; //LED 2+3 on, PA1
                }
            }else if(terminator == 9){ // first LED last to not have an if condition because 255/10!=x.0 //if(terminator == 0 || terminator == 9){
                PORTA.OUT = 0b11110111; //LED 1 on, PA3
                if(loop%7==0){//light the trailing LED half the time
                    PORTA.OUT = 0b11110011; //LED 1+2 on, PA2
                }
            }
        }else if(mode == 7){//strobe mode
            if(loop2 < 30){
                PORTA.OUT = 0b00110001; //all LEDs on
            }else{
                PORTA.OUT = 0b11111111; //all LEDs off
            }
        }else if(mode <= 9){//random modes 8-9 (9 is faster)
            if( ((loop2 % 128) == 0 && loop == 0) || ((loop2 % 32) == 0 && loop == 1 && mode == 9)) { //calculate new random led
                do{
                    randled = rand() | 0b00110001; //pick a random LED, set the non LED bits to 1
                }while(randled == lastled || randled == 0xFF);//don't pick the same LED as last time, and don't turn off all leds
                lastled = randled;//remember new random led for next time
                PORTA.OUT = randled;  //turn on new random LED
            }
        }
        else if(mode == 10){//counter mode
            if(!(loop2 % 128) && loop == 0){//increment the counter
                counter++;
                _delay_us(410);//get it closer to counting 1sec at a time
            }
            _delay_us(3);//get it closer to counting 1sec at a time
            PORTA.OUT = 0b11111111; //all LEDs off since we calculate each one to turn on
            if(counter & 0b00010000){ PORTA.OUT &= 0b11110111; }//if the 16 bit is set light LED 1 on, PA3
            if(counter & 0b00001000){ PORTA.OUT &= 0b11111011; }//if the 8 bit is set light LED 2 on, PA2
            if(counter & 0b00000100){ PORTA.OUT &= 0b11111101; }//if the 4 bit is set light LED 3 on, PA1
            if(counter & 0b00000010){ PORTA.OUT &= 0b01111111; }//if the 2 bit is set light LED 4 on, PA7                
            if(counter & 0b00000001){ PORTA.OUT &= 0b10111111; }//if the 1 bit is set light LED 5 on, PA6
        }else if(mode == 100){//react game
            if(!reacttimer && loop2 < reactblackouttime){//if game hasn't started (reacttimer==0) and in blackout time
                _delay_us(75);//slow the blackout timer down, this delay only is hit during blackout during the start of the game
            }else if(!reacttimer && loop2 == reactblackouttime){//game is just starting, blackout time ends
                reacttimer++;//start game
                loop2 = 0; //reset the loop timer since we use this to increment
            }
            if(reacttimer && reacttimer < 32){//if we are mid game (counting up)
                PORTA.OUT = 0b11111111; //all LEDs off since we calculate each one to turn on
                if(reacttimer & 0b00010000){ PORTA.OUT &= 0b11110111; }//if the 16 bit is set light LED 1 on, PA3
                if(reacttimer & 0b00001000){ PORTA.OUT &= 0b11111011; }//if the 8 bit is set light LED 2 on, PA2
                if(reacttimer & 0b00000100){ PORTA.OUT &= 0b11111101; }//if the 4 bit is set light LED 3 on, PA1
                if(reacttimer & 0b00000010){ PORTA.OUT &= 0b01111111; }//if the 2 bit is set light LED 4 on, PA7                
                if(reacttimer & 0b00000001){ PORTA.OUT &= 0b10111111; }//if the 1 bit is set light LED 5 on, PA6
                if((loop2 % 4==0 && loop == 0) && reacttimer < 32){//increment the counter every so often
                    reacttimer++;//increment the counter until they press the button
                }
            }else if(reacttimer == 32){//timeout, no button pressed during counter, reset to last mode
                _delay_ms(3000);//freeze on all leds lit
                switchMode(reactlastmode);//go back to last mode before entering game
            }
        }else if(mode == 11){//off mode
            PORTA.OUT = 0b11111111;//off
        }else if(mode == 12){//even odd mode (locked initially)
            if(loop2 % 64 < 32){
                PORTA.OUT = 0b10110101; // 1/3/5 odd leds
            }else{
                    PORTA.OUT = 0b01111011; // 2/4 even leds
            }
        }else if(mode == 13){//heartbeat mode (locked initially)
            //5 loops all on, 70 loops fade out, 15 off, 5 all on, 70 fade out, rest off
            if(loop2 < 5){
                PORTA.OUT = 0b00110001; //all LEDs on
            }else if(loop2 < 75){
                PORTA.OUT = 0b11111111; //all LEDs off
                if((loop%70)>(loop2-5)){//fade out over 50 loop2's
                    PORTA.OUT = 0b00110001; //all LEDs on
                }
            }else if(loop2 < 80){
                PORTA.OUT = 0b11111111; //all LEDs off
            }else if(loop2 < 85){
                PORTA.OUT = 0b00110001; //all LEDs on
            }else if(loop2 < 155){
                PORTA.OUT = 0b11111111; //all LEDs off
                if((loop%70)>(loop2-85)){//fade out over 50 loop2's
                    PORTA.OUT = 0b00110001; //all LEDs on
                }
            }else{
                PORTA.OUT = 0b11111111; //all LEDs off
            }
        }//mode
        if(!(PORTA.IN & 0b00000001)){ //is the button pressed? (pin 0)
            if(!debounce){//and it wasn't pressed previously
                debounce = 1;//used to know if the button is held down
                if(mode == 100){//if in the react game
                    if(reacttimer == 0){//if button pushed in blackout mode
                        do{//react fail blink back
                            PORTA.OUT = 0b00111111;//light led 4-5
                            _delay_ms(120);
                            PORTA.OUT = 0b11110011;//light led 1-2
                            _delay_ms(120);
                            reactheld++;//how many times to blink the error
                        }while(reactheld < 7);//blink the error a few times
                        switchMode(reactlastmode);//exit game to last mode
                    }else if(reacttimer < 32){//if game started, and button pressed stop counter
                        reactiongamejustwon = 0;//default to the game not just won
                        if(reacttimer <= 5){//game won if score = 1-5
                            reactiongamejustwon = 1; //used to know this game was just won this time
                            eepromreaction = 1;//unlock new modes
                            if(flashenabled){//can we write to flash?
                                if(eeprom_is_ready()){//store unlock to eeprom
                                    eeprom_write_byte((uint8_t*)1, eepromreaction);
                                    asm("NOP");//almost certainly not needed but whatever
                                    asm("NOP");
                                }
                            }
                        }
                        reacttimer = 254;//freeze counter
                    }else if(reacttimer == 254){//if button pressed when counter frozen
                        if(reactiongamejustwon){
                            switchMode(12);//new game mode
                        }else{//the game was already won (or not won) go to last mode
                            switchMode(reactlastmode);//switch to last mode
                        }
                    }
                }else{ //if in any other mode besides the react game == 100
                    if(buttonmash == 8){
                        buttonmash = 11;//don't start the game at the minimum value
                        switchMode(101);//start buttonmash game
                    }
                    if(buttonmash < 27){//if not maxed out yet
                        buttonmash++;//track how many times a button was pressed quickly (this is auto decremented later)
                    }
                    if(mode <= 13){//if not in a game (so must be in button mash game)
                        mode++;//increment the mode
                        if(mode == 12 && eepromreaction != 1){//is even/odd not unlocked
                            mode++;//increment incase heartbeat is unlocked but even/odd isnt
                        }
                        if(mode>13 || (mode == 13 && eeprombuttonmash != 1) ){//if mode is too high or not new mode is not unlocked yet
                            mode=1;//roll over the mode if it is too high
                        }
                        switchMode(mode);
                    }
                }
            }else{//button held down
                if(loop == 0 )//every so often increment buttonheld 
                    buttonheld++;
                if(mode <= 13 && buttonheld == 255){//if the button was pressed for a few seconds not in a game
                    buttonheld = 0;//reset the counter
                    do{
                        PORTA.OUT = 0b10111111;//light led 5
                        _delay_ms(120);
                        PORTA.OUT = 0b11110111;//light led 1
                        _delay_ms(120);
                        reactheld++;
                    }while(!(PORTA.IN & 0b00000001));//while the button is held
                    reactlastmode = mode-1;//button down increments mode, so undo that now that we entered the game
                    if(reactlastmode == 0 || reactlastmode > 13){//if reactmode = 0 (boot mode which we don't want) and max check
                        reactlastmode = 10; //reset to the last mode
                    }
                    switchMode(100);//change to react game
                }
            }
        }else{//button not pressed
            debounce = 0;//reset to detect debounce
            reactheld = 0;//reset the react game button held counter
            buttonheld = 0;//reset button held counter
        }//end button press check
        
        if(buttonmash > 0){//how far has loop incremented since first buttonmash press, decrement it over time if >0
            if(loop == 0 && buttonmash < 28){//shortcircuit on loop == 0 (could've &&'d but this will be a big if condition), nd 28=game won
                char buttonmashtemp = loop2-50;//used to pre-calc for the if condition ahead
                if( buttonmashtemp % 128 == 0 || //this decrements in game and out of game (before game starts)
                        (buttonmash > 8 && buttonmashtemp % 85 == 0) || //make it harder and harder to increase your buttonmash score
                        (buttonmash > 12 && buttonmashtemp % 64 == 0) ||//64
                        (buttonmash > 16 && buttonmashtemp % 51 == 0) ||//51
                        (buttonmash > 20 && buttonmashtemp % 42 == 0) ||//42
                        (buttonmash > 24 && buttonmashtemp % 42 == 0) ){//36 is too hard, use 42
                    buttonmash--;//decrement the button mash score/leds
                }
            }//loop==0
            if(buttonmash == 7 && mode == 101){ //if decremented out of buttonmash game, exit game
                buttonmash = 0;//force them to press 8 times again to restart the game
                switchMode(1);//go back to first mode after exiting buttonmash game
            }
        }
        loop++;//used for all kinds of counters
        if(loop == 0)//loop2 speed
            loop2++;//increment the next one, this avoids using 16bit variables for compares everywhere
    }//while(1))
}
