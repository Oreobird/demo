//

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.
//#include "bspconfig.h"
#include "led-blink.h"
//#include "wstk-common.h"
#include "app/framework/include/af.h"
//#include "app/framework/util/config.h"
#include "hal/micro/cortexm3/flash.h"
#include "app/framework/plugin-soc/connection-manager/connection-manager.h"
#include EMBER_AF_API_NETWORK_STEERING
#include "app/framework/util/attribute-storage.h"

#define MODEL_ID "EverGrande.switch.004"
#define ENDPOINT_ONE	1
#define ENDPOINT_TWO	2
#define ENDPOINT_THREE	3
#define ENDPOINT_FOUR	4
#define DESTINATIONENDPOINT 1
#define RELAY_NUM   3

#define  MILLISECOND_1000_DELAY    1000
#define  MILLISECOND_500_DELAY     500
#define  MILLISECOND_250_DELAY     250
#define  MILLISECOND_125_DELAY     125
#define  ZERO_SECOND_DELAY         0

#define HEARTBEAT_INTERVAL      (MILLISECOND_1000_DELAY * 3)
#define RELAY_DELAY_TIME        (MILLISECOND_1000_DELAY * 100)
#define DEFAULT_DEVICE 0
#define DISCONNECTION_COUNT   3


// Event function forward declarations
void LedEventHandler(void);
void ButtonEventHandler(void);
void HeartBeatEventHandler(void);
void NoNetworkEventIndicatorHandler(void);
void JoinedNetworkIndicatorHandler(void);
void CheckNetworkStatusEventHandler(void);


void halCustomLedWait( uint16_t timeMs,  uint8_t led);
void halCustomLedBlinkLedOn(uint16_t timeMs, uint8_t led);
void halCustomLedBlinkLedOff(uint16_t timeMs, uint8_t led);
void halCustomLedBlinkBlink(uint16_t count,
                           uint16_t blinkTimeMs,
                           uint8_t led);

// Custom event stubs. Custom events will be run along with all other events in
// the application framework. They should be managed using the Ember Event API
// documented in stack/include/events.h

const char appVersion[] = "BH-SZ100-app-flash-v0.3.3-20171113";


// Event control struct declarations
EmberEventControl LedEvent;
EmberEventControl ButtonEvent;
EmberEventControl ReBootEvent;
EmberEventControl HeartBeatEvent;
EmberEventControl NoNetworkEventIndicator;
EmberEventControl JoinedNetworkIndicator;
EmberEventControl CheckNetworkStatusEvent;
EmberEventControl Relay1Event,
EmberEventControl Relay2Event,
EmberEventControl Relay3Event,

// Plugin private types and enums
typedef enum {
    LED_ON            = 0x00,
    LED_OFF           = 0x01,
    LED_BLINKING_ON   = 0x02,
    LED_BLINKING_OFF  = 0x03,
    LED_BLINK_PATTERN = 0x04,
    LED_WAIT          = 0x05,
    LED_IDLE          = 0x06,

}gpioledBlinkState;

typedef enum{
    PHASE1     = 0x00,
    PHASE2     = 0x01,
    PHASE3     = 0x02,
    PHASE4     = 0x03

}ledindicatorphase;

typedef enum {
    SZ101   = 0x04,
    SZ102   = 0x05,
    SZ103   = 0x06,
    SZ201   = 0x00,
    SZ202   = 0x01,
    SZ203   = 0x02,
    SZ301   = 0x08,
    SZ302   = 0x09,
    SZ303   = 0x0a,
} HardwareIDType;

typedef void (*relay_handle_fn)(bool on);

typedef struct btn_ep_s
{
    unsigned char btn_pin;
    unsigned char endpoint;
} btn_ep_t;

typedef struct switch_s
{
    HardwareIDType type;
    bool auto_report_enable;
    unsigned char relay_count;
    char modelID[10];
    relay_handle_fn relay_fn[RELAY_NUM];
    btn_ep_t btn_ep_map[RELAY_NUM];
} switch_handle_tbl_t;

bool g_is_disconnected = false;
static signed char lastButton;
static   uint16_t LastTick = 0;
static   int16u sequence_number = 0;
static   gpioledBlinkState ledEventState = LED_ON;
static   uint8_t ledBlinkCount = 0;
static   uint16_t ledBlinkTimeMs = 0;
static   uint16_t ledWaitTimeMs  = 0;
static   ledindicatorphase indicatorphase = PHASE1;
bool      newStatus[5] = {false, false, false, false, false};
#ifdef HD_SWITCH
st_boot_param boot_data;
uint8_t g_hardware_id = 0;
static uint8_t g_hw_id_index = DEFAULT_DEVICE;
#endif

