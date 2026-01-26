////////////////////////////////////////////////////////////////////////
//** ENGR-2350 Template Project 
//** NAME: Colby Giunta & Sebastian Brzozowski
//** RIN: 662105414
//** lab 3
//** This is the base project for several activities and labs throughout
//** the course.  The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
////////////////////////////////////////////////////////////////////////

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_mspm0.h" file and adds it here.
#include "engr2350_mspm0.h"

// Add function prototypes here as needed.
void timerInit();
void gpioInit();
void moveIt();
void freezeIt();
void forward(int16_t dist);
void turnClockwise(float angle);
void moveR();
void moveI();

// Add global variables here as needed.
    //timers, encoders, and captures
Timers_TimerConfig pwmTimer;
Timers_TimerConfig encoderTimer;
Timers_CompareConfig pwmSignal;
Timers_CaptureConfig encodeSignal;
    //Encoders

uint8_t LtimerResets=0;
uint16_t LcurrentPeriod;
uint16_t LpreviousPeriod=0;
int32_t Lenc_counts_track;// Keep track the timer counts since the capture event (must be signed!).
int32_t Lenc_counts=0;//Final value of timer counts between capture events.
float Ldelta;
float Lspeed_rpm;
float Laverage;//for rpm

uint8_t RtimerResets=0;
uint16_t RcurrentPeriod;
uint16_t RpreviousPeriod=0;
int32_t Renc_counts_track;// Keep track the timer counts since the capture event (must be signed!).
int32_t Renc_counts=0;//Final value of timer counts between capture events.
float Rdelta;
float Rspeed_rpm;
float Raverage;

    //wheel based non-encoder values
uint16_t radius=35;//THIS IS IN MILIMETERS
uint32_t distance;
uint16_t totalevents;//each detection is one degree of motion
uint16_t goodRPM=900;

uint16_t myCCRN1;
uint16_t myCCRN2;
uint16_t origCCR;
//other
uint8_t averageCount1;
uint8_t averageCount2;
uint16_t LaverageRPM;
uint16_t RaverageRPM;

int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    timerInit();
    gpioInit();
    printf("program starting\n");
    // Place initialization code (or run-once) code here

//note:GPIO_setPins(GPIOB,GPIO_PIN15) can be used to turn on/off motors
    // while(1){
    //     forward(5000);
    // }
    //moveR();
    moveI();
}    //// End Main Function ////  

