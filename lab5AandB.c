
////////////////////////////////////////////////////////////////////////
//** ENGR-2350
//** NAME: Colby Giunta & Sebastain Brzozowski
//** RIN: 662105414
//** Lab 5
//** This is the base project for several activities and labs throughout
//** the course. The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
////////////////////////////////////////////////////////////////////////
#include "engr2350_mspm0.h"

//
// Add function prototypes here as needed.
void timerInit();
void gpioInit();
void I2CInit();
void leftPID();
void rightPID();
void sanitizeRPM();
void turnControl();
void turnControl2();
void angleCalculation();
uint16_t readCompassHeading();
int8_t readCompassPitch();

const uint8_t averageConst=10;
// Add global variables here as needed.
//timers, encoders, caputures, and ADC structs
Timers_TimerConfig pwmTimer;
Timers_TimerConfig encoderTimer;
Timers_CompareConfig pwmSignal;
Timers_CaptureConfig encodeSignal;
I2C_ControllerConfig myI2C;
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
uint32_t LtotalEvents;
float Ldistance;


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
uint32_t RtotalEvents;
float Rdistance;
//wheel based non-encoder values
float wheelRadius=0.035;//this is in meters
float carWidth=0.149;//thisi is in meters

float myCCRN1;//change desiredCCRx values for change in wheel speed.
float myCCRN2;
//other
float turnRatio;
float turnRadius;
//float maxDifferential=
//potentiometer
float linearRPM;
//PID controls
float PConst=10;
float DConst=2;
float IConst=1;
//I2C variables
uint8_t dataArray[2];
int8_t signedDataArray1[1];
int8_t signedDataArray2[1];
//compass
float deltaTheta;
float carHeading;//cars current angle
float compassHeading;
float previousCompassHeading;
float headingError;
float previousHeadingError;
float totalHeadingError;
float PConstAngular=3;//can be adjusted
float DConstAngular=0;
float IConstAngular=0;


int main() { //// Main Function ////
// Add local variables here as needed.

// We always call the sysInit function first to set up the
// microcontroller for how we are going to use it.
    sysInit();
    timerInit();
    gpioInit();
    I2CInit();
    printf("program starting\n");

    delay_cycles(32e6);
// Place initialization code (or run-once) code here
    carHeading=(readCompassHeading()/10.0);//car always starts where the compass is pointing

//test conditions
// carHeading=0;//for testing
//  GPIO_clearPins(GPIOB,GPIO_PIN15);//disables both motors
// GPIO_clearPins(GPIOB,GPIO_PIN16);
//end of test condtions
    
while(1){
delay_cycles(3.2e6); // Wait 1/10 of a second
//remember: CCRN value range: 0 to 10,000
 linearRPM =readCompassPitch()*(200.0f/90.f);//only one value needed for both wheels: range: -200,200
 //linearRPM =0;//freezes car in place. Only allows turning in place to happen
 compassHeading=readCompassHeading()/10.0;//compass reads from 0 to 359.9.
//caps potentiometer so motors doen't chase windmills
sanitizeRPM();
turnControl2();
angleCalculation();
//

// LdesiredRPM=linearRPM;//temporary until turn control function is good
// RdesiredRPM=linearRPM;
//Turning control
//turnControl();

//PID start
leftPID();
// printf("linearRPM: %f ",linearRPM);
// printf("LRPM: %f",LRPM);
// printf("LaverageRPM: %f. ",LaverageRPM);
// printf("Lerror: %f. ",Lerror);
// printf("LPID: %f. ",LPID);
// printf("CCRN1 (left wheel): %f\n",myCCRN1);

rightPID();
// printf("linearRPM: %f ",linearRPM);
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
// printf("LError: %5.2f | ",Lerror);
// printf("LCCRN: %5.2f | ",myCCRN2);
// printf("RCCRN: %5.2f | ",myCCRN2);
// printf("RPID: %5.2f. ",RPID);
// printf("LPID: %5.2f",LPID);
//     printf("\n");

//turn control values
    printf("Compass H: %5.2f | ",compassHeading);//H=heading
    printf("Pitch: %d | ",readCompassPitch());
    printf("Car H: %5.2f | ",carHeading);
    printf("H Error: %5.2f | ",headingError);
    //printf("RRPM-LRPM: %5.2f | ",RaverageRPM-LaverageRPM);
    //printf("deltaTheta %f",deltaTheta);
    printf("DeltaTheta: %f | ",deltaTheta);
    printf("RPID: %f ",RPID);
    printf("turnRatio: %5.2f",turnRatio);


     printf("\n");


}
} //// End Main Function ////

