////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Lab 1
//** NAME: Colby Giunta (662105414) & Sebastain Brozowski()
//** Lab 2 project
//side b
//seat 29
////////////////////////////////////////////////////////////////////////
#include "engr2350_mspm0.h"
#include <stdlib.h> //allows us to have the rand() function
////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Lab 1
//** NAME: Colby Giunta (662105414) & Sebastain Brozowski()
//** Lab 2 project
//side b
//seat 29
////////////////////////////////////////////////////////////////////////
#include "engr2350_mspm0.h"
#include <stdlib.h> //allows us to have the rand() function

struct LedList{//each led in the array needs three values. Here is where they are defined
    uint8_t redValue;
    uint8_t greenValue;
    uint8_t blueValue;
};
// Function Prototypes
Timers_TimerConfig time1;//I do not know why the text is all white, but it works
void GPIOInit();
//the RGB LED that we want to change color and have a set 5-color pattern
void makeLedPattern();
void getInputs();
uint32_t inputs();
void lightShow();
void RGB(int8_t);
void clearRGB();
void TimerInit();
void startingText();
void fullPatternDisplay();
void clearBiLed();
void fetchInputs();
void duplicate();
void pollLogic();
void clearPlayerInputs();
void displayPattern(uint8_t);

//global varibles
struct LedList ledPattern[5]; //LedPattern is an array of the guessLED struct. 
                               //Reading this will tell us what our RGB LED should be
