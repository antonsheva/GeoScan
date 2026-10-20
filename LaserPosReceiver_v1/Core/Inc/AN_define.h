#ifndef _AN_DEFINE_
#define _AN_DEFINE_



#define TDC_FIFO_LEN    16
#define FIFO_LEN        100
#define REF_DIV         40000
#define PULSE_QTY       4 


#define RF      G_tdcData.rfClk
#define ST      G_tdcData.stop 
#define TM      G_tdcData.tmLen
#define OT      G_tdcData.origTime
#define PR_T    G_prevTdcData.tmLen
#define BC      G_tdcData.beacons
#define MR      m_result
#define FT      m_fifoTimes


#endif