//////////////////////////
// Lab 1
// ENGR-2350 F25
// Names: Colby Giunta and Sebastain
// Section: 2
// Side: B
// Seat: 30
//////////////////////////

#include "engr2350_mspm0.h"
#include "lab1lib.h"
#include <stdio.h>

void GPIOInit();
void testIO();
void controlSystem();

uint8_t BMP1;
uint8_t BMP2;
uint8_t BMP3;
uint8_t BMP4;
uint8_t BMP5;
uint8_t BMP6;
uint8_t PB1;
uint8_t PB2;
uint8_t k=1;
//previous inputs (P___ defines previous)
uint8_t PBMP1;
uint8_t PBMP2;
uint8_t PBMP3;
uint8_t PBMP4;
uint8_t PBMP5;
uint8_t PBMP6;
uint8_t PPB1;
uint8_t PPB2;
uint8_t sign=1;//either 1 or -1, for the RGB to change colors

//slide 1 not effected by previous inputs(we read slide one continuously)
uint8_t slide1;
void getInputs(){
    BMP1= GPIO_readPins(GPIOA,GPIO_PIN7)!=0;
    BMP2= GPIO_readPins(GPIOA,GPIO_PIN14)!=0;
    BMP3= GPIO_readPins(GPIOB,GPIO_PIN5)!=0;
    BMP4= GPIO_readPins(GPIOB,GPIO_PIN21)!=0;
    BMP5= GPIO_readPins(GPIOB,GPIO_PIN23)!=0;
    BMP6= GPIO_readPins(GPIOB,GPIO_PIN25)!=0;
    slide1=GPIO_readPins(GPIOA,GPIO_PIN9)!=0;
    PB1= GPIO_readPins(GPIOB,GPIO_PIN2)!=0;
    PB2= GPIO_readPins(GPIOB,GPIO_PIN3)!=0;
}
void duplicate(){
PBMP1=BMP1;
PBMP2=BMP2;
PBMP3=BMP3;
PBMP4=BMP4;
 PBMP5=BMP5;
 PBMP6=BMP6;
 PPB1=PB1;
 PPB2=PB2;
}

int main() {    //// Main Function ////
    clearSequence();//wipes sequence
    sysInit(); // Basic car initialization
    initSequence(); // Initializes the lab1Lib Driver
    GPIOInit();

    printf("\r\n\n"
           "===========\r\n"
           "Lab 1 Begin\r\n"
           "===========\r\n");
    while(1){
       controlSystem();
       //testIO();
    }
}    //// Main Function ////


void GPIOInit(){
    // Add initializations of inputs and outputs
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN26);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN22);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN27);
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN12);
    GPIO_initDigitalOutput(GPIOA,GPIO_PIN13);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN15);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN8);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN16);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN8);
    GPIO_initDigitalOutput(GPIOB,GPIO_PIN0);
    //bumper pins for car movement.
     GPIO_initDigitalInput(GPIOA,GPIO_PIN7);
     GPIO_initDigitalInput(GPIOA,GPIO_PIN14);
     GPIO_initDigitalInput(GPIOB,GPIO_PIN5);
     GPIO_initDigitalInput(GPIOB,GPIO_PIN21);
     GPIO_initDigitalInput(GPIOB,GPIO_PIN23);
     GPIO_initDigitalInput(GPIOB,GPIO_PIN25);
    //button inputs pins
     GPIO_initDigitalInput(GPIOA,GPIO_PIN9);//slide1
     GPIO_initDigitalInput(GPIOB,GPIO_PIN2);//pb 1
     GPIO_initDigitalInput(GPIOB,GPIO_PIN3);//pb 2
     //other thing
     GPIO_setInternalResistor(GPIOA, GPIO_PIN7, GPIO_PULL_UP);
     GPIO_setInternalResistor(GPIOA, GPIO_PIN14, GPIO_PULL_UP);
     GPIO_setInternalResistor(GPIOB, GPIO_PIN5, GPIO_PULL_UP);
    GPIO_setInternalResistor(GPIOB, GPIO_PIN21, GPIO_PULL_UP);
     GPIO_setInternalResistor(GPIOB, GPIO_PIN23, GPIO_PULL_UP);
     GPIO_setInternalResistor(GPIOB, GPIO_PIN25, GPIO_PULL_UP);
}

    // Add printf statement(s) for testing inputs
