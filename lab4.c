
////////////////////////////////////////////////////////////////////////
//** ENGR-2350 
//** NAME: Colby Giunta & Sebastian Brzozowski
//** RIN: 662105414
//** lab 4
//** This is the base project for several activities and labs throughout
//** the course.  The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
////////////////////////////////////////////////////////////////////////

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_mspm0.h" file and adds it here.
#include "engr2350_mspm0.h"

//CURRENTLY LAB 3 WITHOUT PREVIOUS SPEED CONTROL AND MOVE FUNCTIONS

// Add function prototypes here as needed.
void timerInit();
void ADCInit();
void gpioInit();
void leftPID();
void rightPID();
void sanitizeChannel1();
void turnControl();

const uint8_t averageConst=10;
// Add global variables here as needed.
    //timers, encoders, caputures, and ADC structs
Timers_TimerConfig pwmTimer;
Timers_TimerConfig encoderTimer;
Timers_CompareConfig pwmSignal;
Timers_CaptureConfig encodeSignal;
ADC_Config myADC1;
ADC_ChanConfig firstChannel;//left potentiometer (speed)
ADC_ChanConfig secondChannel;//right potentiometer (turn)
    //Encoders
uint64_t LpreviousCount=0;
uint64_t LcurrentCount;
uint64_t LdeltaCount;
uint16_t LtimerResets=0;
float LdeltaT;
float LRPM;
float LaverageRPM=0;
float LRPMReadings[averageConst];
uint8_t Lindex=0;
float Ltotal;
float Lerror;
float LpreviousError;
float LPID;
bool leftReverse=false;
float LtotalError;
float LdesiredRPM;
float Lturn;


uint64_t RpreviousCount=0;
uint64_t RcurrentCount;
uint64_t RdeltaCount;
uint16_t RtimerResets=0;
float RdeltaT;
float RRPM;
float RRPMReadings[averageConst];
uint8_t Rindex=0;
float RaverageRPM=0;
float Rtotal;
float Rerror;
float RpreviousError;
float RPID;
bool rightReverse=false;//need this for the RPM as the encoder cannot handle a negative RPM value
float RtotalError;
float RdesiredRPM;
float Rturn;
    //wheel based non-encoder values
float wheelRadius=0.035;//this is in meters 
float carWidth=0.149;//thisi is in meters

float myCCRN1;//change  desiredCCRx values for change in wheel speed.
float myCCRN2;
//other
float turnRatio;
float turnRadius;
//float maxDifferential=
//potentiometer 
uint16_t channel1;
float linearRPM;
uint16_t channel2;
//PID controls
float PConst=10;
float DConst=2;
float IConst=1;

int main() {    //// Main Function ////
  
    // Add local variables here as needed.

    // We always call the sysInit function first to set up the 
    // microcontroller for how we are going to use it.
    sysInit();
    timerInit();
    gpioInit();
    ADCInit();
    printf("program starting\n");
    delay_cycles(32e6);
    // Place initialization code (or run-once) code here
    
    while(1){
        delay_cycles(32e5);
        //remember: CCRN value range: 0 to 10,000
        ADC_startConversion(ADC0);
        while(ADC_getStatus(ADC0)){
        }
        channel1=ADC_getResult(ADC0,ADC_MEM_INDEX_0);//range: 0 to 4096
        channel2=ADC_getResult(ADC0,ADC_MEM_INDEX_1);//range: 0 to 4096
        //printf("Channel2 : %u  ",channel2);
        linearRPM =((channel1*400.0/4096.0)-200.0);//only one value needed for both wheels: range: -200,200
        LdesiredRPM=linearRPM;
        RdesiredRPM=linearRPM;

        //caps potentiometer so motors doen't chase windmills
        sanitizeChannel1();
        //Turning control
        turnControl();

        //PID start
        leftPID();
        // printf("linearRPM: %f  ",linearRPM);
        // printf("LRPM: %f",LRPM);
        // printf("LaverageRPM: %f. ",LaverageRPM);
        // printf("Lerror: %f. ",Lerror);
        // printf("LPID: %f. ",LPID);
        // printf("CCRN1 (left wheel): %f\n",myCCRN1);
    
        rightPID();
        // printf("linearRPM: %f  ",linearRPM);
        // printf("RRPM: %f ",RRPM);
        // printf("RaverageRPM: %f. ",RaverageRPM);
        // printf("Rerror: %f. ",Rerror);
        // printf("RPID: %f. ",RPID);
        // printf("CCRN2 (right wheel): %f\n",myCCRN2);

        //master display console
        // printf("linearRPM: %5.2f | ",linearRPM);
        // printf("LaverageRPM: %5.2f | ",LaverageRPM);
        // printf("RaverageRPM: %5.2f | ",RaverageRPM);
        // printf("LRPM-RRPM: %5.2f | ",LaverageRPM-RaverageRPM);
        // printf("LCCRN: %5.2f | ",myCCRN2);
        // printf("RCCRN: %5.2f | ",myCCRN2);
        // printf("\n");

    }
}    //// End Main Function ////  

