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
#define INSTRN_SCRN_HT  2   // Refers to number of rows in home and reading screen

#define ENTER_BTN       PD2

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

enum states {HOME, DISP_READINGS, SET_PARAMS};
unsigned char state;
unsigned char options_menu_ind = 0, reading_menu_ind = 0;
struct Data system_data;

void init_system(void);
void update_options_menu(void);
void update_readings_menu(void);
unsigned char check_input(uint8_t *reg, unsigned char bit);
void get_samples(void);
void display_readings(void);
void create_readings_menu(void);

int main()
{
    /* Initialize all the hardware */
    init_system();
    PORTD |= (1 << ENTER_BTN);  // Pull-up resistor for enter btn
    _delay_ms(1000);            // Ensure everything is loaded properly

    /* Initialize to HOME state */
    state = HOME;
    lcd_screen(home_screen, INSTRN_SCRN_HT);
    update_options_menu();

    /* Fill system_data with most recent data */
    get_samples();

    while(1)
    {
        if(state == HOME)
        {
            /* DISPLAY UPDATES */
            if(rot_changed)
            {
                update_options_menu();
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
                    update_readings_menu();                     // Display the Readings menu to the user
                }
                else if(options_menu_ind == 1)
                {
                    state = SET_PARAMS;
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
                update_readings_menu();
            }

            /* STATE TRANSITIONS */
            if(check_input(&PIND, ENTER_BTN))
            {
                state = HOME;
                lcd_screen(home_screen, INSTRN_SCRN_HT);
                update_options_menu();
            }

        }
        else if(state == SET_PARAMS)
        {
            /* What params do we want to set? */
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

/* update_options_menu - creates scrolling effect when rotary
    encoder is turned
*/
void update_options_menu()
{
    /* Get new options menu starting spot */
    if(rot_up) options_menu_ind = (options_menu_ind + 1) % OPT_MENU_LEN;
    else options_menu_ind = (options_menu_ind - 1) % OPT_MENU_LEN;

    /* Print out new options menu */
    int i;
    for(i = INSTRN_SCRN_HT; i < SCREEN_HEIGHT; i++)
    {
        lcd_moveto(i, 0);
        lcd_stringout(options_menu[options_menu_ind+(i-INSTRN_SCRN_HT)]);
    }
}

void update_readings_menu()
{
    if(rot_up) reading_menu_ind = (reading_menu_ind + 1) % READ_MENU_LEN;
    else reading_menu_ind = (reading_menu_ind - 1) % READ_MENU_LEN;

    int i;
    for(i = INSTRN_SCRN_HT; i < SCREEN_HEIGHT; i++)
    {
        lcd_moveto(i, 0);
        lcd_stringout(reading_menu[reading_menu_ind+(i-INSTRN_SCRN_HT)]);
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