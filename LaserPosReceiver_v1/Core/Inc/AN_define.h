#ifndef _AN_DEFINE_
#define _AN_DEFINE_



#define TDC_FIFO_LEN    16
#define FIFO_LEN        100
#define REF_DIV         40000
#define PULSE_QTY       4 

#define UART_BUFF_SIZE 16

#define PRINT_PARAM_TDC 0
#define PRINT_PARAM_CNT 1
#define PRINT_PARAM_MSG 2

#define UART_DATA_OK       0
#define UART_ERR_BAD_DATA -1

#define CMD_CMPSET 1
#define CMD_CMPGET 2
#define CMD_FPSSET 3
#define CMD_FPSGET 4
 
#define PRT G_printTdcData

#define RF      G_tdcData.rfClk
#define ST      G_tdcData.stop 
#define TM      G_tdcData.tmLen
#define OT      G_tdcData.origTime
#define PR_T    G_prevTdcData.tmLen
#define BC      G_tdcData.beacons
#define MR      m_result
#define FT      m_fifoTimes

#endif