// *****************************************************************************
// Functions
void emberHardwareIDInit(void *data)
{
    uint8_t *hardware_id = (uint8_t *)data;

    GPIO_PinModeSet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN1, gpioModeInputPull, 1);
    GPIO_PinModeSet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN0, gpioModeInputPull, 1);
    GPIO_PinModeSet(BSP_GPIO_KEY_TYPE_PORT, BSP_GPIO_KEY_TYPE_PIN1, gpioModeInputPull, 1);
    GPIO_PinModeSet(BSP_GPIO_KEY_TYPE_PORT, BSP_GPIO_KEY_TYPE_PIN0, gpioModeInputPull, 1);

    *hardware_id = (GPIO_PinInGet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN1) << 3
                | GPIO_PinInGet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN0) << 2
                | GPIO_PinInGet(BSP_GPIO_KEY_TYPE_PORT, BSP_GPIO_KEY_TYPE_PIN1) << 1
                | GPIO_PinInGet(BSP_GPIO_KEY_TYPE_PORT, BSP_GPIO_KEY_TYPE_PIN0)) & 0x0F;

    emberAfCorePrintln("pro high:%d, pro low:%d, hardware_id:%d",
        GPIO_PinInGet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN1),
        GPIO_PinInGet(BSP_GPIO_PRO_TYPE_PORT, BSP_GPIO_PRO_TYPE_PIN0),
        *hardware_id);
    #ifdef TEST_IN_OLD
    *hardware_id = 0x06;
    #endif
}

void Relay1EventHandler(void)
{
    emberEventControlSetInactive(Relay1Event);
    if (newStatus[ENDPOINT_ONE])
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL1_PORT, BSP_GPIO_SZ1_RELAY_CTRL1_PIN);
    else
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL1N_PORT, BSP_GPIO_SZ1_RELAY_CTRL1N_PIN);
}

void Relay2EventHandler(void)
{
    emberEventControlSetInactive(Relay2Event);
    if (newStatus[ENDPOINT_TWO])
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL2_PORT, BSP_GPIO_SZ1_RELAY_CTRL2_PIN);
    else
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL2N_PORT, BSP_GPIO_SZ1_RELAY_CTRL2N_PIN);
}

void Relay3EventHandler(void)
{
    emberEventControlSetInactive(Relay3Event);
    if (newStatus[ENDPOINT_THREE])
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL3_PORT, BSP_GPIO_SZ1_RELAY_CTRL3_PIN);
    else
        GPIO_PinOutClear(BSP_GPIO_SZ1_RELAY_CTRL3N_PORT, BSP_GPIO_SZ1_RELAY_CTRL3N_PIN);
}


static void RelayDelayMs(EmberEventControl relay_event)
{
    //halCommonDelayMicroseconds(30000); //datasheet suggests >= 30 ms
    emberEventControlSetDelayMS(relay_event, RELAY_DELAY_TIME);
}

static void RelayModeOut(GPIO_Port_TypeDef ctrl_port, int ctrl_pin,
                        GPIO_Port_TypeDef ctrl_n_port, int ctrl_n_pin)
{
    GPIO_PinModeSet(ctrl_port, ctrl_pin, gpioModePushPull, 0);
    GPIO_PinModeSet(ctrl_n_port, ctrl_n_pin, gpioModePushPull, 0);
}

static void RelayOn(EmberEventControl relay_event,
                    GPIO_Port_TypeDef ctrl_port, int ctrl_pin,
                    GPIO_Port_TypeDef ctrl_n_port, int ctrl_n_pin)
{
    GPIO_PinOutClear(ctrl_n_port, ctrl_n_pin);
    GPIO_PinOutSet(ctrl_port, ctrl_pin);
    RelayDelayMs(relay_event);
    //GPIO_PinOutClear(ctrl_port, ctrl_pin);
}

static void RelayOff(EmberEventControl relay_event,
                    GPIO_Port_TypeDef ctrl_port, int ctrl_pin,
                    GPIO_Port_TypeDef ctrl_n_port, int ctrl_n_pin)
{
    GPIO_PinOutClear(ctrl_port, ctrl_pin);
    GPIO_PinOutSet(ctrl_n_port, ctrl_n_pin);
    RelayDelayMs(relay_event);
    //GPIO_PinOutClear(ctrl_n_port, ctrl_n_pin);
}

void SZ1Relay1Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ1_RELAY_CTRL1_PORT, BSP_GPIO_SZ1_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL1N_PORT, BSP_GPIO_SZ1_RELAY_CTRL1N_PIN);
    if (on)
    {
        RelayOn(Relay1Event,
                BSP_GPIO_SZ1_RELAY_CTRL1_PORT, BSP_GPIO_SZ1_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL1N_PORT, BSP_GPIO_SZ1_RELAY_CTRL1N_PIN);
    }
    else
    {
        RelayOff(Relay1Event,
                BSP_GPIO_SZ1_RELAY_CTRL1_PORT, BSP_GPIO_SZ1_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL1N_PORT, BSP_GPIO_SZ1_RELAY_CTRL1N_PIN);
    }
}

void SZ1Relay2Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ1_RELAY_CTRL2_PORT, BSP_GPIO_SZ1_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL2N_PORT, BSP_GPIO_SZ1_RELAY_CTRL2N_PIN);
    if (on)
    {
        RelayOn(Relay2Event,
                BSP_GPIO_SZ1_RELAY_CTRL2_PORT, BSP_GPIO_SZ1_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL2N_PORT, BSP_GPIO_SZ1_RELAY_CTRL2N_PIN);
    }
    else
    {
        RelayOff(Relay2Event,
                BSP_GPIO_SZ1_RELAY_CTRL2_PORT, BSP_GPIO_SZ1_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL2N_PORT, BSP_GPIO_SZ1_RELAY_CTRL2N_PIN);
    }
}

