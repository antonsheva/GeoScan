#include "AN_tdc.h"
#include <stdint.h>


/**
 * @brief init TDC:
 * LVDS input type
 * STOP input - 4
 * used the reset of the reference clock counter 
 * REFCLK_DIVISIONS = 40000
 * reference clock - external quartz
 */
void ATdcInit(){

  uint8_t dataOut[17] = { 
    0x08,                                       /*[0]   PIN_ENA_RSTIDX, PIN_ENA_DISABLE, PIN_ENA_STOP4 */
    0x08,                                       /*[1]   HIT_ENA_STOP 4 */
    0x00,                                       /*[2]   */
    0x40,0x9C,0x00,                             /*[3-5] REFCLK_DIVISIONS = 40000 */
    0xC0,                                       /*[6]   */
    0xC3,                                       /*[7]   REFCLK_BY_XOSC */
    0xA1,0x13,0x00,0x0A,0xCC,0xCC,0xF1,0x7D,    /*[8-15] Fixed value*/
    0x00                                        /*[16]  */
  };

  uint8_t dataIn[17];                      
  ASpiRstPulse();
  ASpiTransmit(0x30);
  ASpiRstPulse();

  ASpiTransmit(0x80);
  for(int i=0; i<17; i++){
    ASpiTransmit(dataOut[i]);
  }  

  ASpiRstPulse();  
  ASpiTransmit(0x40);
  ASpiReceive(dataIn, 17);
  
  ASpiRstPulse(); 
  ASpiTransmit(0x18);  

}

void ATdcReset(){
  HAL_GPIO_WritePin(TDC_RST_GPIO_Port, TDC_RST_Pin, 1);
  for(int i=0; i<500; i++)__NOP();
  HAL_GPIO_WritePin(TDC_RST_GPIO_Port, TDC_RST_Pin, 0);    
}

void ATdcResetStruct1(){
  for(int i=0; i<8; i++){
    G_tdcData.pulseTm [i] = 0;
    G_tdcData.rfClk   [i] = 0;
    G_tdcData.stop    [i] = 0;
    G_tdcData.tmLen   [i] = 0;
    G_tdcData.origTime[i] = 0;
  }
}

void ATdcResetStruct2(){
  for(int i=0; i<8; i++){
    G_prevTdcData.pulseTm [i] = 0;
    G_prevTdcData.rfClk   [i] = 0;
    G_prevTdcData.stop    [i] = 0;
    G_prevTdcData.tmLen   [i] = 0;
    G_prevTdcData.origTime[i] = 0;
  }
}

/**
 * @brief 
 * 
 * @param beacons 
 * @return int 
 */
int ATdcRead(){
  int dataValid[16] = {1};
  int error = 0;
  uint8_t d[16] = {0};
  int beaconsQty = 0; 
  int errLtch = 0;
  int stop = 0;
  ATdcResetStruct1();

  for(int i=0; i<16;i++){  //PULSE_QTY
    error = 0;
    memset(d,0,16);
	  ASpiRstPulse();
    ASpiTransmit(0x60 + 26);
    ASpiReceive (d,  6);   
    RF[i] = ((d[0]<<16)|(d[1]<<8)|d[2]);
    ST[i] = ((d[3]<<16)|(d[4]<<8)|d[5]);
    if(RF[i] == 0x7FFFFF){
    	if(beaconsQty == 1){
    	    		stop = 1;
    	    	}
    	if(beaconsQty == 2){
    	    		stop = 1;
    	    	}
    	if(beaconsQty == 3){
    	    		stop = 1;
    	    	}
    	if(beaconsQty == 4){
    	    		stop = 1;
    	}

    	break;
    }
    if(i>PULSE_QTY -1 )continue;
    if(G_prevTdcData.rfClk[i] > RF[i]){
        G_prevTdcData.rfClk[i] = RF[i];
        dataValid[i] = 0;
        error = 1;
        errLtch = 1;
    }
    if(ST[i]>REF_DIV/2){
        ST[i] = ST[i] & 0xFF7FFFFF; //reset MSB         
        if(ST[i]>REF_DIV/2){
            TM[i] = 0;
            dataValid[i] = 0;
            error = 1;
            errLtch = 1;
        }else{
            ST[i] += REF_DIV/2;
        }        
    }
                 
    if(!error){
        beaconsQty++;
        OT[i] = RF[i]*REF_DIV+ST[i]; 
        if(i > 0){
            TM[i-1]=OT[i]-OT[i-1];  
        }
    }
  }

  if(!errLtch){
    G_tdcData.packPause = OT[0]-G_prevTdcData.origTime[0];
    ATdcResetStruct2();
    for(int i=0; i<PULSE_QTY; i++){
      G_prevTdcData.rfClk    [i] = RF[i];
      G_prevTdcData.stop     [i] = ST[i];
      G_prevTdcData.tmLen    [i] = TM[i];
      G_prevTdcData.origTime [i] = OT[i];
    }  
  }

  return beaconsQty;
}





