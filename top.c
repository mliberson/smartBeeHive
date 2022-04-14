#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "lcd/serial.h"
#include "lcd/lcd.h"
#include "rotary_encoder/rotary_encoder.h"
#include "adc/adc.h"
#include "HX711-master/HX711.h"
#include "data.h"
#include "uv\uv.h"

#define SCREEN_WIDTH    20
#define SCREEN_HEIGHT   4
#define OPT_MENU_LEN    2
#define READ_MENU_LEN   4
#define SET_MENU_LEN    1
#define INSTRN_SCRN_HT  2   // Refers to number of rows in home and reading screen

#define ENTER_BTN       PD2
#define HEAT_OUT        /*pin controlling heaters*/
#define FAN_OUT         /*pin controlling fans*/

#define MAX_TEMP_VAL    80
#define MIN_TEMP_VAL    60
#define DELTA_TEMP      3

char *splash_screen[SCREEN_WIDTH] =  {"                    ",
                                      "      WELCOME       ",
                                      "    TO THE HIVE     ",
                                      "                    "};
char *options_menu[SCREEN_WIDTH] =   {"SEE CURRENT READINGS",       // If changing the order of options menu
                                      "  SET A PARAMETER   "};      // be sure to change order in state transitions below
char *home_screen[SCREEN_WIDTH] =    {"   PRESS TO SELECT  ",
                                      " SCROLL TO SEE MORE "};
char *reading_screen[SCREEN_WIDTH] = {"  PRESS TO GO BACK  ",
                                      " SCROLL TO SEE MORE "};
char *reading_menu[SCREEN_WIDTH];
char *set_menu[SCREEN_WIDTH] =       {"Set Temperature     "};
char *set_screen[SCREEN_WIDTH] =     {"SCROLL TO CHANGE VAL",
                                      "PRESS TO SELECT     "};

enum states {HOME, DISP_READINGS, SET_PARAMS};
unsigned char state;
unsigned char options_menu_ind = 0, reading_menu_ind = 0, set_menu_ind = 0;
unsigned char set_temp_val = 68;
unsigned char set_flag = 0;
struct Data system_data;
struct Data op_conditions;

void init_system(void);
void update_menu(char** menu, unsigned char *menu_ind, const unsigned char menu_len);
unsigned char check_input(uint8_t *reg, unsigned char bit);
void get_samples(void);
void display_readings(void);
void create_readings_menu(void);
void update_set_param_display(void);

int main()
{
    /* Initialize all the hardware */
    init_system();
    PORTD |= (1 << ENTER_BTN);  // Pull-up resistor for enter btn
    _delay_ms(1000);            // Ensure everything is loaded properly

    /* Initialize to HOME state */
    state = HOME;
    lcd_screen(home_screen, INSTRN_SCRN_HT);
    update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN);

    /* Fill system_data with most recent data */
    get_samples();

    while(1)
    {
        /*******************
            STATE MACHINE
         *******************/
        if(state == HOME)
        {
            /* DISPLAY UPDATES */
            if(rot_changed)
            {
                update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN);
            }

            /* STATE TRANSITIONS */
            /* If enter btn is pressed, go to new state */
            if(check_input(&PIND, ENTER_BTN))
            {
                if(options_menu_ind == 0)
                {
                    state = DISP_READINGS;
                    create_readings_menu();                     // Construct Readings menu
                    lcd_screen(reading_screen, INSTRN_SCRN_HT); // Display the Readings screen instruction menu
                    update_menu(reading_menu, &reading_menu_ind, READ_MENU_LEN);
                }
                else if(options_menu_ind == 1)
                {
                    state = SET_PARAMS;
                    lcd_screen(home_screen, INSTRN_SCRN_HT);
                    update_menu(set_menu, &set_menu_ind, SET_MENU_LEN);
                }
                /* Can add new state transitions HERE when we get them */
                else
                {
                    // should never get here (ideally)
                }
            }
        }
        else if(state == DISP_READINGS)
        {
            /* DISPLAY UPDATES */
            if(rot_changed)
            {
                update_menu(reading_menu, &reading_menu_ind, READ_MENU_LEN);
            }

            /* STATE TRANSITIONS */
            if(check_input(&PIND, ENTER_BTN))
            {
                state = HOME;
                lcd_screen(home_screen, INSTRN_SCRN_HT);
                update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN);
            }

        }
        else if(state == SET_PARAMS)
        {
            /* Scroll functionality */
            if(rot_changed && !set_flag)
            {
                update_menu(set_menu, &set_menu_ind, SET_MENU_LEN);
            }

            /* Denote which paramter to change */
            if(check_input(&PIND, ENTER_BTN))
            {
                set_flag = 1;
                lcd_screen(set_screen, INSTRN_SCRN_HT);
                update_set_param_display();
            }

            /* Change and select the desired parameter value */
            if(set_flag)
            {
                if(rot_changed)
                {
                    if(rot_up) set_temp_val = 60 + (set_temp_val + 1) % (MAX_TEMP_VAL-MIN_TEMP_VAL);
                    else set_temp_val = 60 + (set_temp_val - 1) % (MAX_TEMP_VAL - MIN_TEMP_VAL);
                    update_set_param_display();
                }
                if(check_input(&PIND, ENTER_BTN))
                {
                    op_conditions.temperature = set_temp_val;   // will need to be made more generic for other conditions
                    set_flag = 0;
                    state = HOME;
                    lcd_screen(home_screen, INSTRN_SCRN_HT);
                    update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN);
                }
            }
        }

        /*******************
            OUTPUT LOGIC
         *******************/
        /* turn on heaters or fans to change internal temperataure */
        if(system_data.temperature < op_conditions.temperature - DELTA_TEMP)
        {
            // turn on heaters
        }
        else if(system_data.temperature > op_conditions.temperature + DELTA_TEMP)
        {
            // turn on fans
            // maybe make this pwm? idk
        }
    }

    return 0;
}