void SZ1Relay3Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ1_RELAY_CTRL3_PORT, BSP_GPIO_SZ1_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL3N_PORT, BSP_GPIO_SZ1_RELAY_CTRL3N_PIN);
    if (on)
    {
        RelayOn(Relay3Event,
                BSP_GPIO_SZ1_RELAY_CTRL3_PORT, BSP_GPIO_SZ1_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL3N_PORT, BSP_GPIO_SZ1_RELAY_CTRL3N_PIN);
    }
    else
    {
        RelayOff(Relay3Event,
                BSP_GPIO_SZ1_RELAY_CTRL3_PORT, BSP_GPIO_SZ1_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ1_RELAY_CTRL3N_PORT, BSP_GPIO_SZ1_RELAY_CTRL3N_PIN);
    }
}

void SZ2Relay1Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ2_RELAY_CTRL1_PORT,BSP_GPIO_SZ2_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL1N_PORT,BSP_GPIO_SZ2_RELAY_CTRL1N_PIN);
    if (on)
    {
        RelayOn(Relay1Event,
                BSP_GPIO_SZ2_RELAY_CTRL1_PORT, BSP_GPIO_SZ2_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL1N_PORT, BSP_GPIO_SZ2_RELAY_CTRL1N_PIN);
    }
    else
    {
        RelayOff(Relay1Event,
                BSP_GPIO_SZ2_RELAY_CTRL1_PORT, BSP_GPIO_SZ2_RELAY_CTRL1_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL1N_PORT, BSP_GPIO_SZ2_RELAY_CTRL1N_PIN);
    }
}

void SZ2Relay2Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ2_RELAY_CTRL2_PORT, BSP_GPIO_SZ2_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL2N_PORT, BSP_GPIO_SZ2_RELAY_CTRL2N_PIN);
    if (on)
    {
        RelayOn(Relay2Event,
                BSP_GPIO_SZ2_RELAY_CTRL2_PORT, BSP_GPIO_SZ2_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL2N_PORT, BSP_GPIO_SZ2_RELAY_CTRL2N_PIN);
    }
    else
    {
        RelayOff(Relay2Event,
                BSP_GPIO_SZ2_RELAY_CTRL2_PORT, BSP_GPIO_SZ2_RELAY_CTRL2_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL2N_PORT, BSP_GPIO_SZ2_RELAY_CTRL2N_PIN);
    }
}

void SZ2Relay3Ctrl(bool on)
{
    RelayModeOut(BSP_GPIO_SZ2_RELAY_CTRL3_PORT, BSP_GPIO_SZ2_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL3N_PORT, BSP_GPIO_SZ2_RELAY_CTRL3N_PIN);
    if (on)
    {
        RelayOn(Relay3Event,
                BSP_GPIO_SZ2_RELAY_CTRL3_PORT, BSP_GPIO_SZ2_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL3N_PORT, BSP_GPIO_SZ2_RELAY_CTRL3N_PIN);
    }
    else
    {
        RelayOff(Relay3Event,
                BSP_GPIO_SZ2_RELAY_CTRL3_PORT, BSP_GPIO_SZ2_RELAY_CTRL3_PIN,
                BSP_GPIO_SZ2_RELAY_CTRL3N_PORT, BSP_GPIO_SZ2_RELAY_CTRL3N_PIN);
    }
}

switch_handle_tbl_t g_switch_handle_tbl[] =
{
    {SZ101, true, 1, {'B','H','-','S','Z','1','0','1',0,0}, {SZ1Relay1Ctrl, NULL, NULL}, {{BUTTON1, ENDPOINT_ONE}}},
    {SZ102, true, 2, {'B','H','-','S','Z','1','0','2',0,0}, {SZ1Relay1Ctrl, SZ1Relay2Ctrl, NULL}, {{BUTTON0, ENDPOINT_ONE},{BUTTON2, ENDPOINT_TWO}}},
    {SZ103, true, 3, {'B','H','-','S','Z','1','0','3',0,0}, {SZ1Relay1Ctrl, SZ1Relay2Ctrl, SZ1Relay3Ctrl}, {{BUTTON0, ENDPOINT_ONE},{BUTTON1, ENDPOINT_TWO},{BUTTON2, ENDPOINT_THREE}}},

    {SZ201, true, 1, {'B','H','-','S','Z','2','0','1',0,0}, {SZ2Relay1Ctrl, NULL, NULL}, {{BUTTON1, ENDPOINT_ONE}}},
    {SZ202, true, 2, {'B','H','-','S','Z','2','0','2',0,0}, {SZ2Relay1Ctrl, SZ2Relay2Ctrl, NULL}, {{BUTTON0, ENDPOINT_ONE}, {BUTTON2, ENDPOINT_TWO}}},
    {SZ203, true, 3, {'B','H','-','S','Z','2','0','3',0,0}, {SZ2Relay1Ctrl, SZ2Relay2Ctrl, SZ2Relay3Ctrl}, {{BUTTON0, ENDPOINT_ONE}, {BUTTON1, ENDPOINT_TWO}, {BUTTON2, ENDPOINT_THREE}}},

    {SZ301, false, 1, {'B','H','-','S','Z','3','0','1',0,0}, {NULL, NULL, NULL}, {{BUTTON1, ENDPOINT_ONE}}},
    {SZ302, false, 2, {'B','H','-','S','Z','3','0','2',0,0}, {NULL, NULL, NULL}, {{BUTTON0, ENDPOINT_ONE}, {BUTTON2, ENDPOINT_TWO}}},
    {SZ303, false, 3, {'B','H','-','S','Z','3','0','3',0,0}, {NULL, NULL, NULL}, {{BUTTON0, ENDPOINT_ONE}, {BUTTON1, ENDPOINT_TWO}, {BUTTON2, ENDPOINT_THREE}}}
};

