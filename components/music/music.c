/*
 * music.c
 *
 *  Created on: 2017年10月19日
 *      Author: silezhi
 */
#include "music.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

#include "Buzz_Driver.h"

static const char *TAG="MUSIC";
const int notefr[9][12]={
            {      0,       0,      0,       0,      0,      0,       0,      0,       0,      0,       0,NOTE_B0},  //....n
            {NOTE_C1,NOTE_CS1,NOTE_D1,NOTE_DS1,NOTE_E1,NOTE_F1,NOTE_FS1,NOTE_G1,NOTE_GS1,NOTE_A1,NOTE_AS1,NOTE_B1},  //...n
            {NOTE_C2,NOTE_CS2,NOTE_D2,NOTE_DS2,NOTE_E2,NOTE_F2,NOTE_FS2,NOTE_G2,NOTE_GS2,NOTE_A2,NOTE_AS2,NOTE_B2},  //..n
            {NOTE_C3,NOTE_CS3,NOTE_D3,NOTE_DS3,NOTE_E3,NOTE_F3,NOTE_FS3,NOTE_G3,NOTE_GS3,NOTE_A3,NOTE_AS3,NOTE_B3},  //.n
            {NOTE_C4,NOTE_CS4,NOTE_D4,NOTE_DS4,NOTE_E4,NOTE_F4,NOTE_FS4,NOTE_G4,NOTE_GS4,NOTE_A4,NOTE_AS4,NOTE_B4},  //n
            {NOTE_C5,NOTE_CS5,NOTE_D5,NOTE_DS5,NOTE_E5,NOTE_F5,NOTE_FS5,NOTE_G5,NOTE_GS5,NOTE_A5,NOTE_AS5,NOTE_B5},  //n.
            {NOTE_C6,NOTE_CS6,NOTE_D6,NOTE_DS6,NOTE_E6,NOTE_F6,NOTE_FS6,NOTE_G6,NOTE_GS6,NOTE_A6,NOTE_AS6,NOTE_B6},  //n..
            {NOTE_C7,NOTE_CS7,NOTE_D7,NOTE_DS7,NOTE_E7,NOTE_F7,NOTE_FS7,NOTE_G7,NOTE_GS7,NOTE_A7,NOTE_AS7,NOTE_B7},  //n...
            {NOTE_C8,NOTE_CS8,NOTE_D8,NOTE_DS8,      0,      0,       0,      0,       0,      0,       0,      0}   //n....
            //     1,      #1,      2,      #2,      3,      4,      #4,      5,      #5,      6,      #6,      7
};

static void tone(int pinNum, uint32_t fre, uint32_t lastTime)
{
    SetBuzz(1,fre);
    vTaskDelay(pdMS_TO_TICKS(lastTime));
    SetBuzz(0,fre);
}

static void noTone(int pinNum)
{
    SetBuzz(0,100);
}

void playMelody(char *Melody,int playSpeed, int pinNum)
{
    //const char *d = ",";
    char *p;
    char cgy[10];
    int noteDuration=0;
    int i,j;
    uint8_t thisNote1=0,thisNote2=0;
    p = strtok(Melody,",");
    sprintf(cgy, "%s", p);
    while(p)
    {
        char note[]="0000000000";
        noteDuration=0;
        for (i=0;*(p+i)!='\0';i++){
            note[i]=*(p+i);
        }
        for (int j=4;j<i;j++){
            int time = 1;
            switch(note[j]){
                case '1':time=1;
                break;
                case '2':time=2;
                break;
                case '3':time=4;
                break;
                case '4':time=8;
                break;
                case '5':time=16;
                break;
                case '6':time=32;
                break;
                case '7':time=6;
                break;
            }
            noteDuration += playSpeed/(time);
        }

        if (note[0]=='n'){
            switch (note[1]){
                case '1':thisNote1=0;
                break;
                case '2':thisNote1=2;
                break;
                case '3':thisNote1=4;
                break;
                case '4':thisNote1=5;
                break;
                case '5':thisNote1=7;
                break;
                case '6':thisNote1=9;
                break;
                case '7':thisNote1=11;
                break;
            }
        }
        else if (note[0]=='s'){
            switch (note[1]){
                case '1':thisNote1=1;
                break;
                case '2':thisNote1=3;
                break;
                case '4':thisNote1=6;
                break;
                case '5':thisNote1=8;
                break;
                case '6':thisNote1=10;
                break;
            }
        }

        if (note[3]=='f'){
            switch (note[2]){
                case '0':thisNote2=4;
                break;
                case '1':thisNote2=3;
                break;
                case '2':thisNote2=2;
                break;
                case '3':thisNote2=1;
                break;
                case '4':thisNote2=0;
                break;
            }
        }
        else if (note[3]=='s'){
            switch (note[2]){
                case '1':thisNote2=5;
                break;
                case '2':thisNote2=6;
                break;
                case '3':thisNote2=7;
                break;
                case '4':thisNote2=8;
                break;
            }
        }
        if (note[1]=='0'){
            thisNote2=0;
            thisNote1=0;
        }
        tone(pinNum, notefr[thisNote2][thisNote1],noteDuration);

        noTone(pinNum);
        p=strtok(NULL,",");
        sprintf(cgy, "%s", p);
    }
}

static char melody[]= "n61f4,n71f4,n10f34,n71f4,n10f3,n30f3,n71f333,n31f3,n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f34,n31f4,n41f4,n10f43,\
                            n31f333,n10f3,n71f34,s41f4,s41f3,n71f3,n71f33,n00f3,n61f4,n71f4,n10f34,n71f4,n10f3,n10f3,n71f333,n31f4,n31f4\
                            n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f3,n10f4,n71f43,n10f3,n20f3,n30f4,n10f33,n00f4,n10f4,n71f4,n61f3,n71f3,s51f3\
                            n61f33,n00f3,n10f4,n20f4,n30f34,n20f7,n30f7,n40f7,n50f4,n20f333,n51f3,n20f5,n10f5,n71f5,n10f534,n20f3,n30f4\
                            n30f333,n61f4,n71f4,n10f3,n71f4,n10f4,n20f3,n10f34,n51f4,n51f33,n40f3,n30f3,n20f3,n10f3,n30f333,n61f4,n71f4,n10f34,n71f4,n10f3,n30f3,\
                            n71f333,n31f3,n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f3,n10f4,n71f43,n10f3,n20f3,n30f4,n10f433,\
                            n10f4,n71f4,n61f3,n71f3,s51f3,n61f333";

void MUSIC_TEST_TASK(void *parm)
{
    BuzzGpioInit();
    ESP_LOGI(TAG,"Music Test Start");
    playMelody(melody,2000,0);
    ESP_LOGI(TAG,"Music Test End");
    vTaskDelete(NULL);
}