// Add function declarations here as needed
void timerInit(){
    //two timers need to be delclared. One for controlling PWM output, and another for wheel encoding.

    //pwm timer (TIMG8)
    pwmTimer.mode=TIMER_MODE_PERIODIC_UP;
    pwmTimer.clksrc=TIMER_CLOCK_BUSCLK;//32 megahertz clock. chop period off for 1/3200s PWM period
    pwmTimer.clkprescale=0;
    pwmTimer.clkdivratio=TIMER_CLOCK_DIVIDE_1;// REMEMBER effective divider value is (.clkdivratio)*(.clkprescale+1)
    pwmTimer.period=10000;//sets PWM drive frequency to 32Khz (period of 1/32000 seconds). 
    Timers_initTimer(TIMG8,&pwmTimer);//do interrupt code after this
    //Timers_enableInterrupt(TIMG8,TIMER_INTSRC_ZERO);//interrupt everytime the timer is zero.
    NVIC_EnableIRQ(TIMG8_INT_IRQn);

    pwmSignal.ccrn=TIMER_CCR_CCR0|TIMER_CCR_CCR1;//1 for left wheel, 2 for right wheel
    pwmSignal.action=TIMER_CCR_ACTION_ZERO_SET | TIMER_CCR_ACTION_UPCOMPARE_CLEAR;//cleared on zero, set high on compare
    pwmSignal.value=10000/3;//be default we run the car at 30% of its max power.
    myCCRN1=10000/3;
    myCCRN2=myCCRN1;
    origCCR=myCCRN2;
    pwmSignal.invertOutput=false;
    Timers_initTimer(TIMG8,&pwmTimer);
    Timers_initCompare(TIMG8,&pwmSignal);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,myCCRN1);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,myCCRN2);


    Timers_startTimer(TIMG8);

    //encoder timer(TIMG6)
    encoderTimer.mode=TIMER_MODE_PERIODIC_UP;
    encoderTimer.clksrc=TIMER_CLOCK_BUSCLK;
    encoderTimer.clkprescale=0;
    encoderTimer.clkdivratio=TIMER_CLOCK_DIVIDE_1;
    encoderTimer.period=65535;//largest it can be (as it should)
    Timers_initTimer(TIMG6,&encoderTimer);
    Timers_enableInterrupt(TIMG6,TIMER_INTSRC_ZERO);//interrupt everytime the timer is zero.
    NVIC_EnableIRQ(TIMG6_INT_IRQn);

    //encoder signals
    encodeSignal.ccrn=TIMER_CCR_CCR0|TIMER_CCR_CCR1;
    encodeSignal.inputSel=TIMER_CCR_INPUT_CCPn;
    encodeSignal.edge=TIMER_CCR_EDGE_RISE;
    encodeSignal.invertInput=false;
    Timers_initCapture(TIMG6,&encodeSignal);
    Timers_enableInterrupt(TIMG6,TIMER_INTSRC_CCR0_UP);
    Timers_enableInterrupt(TIMG6,TIMER_INTSRC_CCR1_UP);

    Timers_startTimer(TIMG6);
}
void gpioInit(){
    //left motor
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN15);//enable
        GPIO_setPins(GPIOB,GPIO_PIN15);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN8);//direction (forward if low)
        GPIO_clearPins(GPIOB,GPIO_PIN8);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN7);//speed
        //encoder
    GPIO_initDigitalInput(GPIOB,GPIO_PIN10);//left wheel encoder
    GPIO_setInternalResistor(GPIOB,GPIO_PIN10,GPIO_PULL_UP);
        //left motor uses CCR0
    GPIO_initPeripheralFunction(GPIOB,GPIO_PIN10,5);//links digital input to encoder
    //right motor
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN16);//enable
        GPIO_setPins(GPIOB,GPIO_PIN16);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN0);//direction (forward if low)
        GPIO_clearPins(GPIOB,GPIO_PIN0);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN6);//speed
        //encoder
    GPIO_initDigitalInput(GPIOB,GPIO_PIN11);//right wheel encoder
    GPIO_setInternalResistor(GPIOB,GPIO_PIN11,GPIO_PULL_UP);
        //peripheral function links pins to CCCR through function. B 11 (right motor) uses CCR1
    GPIO_initPeripheralFunction(GPIOB,GPIO_PIN11,5);//links digital input to encoder