#define SWITCH_MAX_RELAY  (g_switch_handle_tbl[g_hw_id_index].relay_count)
#define SWITCH_MODEL_ID     (g_switch_handle_tbl[g_hw_id_index].modelID)
#define SWITCH_TOGGLE(num, status) \
    if (g_switch_handle_tbl[g_hw_id_index].relay_fn[num]) \
        (g_switch_handle_tbl[g_hw_id_index].relay_fn[num])(status)

int HwIndexInit(void)
{
    int i = 0;
    int ret = -1;

    for (i = 0; i < COUNTOF(g_switch_handle_tbl); i++)
    {
        if (g_switch_handle_tbl[i].type == g_hardware_id)
        {
            g_hw_id_index = i;
            ret = 0;
            break;
        }
    }

    return ret;
}

bool switch_auto_report_enable(void)
{
    return g_switch_handle_tbl[g_hw_id_index].auto_report_enable;
}

void ReportOnOffToCoordinator(uint8_t endpoint, bool newStatus)
{
    static int16u sequence_number = 0;
    EmberApsFrame ApsFrame;
    int8u ZclBuffer[EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH]={0};
    int16u ZclBufferLen;
    int8u index = 0;

    ApsFrame.sourceEndpoint = endpoint;
    ApsFrame.destinationEndpoint = DESTINATIONENDPOINT;
    ApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
    ApsFrame.clusterId = ZCL_ON_OFF_CLUSTER_ID;
    ApsFrame.profileId = HA_PROFILE_ID;

    ZclBuffer[index++] = 0x18;    //do not need the coordiantor response
    ZclBuffer[index++] = LOW_BYTE(sequence_number);
    ZclBuffer[index++] = 0x0A;
    ZclBuffer[index++] = LOW_BYTE(ZCL_ON_OFF_ATTRIBUTE_ID);
    ZclBuffer[index++] = HIGH_BYTE(ZCL_ON_OFF_ATTRIBUTE_ID);
    ZclBuffer[index++] = ZCL_BOOLEAN_ATTRIBUTE_TYPE;

    //ZclBuffer[index++] = sizeof(bool);
    ZclBuffer[index++] = newStatus;
    ZclBufferLen = index;
    emberAfCorePrintBuffer(ZclBuffer, ZclBufferLen, 1);

    emberAfSendUnicast(EMBER_OUTGOING_DIRECT, 0x0000, &ApsFrame, ZclBufferLen, ZclBuffer);
    sequence_number++;
}

/********************************************************************
Author	: Carl
CopyRight: EverGrande
Function: Report ModelID of device to the Coordinator
Input	:
Output	:
Return	:
Notes	:
Release	: Date         Author      Notes
         2017-6-13 09:06  Carl     Create
********************************************************************/
void ReportModelIDToCoordinator(int8u *ModelID)
{
    EmberApsFrame ApsFrame;
    int8u ZclBuffer[EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH]={0};
    int16u ZclBufferLen;
    int8u index = 0;

    ApsFrame.sourceEndpoint = ENDPOINT_ONE;
    ApsFrame.destinationEndpoint = DESTINATIONENDPOINT;
    ApsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS & (~EMBER_APS_OPTION_RETRY);
    ApsFrame.clusterId = ZCL_BASIC_CLUSTER_ID;
    ApsFrame.profileId = HA_PROFILE_ID;

    ZclBuffer[index++] = 0x18; //do not need the coordiantor response
    ZclBuffer[index++] = LOW_BYTE(sequence_number);
    ZclBuffer[index++] = 0x0A;
    ZclBuffer[index++] = LOW_BYTE(ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID);
    ZclBuffer[index++] = HIGH_BYTE(ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID);
    ZclBuffer[index++] = ZCL_CHAR_STRING_ATTRIBUTE_TYPE;

    ZclBuffer[index++] = strlen(ModelID);

    halCommonMemMove((int8u*)&ZclBuffer[index], ModelID, strlen(ModelID));

    ZclBufferLen = index+strlen(ModelID);

    emberAfCorePrintBuffer(ZclBuffer, ZclBufferLen, 1);

    emberAfSendUnicast(EMBER_OUTGOING_DIRECT, 0x0000, &ApsFrame, ZclBufferLen, ZclBuffer);
    sequence_number++;
}

void ReBootEventHandler (void)
{
    static uint8_t phase = 0;
    if(phase == 0)
    {
        phase = 1;
        emberEventControlSetDelayMS(ReBootEvent, MILLISECOND_1000_DELAY);
    }
    else
    {
        phase = 0;
        emberEventControlSetInactive(ReBootEvent);
        halReboot();
    }

}