/* init_system - Initializes all the hardware and flashes splashscreen */
void init_system()
{
    serial_init();          // Initialize serial communications
    lcd_init();             // Initialize LCD screen
    lcd_screen(splash_screen, SCREEN_HEIGHT);
    adc_init();             // Initialize Analog-to-Digital Converter
    rot_encoder_init();     // Initialize Rotary Encoder
    HX711_init(128);        // Initialize weight sensor
}

/* update_menu - Creates the scrolling effect for any given menu on the LCD 
    Parameters:
        - menu --> the array of strings to be scrolled through
        - menu_ind --> the integer denoting which index of the array is selected
        - menu_len --> the length of the array of strings denoted by 'menu'
*/
void update_menu(char** menu, unsigned char *menu_ind, const unsigned char menu_len)
{
    // Increment or decrement index based on rotary encoder movement
    if(rot_up) *menu_ind = (*menu_ind + 1) % menu_len;
    else *menu_ind = (*menu_ind - 1) % menu_len;

    // Print out the current menu
    int i;
    for(i = INSTRN_SCRN_HT; i < SCREEN_HEIGHT; i++)
    {
        lcd_moveto(i, 0);
        lcd_stringout(menu[*menu_ind+(i-INSTRN_SCRN_HT)]);
    }
}

/* check_input - returns whether or not the button in register reg
    bit bit has been pushed
    Note:
        reg = 0 <== PINB
        reg = 1 <== PINC
        reg = 2 <== PIND
*/
unsigned char check_input(uint8_t *reg, unsigned char bit)
{
    if((*reg & (1 << bit)) != 0)
    {
        // Debouncing
        _delay_ms(5);
        while((*reg & (1 << bit)) != 0);
        _delay_ms(5);
        return 0;
    }
    // Debouncing
    _delay_ms(5);
    while((*reg & (1 << bit)) == 0);
    _delay_ms(5);
    return 1;
}

/* get_samples - populates the Data structure with most recent
    sensor readings
*/
void get_samples()
{
    system_data.uv = get_uv_sample();
    /* The accessor functions below need to be implemented */
    // system_data.temperature = get_temp_sample();
    // system_data.humidity = get_humidity_sample();
    // system_data.weight = get_weight_sample();
}

/* create_readings_menu - creates the char array menu with the
    most recent sensor readings
*/
void create_readings_menu()
{
    char buf[SCREEN_WIDTH+1];

    // Doesn't handle decimals right now
    /* Setup UV index string */
    sprintf(buf, "UV Index:%d", system_data.uv);
    reading_menu[0] = buf;
    memset(buf, 0, SCREEN_WIDTH+1);
    /* Setup Temperature string */
    sprintf(buf, "Temperature:%d F", system_data.temperature);
    reading_menu[1] = buf;
    memset(buf, 0, SCREEN_WIDTH+1);
    /* Setup Humidity String */
    sprintf(buf, "Humidity:%d %%", system_data.humidity);
    reading_menu[2] = buf;
    memset(buf, 0, SCREEN_WIDTH+1);
    /* Setup Weight string */
    sprintf(buf, "Weiht:%d lbs", system_data.weight);
    reading_menu[3] = buf;
    memset(buf, 0, SCREEN_WIDTH+1);
    
}

/* update_set_param_display - displays the paramter to be set and the
    value the user is currently considering
    NOTE: THIS FUNCTION CURRENTLY ONLY WORKS FOR TEMPERATURE
*/
void update_set_param_display()
{
    char buf[SCREEN_WIDTH+1];

    sprintf(buf, "Temperature%9d", set_temp_val);
    lcd_moveto(INSTRN_SCRN_HT, 0);
    lcd_stringout(buf);
}
