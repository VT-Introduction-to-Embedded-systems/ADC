// This application uses button 1, LED 1, and LED 2 on Launchpad
// Pushing button 1 toggles LED1 status.
// Pushing a button consists of "pressing, followed by releasing".
// This a typical push-button behavior: as soon as you press the button, LED toggles,
// as you keep the button pressed nothing happens, once you release it, if you press again, the LED toggles again.

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "ButtonLED_HAL.h"

#define UP_THRESHOLD  0x3000
#define DOWN_THRESHOLD 0x1000
#define LEFT_THRESHOLD  0x3000
#define RIGHT_THRESHOLD 0x1200


void initADC() {
    ADC14_enableModule();

    // This sets the conversion clock to 3MHz
    ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC,
                     ADC_PREDIVIDER_1,
                     ADC_DIVIDER_1,
                      0
                     );

    // This configures the ADC to store output results
    // in ADC_MEM0 up to ADC_MEM1. Each conversion will
    // thus use two channels.
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);

    // This configures the ADC in manual conversion mode
    // Software will start each conversion.
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}

void startADC() {
   // Starts the ADC with the first conversion
   // in repeat-mode, subsequent conversions run automatically
   ADC14_enableConversion();
   ADC14_toggleConversionTrigger();
}

void initJoyStick() {

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    // (non-differential means: only a single input pin)
    // The reference for Vref- and Vref+ are VSS and VCC respectively
    ADC14_configureConversionMemory(ADC_MEM0,
                                  ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                  ADC_INPUT_A15,                 // joystick X
                                  ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input
    // A15 is multiplexed on GPIO port P6 pin PIN0
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    // (non-differential means: only a single input pin)
    // The reference for Vref- and Vref+ are VSS and VCC respectively
    ADC14_configureConversionMemory(ADC_MEM1,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9,                 // joystick Y
                                    ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input
    // A9 is multiplexed on GPIO port P4 pin PIN4
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);
}

void getSampleJoyStick(unsigned *X, unsigned *Y) {
    // ADC runs in continuous mode, we just read the conversion buffers
    *X = ADC14_getResult(ADC_MEM0);
    *Y = ADC14_getResult(ADC_MEM1);
}

// This function initializes all the peripherals except graphics
void initialize();

// This function initializes the graphics part
void InitGraphics(Graphics_Context *g_sContext_p);


void ModifyLEDColor(bool leftButtonWasPushed, bool rightButtonWasPushed);

void make_3digit_NumString(unsigned int num, char *string)
{
    string[0]= (num/100)+'0';
    string[1]= ((num%100) / 10) + '0';
    string[2]= (num%10)+'0';
    string[3] =0;

}

void MoveCircle(Graphics_Context *g_sContext_p, bool leftButtonWasPushed, bool rightButtonWasPushed)
{
    static unsigned int x = 63;
    static unsigned int moveCount = 0;
    char string[4];

    if ((leftButtonWasPushed && (x>20)) || (rightButtonWasPushed && (x<110)))
    {

        Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
        Graphics_fillCircle(g_sContext_p, x, 63, 10);

        if (leftButtonWasPushed)
            x = x-10;
        else
            x = x+10;

        Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
        Graphics_fillCircle(g_sContext_p, x, 63, 10);

        moveCount++;
        make_3digit_NumString(moveCount, string);
        Graphics_drawString(g_sContext_p, string, -1, 10, 110, true);
    }

}

int main(void)
{

    button_t LauchpadLeftButton = {GPIO_PORT_P1, GPIO_PIN1, Stable_R, RELEASED_STATE, TIMER32_0_BASE};
    button_t LauchpadRightButton = {GPIO_PORT_P1, GPIO_PIN4, Stable_R, RELEASED_STATE, TIMER32_1_BASE};
    Graphics_Context g_sContext;

    initialize();
    InitGraphics(&g_sContext);

    Graphics_Rectangle R;
    R.xMin = 0;
    R.xMax = 127;
    R.yMin = 32;
    R.yMax = 96;

    Graphics_drawRectangle(&g_sContext, &R);
    Graphics_fillCircle(&g_sContext, 63, 63, 10);
    Graphics_drawString(&g_sContext, "circle move #:", -1, 10, 100, false);
    Graphics_drawString(&g_sContext, "000", -1, 10, 110, true);



    unsigned vx, vy;

    initADC();
    initJoyStick();
    startADC();




    while (1)
     {
         bool leftButtonPushed = ButtonPushed(&LauchpadLeftButton);
         bool rightButtonPushed = ButtonPushed(&LauchpadRightButton);

//       ModifyLEDColor(leftButtonPushed,rightButtonPushed);
//       MoveCircle(&g_sContext, leftButtonPushed,rightButtonPushed);


         getSampleJoyStick(&vx, &vy);
         bool joyStickPushedtoTop = false;
         bool joyStickPushedtoBottom = false;
         bool joyStickPushedtoRight = false;
         bool joyStickPushedtoLeft = false;

         if (vx > RIGHT_THRESHOLD)
         {
             joyStickPushedtoRight = true;
         }
         else if (vx < LEFT_THRESHOLD)
         {
             joyStickPushedtoLeft = true;
         }

         ModifyLEDColor(joyStickPushedtoLeft,joyStickPushedtoRight);
         MoveCircle(&g_sContext, joyStickPushedtoLeft,joyStickPushedtoRight);

     }
}


void InitFonts() {
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
}


void InitGraphics(Graphics_Context *g_sContext_p) {

    Graphics_initContext(g_sContext_p,
                         &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_YELLOW);
    Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLUE);
    Graphics_setFont(g_sContext_p, &g_sFontCmtt12);

    InitFonts();

    Graphics_clearDisplay(g_sContext_p);
}