/********************************************************************
Author	: Deane
CopyRight: EverGrande
Function: check the network status function
Input	:
Output	:
Return	:
Notes	:
Release	: Date            Author      Notes
         2017-8-8   14:37  Deane     Create
********************************************************************/
void CheckNetworkStatusEventHandler( void )
{
    static uint16_t count = 0;

    if( emberAfNetworkState() == EMBER_JOINING_NETWORK )
    {
        halToggleLed(BOARDLED0);
        emberEventControlSetDelayMS(CheckNetworkStatusEvent,
                                MILLISECOND_125_DELAY);
    }
    if( emberAfNetworkState() == EMBER_JOINED_NETWORK )
    {
        halCustomLedBlinkLedOn(MILLISECOND_1000_DELAY, 0);
        emberEventControlSetInactive(CheckNetworkStatusEvent);
    }
    //
    if( emberAfNetworkState() == EMBER_NO_NETWORK )
    {
        halToggleLed(BOARDLED0);
        emberEventControlSetDelayMS(CheckNetworkStatusEvent,
                                MILLISECOND_125_DELAY);
        count++;
    }

    if ( count == 300 * 8 )// one second composed by 8*125ms, try to find network 300s(5 minutes)
    {
        count = 0;
        halCustomLedBlinkBlink(10, MILLISECOND_500_DELAY, 0);
        emberEventControlSetInactive(CheckNetworkStatusEvent);
    }

}

/********************************************************************
Author	: Deane
CopyRight: EverGrande
Function: Handle LED behavior, when no_network status happened
Input	:
Output	:
Return	:
Notes	:
Release	: Date            Author      Notes
         2017-8-8   14:37  Deane     Create
********************************************************************/
void NoNetworkEventIndicatorHandler(void)
{
    switch( indicatorphase )
    {
    case PHASE1:
        halCustomLedBlinkBlink(1, MILLISECOND_125_DELAY, 0);
        indicatorphase = PHASE2;
        // add 5 more millisecond to avoid overlap
        emberEventControlSetDelayMS(NoNetworkEventIndicator,
                                 (MILLISECOND_250_DELAY+5));
        break;
    case PHASE2:
        halClearLed(BOARDLED0);
        indicatorphase = PHASE3;
        emberEventControlSetDelayMS(NoNetworkEventIndicator,
                                    MILLISECOND_1000_DELAY);
        break;
    case PHASE3:
        indicatorphase = PHASE4;
        halCustomLedBlinkBlink(4, MILLISECOND_125_DELAY, 0);
        emberEventControlSetDelayMS(NoNetworkEventIndicator,
                              (MILLISECOND_250_DELAY*4+20));
        break;
    case PHASE4:
        indicatorphase = PHASE1;
        halClearLed(BOARDLED0);
        emberEventControlSetInactive(NoNetworkEventIndicator);
        //emberAfPluginConnectionManagerResetJoinAttempts();
        //emberAfPluginConnectionManagerStartSearchForJoinableNetwork();
        //emberEventControlSetActive(CheckNetworkStatusEvent);
        break;
    default:
        break;

    }
}

/********************************************************************
Author	: Deane
CopyRight: EverGrande
Function: Handle LED behavior, when the network is connected will
Input	:
Output	:
Return	:
Notes	:
Release	: Date            Author      Notes
         2017-8-8   14:37  Deane     Create
********************************************************************/
void JoinedNetworkIndicatorHandler(void)
{
    switch( indicatorphase )
    {
    case PHASE1:
        halCustomLedBlinkBlink(1, MILLISECOND_125_DELAY, 0);
        indicatorphase = PHASE2;
        // add 5 more millisecond to avoid overlap
        emberEventControlSetDelayMS(JoinedNetworkIndicator,
                                 (MILLISECOND_250_DELAY+5));
        break;
    case PHASE2:
        halClearLed(BOARDLED0);
        indicatorphase = PHASE3;
        emberEventControlSetDelayMS(JoinedNetworkIndicator,
                                    MILLISECOND_1000_DELAY);
        break;
    case PHASE3:
        indicatorphase = PHASE4;
        halSetLed(BOARDLED0);

        emberEventControlSetDelayMS(JoinedNetworkIndicator,
                              MILLISECOND_1000_DELAY);
        break;
    case PHASE4:
        indicatorphase = PHASE1;
        halClearLed(BOARDLED0);
        emberEventControlSetInactive(JoinedNetworkIndicator);
        break;
    default:
        break;

    }
}
/********************************************************************
Author	: Deane
CopyRight: EverGrande
Function: Handle the LED behavior
Input	:
Output	:
Return	:
Notes	:
Release	: Date            Author      Notes
         2017-8-8   14:37  Deane     Create
********************************************************************/