struct LedList playerInput[5];//players ledInput, will determine correct answers "on the spot" (we check right after input).
//what index the of the list is the player entering data.
uint32_t inputState;//what the value of the input pins are.
uint8_t abc;
uint32_t previousInputState;
bool canRemind;//reminder flag. start off true, then, if used, becomes false.
uint8_t timerCount1=0;//only use for the delay functions. Other functions need to track time seperately
uint8_t timerCount2=0;
uint8_t timerCount3Seconds=0;
int8_t sign=1;//for blinking the lights
//bumper varibles (if done by polling)
bool BMP1, BMP2, BMP3, BMP4, BMP5, BMP6, PB1;
bool PBMP1, PBMP2, PBMP3, PBMP4, PBMP5, PBMP6, PPB1;//previous to the varibles above 
//we are going to need a lot of flags for just random stuff
bool haveStarted;
uint8_t numCorrect;
bool canInput=true;
bool firstTime=true;
bool hasPlayerStarted=false;
bool isIncorrect=false;//make this true when a mistake happens
bool hasRedInput;
bool alreadyDisplayed=true;
int main() {    //// Main Function ////
    printf("program starting\n");
    sysInit();
    GPIOInit();
    TimerInit();
    Timers_startTimer(TIMG7);
    clearRGB();
    clearBiLed();
    GPIO_setPins(GPIOA, GPIO_PIN13);
    /*we will run the program such that the timer is always ticking, and we use different count variables to track resets.
    set the count varibles to zero before tracking time and we can have time mesaured across multiple events*/
    startingText();
    while(1){  //we build the flowchart in here. polling is not an issue that all that changes are flags we make.

        pollLogic();//must be done here to detect hasPlayerStarted
        if(hasPlayerStarted==true){//wait for any input to start the game.

            if(firstTime==true){//go here to start new round =
                firstTime=false;
                canRemind=true;
                numCorrect=0;
                clearPlayerInputs();
                isIncorrect=false;

                printf("\n displaying full sequence. You only get to see the full sequence once after this with your reminder.\n");
                printf("you should only see this message on start and reset.\n");
                makeLedPattern();//hopefully makes a new random pattern of leds to display. Don't want new pattens happening every loop
                fullPatternDisplay();
                printf("inital pattern display finished\n\n");
            }
        //here we loop from displaying patterns until victory or defeat
        //check if conditional statement should have && or ||
        printf("numCorrect (rounds completed): %u\n",numCorrect);
        printf("isIncorrect: %d\n",isIncorrect);//should happen before round check
        if(numCorrect<5 && isIncorrect==false){//infinite loop directs here until victory or defeat
            //code goes here after player guesses round correctly.
                //display pattern already manages biLed Turning red
                //makes biLEd green
            clearBiLed();
            GPIO_setPins(GPIOA,GPIO_PIN13);//turns biled on for green
            displayPattern(numCorrect+1);
            for(abc=(5-numCorrect-1);abc<5;abc++) {//checks correct answers for a given length.
             printf("\ninside for loop. Taking inputs. index varible=%u ",abc);
                //numCorrect is the number of rounds correct
                if(isIncorrect==false){//when an incorrect answer is given. This ensures everything is bypassed
                    printf("past the isIncorrect if statement\n");
                    printf("hasRedInput value = %d\n\n",hasRedInput);
                    timerCount3Seconds=0;
                    canInput=true;//player should be expected to input values now. will sit here
                        //make the led green
                    printf("\nPlease finish the sequence by inputing the correct colors\n");
                    hasRedInput=false;
                    hasPlayerStarted=false;//janky work around
                    clearBiLed();
                    GPIO_setPins(GPIOA,GPIO_PIN13);
                    clearRGB();
                    if(isIncorrect==false){
                        timerCount3Seconds=0;
                    }
                    while(hasPlayerStarted==false&&timerCount3Seconds<12){//program chills here until response
                        // printf("while loop within for loop to read inputs\n");
                        pollLogic();//gets us a poll
                        //reminder logic managed in pollLogic
                        //printf("timerCount3Seconds: %u\n",timerCount3Seconds);
                        }
                   // printf("index for playerInput: %u\n",abc);
                   // printf("playerInput: red value: %u, greenValue: %u, blue value: %u\n",playerInput[abc].redValue,playerInput[abc].greenValue,playerInput[abc].blueValue);
                   // printf("ledPattern_: red value: %u, greenValue: %u, blue value: %u\n",ledPattern[abc].redValue,ledPattern[abc].greenValue,ledPattern[abc].blueValue);
                   printf("abc = %d\n", abc);
                   printf("R:%d vs %d  G:%d vs %d  B:%d vs %d\n",playerInput[abc].redValue, ledPattern[abc].redValue,playerInput[abc].greenValue, ledPattern[abc].greenValue,playerInput[abc].blueValue, ledPattern[abc].blueValue);
                    //checks if the player index matches the ledPattern index. conditional is only true if the answer is incorrect (notice the "!")
                    if(timerCount3Seconds<=11&&(playerInput[abc].redValue == ledPattern[abc].redValue && playerInput[abc].greenValue == ledPattern[abc].greenValue&&playerInput[abc].blueValue == ledPattern[abc].blueValue)){
                        printf("\ncorrect answer\n");
                    }else{
                         //if were here because of time. the player input value will be all zero. Guarunteed to be incorrect
                        isIncorrect=true;//breaks out of the while loop
                        printf("incorrect answer. You are bad at the game\n");
                        hasPlayerStarted=true;//janky solution
                        break;//exits the for loop
                    } 
                }
                 printf("index value (abc): %u\n",abc);//abc is what iterates on its own (in the for loop)
            }//out of for loop.
                numCorrect++;//numcorrect will always be one off because it has to be called
        }else{//new addition so that we 

            printf("\nentered else staement\n\n");
                    RGB(10);
         if(numCorrect==5){//this if statement only displays text once as next segment is an infinite loop
            printf("\ncongratulations on winning: press any button to start new round ");
            }else{//loss (either by running out of time or incorrect button press)
            printf("\nyou lose. Ran out of time or guessed wrong color. press any button to start new round.\n");
            printf("rounds passed: %u\n",(numCorrect-1));
            }
             hasPlayerStarted=false;//set true by pressing any button

        while(hasPlayerStarted==false){//blink leds until another player input.
            pollLogic();
            if(numCorrect==5){//player won. blink green light
                if(timerCount1%2){
                    GPIO_clearPins(GPIOA,GPIO_PIN12|GPIO_PIN13);//clears both pins
                    GPIO_setPins(GPIOA,GPIO_PIN13);//this high and PIN A12 low makes green
                }else{
                  GPIO_clearPins(GPIOA,GPIO_PIN12|GPIO_PIN13);//clears both pins
                }
             }else{
                 if(timerCount1%2){
                     GPIO_clearPins(GPIOA,GPIO_PIN12|GPIO_PIN13);//clears both pins
                     GPIO_setPins(GPIOA,GPIO_PIN12);//this high and PIN A13 low makes red
                 }else{
                  GPIO_clearPins(GPIOA,GPIO_PIN12|GPIO_PIN13);//clears both pins
                  }
              }
        }
            printf("\nresetting:\n");//clearing varibles happens in the first displaying of the characters
            firstTime=true;//sends us to the displaying full sequence

        }//end of hasPlayerStarted
        
      }//out of the if statement for doing inputs. code after prints if hasPlayerStarted==false. code here is for victory, defeat, or maybe reseting
        //printf("issue detected");//when run out of time, or we never startwe enter here.
    } //end of big while loop 
    printf("\nthis should never print. If You see this you did something wrong.");
}  //// Main Function //// 
void TimerInit(){//want easy acess to 250ms and 500ms (currently 250ms)
    time1.mode = TIMER_MODE_PERIODIC_DOWN; // Set mode to down counting and repeating
    time1.clksrc = TIMER_CLOCK_BUSCLK;//32 MHZ frequency
    time1.clkprescale = 99; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
    time1.clkdivratio = TIMER_CLOCK_DIVIDE_5;//total divisor of (99+1)*5=500
    //time1.period = 32999;//gives a reset every 500ms
    time1.period=15999;//gives a reset every 250ms
    Timers_initTimer(TIMG7,&time1); // Apply the configuration
    Timers_startTimer(TIMG7);
    //Timers_startTimer(TIMG7); prevent timer from starting in this function (doc said so)
    Timers_enableInterrupt(TIMG7, TIMER_INTSRC_ZERO);//detects when timer is zero
    NVIC_EnableIRQ(TIMG7_INT_IRQn);//timer interrrupt
}
void TIMG7_IRQHandler(){//gives us acesss to clean 250 or 500ms inciments of time (currently 250ms)
  GPIO_clearInterrupt(TIMG7,TIMER_INTSRC_ZERO);//always clear the interrupt flag
  timerCount1++;//count variables will overflow but that's okay. They are set to zero before use in their specific functions
  timerCount2++;//for the timers related to pausing inbetween showing the random RGB sequence.
  timerCount3Seconds++;
}
void GPIOInit(){//boring GPIO initialization information.
//OUTPUTS: 
    //BILED
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN12);//this high and PIN A13 low makes red
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN13);//this high and PIN A12 low makes green
    //RGBLED
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN26);//red LED
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN27);//green LED
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN22);//blue LED