// Add function declarations here as needed
void leftPID(){
    Lerror=LdesiredRPM-LaverageRPM;
    LtotalError+=Lerror;
    LPID= (PConst*Lerror)+(DConst*(Lerror-LpreviousError))+(IConst*(LtotalError));
    myCCRN1+=LPID;

    if(abs(myCCRN1/1)>9000){//90% duty cycle cap in both directions (needed when turning big)
        if(myCCRN1>0){
            myCCRN1=9000;
        }else{
            myCCRN1=-9000;
        }
    }

    if(myCCRN1<0){
        GPIO_setPins(GPIOB,GPIO_PIN8);//left wheel in reverse.
        leftReverse=true;
    }else{
        GPIO_clearPins(GPIOB,GPIO_PIN8);
        leftReverse=false;
        }
    //myCCRN1=0;//freezes left wheel
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1,abs(myCCRN1/1));    
    LpreviousError=Lerror;
}
void rightPID(){
    Rerror=RdesiredRPM-RaverageRPM;//too fast, error is negative, too slow, error positive
    RtotalError+=Rerror;
    RPID= (PConst*Rerror)+(DConst*(Rerror-RpreviousError))+(IConst*(RtotalError));//all of PID in one
    myCCRN2+=RPID;

    if(abs(myCCRN2/1)>9000){//50% duty cycle cap
        if(myCCRN2>0){
            myCCRN2=9000;
        }else{
            myCCRN2=-9000;
        }
    }

    if(myCCRN2<0){
        GPIO_setPins(GPIOB,GPIO_PIN0);//right wheel in reverse.
        rightReverse=true;
    }else{
        GPIO_clearPins(GPIOB,GPIO_PIN0);
        rightReverse=false;
    }
    //myCCRN2=0;//freezes right wheel
    Timers_setCCRValue(TIMG8,TIMER_CCR_CCR0,abs(myCCRN2/1));
    RpreviousError=Rerror;
}
void sanitizeChannel1(){
    if(linearRPM>140){
            linearRPM=140;
        }else if(linearRPM<-140){
            linearRPM=-140;
        }
        if(linearRPM<30&&linearRPM>-30){
            // GPIO_clearPins(GPIOB,GPIO_PIN15); //clears the pins
             //GPIO_clearPins(GPIOB,GPIO_PIN16);
            linearRPM=0;
            myCCRN1=myCCRN1/2;
            myCCRN2=myCCRN2/2;
        }else{
            GPIO_setPins(GPIOB,GPIO_PIN15);//enables both motors
            GPIO_setPins(GPIOB,GPIO_PIN16);
        }
}
void turnControl(){
    turnRatio=(((float)channel2*100.0)/4096.0)-50.0;//to the right is turn right, to the left is turn left. 
    //Range: -50 to 50, maps directly onto the max RPM change 
    //pot all the way left: pot has high value, right wheel moves quick, left slow
    //turnRatio is fixed as specified by the document. It does not change with car speed.
    if(turnRatio<9&&turnRatio>-9){//snaps the value to zero
        turnRatio=0;
    }
    printf("channel2: %u | ",channel2);
    printf("turnRatio: %5.2f",turnRatio);
    
    LdesiredRPM= linearRPM-turnRatio;//turnRatio% slower
    RdesiredRPM= linearRPM+turnRatio;//turnRatio% faster
        //printf("turn Ratio. Turning left %f",turnRatio);
     printf("\n");
}
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

    pwmSignal.ccrn=TIMER_CCR_CCR0|TIMER_CCR_CCR1;//0 for left wheel, 1 for right wheel
    pwmSignal.action=TIMER_CCR_ACTION_ZERO_SET | TIMER_CCR_ACTION_UPCOMPARE_CLEAR;//cleared on zero, set high on compare
    pwmSignal.value=10000/3;//be default we run the car at 30% of its max power.
    myCCRN1=0;
    myCCRN2=myCCRN1;
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
void ADCInit(){
    myADC1.res=ADC_RESOLUTION_12BIT;
    myADC1.repeat=ADC_REPEAT_DISABLED;
    myADC1.sequence=ADC_SEQ_SEQUENCE;
    myADC1.trigsrc=0;
    myADC1.memStart=ADC_MEM_INDEX_0;
    myADC1.memEnd=ADC_MEM_INDEX_1;
    ADC_initADC(ADC0,&myADC1);

    //ADC_Channel config 1 left potentiometer (same as in activity 12) (pin A22)
    firstChannel.channel=7;//corresponds to pin A22
    firstChannel.mem=ADC_MEM_INDEX_0;
    firstChannel.vref=ADC_VREF_INT;//3.3V inernal reference
    firstChannel.trigMode=ADC_TRIGMODE_AUTO;//manual trigger
    ADC_initChannel(ADC0,&firstChannel);

    //ADC_Channel config 2: right potentiometer (pin A25)
    secondChannel.channel=2;//corresponds to  pin A25
    secondChannel.mem=ADC_MEM_INDEX_1;
    secondChannel.vref=ADC_VREF_INT;//3.3V inernal reference
    secondChannel.trigMode=ADC_TRIGMODE_AUTO;//manual trigger
    ADC_initChannel(ADC0,&secondChannel);
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

    //potentiometer ADC
    GPIO_initPeripheralAnalogFunction(GPIOA,GPIO_PIN22);//left pot (speed)
    GPIO_initPeripheralAnalogFunction(GPIOA,GPIO_PIN25);//right pot (turn)
    
//LINKS THE COMPARE TO THE PINS
//PWM is on TIMG8
     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN7,5);
     GPIO_initPeripheralFunction( GPIOB,GPIO_PIN6,5);

}
// Add interrupt functions last so they are easy to find
void TIMG6_IRQHandler(){//for encoders. changes to CCRN values for PWM are also done here.
//code for the changing of wheel speed is deleted
//note: if too fast, decrease CCRN value. if too slow, increase CCRN value
    uint32_t check = Timers_getPendingInterrupts(TIMG6);
    if(check & TIMER_INTSRC_ZERO){
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_ZERO);
        LtimerResets++;
        RtimerResets++;
       
        //printf("test inside TIMG6");
    }
    if(check & TIMER_INTSRC_CCR0_UP){//left motor movement
        //get info
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_CCR0_UP);
        LcurrentCount=Timers_getCounter(TIMG6);
        //do math on info
        LdeltaCount=(LcurrentCount-LpreviousCount)+(LtimerResets*65536);
        LdeltaT =LdeltaCount/320000000.0; // Calculate the time between previous and current event using enc_counts
        LRPM=(1/(60*LdeltaT));//this works
        if(leftReverse){
            LRPM*=-1;
        }else{
            LRPM=LRPM;
        }
        // Timers_setCCRValue(TIMG8,TIMER_CCR_CCR1, myCCRN1); CHANGES LEFT WHEEL RPM

        //averaging function
        //prevents very high values
        if(abs(LRPM/1)>400){//for some reason the left encoder likes reading a random RPM of over 8 thousand, I just set the value to the current average.
           // printf("LRPM Cap. Previous RPM:%f . speed adjusted to %f\n",LRPM,LaverageRPM);
            LRPM=LaverageRPM;
        }
        Ltotal-=LRPMReadings[Lindex]; //deletes the index were going to remove
        LRPMReadings[Lindex]=LRPM; //inserts new value into that index
        //printf("LRPMReadings[Lindex]: %f\n",LRPMReadings[Lindex]);
        Ltotal+=LRPM; //adds that value to the total
        Lindex= (Lindex+1)%averageConst;//zero to averageConst, then resets
        LaverageRPM=Ltotal/(0.0+averageConst);
        //printf("%u\n",Lindex);
       
        //always at the end
        LpreviousCount=LcurrentCount;
        LtimerResets=0;
    }   
    if(check & TIMER_INTSRC_CCR1_UP){//right motor movement
        //get info
        Timers_clearInterrupt(TIMG6,TIMER_INTSRC_CCR1_UP);
        RcurrentCount=Timers_getCounter(TIMG6);
        //do math on info
        RdeltaCount=(RcurrentCount-RpreviousCount)+(RtimerResets*65536);
        RdeltaT=RdeltaCount/320000000.0; 
        RRPM=(1/(60*RdeltaT));
        if(rightReverse){
            RRPM*=-1;
        }else{
            RRPM=RRPM;
        }
        if(abs(RRPM/1)>400){//for some reason the left encoder likes reading a random RPM of over 8 thousand, I just set the value to the current average.
           // printf("LRPM Cap. Previous RPM:%f . speed adjusted to %f\n",LRPM,LaverageRPM);
            RRPM=RaverageRPM;
        }
        //average out RRPM
        Rtotal-=RRPMReadings[Rindex];
        RRPMReadings[Rindex]=RRPM;
        Rtotal+=RRPM;
        Rindex=(Rindex+1)%averageConst;
        RaverageRPM=Rtotal/(0.0+averageConst);

       //always last. Do not code past
       RpreviousCount=RcurrentCount;
       RtimerResets=0;
    }    
}
//distance =totalevents*radius*(1.0/180)*3.14159; // Calculate distance travelled in mm using enc_total_events