void LedEventHandler(void)
{
     switch (ledEventState)
     {
     case LED_ON:
         halClearLed(BOARDLED0);
         emberEventControlSetInactive(LedEvent);
         break;

     case LED_OFF:
         halSetLed(BOARDLED0);
         emberEventControlSetInactive(LedEvent);
         break;

     case LED_BLINKING_ON:
          halClearLed(BOARDLED0);
          if (ledBlinkCount> 0)
          {
              //if (ledBlinkCount != 255)
              //{ // blink forever if count is 255
                  ledBlinkCount --;
              //}
              if (ledBlinkCount > 0)
              {
                  ledEventState = LED_BLINKING_OFF;
                  emberEventControlSetDelayMS( LedEvent,
                                            ledBlinkTimeMs );
              }
              else
              {
                  ledEventState = LED_OFF;
                  emberEventControlSetInactive( LedEvent );
              }
          }
          else
          {
              ledEventState = LED_BLINKING_OFF;
              emberEventControlSetDelayMS(LedEvent,
                                          ledBlinkTimeMs);
          }
         break;

     case LED_BLINKING_OFF:
         halSetLed(BOARDLED0);
         ledEventState = LED_BLINKING_ON;
         emberEventControlSetDelayMS(LedEvent,
                                ledBlinkTimeMs);
         break;

     case LED_WAIT:
         ledEventState = LED_IDLE;
         emberEventControlSetDelayMS(LedEvent,
                                ledWaitTimeMs);
         break;

     case LED_IDLE:
         emberEventControlSetInactive( LedEvent );
         break;


     case LED_BLINK_PATTERN:
       /*
         if (ledBlinkCount == 0)
         {
             turnLedOff(activeLed[ledIndex]);
             ledEventState = LED_OFF;
             emberEventControlSetInactive(LedEvent);
             break;
         }
         if (blinkPatternIndex[ledIndex] %2 == 1)
         {
             turnLedOff(activeLed[ledIndex]);
         }
         else
         {
             turnLedOn(activeLed[ledIndex]);
         }
         emberEventControlSetDelayMS(*(ledEventArray[ledIndex]),
          blinkPattern[ledIndex][blinkPatternIndex[ledIndex]]);
         blinkPatternIndex[ledIndex] ++;
         if (blinkPatternIndex[ledIndex] >= blinkPatternLength[ledIndex])
         {
             blinkPatternIndex[ledIndex] = 0;
             if (ledBlinkCount[ledIndex] != 255)
             { // blink forever if count is 255
                 ledBlinkCount[ledIndex] --;
             }
         }
        */
         break;

     default:
         break;
     }

}

void halCustomLedWait( uint16_t timeMs,  uint8_t led)
{

    ledEventState = LED_WAIT;

    emberEventControlSetActive(LedEvent);

    if ( timeMs > 0 )
    {
          ledWaitTimeMs = timeMs;
    }
    else
    {

    }

}

void halCustomLedBlinkLedOn(uint16_t timeMs, uint8_t led)
{

    halSetLed(BOARDLED0);

    ledEventState = LED_ON;

    if (timeMs > 0)
    {
        emberEventControlSetDelayMS( LedEvent ,
                                ((uint16_t) timeMs) );
    }
    else
    {
        emberEventControlSetInactive(LedEvent);
    }
}

void halCustomLedBlinkLedOff(uint16_t timeMs, uint8_t led)
{

    halClearLed(BOARDLED0);

    ledEventState = LED_OFF;

    if (timeMs > 0)
    {
        emberEventControlSetDelayMS( LedEvent,
                                ((uint16_t) timeMs) );
    }
    else
    {
        emberEventControlSetInactive(LedEvent);
    }
}

void halCustomLedBlinkBlink(uint16_t count,
                           uint16_t blinkTimeMs,
                           uint8_t led)
{

    ledBlinkTimeMs = blinkTimeMs;

    halClearLed(BOARDLED0);

    ledEventState = LED_BLINKING_OFF;

    ledBlinkCount = count;

    //emberEventControlSetDelayMS( LedEvent,
    //                            ledBlinkTimeMs );

    emberEventControlSetActive(LedEvent);

}



/********************************************************************
Author	: Carl
CopyRight: EverGrande
Function: Hander the button event which was pessed
Input	:
Output	:
Return	:
Notes	:
Release	: Date         Author      Notes
         2017-6-13 09:06  Carl     Create
********************************************************************/
void ButtonEventHandler(void)
{
    uint16_t CurTick;

    emberEventControlSetInactive(ButtonEvent);
    emberAfCorePrintln("Button Event Hander exe!");

    if(lastButton == BUTTON4)
    {

        CurTick = halCommonGetInt16uQuarterSecondTick();

        emberAfAppPrintln("CurTick=%d, LastTick=%d", CurTick, LastTick);
        emberAfAppPrintln("%s",appVersion);
        if( (CurTick-LastTick)>=20 )//press more than 5s leave the network
        {
              if( emberAfNetworkState() == EMBER_JOINED_NETWORK )
              {
                  emberClearBindingTable();
                  //clearNetworkTables();
                  emberAfClearReportTableCallback();
                  emberLeaveNetwork();
              }

              boot_data.factory_reset = 1;
              halCommonSetToken(TOKEN_BOOT_PARAM, &boot_data);
              emberEventControlSetActive(ReBootEvent);
        }
        else
        {
            if((emberAfNetworkState() == EMBER_JOINED_NETWORK)&&
               ( !g_is_disconnected ))
            {
                 indicatorphase = PHASE1;
                 emberEventControlSetActive( JoinedNetworkIndicator );
            }

            if((emberAfNetworkState() == EMBER_JOINED_NETWORK) && (g_is_disconnected))
            {
                indicatorphase = PHASE1;
                emberEventControlSetActive( NoNetworkEventIndicator );
            }

            if((emberAfNetworkState() == EMBER_NO_NETWORK) && (!g_is_disconnected))
            {
                halClearLed(BOARDLED0);
                emberEventControlSetInactive(NoNetworkEventIndicator);
                emberAfPluginConnectionManagerResetJoinAttempts();
                emberAfPluginConnectionManagerStartSearchForJoinableNetwork();
                emberEventControlSetActive(CheckNetworkStatusEvent);
            }

            LastTick = 0;

         }
    }
    else
    {
        int i = 0;
        for (i = 0; i < RELAY_NUM; i++)
        {
            if (g_switch_handle_tbl[g_hw_id_index].btn_ep_map[i].btn_pin == lastButton)
            {
                emberAfCorePrintln("\nBUTTON%d IS PRESSED!\n", (i+1));
                unsigned char ep_num = g_switch_handle_tbl[g_hw_id_index].btn_ep_map[i].endpoint;
                newStatus[ep_num] = newStatus[ep_num] ? false : true;
                emberAfWriteServerAttribute(ep_num, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                            (uint8_t*)&(newStatus[ep_num]),ZCL_BOOLEAN_ATTRIBUTE_TYPE);
                break;
            }
        }
    }
}


