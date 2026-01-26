////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Activity: Stopwatch Template Project
//** NAME: Colby Giunta
//** RIN: 662105414
////////////////////////////////////////////////////////////////////////

#include "engr2350_mspm0.h"

// Function Prototypes
void GPIOInit();
void TimerInit();
void UpdateTime(uint8_t * time_arry); // Take time array and add 1/100 s to it.
// **ACTIVITY** Add needed function prototypes
void TIMG7_IRQHandler();//timer interrupt
void GPIO_IRQHandler();//timer interrupt
void LEDCheck();//added to make interrupt function cleaner

// Global Variables
int8_t sign2=1;
 int8_t sign=1;
 uint8_t count=0;
Timers_TimerConfig time1;
uint8_t total_time[4]; // array to keep track of [tenths of seconds,seconds,minutes,hours]
uint8_t lap_time[4]; // Current lap time (previous lap not stored, only printed once)
uint8_t lap_num = 1; // Current lap number, start on lap 1
uint8_t print_flag = 1; // Flag to note that time should be printed on screen.
                        // We start this at 1 to print time at 0 (instead of leaving blank)
uint8_t counter = 0; // For toggling the LED
// **ACTIVITY** Add remaining variables needed here

int main() {    //// Main Function ////
    
    sysInit();
    GPIOInit();
    TimerInit();
    // Timers_startTimer(TIMG7);//buttons determine start and stop
    printf("timer program starting: by Colby Giunta\n");
    printf("Lap #\tLap Time\tTotal Time \r\n"); // Print header
    while(1){  
        // Place code that runs continuously in here

        // Check if new lap is requested. Will need to replace 0
        /*if(0) {
            putchar('\n'); // Make a new line in the printing for next lap
            // **ACTIVITY** Reset flag, reset lap time array and increment lap number

        }*/
        // Check if time to update print. This is complete, no changes needed
        if(print_flag=1) {

               printf("\r%6u\t%02u:%02u:%02u.%02u\t%02u:%02u:%02u.%02u\t",lap_num,lap_time[3],lap_time[2],lap_time[1],lap_time[0]
                                                              ,total_time[3],total_time[2],total_time[1],total_time[0]);
        }
    }
}    //// Main Function ////  

// Add function declarations here as needed
void GPIOInit( ) {
    // **ACTIVITY**
    // Configure pin for PA0 for LED1 to blink
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN0);//led1


    // Configure used GPIO - PA18,PB21 for Devboard switches S1 and S2, OR
                       //  - PA7,PB25 for RPI-RSLK bumpers BMP1 and BMP6
    //sets bumper button at A7 to a rising edge interrupt
    GPIO_initDigitalInput(GPIOA,GPIO_PIN7);//bumper start/stop
    GPIO_enableInterrupt( GPIOA,GPIO_PIN7);
    GPIO_setInterruptPolarity(GPIOA,GPIO_PIN7,GPIO_INT_EDGE_RISE);
    GPIO_setInternalResistor(GPIOA,GPIO_PIN7,GPIO_PULL_UP);

    //sets bumper button at B25 to a rising edge interrupt
    GPIO_initDigitalInput(GPIOB,GPIO_PIN25);//bumper lab 
    GPIO_enableInterrupt(GPIOB,GPIO_PIN25);
    GPIO_setInterruptPolarity(GPIOB,GPIO_PIN25,GPIO_INT_EDGE_RISE);
     GPIO_setInternalResistor(GPIOB,GPIO_PIN25,GPIO_PULL_UP);


    NVIC_EnableIRQ(GPIO_INT_IRQn);//final initalization for the any pin interrupt
    // The pins should be configured as inputs and should trigger
    // the associated interrupt ON PRESS ONLY.


}

