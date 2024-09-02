#ifndef __KEY_BOARD_CONFIG_H
#define __KEY_BOARD_CONFIG_H

#define KEY_ENABLE    (1)
#define KEY_DISABLE   (0)

#define KEY_TRIG_REPORT  (1)
#define KEY_TRIG_QUERY   (0)

#define KEY_SCAN_RATE           (200)
//\\\\\\\\\\\\\\\\\\\\\\\\\config item\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#define KEY_EVENT_TRIG_MODE         				   KEY_TRIG_REPORT
//max key bord number
#define KEY_BOARD_MAX_NUM                             (1)

//max key number
#define KEY_CTRL_NUM                                  (1)
#define KEY_SIG_NUM                                   (3)
#define KEY_MAX_NUM                                   (KEY_CTRL_NUM*KEY_SIG_NUM)
#define KEY_SCAN_FREQ                                 (5)
//max combine ke number
#define KEY_COMBINE_NUM                               (1000/KEY_SCAN_RATE)
//default long press timer
#define KEY_DEFAULT_LONG_TRRIGER_TIME                 (2000/KEY_SCAN_FREQ)

//By default, the allowed interval between two consecutive clicks during multi click
#define KEY_DEFAULT_MULTI_INTERVAL_TIME               (500/KEY_SCAN_FREQ)

//Default initial trigger time of continuous pressing
#define KEY_DEFAULT_CONTINUOUS_INIT_TRRIGER_TIME      (500/KEY_SCAN_FREQ)

//Default continuous press cycle trigger time
#define KEY_DEFAULT_CONTINUOUS_PERIOD_TRRIGER_TIME    (500/KEY_SCAN_FREQ)

//The allowable interval between two consecutive triggers in default combination
#define KEY_DEFAULT_COMBINE_INTERVAL_TIME             (400/KEY_SCAN_FREQ)

//Default shake elimination time (ms), when it is 0, there is no shake elimination function
#define KEY_DEFAULT_DEBOUNCE_TIME                     (10/KEY_SCAN_FREQ)

//Long press detection support
#define KEY_LONG_SUPPORT                              KEY_ENABLE

//Multi hit support
#define KEY_MULTI_SUPPORT                             KEY_ENABLE

//Continuous click support
#define KEY_CONTINUOUS_SUPPORT                        KEY_ENABLE

//Key combination support
#define KEY_COMBINE_SUPPORT                           KEY_ENABLE

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#endif/*__KEY_BOARD_CONFIG_H*/