// Add function declarations here as needed
void leftPID(){
Lerror=LdesiredRPM-LaverageRPM;
LtotalError+=Lerror;
LPID= (PConst*Lerror)-(DConst*(Lerror-LpreviousError))+(IConst*(LtotalError));
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
RPID= (PConst*Rerror)-(DConst*(Rerror-RpreviousError))+(IConst*(RtotalError));//all of PID in one
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
void sanitizeRPM(){
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
void turnControl(){//not used in part B. Kept for graders to see
turnRatio=readCompassRoll()*(50.0f/90.0f);//to the right is turn right, to the left is turn left.
//Range: -50 to 50, maps directly onto the max RPM change
//pot all the way left: pot has high value, right wheel moves quick, left slow
//turnRatio is fixed as specified by the document. It does not change with car speed.
if(turnRatio<9&&turnRatio>-9){//snaps the value to zero
turnRatio=0;
}
printf("turnRatio: %5.2f",turnRatio);
LdesiredRPM= linearRPM-turnRatio;//turnRatio% slower
RdesiredRPM= linearRPM+turnRatio;//turnRatio% faster
//printf("turn Ratio. Turning left %f",turnRatio);
printf("\n");
}
void turnControl2(){
    if(compassHeading>360){
        compassHeading=previousCompassHeading;
    }
    previousCompassHeading=compassHeading;//always one behind
    headingError=compassHeading-carHeading;
    //Sanitize error to be within bounds

    if(headingError>180){//now our error terms are always within -180 to 180
        headingError-=360;
    }else if(headingError<-180){
        headingError+=360;
    }
    
    //PID action
    totalHeadingError+=headingError;
    turnRatio=PConstAngular*headingError-DConstAngular*(headingError-previousHeadingError);//+IConstAngular*totalHeadingError;

    if(turnRatio>linearRPM/5.0){
        turnRatio=linearRPM/5.0;//20%of car speed stupid rule
    }
    LdesiredRPM= linearRPM+turnRatio;//turnRatio% change
    RdesiredRPM= linearRPM-turnRatio;//turnRatio% change
    previousHeadingError=headingError;
}
uint16_t readCompassHeading(){
    volatile uint8_t results1=I2C_readData(I2C1,0x60,2,dataArray,2);
    return (dataArray[0]<<8)+dataArray[1];//bitshift the dataArray to make room for the other data
    //void I2C_readData( I2C_Regs * i2c , uint8_t PeriphAddress , uint8_t StartReg , uint8_t * data , uint8_t len )
}
int8_t readCompassPitch(){
    volatile int8_t results2=I2C_readData(I2C1,0x60,4,signedDataArray1,1);
    return signedDataArray1[0];//no need for fancy bitshifting, its just one signed byte
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
void I2CInit(){
myI2C.busclkRate=32e6;//BUSCLK frequency (32 megahertz)
myI2C.bitRate=100000;//100 kilibit (100 thousand bits)
myI2C.addrMode=I2C_ADDR_MODE_7BIT;
I2C_initController(I2C1,&myI2C);
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
//I2C for compass
    GPIO_initPeripheralFunction(GPIOA, GPIO_PIN15|GPIO_PIN16,4);
    GPIO_setOpenDrain(GPIOA, GPIO_PIN15|GPIO_PIN16);
}
void angleCalculation(){//done every tenth of a second
    Ldistance=LtotalEvents*(3.14159*wheelRadius/180.0);
    if(leftReverse==true){
        Ldistance*=-1;
    }
    Rdistance=RtotalEvents*(3.14159*wheelRadius/180.0);
    if(rightReverse==true){
        Rdistance*=-1;
    }
    //clockwise is + angle change, so left wheel turns faster
    deltaTheta=((Ldistance-Rdistance)/carWidth)*(180/3.14159);//makes it in degrees
    carHeading+=deltaTheta;
    if(carHeading>360){
        carHeading=0;
    }
    if(carHeading<0){
        carHeading=360;
    }
    LtotalEvents=0;
    RtotalEvents=0;
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
//wheel not moving actually says its not moving
if(LtimerResets>30){
    LRPM=0;
    LaverageRPM=0;
    //myCCRN1=0;
}
if(RtimerResets>30){
    RRPM=0;
    RaverageRPM=0;
    //myCCRN2=0;
}
//printf("test inside TIMG6");
}
if(check & TIMER_INTSRC_CCR0_UP){//left motor movement

//get info
LtotalEvents++;
Timers_clearInterrupt(TIMG6,TIMER_INTSRC_CCR0_UP);
LcurrentCount=Timers_getCounter(TIMG6);
//do math on info
LdeltaCount=(LcurrentCount-LpreviousCount)+(LtimerResets*65536);
LdeltaT =LdeltaCount/320000000.0; // Calculate the time between previous and current event using enc_counts
LRPM=(1/(60*LdeltaT));//always produces positive values
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

//always at the end
LpreviousCount=LcurrentCount;
LtimerResets=0;

}

if(check & TIMER_INTSRC_CCR1_UP){//right motor movement
//get info
RtotalEvents++;
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