void initialize()
{
    // stop the watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // initialize the boosterPack LEDs and turn them off except for red LED
    initialize_BoosterpackLED_red();
    initialize_BoosterpackLED_green();
    initialize_BoosterpackLED_blue();
    turnOn_BoosterpackLED_red();
    turnOff_BoosterpackLED_green();
    turnOff_BoosterpackLED_blue();

    // initialize the Launchpad buttons
    initialize_LaunchpadLeftButton();
    initialize_LaunchpadRightButton();


    // Initialize the timers needed for debouncing
    Timer32_initModule(TIMER32_0_BASE, // There are two timers, we are using the one with the index 0
                       TIMER32_PRESCALER_1, // The prescaler value is 1; The clock is not divided before feeding the counter
                       TIMER32_32BIT, // The counter is used in 32-bit mode; the alternative is 16-bit mode
                       TIMER32_PERIODIC_MODE); //This options is irrelevant for a one-shot timer

    Timer32_initModule(TIMER32_1_BASE, // There are two timers, we are using the one with the index 1
                       TIMER32_PRESCALER_1, // The prescaler value is 1; The clock is not divided before feeding the counter
                       TIMER32_32BIT, // The counter is used in 32-bit mode; the alternative is 16-bit mode
                       TIMER32_PERIODIC_MODE); //This options is irrelevant for a one-shot timer


}

// This FSM has two inputs each of them the FSM if a button has been pushed or not
// The FSM has three states: Red, Green, Blue. The initial state is Red
// The FSM has three outputs, each output is a boolean that decides if an LED should be on or off
// When the left button is pressed, the FSM goes
void ModifyLEDColor(bool leftButtonWasPushed, bool rightButtonWasPushed)
{
    typedef enum {red, green, blue} LED_state_t;

    static LED_state_t BoosterLED = red;

    // outputs of the FSM and their default
    bool toggleGreen = false;
    bool toggleBlue = false;
    bool toggleRed = false;

    switch(BoosterLED)
    {
    case red:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = green;

            //outputs
            // This turns green on
            toggleGreen = true;

            // This turns red off
            toggleRed = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = blue;

            //outputs
            toggleBlue = true;
            toggleRed = true;
        }
        break;
    case green:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = blue;

            //outputs
            toggleBlue = true;
            toggleGreen = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = red;

            //outputs
            toggleRed = true;
            toggleGreen = true;
        }
        break;
    case blue:
        if (leftButtonWasPushed)
        {
            // next state
            BoosterLED = red;

            //outputs
            toggleRed = true;
            toggleBlue = true;
        }
        else if (rightButtonWasPushed)
        {
            BoosterLED = green;

            //outputs
            toggleGreen = true;
            toggleBlue = true;
        }
    }

    if (toggleRed)
        toggle_BoosterpackLED_red();

    if (toggleGreen)
        toggle_BoosterpackLED_green();

    if (toggleBlue)
        toggle_BoosterpackLED_blue();

}


