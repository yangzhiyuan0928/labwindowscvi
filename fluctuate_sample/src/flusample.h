/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
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
#define  PANEL_TAB                        2       /* control type: tab, callback function: (none) */
#define  PANEL_TCP_LED                    3       /* control type: LED, callback function: (none) */
#define  PANEL_DATA_LED                   4       /* control type: LED, callback function: (none) */
#define  PANEL_UNREGISTER_SERVER          5       /* control type: command, callback function: UnregisterServer */
#define  PANEL_TCP_CONFIG                 6       /* control type: command, callback function: Tcp_Config */

     /* tab page panel controls */
#define  TABPANEL1_PROGRESS               2       /* control type: scale, callback function: (none) */
#define  TABPANEL1_SWITCH_TIME            3       /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DOWNLOAD               4       /* control type: command, callback function: DownLoad */
#define  TABPANEL1_SWITCH_TURN            5       /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DELAY_TIME             6       /* control type: numeric, callback function: (none) */
#define  TABPANEL1_TURN_TIME              7       /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DECORATION_3           8       /* control type: deco, callback function: (none) */
#define  TABPANEL1_TEXTMSG_9              9       /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG_10             10      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG_8              11      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_WAVE_LEN               12      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_SAMPLE_RATE            13      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_AVERAGE_NUM            14      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_SAMPLE_LEN             15      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_FCLK_DAC               16      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DECORATION_2           17      /* control type: deco, callback function: (none) */
#define  TABPANEL1_TEXTMSG_6              18      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG                19      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG_3              20      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG_5              21      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_DECORATION             22      /* control type: deco, callback function: (none) */
#define  TABPANEL1_TEXTMSG_7              23      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_TEXTMSG_2              24      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_PARAM_SET              25      /* control type: command, callback function: SetParameter */
#define  TABPANEL1_PARAM_CONFIG_LED       26      /* control type: LED, callback function: (none) */
#define  TABPANEL1_DATA_DIR               27      /* control type: string, callback function: (none) */
#define  TABPANEL1_PARAM_DIR              28      /* control type: string, callback function: (none) */
#define  TABPANEL1_PARAM_BROWSER          29      /* control type: command, callback function: ParamFileBrowser */
#define  TABPANEL1_DELETE_FILE            30      /* control type: command, callback function: Delete_File */
#define  TABPANEL1_SELFCHECK              31      /* control type: command, callback function: SelfCheck */
#define  TABPANEL1_SWITCH_TABLE           32      /* control type: string, callback function: (none) */
#define  TABPANEL1_TAB_BROWSER            33      /* control type: command, callback function: Table_Config */
#define  TABPANEL1_WAVEDATA               34      /* control type: string, callback function: (none) */
#define  TABPANEL1_TRIG_MODE              35      /* control type: binary, callback function: (none) */
#define  TABPANEL1_WAVEBROSWER            36      /* control type: command, callback function: Wave_Config */
#define  TABPANEL1_DIVIDE2_3              37      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DIVIDE0_1              38      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_PLL2_N                 39      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_PLL2_N_PRE             40      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_PLL2_R                 41      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_PLL1_N                 42      /* control type: numeric, callback function: (none) */
#define  TABPANEL1_DECORATION_5           43      /* control type: deco, callback function: (none) */
#define  TABPANEL1_DECORATION_6           44      /* control type: deco, callback function: (none) */
#define  TABPANEL1_TEXTMSG_4              45      /* control type: textMsg, callback function: (none) */
#define  TABPANEL1_DECORATION_4           46      /* control type: deco, callback function: (none) */
#define  TABPANEL1_DATA_BROSWER           47      /* control type: command, callback function: SampleDataBoswer */
#define  TABPANEL1_TEXTMSG_11             48      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK Delete_File(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DownLoad(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ParamFileBrowser(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SampleDataBoswer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelfCheck(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetParameter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Table_Config(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Tcp_Config(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UnregisterServer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Wave_Config(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
