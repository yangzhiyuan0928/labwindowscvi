/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2016. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: panelCB */
#define  PANEL_TRAFFIC2_NUM               2       /* control type: ring, callback function: Traffic2PortEnablde */
#define  PANEL_TRAFFIC1_NUM               3       /* control type: ring, callback function: Traffic1PortEnablde */
#define  PANEL_TRAFFIC_LED_2              4       /* control type: LED, callback function: (none) */
#define  PANEL_PORT_NUM                   5       /* control type: ring, callback function: PortEnable */
#define  PANEL_TRAFFIC_LED_1              6       /* control type: LED, callback function: (none) */
#define  PANEL_UART_LED                   7       /* control type: LED, callback function: (none) */
#define  PANEL_PORT_CLOSE                 8       /* control type: command, callback function: PortClose */
#define  PANEL_VALVE1_CTRL                9       /* control type: binary, callback function: ValveCtrl_1 */
#define  PANEL_VALVE2_CTRL                10      /* control type: binary, callback function: ValveCtrl_2 */
#define  PANEL_TRAFFIC_2                  11      /* control type: scale, callback function: (none) */
#define  PANEL_QUIT                       12      /* control type: command, callback function: QuitCallBack */
#define  PANEL_RECEIVE_MESSAGE            13      /* control type: textMsg, callback function: (none) */
#define  PANEL_TRAFFIC_1                  14      /* control type: scale, callback function: (none) */
#define  PANEL_TEXTMSG                    15      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG_3                  16      /* control type: textMsg, callback function: (none) */
#define  PANEL_DECORATION                 17      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_7               18      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_6               19      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_2               20      /* control type: deco, callback function: (none) */
#define  PANEL_TRAFFIC1_CLEAR             21      /* control type: command, callback function: Traffic1_Clear */
#define  PANEL_TRAFFIC2_CLEAR             22      /* control type: command, callback function: Traffic2_Clear */
#define  PANEL_DECORATION_3               23      /* control type: deco, callback function: (none) */
#define  PANEL_TRAFFIC1_TIMER             24      /* control type: timer, callback function: TrafficTimer1 */
#define  PANEL_TRAFFIC2_TIMER             25      /* control type: timer, callback function: TrafficTimer2 */
#define  PANEL_TRAFFICSPEEDCHART_2        26      /* control type: strip, callback function: (none) */
#define  PANEL_FLOW1METRECHART_1          27      /* control type: strip, callback function: (none) */
#define  PANEL_FLOW1METRECHART_2          28      /* control type: strip, callback function: (none) */
#define  PANEL_TRAFFICSPEEDCHART_1        29      /* control type: strip, callback function: (none) */
#define  PANEL_DECORATION_4               30      /* control type: deco, callback function: (none) */
#define  PANEL_COLOR_SWITCH               31      /* control type: binary, callback function: (none) */
#define  PANEL_DIR_SWITCH                 32      /* control type: binary, callback function: (none) */
#define  PANEL_NUM_SELE                   33      /* control type: ring, callback function: (none) */
#define  PANEL_GRAPH_SETTING              34      /* control type: command, callback function: LedGraphSet */
#define  PANEL_DECORATION_5               35      /* control type: deco, callback function: (none) */
#define  PANEL_TEXTMSG_2                  36      /* control type: textMsg, callback function: (none) */
#define  PANEL_DECORATION_8               37      /* control type: deco, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK LedGraphSet(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PortClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PortEnable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallBack(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Traffic1_Clear(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Traffic1PortEnablde(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Traffic2_Clear(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Traffic2PortEnablde(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TrafficTimer1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TrafficTimer2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ValveCtrl_1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ValveCtrl_2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
