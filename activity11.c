
////////////////////////////////////////////////////////////////////////
//** ENGR-2350 activity 11
//** NAME: COlby Giunta and Sebastian W
//** RIN: 662105414
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
// Add global variables here as needed.
Timers_TimerConfig time1;
Timers_CaptureConfig myCapture;
uint16_t radius =35;//THIS IS IN MILIMETERS
uint8_t timerResets=0;
uint16_t currentPeriod;
uint16_t previousPeriod=0;
uint32_t enc_total_events;// Keep track of total encoder events, useful in measuring distance traveled.
int32_t enc_counts_track;// Keep track the timer counts since the capture event (must be signed!).
int32_t enc_counts=0;//Final value of timer counts between capture events.
uint8_t enc_flag=0;// A flag to denote that a capture event has occurred.

uint16_t distance;
float delta;
float speed_rpm;
float speed_mm;
int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    GPIOInit();
    timerInit();
    // Place initialization code (or run-once) code here
    printf("\r\nDistance\tEnc_Counts\tDelta T\t\tAng.Speed\tLin.Speed\r\n"); // Column headers
    while(1){  
        // Place code that runs continuously in here
        if(enc_flag){ // Check to see if capture occurred
            enc_flag = 0; // reset capture flag
            distance =enc_total_events*radius*(1.0/180)*3.14159; // Calculate distance travelled in mm using enc_total_events
            delta =((enc_counts))/320000000.0; // Calculate the time between previous and current event using enc_counts
            speed_rpm = 1/(6*delta);// Calculate the instantaneous wheel speed in rpm
            speed_mm =(speed_rpm*radius*3.14159)/30; // Calculate the instantaneous car speed in mm/s
            printf("%5u mm\t%8u\t%7.1f ms\t%5.2f rpm\t%5.2f mm/s\r\n",distance,enc_counts,delta*1000,speed_rpm,speed_mm);
            previousPeriod=currentPeriod;
            timerResets=0;
        }
    }   
}    //// End Main Function ////  

// Add function declarations here as needed
void GPIOInit(){
    //encoders
    GPIO_initDigitalInput(GPIOB,GPIO_PIN10);//left wheel encoder
    GPIO_setInternalResistor(GPIOB,GPIO_PIN10,GPIO_PULL_UP);
    GPIO_initPeripheralFunction(GPIOB,GPIO_PIN10,1 );//documentation clais any function works
    //im not sure if internal resistor function needs to be called (it needs to be called for push buttons)
    GPIO_initPeripheralFunction( GPIOB,GPIO_PIN10,3);
    //GPIO_setInterruptPolarity(GPIOB,GPIO_PIN10,GPIO_INT_EDGE_RISE);
    //NVIC_EnableIRQ(GPIO_INT_IRQn);

    //"Create a GPIOInit() function and initialize one of the pins" only one pin needs to be our tape measure
}
void timerInit(){
    //timer code
    time1.mode = TIMER_MODE_PERIODIC_UP; // starts at zero and counts up
    //document doesn't specify which clock to use. I am using what was used in activiy 10
    time1.clksrc = TIMER_CLOCK_BUSCLK;//32 mega hertz
    time1.clkprescale = 0; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
    time1.clkdivratio = TIMER_CLOCK_DIVIDE_1;//
    time1.period=65535;//can be changed (keep duty cycle constant) (by defalt its one second period)
    Timers_initTimer(TIMG8,&time1);
    Timers_enableInterrupt(TIMG8, TIMER_INTSRC_ZERO);//detects when timer is zero
    Timers_enableInterrupt(TIMG8,TIMER_INTSRC_CCR0_UP);
    NVIC_EnableIRQ(TIMG8_INT_IRQn);

    //capture stuff (nothing is specified in the activity 11 document)
    //note: we only measure distance for one wheel
    myCapture.ccrn=TIMER_CCR_CCR0;//only one ccr souce is needed
    myCapture.inputSel=TIMER_CCR_INPUT_CCPn;//specified under 11.6.4
    myCapture.edge=TIMER_CCR_EDGE_RISE;
    myCapture.invertInput=0;
    Timers_initCapture(TIMG8, &myCapture);
    

    //starting timer
    Timers_startTimer(TIMG8);
    printf("starting timer.\n");
}
// Add interrupt functions last so they are easy to find
void TIMG8_IRQHandler(){//I have no idea how to update this without an interrupt
    uint32_t check=Timers_getPendingInterrupts(TIMG8);
    if(check&TIMER_INTSRC_ZERO){
        GPIO_clearInterrupt(TIMG8,TIMER_INTSRC_ZERO);
        timerResets++;//gets set to zero once a change in movement.
        enc_counts_track+=65536;
        //printf("timer reset");
    }else{
        GPIO_clearInterrupt(TIMG8,TIMER_INTSRC_CCR0_UP);
        enc_total_events++;
        enc_flag=1;
        enc_counts=enc_counts_track+Timers_getCounter(TIMG8);
        enc_counts_track=-Timers_getCounter(TIMG8);
    }
}