// Event function stubs
/********************************************************************
Author	: Carl
CopyRight: EverGrande
Function: report on/off Attr to the Coordinator
Input	:
Output	:
Return	:
Notes	:
Release	: Date         Author      Notes
         2017-1-4 09:06  Carl     Create
********************************************************************/
void HeartBeatEventHandler(void)
{
    #if 0
    emberEventControlSetInactive(HeartBeatEvent);
    if(emberAfNetworkState() == EMBER_JOINED_NETWORK)
    {
        emberAfAppPrintln("--->> HeartBeat! <<---");
        ReportModelIDToCoordinator(SWITCH_MODEL_ID);
    }
    emberEventControlSetDelayMinutes(HeartBeatEvent,15);
    #endif

    emberEventControlSetInactive(HeartBeatEvent);
    if (g_is_disconnected && emberAfNetworkState() == EMBER_JOINED_NETWORK)
    {
        emberAfAppPrintln("--->> Report to coordinator for checking connection <<---");
        ReportModelIDToCoordinator(SWITCH_MODEL_ID);
        //ReportOnOffToCoordinator(ENDPOINT_ONE, newStatus[ENDPOINT_ONE]);
        emberEventControlSetDelayMS(HeartBeatEvent, HEARTBEAT_INTERVAL);
    }
}


/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
void emberAfHalButtonIsrCallback(int8u button,
                                 int8u state)
{
    if (state == BUTTON_RELEASED)
    {
        //emberAfCorePrintln("Detect a button pess,button=%02X",button);
        lastButton = button;
        emberEventControlSetActive(ButtonEvent);
    }
    else
    {
        if( button == BUTTON4 )
        {
            LastTick = halCommonGetInt16uQuarterSecondTick();
        }
    }
}

/** @brief On/off Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
    if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        bool onOff;
        if (emberAfReadServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                       (uint8_t *)&onOff, sizeof(onOff)) == EMBER_ZCL_STATUS_SUCCESS)
        {
            //emberAfAppPrintln("This is endpoint %d!", endpoint);
            if (endpoint > RELAY_NUM)
            {
                return;
            }

            int relay_fn_index = endpoint - 1;
            newStatus[endpoint] = onOff;

            if (relay_fn_index >= 0)
            {
                SWITCH_TOGGLE(relay_fn_index, onOff);
            }

            if(onOff)
            {
                SETBIT(boot_data.onoff_state, relay_fn_index);
            }
            else
            {
                CLEARBIT(boot_data.onoff_state, relay_fn_index);
            }
            emberAfCorePrintln("onoff_state:%d", boot_data.onoff_state);
            halCommonSetToken(TOKEN_BOOT_PARAM, &boot_data);
        }
        //ReportOnOffToCoordinator(endpoint, newStatus[endpoint]);
    }
}

/** @brief Main Start
 *
 * This function is called at the start of main after the HAL has been
 * initialized.  The standard main function arguments of argc and argv are
 * passed in.  However not all platforms have support for main() function
 * arguments.  Those that do not are passed NULL for argv, therefore argv should
 * be checked for NULL before using it.  If the callback determines that the
 * program must exit, it should return true.  The value returned by main() will
 * be the value written to the returnCode pointer.  Otherwise the callback
 * should return false to let normal execution continue.
 *
 * @param returnCode   Ver.: always
 * @param argc   Ver.: always
 * @param argv   Ver.: always
 */
