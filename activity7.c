////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Template Project 
//** NAME: XXXX
//** RIN: XXXX
//** TEMPLATE VERSION: v.92 (2025-09-25)
//** This is the base project for several activities and labs throughout
//** the course.  The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
////////////////////////////////////////////////////////////////////////

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_mspm0.h"

// Add function prototypes here as needed.
void timerInit();
uint8_t UpdateCountdown();
// Add global variables here as needed.
Timers_TimerConfig time1;
uint8_t minutes=2;
uint8_t seconds=24;
uint8_t timerResets;

void GPIOInit(){
GPIO_initDigitalOutput(GPIOA,GPIO_PIN0);//led1
GPIO_initDigitalOutput(GPIOB,GPIO_PIN13);//unknon reason
GPIO_setPins(GPIOA,GPIO_PIN0);
GPIO_setPins(GPIOB,GPIO_PIN13);//sets the pins low as insturcted
}
int main() {    //// Main Function ////
  uint32_t status;
  uint32_t count=0;
  int8_t sign=1;
    sysInit();
    GPIOInit();
    printf("Program starting\n");//just to see if things are connected and the serial console works
    timerInit();
    while(1){  
       // printf("in the while loop"); program enters the while loop
        if(Timers_getAllPendingInterrupts(TIMG7) & TIMER_INTSRC_ZERO){//checks if an interrupt happened
        //printf("interrupt");
            GPIO_setPins(GPIOB,GPIO_PIN13);//sets pin A0 high on for every timer reset
            Timers_clearInterrupt(TIMG7, TIMER_INTSRC_ZERO); //clears the flag, now we can roll
            if(count==99){//done every second
                UpdateCountdown();
                if(UpdateCountdown()==1){
                    Timers_stopTimer(TIMG7);
                    printf("Timer stopped.");
                }
                count=0;
                sign=sign*-1;//flips sign of the digit
                if(sign==1){
                    GPIO_setPins(GPIOA,GPIO_PIN0);
                    //printf("light on \n");
                    
                }else if (sign==-1){
                    GPIO_clearPins(GPIOA,GPIO_PIN0);
                    //printf("light off \n");
               
                }
            }
            count++;
        }

   // printf("count: %u\n",count);
        GPIO_clearPins(GPIOB,GPIO_PIN13);
    }   
}    //// End Main Function ////  

void timerInit(){
time1.mode = TIMER_MODE_PERIODIC_DOWN; // Set mode to down counting and repeating
time1.clksrc = TIMER_CLOCK_BUSCLK;//32 MHZ frequency
time1.clkprescale=27; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
time1.period=11428;// since 0 is a number, we want total counts=N-1
time1.clkdivratio=TIMER_CLOCK_DIVIDE_1;
Timers_initTimer(TIMG7,&time1); // Apply the configuration
Timers_startTimer(TIMG7);
}
uint8_t UpdateCountdown(){
    if(seconds==0&&minutes!=0){
        seconds=60;
        minutes=minutes-1;
    }else if(seconds>0){
        seconds=seconds-1;
    }else 
    printf("Time: ");
    printf("%u:",minutes);
    printf("%u\n",seconds);
    if(seconds==0&&minutes==0){
        return 1;
    }else{
        return 0;
    }
}
