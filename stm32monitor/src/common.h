/* common.h:
 *
 *  Common include file
 */
#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <time.h>


/* Include HAL definitions */
#include "stm32f3xx_hal.h"

/* How many system ticks per second */
#define TICK_RATE       (1000)
#define MILLISECONDS(x) ((x*1000)/TICK_RATE)
#define SECONDS(x)      ((x)*TICK_RATE)
#define MINUTES(x)      ((x)*SECONDS(60))
#define HOURS(x)        ((x)*MINUTES(60))


/*
 *  Simple table driven parser for a command line interface.
 *
 */

#define CMD_INTERACTIVE   0
#define CMD_SHORT_HELP    1
#define CMD_LONG_HELP     2

#define NODE_ID 0x01

/* Include command return codes */
typedef enum ParserReturnVal_e {
#define DEF_ENUM(name)   CmdReturn##name,
#include "command_codes.inc"
  MAXCmdReturns
} ParserReturnVal_t;


typedef struct {
  char *cmdname;       /* String containing the name of the command */
  ParserReturnVal_t (*func)(int);   /* Pointer to the action function */
  char *help;          /* Help string for the command */
} parse_table;

#define ADD_CMD(name,f,helptxt) \
const parse_table f##E __attribute__ ((used));				\
const parse_table f##E __attribute__ ((section(".parsetable." name))) = { \
    .cmdname = name,  \
    .func    = f, \
    .help    = helptxt };




void TaskInputInit(void);
void TaskInput(void *data);
ParserReturnVal_t parse(char *buf, int len,
			const parse_table *table);  /* Parse the
						       buffer and call
						       commands */ 
ParserReturnVal_t ParseCmd(char *buf); /* Parse a buffer against builtins */
int fetch_int32_arg(int32_t *dest);   /* Fetch an integer argument */
int fetch_uint32_arg(uint32_t *dest); /* Fetch a UWORD argument */
int fetch_int16_arg(int16_t *dest);   /* Fetch an integer argument */
int fetch_uint16_arg(uint16_t *dest); /* Fetch a UWORD argument */
int fetch_double_arg(double *dest);   /* Fetch a floating point argument */
int fetch_float_arg(float *dest);     /* Fetch a floating point argument */
int fetch_string_arg(char **dest);    /* Fetch a string argument */
int fetch_cmd_args(char **dest);      /* Fetch the rest of the command */
const char *ParserReturnValToString(ParserReturnVal_t val); /* Map a
							       return
							       code to
							       a
							       string */

/* tasking.c functions.
 *  Simple task interface.
 */
typedef struct TaskingTable_s {
  char *name;                   /* String containing the task name */
  void (*func)(void *data);     /* Task function */
  void (*initFunc)(void *data); /* Initialization function for task */
  void *data;                   /* User Data function */
  char *desc;                   /* Task description */
} TaskingTable_t;

#define ADD_TASK(f,f2, tdata,tdesc)					\
  const TaskingTable_t f##Task __attribute__ ((used));			\
  const TaskingTable_t f##Task __attribute__ ((section(".tasktable." #f))) = { \
    .name = #f,								\
    .func = f,								\
    .initFunc = f2,							\
    .data = tdata,							\
    .desc = tdesc };

void TaskingInit(void);      /* Initialize the task system */
void TaskingRun(void);       /* Run all the tasks once. */

/*
 *  Dumping functions
 */
void DumpBuffer(uint8_t *buffer, uint32_t count, uint32_t address);
void DumpBufferBinary(uint8_t *buffer, uint32_t count, uint32_t address);

/*
 * terminal.c functions 
 */

typedef enum {
#define TERMINAL(name,periph,irq,desc)	\
  INDEX_##name,
#include "terminal.inc"
  INDEX_MAX } PortIndex_e;
  


void TerminalInit(void);
uint32_t TerminalRead(PortIndex_e index, uint8_t *ptr, uint32_t len);
int TerminalReadNonBlock(PortIndex_e index,char *c);
int TerminalReadAnyNonBlock(char *c);
uint32_t TerminalInputBufferWrite(PortIndex_e index, char *p, uint32_t len);
uint32_t TerminalOutputBufferWrite(PortIndex_e index,const char *p, uint32_t len);
uint32_t TerminalWritePolled(PortIndex_e index, const char *p, uint32_t len);
uint32_t TerminalOutputBufferWriteCond(PortIndex_e index,const char *p,uint32_t len);
uint32_t TerminalReadCount(PortIndex_e index);
void PolledPrintf(const char * restrict fmt, ...);

/*
 * main.c functions
 */
int32_t TaskAdd(void (*f)(void *data), void *data);
int32_t TaskKill(int32_t id);
//int32_t TaskCurrent(void);
int32_t TaskSwitcher(void);
 int32_t TaskNext(void);

uint8_t RxMode();
void GPIO_Init(void);
void spi_init(void);
void send_data_to_spi(uint8_t, uint8_t);
uint8_t receive_data_from_spi(uint8_t, uint8_t);
void config_nrf24l01(uint8_t);
extern uint8_t sFlag;
void send_payload_to_spi(uint8_t *, uint8_t);
void receive_payload_from_spi(uint8_t *, uint8_t);
void txMode(uint8_t *);
void sendControlMsg(uint8_t *, uint8_t, uint8_t);
uint8_t configPipe(uint8_t);

uint8_t configTxAddress(uint8_t);
void readpipeAdress(uint8_t);
//uint8_t txActive = 0;

/* version info functions */
void VersionPrint(void);
extern const time_t VersionBuildDate;

/*
 * Watchdog timer functions
 */
void WDTCheckReset(void);
void WDTInit(void);
void WDTFeed(void);

/* dfu.c: Device Firmware Update functions */
void DFUInit(void);
void DFUStart(void);

/* Assembly language hooks */
void my_Loop(void);
void my_Init(void);


#endif


