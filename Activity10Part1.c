////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Activity 10
//** NAME: Colby Giunta & Sebastain B
//** RIN: 662105414
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
void TimerInit();
void GPIOInit();
void task1();
void updateBoth();
Timers_TimerConfig time1;
Timers_CompareConfig time2;

// Add global variables here as needed.
float dutyCycle;
uint16_t myCCRN;
uint16_t myPeriod;

int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    TimerInit();
    GPIOInit();
    Timers_startTimer(TIMG8);
    printf("code starting\n");

    // Place initialization code (or run-once) code here
    task1();
}    //// End Main Function ////  
void task1(){
while(1){
    dutyCycle=(myPeriod*1.00)/myCCRN;//makes it a float
    if(!GPIO_readPins(GPIOA,GPIO_PIN7)){ // BMP1 Pressed
    delay_cycles(320e3);
    while(!GPIO_readPins(GPIOA,GPIO_PIN7));
    delay_cycles(320e3);
        //increase blink frequence by 10%
        myCCRN*=1.1;
        myPeriod*=1.1;
        if(myPeriod>32768){
            printf("ceiling hit. period capped");
            myPeriod=32768;
            myCCRN=myPeriod;
        }else{
            printf("period increased as normal\n");
        }
        updateBoth();
        
    }else if(!GPIO_readPins(GPIOA,GPIO_PIN14)){ // BMP2 Pressed
    delay_cycles(320e3);
    while(!GPIO_readPins(GPIOA,GPIO_PIN14));
    delay_cycles(320e3);
    // Add code to decrease blink frequency by 10 %
        myCCRN*=0.9;
        myPeriod*=0.9;
        if(myCCRN<(16384*0.1)){//caps period
            printf("floor hit for period\n");
            myPeriod=16384*0.1;
            myCCRN=myPeriod*dutyCycle;
        }else{
            printf("period decreased as normal\n");
        }
        updateBoth();
        
    }else if(!GPIO_readPins(GPIOB,GPIO_PIN23)){ // BMP5 Pressed
    delay_cycles(320e3);
    while(!GPIO_readPins(GPIOB,GPIO_PIN23));
    delay_cycles(320e3);
    // Add code to decrease blink frequency by 10 % 
        myCCRN*=0.9;
        if(myCCRN<0){
            myCCRN=1;
            printf("floor hit: compare value is 1\n");
        }else{
            printf("decreased comparevalue\n");
        }
        updateBoth();
    
    
    }else if(!GPIO_readPins(GPIOB,GPIO_PIN25)){ // BMP6 Pressed
    delay_cycles(320e3);
    while(!GPIO_readPins(GPIOB,GPIO_PIN25));
        delay_cycles(320e3);
        // Add code to increase blink frequency by 10 %
        myCCRN*=1.1;
        if(((myCCRN*1.0)/myPeriod)>=1){
        printf("compare value ceiling hit. Compare value capped\n");
        myCCRN=myPeriod;
        }else{
            printf("compare value increases as normal.\n");
        }
        updateBoth();
        
    }//end of if statements to check inputs
}//end of while loop
}//end of task 1
void TimerInit(){
time1.mode = TIMER_MODE_PERIODIC_UP; // starts at zero and counts up
    time1.clksrc = TIMER_CLOCK_LFCLK;//32.768 KHz
    time1.clkprescale = 0; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
    time1.clkdivratio = TIMER_CLOCK_DIVIDE_2;//
    time1.period=16384;//can be changed (keep duty cycle constant) (by defalt its one second period)
    //clock ticks at 16384 times a second
    myPeriod=16384;
  

//Timers_CompareConfig stuff
    time2.ccrn=TIMER_CCR_CCR1;
    time2.action=TIMER_CCR_ACTION_ZERO_CLEAR | TIMER_CCR_ACTION_UPCOMPARE_SET;//copied from activity 10 document
    time2.value=16384/2;//can only be changed by 
    myCCRN=16384/2;
    time2.invertOutput=0;
    Timers_initTimer(TIMG8,&time1);//not sure to add & before time1
    Timers_initCompare(TIMG8,&time2);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN);
}
void GPIOInit(){
    //bumpers
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

    //led
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN0);
    //peripheral interrrupt function
    GPIO_initPeripheralFunction(GPIOA,GPIO_PIN0,6);
}
void updateBoth(){
    Timers_setPeriod(TIMG8,myPeriod);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN);
}
// Add interrupt functions last so they are easy to find