boolean emberAfMainStartCallback(int* returnCode,
                                 int argc,
                                 char** argv)
{
  // NOTE:  argc and argv may not be supported on all platforms, so argv MUST be
  // checked for NULL before referencing it.  On those platforms without argc
  // and argv "0" and "NULL" are passed respectively.
    int ret = -1;
    int i = 0;
    bool save = false;
    bool keep_status = true;
    bool state = false;

    emberHardwareIDInit(&g_hardware_id);

    ret = HwIndexInit();
    if (ret < 0)
    {
        //shoud be reboot?
        return false;
    }
    // Read the stored token data
    halCommonGetToken(&boot_data, TOKEN_BOOT_PARAM);
    emberAfCorePrintln("boot_data.fw_version=%d, boot_data.factory_reset=%d, onoff_state=%d\n",
                boot_data.fw_version, boot_data.factory_reset, boot_data.onoff_state);

    /* factory reset sw reboot or poweron,
       turn off all relay and reset onoff state token */
    if (boot_data.factory_reset != 0 && halGetResetInfo() == 0x06)
    {
        boot_data.factory_reset = 0;
        keep_status = false;
        save = true;
    }
    else if (halGetResetInfo() == 0x04) // poweron
    {
        keep_status = false;
    }

    for (i = 0; i < RELAY_NUM; i++)
    {
        if (SWITCH_MAX_RELAY > i)
        {
            if (!keep_status)
            {
                SWITCH_TOGGLE(i, false);   // new relay has memory, turn off here
                CLEARBIT(boot_data.onoff_state, i);
                newStatus[i + 1] = false;
                save = true;
            }
            else if (keep_status)
            {
                state = READBIT(boot_data.onoff_state, i);
                //SWITCH_TOGGLE(i, state); // new relay has memory, no need to toggle
                newStatus[i + 1] = state;
            }
        }
    }

    if (save)
    {
        halCommonSetToken(TOKEN_BOOT_PARAM, &boot_data);
    }

    return false;  // exit?
}

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
boolean emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                   int16u indexOrDestination,
                                   EmberApsFrame* apsFrame,
                                   int16u msgLen,
                                   int8u* message,
                                   EmberStatus status)
{
    static uint8_t l_count = 0;

    if(status == EMBER_SUCCESS)
    {
        //clear the flag to indicate the network connection is well!
        g_is_disconnected = false;
        l_count = 0;
    }
    else
    {
        l_count ++;
        if( l_count >= DISCONNECTION_COUNT )
        {
            l_count = 0;
            //set the flag to indicate the network connection is broken!
            g_is_disconnected = true;
            emberAfAppPrintln("[HD debug] Send FAILED times reach maximum, Start heartbeat check");
            emberEventControlSetActive(HeartBeatEvent);
        }
    }

    return false;
}

/********************************************************************
Author	: Carl
CopyRight: EverGrande
Function: Get the endpoint number of the device.
Input	:
Output	:
Return	:
Notes	:
Release	: Date         Author      Notes
         2017-11-17 09:06  Carl     Create
********************************************************************/
unsigned char GetEndpointNum(void)
{
    return g_switch_handle_tbl[g_hw_id_index].relay_count;
}

/********************************************************************
Author	: Carl
CopyRight: EverGrande
Function: change model id of the specify endpoint.
Input	:
Output	:
Return	:
Notes	:
Release	: Date         Author      Notes
         2017-11-17 09:06  Carl     Create
********************************************************************/
void ChangeModelIDOfEndpoint(EmberAfDefinedEndpoint *Endpoint)
{
    EmberAfAttributeMetadata *pAttr;
    EmberAfCluster *pCluster;
    int len = 0;
    int i = 0;

    pCluster = Endpoint->endpointType->cluster;//the index of basic cluster is 0
    pAttr = Endpoint->endpointType->cluster->attributes;
    for(i=0; i < pCluster->attributeCount; i++)
    {
        emberAfAppPrintln("i=%d", i);
        if (pAttr[i].attributeId == 5) //attrid of the modelid is 5
        {
            emberAfAppPrintln("old model ID=%s", pAttr[i].defaultValue.ptrToDefaultValue);
            MEMSET(pAttr[i].defaultValue.ptrToDefaultValue, 0, 33);
            len = strlen(SWITCH_MODEL_ID);
            pAttr[i].size = len + 1;
            (pAttr[i].defaultValue.ptrToDefaultValue)[0] = len;
            MEMCOPY(pAttr[i].defaultValue.ptrToDefaultValue + 1, SWITCH_MODEL_ID, len);
            emberAfAppPrintln("size:%d, new model ID=%s", pAttr[i].size, pAttr[i].defaultValue.ptrToDefaultValue);
        }
    }
}

void ChangeOnoffAttrOfEndpoint(EmberAfPluginReportingEntry entry)
{
    if ((boot_data.factory_reset != 0 && halGetResetInfo() == 0x06)
        || halGetResetInfo() == 0x04)
    {
        return;
    }

    if (entry.clusterId == 0x0006 && entry.attributeId == 0x0000)
    {
        EmberAfAttributeSearchRecord record;
        uint8_t onoff_state;
        EmberAfStatus status;

        record.endpoint = entry.endpoint;
        record.clusterId = entry.clusterId;
        record.clusterMask = entry.mask;
        record.attributeId = entry.attributeId;
        record.manufacturerCode = entry.manufacturerCode;
        // write the attribute
        onoff_state = READBIT(boot_data.onoff_state, entry.endpoint - 1) ? 1 : 0;
        status = emAfReadOrWriteAttribute(&record,
                                            NULL,    // metadata
                                            (uint8_t *)&(onoff_state),
                                            0,       // buffer size - unused
                                            true);   // write?

        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfAppPrintln("====ChangeOnoffAttrOfEndpoint failed===");
        }
    }
}