//LINKS THE COMPARE TO THE PINS
//PWM is on TIMG8
     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN7,5);//left motor
     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN6,5);//right motor

}
void freezeIt(){
    GPIO_clearPins(GPIOB,GPIO_PIN15);
    GPIO_clearPins(GPIOB,GPIO_PIN16);
}
void moveIt(){
    GPIO_setPins(GPIOB,GPIO_PIN15);
    GPIO_setPins(GPIOB,GPIO_PIN16);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,origCCR);
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,origCCR);
}
void forward(int16_t dist){
    distance=0;//doesn't inherit any previous distance
    totalevents=0;
    printf("full Distance: %u\n",dist);
    printf("inital Distance: %u\n",distance);
    if(dist<=0){//make car go in reverse
        GPIO_setPins(GPIOB,GPIO_PIN8);//direction (forward if low)
        GPIO_setPins(GPIOB,GPIO_PIN0);
    }
    while(distance<abs(dist)){
        moveIt();
        printf("current Distance: %u\n",distance);
    }
    freezeIt();
    GPIO_clearPins(GPIOB,GPIO_PIN8);//clears direction pins
    GPIO_clearPins(GPIOB,GPIO_PIN0);
    delay_cycles(320e4);
}
void turnClockwise(float angle){//angle is in degrees
   //printf("set angle %u degrees\n",angle);
    distance=0;
    totalevents=0;
    GPIO_setPins(GPIOB,GPIO_PIN0);//sets right motor to turn backwards
    angle=2.0*(angle*3.14159/180.0)*radius;//converts angle into a distance (in mm)
    //printf("angle in mm %u\n",angle);
    while(distance<angle){
        moveIt();
    }
    GPIO_clearPins(GPIOB,GPIO_PIN0);
    delay_cycles(320e4);
}
void moveR(){//moves around the R (because our distance is is mm. all distance values are also in mm)
    printf("moving in the shape of an R\n");
    forward(1830);
    turnClockwise(90);
    forward(860);
    turnClockwise(90);
    forward(860);
    turnClockwise(90);
    forward(860);
    turnClockwise(228);//going the long way around
    forward(1300);
}
void moveI(){//moves around the I
    forward(860);
    forward(-430);
    turnClockwise(235);
    forward(1830);
    turnClockwise(90);
    forward(430);
    forward(-860);
}
// Add interrupt functions last so they are easy to find
void TIMG6_IRQHandler(){//for encoders. changes to CCRN values for PWM are also done here.
     uint32_t check = Timers_getPendingInterrupts(TIMG6);
    if(check & TIMER_INTSRC_ZERO){
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_ZERO);
        LtimerResets++;
        RtimerResets++;
        Lenc_counts_track+=65536;
        Renc_counts_track+=65536;
        //printf("test inside TIMG6");
    }
    if(check & TIMER_INTSRC_CCR0_UP){//left motor movement
        //get info
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_CCR0_UP);
        totalevents++;//only the right wheel measures the distance.
        //printf("totalevents: %u\n",totalevents);
        Lenc_counts=Lenc_counts_track+Timers_getCounter(TIMG6);
        Lenc_counts_track=-Timers_getCounter(TIMG6);
        //do math on info
        distance =totalevents*radius*(1.0/180)*3.14159; // Calculate distance travelled in mm using enc_total_events
        Ldelta =((Lenc_counts))/320000000.0; // Calculate the time between previous and current event using enc_counts
        Lspeed_rpm = 1/(6*Ldelta);// Calculate the instantaneous wheel speed in rpm
       // printf("left RPM: %5.2f\n",Lspeed_rpm); //RPM works, move on
       if(averagecount1==5){//average out five rpms
            averagecount1=1;
            LaverageRPM=LaverageRPM/5
       if(LaverageRPM>600){
        if(LaverageRPM>goodRPM){
            myCCRN1--;
            if(myCCRN1<1){
                myCCRN1=1;//prevent underflow
            }
            //printf("left motor CCRN--: CCRN1=%u : LRPM=%5.2f : goodRPM=%u\n",myCCRN1,Lspeed_rpm,goodRPM);
        }else{
            myCCRN1++;
          //  printf("left motor CCRN++: CCRN1=%u : LRPM=%5.2f : goodRPM=%u\n",myCCRN1,Lspeed_rpm,goodRPM);
        }
        Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1, myCCRN1);

       }
     }else{
        averagecount1++;
        LaverageRPM+=Lspeed_rpm;
     }
    }
    if(check & TIMER_INTSRC_CCR1_UP){//right motor movement
        //get info
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_CCR1_UP);
        Renc_counts=Renc_counts_track+Timers_getCounter(TIMG6);
        Renc_counts_track=-Timers_getCounter(TIMG6);
        //do math on info
        Rdelta =((Renc_counts))/320000000.0; // Calculate the time between previous and current event using enc_counts
        Rspeed_rpm = 1/(6*Rdelta);// Calculate the instantaneous wheel speed in rpm
        //printf("right RPM: %5.2f\n",Rspeed_rpm);// RPM works, move on
        if(averageCount2==5){
            averageCount2=1;
            RaverageRPM=RaverageRPM/5
        if(RaverageRPM>600){
        if(RaverageRPM>goodRPM){
            myCCRN2--;
            //printf("right motor CCRN--: CCRN2=%u : RRPM=%5.2f : goodRPM=%u\n",myCCRN2,Rspeed_rpm,goodRPM);
            if(myCCRN2<1){
                myCCRN2=1;//prevent underflow
            }
        }else{
            //printf("right motor CCRN++: CCRN2=%u : RRPM=%5.2f : goodRPM=%u\n",myCCRN2,Rspeed_rpm,goodRPM);
            myCCRN2++;
        }
        }
        }else{
            averageCount2++;
            RaverageRPM+=Rspeed_rpm;
        }
        Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0, myCCRN2);
        
    }
}