//INPUTS: ALL INPUTS CAN AND WILL BE USED IN INTERRUPTS
    //push button
    GPIO_initDigitalInput(GPIOA,GPIO_PIN9);//currently unassigned

    //bumper button pins (inputs)
  
    //based off the video, we do not have to do additive color mixing
    GPIO_initDigitalInput(GPIOA,GPIO_PIN7);//red
    GPIO_setInternalResistor(GPIOA,GPIO_PIN7,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOA,GPIO_PIN14);//green
    GPIO_setInternalResistor(GPIOA,GPIO_PIN14,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN5);//blue
    GPIO_setInternalResistor(GPIOB,GPIO_PIN5,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN21);//currently unassigned
    GPIO_setInternalResistor(GPIOB,GPIO_PIN21,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN23);//currently unassigned
    GPIO_setInternalResistor(GPIOB,GPIO_PIN23,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN25);//currently unassigned
    GPIO_setInternalResistor(GPIOB,GPIO_PIN25,GPIO_PULL_UP);

    NVIC_EnableIRQ(GPIO_INT_IRQn);//final initalization for the any pin interrupt
}
void makeLedPattern(){//sets value to each RGB value with each color having a 50% chance to be 1 or 0 (on or off).
//honestly could just throw this code into the big while loop. I just think it makes things cleaner to segregate.
    uint8_t temp1=0;
    for (uint8_t i=0;i<5;i++){//indexes are from 0 to four
        temp1=1+((rand()+1)%6);//random values from 1 to 6 (makes mapping it to the buttons easier)
       /* ledPattern[i].redValue=randomBinary();
        ledPattern[i].greenValue=randomBinary();
        ledPattern[i].blueValue=randomBinary();
        */
        if(temp1==1){//maps to red
            ledPattern[i].redValue=1;
            ledPattern[i].greenValue=0;
            ledPattern[i].blueValue=0;
        }else if(temp1==2){//maps to green
            ledPattern[i].redValue=0;
            ledPattern[i].greenValue=1;
            ledPattern[i].blueValue=0;
        }else if(temp1==3){//maps to blue
            ledPattern[i].redValue=0;
            ledPattern[i].greenValue=0;
            ledPattern[i].blueValue=1;
        }else if(temp1==4){//maps to magneta
            ledPattern[i].redValue=1;
            ledPattern[i].greenValue=0;
            ledPattern[i].blueValue=1;
        }else if(temp1==5){//maps to cyan
            ledPattern[i].redValue=0;
            ledPattern[i].greenValue=1;
            ledPattern[i].blueValue=1;
        }else if(temp1==6){//maps to yellow
            ledPattern[i].redValue=1;
            ledPattern[i].greenValue=1;
            ledPattern[i].blueValue=0;
        }else{
            printf("random number generater faulty. should only print from 1 to 6. Current random number: %u\n",temp1);
        }

    }
}
void RGB(int8_t x){//for displaying RGB values
        clearRGB();
        if (x==1){
            GPIO_setPins(GPIOB,GPIO_PIN26);//red
        }else if (x==2){
            GPIO_setPins(GPIOB,GPIO_PIN27);//green
        }else if (x==3){
            GPIO_setPins(GPIOB,GPIO_PIN22);//blue
        }
        else if (x==4){//magenta
            GPIO_setPins(GPIOB,GPIO_PIN26);//red
            GPIO_setPins(GPIOB,GPIO_PIN22);//blue 
        }
        else if (x==5){//cyan
            GPIO_setPins(GPIOB,GPIO_PIN27);//green
            GPIO_setPins(GPIOB,GPIO_PIN22);//blue 
        }
        else if (x==6){//magneta
            GPIO_setPins(GPIOB,GPIO_PIN27);//green
            GPIO_setPins(GPIOB,GPIO_PIN26);//red
        }else{
           // printf("error: RGB function recieved incorrect value. All values should be from 1 to 6. Value received: %u\n",x);
        }
}
void displayPattern(uint8_t b){//b is the amound missing from the sequence
    printf("\nprogram will not be taking further inputs as program is displaying sequence");
    printf("\ndisplaying pattern with %u lights missing", b);
    clearRGB();
    canInput=false;//function manages if the player can input
    GPIO_clearPins(GPIOA,GPIO_PIN12|GPIO_PIN13);//clears both pins
    GPIO_setPins(GPIOA,GPIO_PIN12);//this high and PIN A13 low makes red
    for(uint8_t i=0;i<(5-b);i++){//iterates through the sequence for the given amount of correct answers (note the 5-b term)
        if(ledPattern[i].redValue==1){
            GPIO_setPins(GPIOB,GPIO_PIN26);
        }
        if(ledPattern[i].greenValue==1){
           GPIO_setPins(GPIOB,GPIO_PIN27); 
        }
        if(ledPattern[i].blueValue==1){
            GPIO_setPins(GPIOB,GPIO_PIN22);
        }
        timerCount2=0;
        while (timerCount2<2){//half second delay on
        }
        timerCount2=0;
        clearRGB();
        while(timerCount2<2){//half second delay off
        }
    }
    canInput=true;
    timerCount2=0;
    while (timerCount2<2){//half second delay on
    }
    clearRGB();
}
void clearRGB(){//get rid of in the future.
            GPIO_clearPins(GPIOB,GPIO_PIN26);//red LED
            GPIO_clearPins(GPIOB,GPIO_PIN27);//green LED
            GPIO_clearPins(GPIOB,GPIO_PIN22);//blue LED
}
void clearBiLed(){        
    GPIO_clearPins(GPIOA,GPIO_PIN12);//biled red
    GPIO_clearPins(GPIOA,GPIO_PIN13);//biled green
}
//lightshow was deleted
void fullPatternDisplay(){//manages 2 seconds delay for us
    displayPattern(0);//should be only timer displayPattern can take an argument thats not numCorrect
            timerCount1=0;
            while(timerCount1<8){//two second delay
                }
}
void startingText(){
    printf("\nprogram is starting\n\n");//test that the board and serial monitor work
    printf("Here are the rules of the game:\n 1: the five colors will flash from the RGB LED on the microcontroller board itself.\n");
    printf("Memorize the pattern of lights that come to you--they're random. \n2: Once the BILED turns from to green, you can start\n");
    printf("inputing in the remaining part of the sequence. \n3: If corect, you will the the sequence with one less light.\n");
    printf("complete the squence unitl the very end and you win\n");
    printf("press any button and the game will start\n");
}
void fetchInputs(){//true if high, false if low.
    BMP1=GPIO_readPins(GPIOA,GPIO_PIN7);
    BMP2=GPIO_readPins(GPIOA,GPIO_PIN14);
    BMP3=GPIO_readPins(GPIOB,GPIO_PIN5);
    BMP4=GPIO_readPins(GPIOB,GPIO_PIN21);
    BMP5=GPIO_readPins(GPIOB,GPIO_PIN23);
    BMP6=GPIO_readPins(GPIOB,GPIO_PIN25);
    PB1=GPIO_readPins(GPIOA,GPIO_PIN9);
}
void duplicate(){//previous inputs equal current inputs. P___ will always be one moment in time behind. Allows for edge detection
    PBMP1=BMP1;
    PBMP2=BMP2;
    PBMP3=BMP3;
    PBMP4=BMP4;
    PBMP5=BMP5;
    PBMP6=BMP6;
    PPB1=PB1;
}
void clearPlayerInputs(){
    for(uint8_t z=0;z<5;z++){//no color (all zero) is impossible to input, so game will read incorrect answer
        playerInput[z].redValue=0;
        playerInput[z].greenValue=0;
        playerInput[z].blueValue=0;
    }
}
void pollLogic(){
if(canInput==true){
    fetchInputs();
//rising edge interactions
    if(BMP1==true&&PBMP1==false){
        RGB(1);//already manages clearing the pins
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
        printf("1 on\n");
        delay_cycles(320e3);
    }else if(BMP2==true&&PBMP2==false){
        RGB(2);
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
         printf("2 on\n");
        delay_cycles(320e3);
    }else if(BMP3==true&&PBMP3==false){
        RGB(3);
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
        printf("3 on\n");
        delay_cycles(320e3);
    }else if(BMP4==true&&PBMP4==false){
        RGB(4);
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
        printf("4 on\n");
        delay_cycles(320e3);
    }else if(BMP5==true&&PBMP5==false){
        RGB(5);
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
        printf("5 on\n");
        delay_cycles(320e3);
    }else if(BMP6==true&&PBMP6==false){
        RGB(6);
        timerCount3Seconds=0;//gets three seconds timer rule to zero
        hasRedInput=true;
        printf("6 on\n");
        delay_cycles(320e3);
    }else if(PB1==true&&PPB1==false){//reseting the game (unlike all other inputs)
                printf("pushbutton pressed: \n");
                hasPlayerStarted=true;
        if(canRemind==true){
            canRemind=false;
            printf("Reminder will be displayed. You can no longer remind for the remainder of the game.\n");
            fullPatternDisplay();
        }else{
            printf("Reminder already used. You cannot remind any more\n");
        }
        delay_cycles(320e3);
    }
//falling edge interactions (required by lab 2 document)

    if(PBMP1==true&&BMP1==false){//inputs red
        RGB(10);
        playerInput[abc].redValue=1;
        playerInput[abc].greenValue=0;
        playerInput[abc].blueValue=0;
        printf("1 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }else if(PBMP2==true&&BMP2==false){//inputs green
        RGB(10);
        playerInput[abc].redValue=0;
        playerInput[abc].greenValue=1;
        playerInput[abc].blueValue=0;
        printf("2 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }else if(PBMP3==true&&BMP3==false){//inputs blue
        RGB(10);
        playerInput[abc].redValue=0;
        playerInput[abc].greenValue=0;
        playerInput[abc].blueValue=1;
        printf("3 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }else if(PBMP4==true&&BMP4==false){//inputs magenta (red + blue)
        RGB(10);
        playerInput[abc].redValue=1;
        playerInput[abc].greenValue=0;
        playerInput[abc].blueValue=1;
        printf("4 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }else if(PBMP5==true&&BMP5==false){//inputs cyan (blue + green)
        RGB(10);
        playerInput[abc].redValue=0;
        playerInput[abc].greenValue=1;
        playerInput[abc].blueValue=1;
        printf("5 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }else if(PBMP6==true&&BMP6==false){//inputs yellow (red + green)
        playerInput[abc].redValue=1;
        playerInput[abc].greenValue=1;
        playerInput[abc].blueValue=0;
        printf("6 off\n");
        hasPlayerStarted=true;
        delay_cycles(320e3);
    }//PB1 does not need any falling edge code
  }  
    duplicate();
}