void TimerInit(){
    // **ACTIVITY** Configure a timer to run at 10 Hz and trigger reset interrupt (most copied from Activity 7)
time1.mode = TIMER_MODE_PERIODIC_DOWN; // Set mode to down counting and repeating
time1.clksrc = TIMER_CLOCK_BUSCLK;//32 MHZ frequency
time1.clkprescale=27; // REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
time1.period=11428;// since 0 is a number, we want total counts=N-1
time1.clkdivratio=TIMER_CLOCK_DIVIDE_1;
Timers_initTimer(TIMG7,&time1); // Apply the configuration
Timers_startTimer(TIMG7);

//Timers_startTimer(TIMG7); prevent timer from starting in this function (doc said so)
Timers_enableInterrupt(TIMG7, TIMER_INTSRC_ZERO);//detects when timer is zero
NVIC_EnableIRQ(TIMG7_INT_IRQn);//timer interrrupt
}

// Use this function to update the time arrays. Note that the argument requested is
// the time array itself (arrays are by definition pointers, no & needed).
void UpdateTime(uint8_t * time_arry){
    time_arry[0]++;  // Increment hundredths of seconds
    if(time_arry[0] == 100){  // If a whole second has passed...
        time_arry[0] = 0;    // Reset tenths of seconds
        time_arry[1]++;      // And increment seconds
        if(time_arry[1] == 60){ // If a minute has passed...
            time_arry[1] = 0;   // Reset seconds
            time_arry[2]++;     // Increment minutes
            if(time_arry[2] == 60){  // and so on...
                time_arry[2] = 0;
                time_arry[3]++;
                if(time_arry[3] == 24){
                    time_arry[3] = 0;
                }
            }
        }
    }
}

// **ACTIVITY** Interrupt for the timer
void TIMG7_IRQHandler(){//code in this function
  GPIO_clearInterrupt(TIMG7,TIMER_INTSRC_ZERO);//prevents infinite loops.
  //Timers_clearInterrupt(TIMG7,TIMER_INTSRC_ZERO);//clears zero flag
    //triggered when the timer overflows. Inital timer set to 10ms period to overflow
    LEDCheck();
    UpdateTime(total_time);//I believe that the function does all the lifting for me.
    UpdateTime(lap_time);
}

// **ACTIVITY** Interrupt for the switches/bumpers
void GPIO_IRQHandler(){
    delay_cycles(320e4); // 10 ms delay (32 MHz clock) for debouncing
    //clears flags for all presses
uint32_t active_pins = GPIO_getPendingInterrupts(GPIOA)+GPIO_getPendingInterrupts(GPIOB);
  
    if(active_pins & GPIO_PIN7){//start/stop
        GPIO_clearInterrupt(GPIOA, GPIO_PIN7);

        if(!(GPIO_readPins(GPIOA,GPIO_PIN7))){
            if(sign2==1){
                sign2=-1;
                Timers_startTimer(TIMG7);
            }else if(sign2==-1){
                sign2=1;
                Timers_stopTimer(TIMG7);
                 print_flag=0;
                 printf("\n\nreset happening\n\n");
                 lap_time[0]=0;
                lap_time[1]=0;
                lap_time[2]=0;
                 lap_time[3]=0;
             lap_num=1;
             total_time[0]=0;
                total_time[1]=0;
                total_time[2]=0;
                 total_time[3]=0;
            }

        }
    }else if(active_pins & GPIO_PIN25){//lap time
        GPIO_clearInterrupt(GPIOB, GPIO_PIN25);
      if(!(GPIO_readPins(GPIOB,GPIO_PIN25))){
            lap_num++;//updates lap number
            lap_time[0]=0;
            lap_time[1]=0;
            lap_time[2]=0;
            lap_time[3]=0;
            printf("\n");
      }
    }
}
void LEDCheck(){//responsible for turning off light for a second then on for a second.
     count++;//happens every 10 1/100th of a second.
     
    if(count>99){
        count=0;//resets count to origional state
        sign=sign*-1;//flips sign of the digit
                if(sign==1){
                    GPIO_setPins(GPIOA,GPIO_PIN0);
                    //printf("light on \n");
                }else if (sign==-1){
                    GPIO_clearPins(GPIOA,GPIO_PIN0);
                   //printf("light off \n");
                }
    }
}
