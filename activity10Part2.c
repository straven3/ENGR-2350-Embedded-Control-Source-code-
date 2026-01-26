////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Template Project 
//** NAME: XXXX
//** RIN: XXXX
//** TEMPLATE VERSION: v.93 (2025-10-21)
//** This is the base project for several activities and labs throughout
//** the course.  The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
////////////////////////////////////////////////////////////////////////

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_mspm0.h"

// Add function prototypes here as needed.
void GPIOInit();
void timerInit();
void infiniteLoop();
Timers_TimerConfig time1;
Timers_CompareConfig time2;

// Add global variables here as needed.
float dutyCycle;
uint16_t myCCRN1;
uint16_t myCCRN2;
uint16_t myPeriod;
bool isTrue1=true;
bool isTrue2=true;

int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    timerInit();
    GPIOInit();
    Timers_startTimer(TIMG8);
    // Place initialization code (or run-once) code here
    printf("program starting\n");
    infiniteLoop();
}    //// End Main Function ////  
void infiniteLoop(){
    while(1){
    if(!GPIO_readPins(GPIOA,GPIO_PIN7)){ // BMP1 Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOA,GPIO_PIN7));
            delay_cycles(320e3);
            myCCRN1+=99;

            if(myCCRN1>900){
                myCCRN1=900;
            }
            printf("left wheel speed decreased\n");
            Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,myCCRN1);

    }else if(!GPIO_readPins(GPIOA,GPIO_PIN14)){ // BMP2 Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOA,GPIO_PIN14));
            delay_cycles(320e3);
            myCCRN1-=99;
            if(myCCRN1<0){
                myCCRN1=1;
            }
            printf("left wheel speed increased\n");
            Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,myCCRN1);

    }else if(!GPIO_readPins(GPIOB,GPIO_PIN5)){ // BMP3Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOB,GPIO_PIN5));
            delay_cycles(320e3);
            if(isTrue1==false){
        GPIO_setPins(GPIOB,GPIO_PIN8); 
            isTrue1=true;
            printf("left wheel reverse\n");
            }else{
            GPIO_clearPins(GPIOB,GPIO_PIN8);
            isTrue1=false;
            printf("left wheel reverse\n");
            }

    }else if(!GPIO_readPins(GPIOB,GPIO_PIN21)){ // BMP4 Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOB,GPIO_PIN21));
            delay_cycles(320e3);
            myCCRN2+=99;
            if(myCCRN2>900){
                myCCRN2=900;
            }
            printf("right wheel speed decreased\n");
            Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN2);
    }else if(!GPIO_readPins(GPIOB,GPIO_PIN23)){ // BMP5 Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOB,GPIO_PIN23));
            delay_cycles(320e3);
            myCCRN2-=99;
            if(myCCRN2<0){
                myCCRN2=16384;
            }
            printf("right wheel speed increased\n");
            Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN2);

    }else if(!GPIO_readPins(GPIOB,GPIO_PIN25)){ // BMP6 Pressed
        delay_cycles(320e3);
        while(!GPIO_readPins(GPIOB,GPIO_PIN25));
            delay_cycles(320e3);
            GPIO_setPins(GPIOB,GPIO_PIN0);
            if(isTrue2==false){
                GPIO_setPins(GPIOB,GPIO_PIN0); 
                isTrue2=true;
                printf("right wheel reverse\n");
            }else{
                GPIO_clearPins(GPIOB,GPIO_PIN0);
                isTrue2=false;
                printf("right wheel reverse\n");
            }
    }//end of large if statement
    }
}//end of function
void GPIOInit(){
    //bumper buttons
    GPIO_initDigitalInput(GPIOA,GPIO_PIN7);//Bumper 1
    GPIO_setInternalResistor(GPIOA,GPIO_PIN7,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOA,GPIO_PIN14);//Bumper 2
    GPIO_setInternalResistor(GPIOA,GPIO_PIN14,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN5);//Bumper 3
    GPIO_setInternalResistor(GPIOB,GPIO_PIN5,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN21);//Bumper 4
    GPIO_setInternalResistor(GPIOB,GPIO_PIN21,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN23);//Bumper 5
    GPIO_setInternalResistor(GPIOB,GPIO_PIN23,GPIO_PULL_UP);

    GPIO_initDigitalInput(GPIOB,GPIO_PIN25);//Bumper 6
    GPIO_setInternalResistor(GPIOB,GPIO_PIN25,GPIO_PULL_UP);

    //motor outputs (copied from the activity document)
    //left motor
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN15);//enable
        GPIO_setPins(GPIOB,GPIO_PIN15);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN8);//direction (forward if low)
        GPIO_clearPins(GPIOB,GPIO_PIN8);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN7);//speed
    //right motor
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN16);//enable
        GPIO_setPins(GPIOB,GPIO_PIN16);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN0);//direction (forward if low)
        GPIO_clearPins(GPIOB,GPIO_PIN0);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN6);//speed

     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN7,5);
     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN6,5);
}
void timerInit(){
    time1.mode = TIMER_MODE_PERIODIC_UP; // starts at zero and counts up
    time1.clksrc = TIMER_CLOCK_LFCLK;//32.768 KHz
    time1.clkprescale = 0; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
    time1.clkdivratio = TIMER_CLOCK_DIVIDE_1;//
    time1.period=999;//can be changed (keep duty cycle constant) (by defalt its one second period)
    myPeriod=999;
    //clock ticks at 16384 times a second
  
    //Timers_CompareConfig stuff
    time2.ccrn=TIMER_CCR_CCR0|TIMER_CCR_CCR1;//1 for left wheel, 2 for right wheel
    time2.action=TIMER_CCR_ACTION_ZERO_CLEAR | TIMER_CCR_ACTION_UPCOMPARE_SET;//copied form activity 10 document
    time2.value=999/10;//also be changed
    myCCRN1=999/10;
    myCCRN2=myCCRN1;
    time2.invertOutput=0;
    Timers_initTimer(TIMG8,&time1);
    Timers_initCompare(TIMG8,&time2);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,myCCRN1);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN2);
}