void testIO(){
    // Example code for testing outputs
       //uint8_t cmd = getchar();
       uint8_t cmd=0;
        getInputs();
        if(cmd == 'r'){
            // Turn RGB LED Red
             GPIO_setPins(GPIOB,GPIO_PIN26);
             printf("red is on\n");
        }else if(cmd == 'g'){
            // Turn RGB LED Greenr
            GPIO_setPins(GPIOB,GPIO_PIN27);
             printf("green is on\n");
        }else if(cmd=='b'){
            GPIO_setPins(GPIOB,GPIO_PIN22);
             printf("blue is on\n");
        }else if(cmd=='R'){
            GPIO_clearPins(GPIOB,GPIO_PIN26);
             printf("red is off\n");
        }else if(cmd=='G'){
            GPIO_clearPins(GPIOB,GPIO_PIN27);
             printf("green is off\n");
        }else if(cmd=='B'){
            GPIO_clearPins(GPIOB,GPIO_PIN22);
             printf("blue is off\n");
        }else if(cmd=='q'){
            GPIO_setPins(GPIOA,GPIO_PIN12);
             printf("BiLED is green\n");
        }else if(cmd=='w'){
            GPIO_clearPins(GPIOA,GPIO_PIN12);
            GPIO_clearPins(GPIOA,GPIO_PIN13);
             printf("BiLED is off\n");
        }else if(cmd=='e'){
            GPIO_setPins(GPIOA,GPIO_PIN13);
             printf("BiLED is red\n");
        }else if(PB1==1){
            printf("button 1 is on\n");
        }else if(PB2==1){
             printf("button  2 is on\n");
        }else if(slide1==1){
            printf("slide is on\n");
        }else{
            printf("nothing");
        }

}
void execute(){
     GPIO_clearPins(GPIOA,GPIO_PIN12);//
     GPIO_clearPins(GPIOA,GPIO_PIN13);//
   GPIO_setPins(GPIOA,GPIO_PIN12);//makes it green flash
        runSequence();
        while(statusSegment()!=-128){
             GPIO_setPins(GPIOA,GPIO_PIN12);
        }
        GPIO_clearPins(GPIOA,GPIO_PIN12);//
     GPIO_clearPins(GPIOA,GPIO_PIN13);//
    GPIO_setPins(GPIOA,GPIO_PIN13);//makes it red flash
}

void RGB(){
    GPIO_clearPins(GPIOB,GPIO_PIN26);
    GPIO_clearPins(GPIOB,GPIO_PIN27);
    GPIO_clearPins(GPIOB,GPIO_PIN22);
    if(sign==1){
        GPIO_setPins(GPIOB,GPIO_PIN27);
    }else{
         GPIO_setPins(GPIOB,GPIO_PIN22);
    }
    sign=sign*-1;
}
void addInputs(){
    getInputs();
        if(PB1==1&&PPB1==0){
            popSegment();//removes last segment
            printf("PB1 pressed\n");
            delay_cycles(320e3);//small delay
        
        }else if(PB2==1&&PPB2==0){
            delay_cycles(320e3);//small delay
            clearSequence();//wipes sequence
             printf("PB2 pressed: wipes sequence\n");
             RGB();
        }else if(BMP1==1&&PBMP1==0){
            delay_cycles(320e3);
            recordSegment(-90);
               printf("BMP1 pressed:\n ");
               RGB();
        }else if(BMP2==1&&PBMP2==0){
            delay_cycles(320e3);
            recordSegment(-45);
            printf("BMP2 pressed: \n");
            RGB();
        }else if(BMP3==1&&PBMP3==0){
            delay_cycles(320e3);
            recordSegment(0);
            printf("BMP3 pressed: \n");
            RGB();
        }else if(BMP4==1&&PBMP4==0){
            delay_cycles(320e3);
            recordSegment(6);
            printf("BMP4 pressed: \n");
            RGB();
        }else if(BMP5==1&&PBMP5==0){
            delay_cycles(320e3);
            recordSegment(45);
            printf("BMP5 pressed: \n");
            RGB();
        }else if(BMP6==1&&PBMP6==0){
            delay_cycles(320e3);
            recordSegment(45);
            printf("BMP6 pressed: \n");
            RGB();
        }
        getInputs();
        duplicate();
}
void controlSystem(){
  //printf("made it to control system");
         getInputs();
         if(slide1==0){
            printf("slide is 0: getting inputs:   \n");
                 GPIO_clearPins(GPIOA,GPIO_PIN12);//
         GPIO_clearPins(GPIOA,GPIO_PIN13);//
            addInputs();
            k=1;
         }else if (slide1==1&&k==1){
            k++;//==1 at the start
            printf("slide is 1: starting sequence:  \n");
            execute();
         }else if(k>1){
         }
    }
