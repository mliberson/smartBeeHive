#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd/serial.h"
#include "lcd/lcd.h"
#include "rotary_encoder/rotary_encoder.h"
#include "adc/adc.h"
#include "eeprom/eeprom.h"
#include "HX711-master/HX711.h"
#include "data.h"
#include "uv\uv.h"
#include "i2c\i2c.h"
#include "DHT11\DHT11.h"

#define SCREEN_WIDTH    20
#define SCREEN_HEIGHT   4
#define OPT_MENU_LEN    4
#define READ_MENU_LEN   4
#define SET_MENU_LEN    1
#define INSTRN_SCRN_HT  2   // Refers to number of rows in home and reading screen

#define ENTER_BTN       PB1
#define HEAT_OUT        PD5
#define FAN_OUT         PD6

#define MAX_TEMP_VAL    80
#define MIN_TEMP_VAL    60
#define DELTA_TEMP      3

#define EEPROM_INIT     50

char *splash_screen[SCREEN_HEIGHT] = {"                    ",
                                      "      WELCOME       ",
                                      "    TO THE HIVE     ",
                                      "                    "};
char *options_menu[OPT_MENU_LEN] =   {"SEE CURRENT READINGS",       // If changing the order of options menu
                                      "  SET A PARAMETER   ",
                                      "POOPPEEPEEPEEPOOPPEE",
                                      "TEST  TEST  TEST    "};      // be sure to change order in state transitions below
char *home_screen[INSTRN_SCRN_HT] =  {"   PRESS TO SELECT  ",
                                      " SCROLL TO SEE MORE "};
char *reading_screen[INSTRN_SCRN_HT]={"  PRESS TO GO BACK  ",
                                      " SCROLL TO SEE MORE "};
char *reading_menu[READ_MENU_LEN];
char *set_menu[SET_MENU_LEN] =       {"Set Temperature     "};
char *set_screen[INSTRN_SCRN_HT] =   {"SCROLL TO CHANGE VAL",
                                      "PRESS TO SELECT     "};

enum states {HOME, DISP_READINGS, SET_PARAMS};
unsigned char state;
unsigned char options_menu_ind = 0, reading_menu_ind = 0, set_menu_ind = 0;
unsigned char set_temp_val = 68;
unsigned char set_flag = 0;
unsigned int eeprom_internal_addr = EEPROM_INIT, eeprom_timer_count = 0;
char *temp_humid_sample = NULL;
struct Data system_data;
struct Data op_conditions;

void init_system(void);
void update_menu(char** menu, unsigned char *menu_ind, const unsigned char menu_len);
unsigned char check_input(unsigned char bit);
void get_samples(void);
void display_readings(void);
void create_readings_menu(void);
void update_set_param_display(void);
void save_data_to_eeprom(void);
void init_timer1(void);

int main()
{
    /* Initialize all the hardware */
    init_system();
    PORTB |= (1 << ENTER_BTN);  // Pull-up resistor for enter btn
    _delay_ms(1000);            // Ensure everything is loaded properly

    /* Initialize to HOME state */
    state = HOME;
    lcd_clear();
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
                rot_changed = 0;
            }

            /* STATE TRANSITIONS */
            /* If enter btn is pressed, go to new state */
            if(check_input(ENTER_BTN))
            {
                if(options_menu_ind == 0)
                {
                    state = DISP_READINGS;
                    create_readings_menu();                     // Construct Readings menu
                    lcd_clear();                                // Clear Screen
                    lcd_screen(reading_screen, INSTRN_SCRN_HT); // Display the Readings screen instruction menu
                    update_menu(reading_menu, &reading_menu_ind, READ_MENU_LEN);
                }
                else if(options_menu_ind == 1)
                {
                    state = SET_PARAMS;
                    lcd_clear();
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
                rot_changed = 0;
            }

            /* STATE TRANSITIONS */
            if(check_input(ENTER_BTN))
            {
                state = HOME;
                lcd_clear();
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
                rot_changed = 0;
            }

            /* Denote which paramter to change */
            if(check_input(ENTER_BTN))
            {
                set_flag = 1;
                lcd_clear();
                lcd_screen(set_screen, INSTRN_SCRN_HT);
                update_set_param_display();
            }

            /* Change and select the desired parameter value */
            if(set_flag)
            {
                if(rot_changed)
                {
                    if(rot_up)
                    {
                        if(set_temp_val < MAX_TEMP_VAL) set_temp_val += 1;
                    }
                    else
                    {
                        if(set_temp_val > MIN_TEMP_VAL) set_temp_val -= 1;
                    }
                    update_set_param_display();
                    rot_changed = 0;
                }
                if(check_input(ENTER_BTN))
                {
                    op_conditions.temperature_int = set_temp_val;   // will need to be made more generic for other conditions
                    set_flag = 0;
                    state = HOME;
                    lcd_clear();
                    lcd_moveto(1,0);
                    lcd_stringout("   PARAMETER SET!   ");
                    _delay_ms(1000);
                    lcd_clear();
                    lcd_screen(home_screen, INSTRN_SCRN_HT);
                    update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN);
                }
            }
        }

        /*******************
            OUTPUT LOGIC
         *******************/
        /* turn on heaters or fans to change internal temperataure */
        if(system_data.temperature_int < op_conditions.temperature_int - DELTA_TEMP)
        {
            // turn on heaters
        }
        else if(system_data.temperature_int > op_conditions.temperature_int + DELTA_TEMP)
        {
            // turn on fans
            // maybe make this pwm? idk
        }

        /*Save to EEPROM every hour */
        if(eeprom_timer_count == 2)
        {
            eeprom_timer_count = 0;
            lcd_moveto(0,0);
            lcd_stringout("Saving to EEPROM");
            // Save to EEPROM
            get_samples();
            save_data_to_eeprom();

            char* buf;
            int start_addr = EEPROM_INIT;
            buf = read_eeprom(start_addr);
            lcd_moveto(1,0);
            lcd_stringout(buf);
            _delay_ms(3000);
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
    i2c_init();             // Initialize I2C for EEPROM
    adc_init();             // Initialize Analog-to-Digital Converter
    rot_encoder_init();     // Initialize Rotary Encoder
    HX711_init(128);        // Initialize weight sensor
    init_timer1();
    sei();
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
    if(rot_up)
    {
        if(*menu_ind == (menu_len - 1)) *menu_ind = 0;
        else *menu_ind += 1;
    }
    else
    {
        if(*menu_ind == 0) *menu_ind = menu_len - 1;
        else *menu_ind -= 1;
    }

    // Print out the current menu
    unsigned char i, temp_ind;
    for(i = INSTRN_SCRN_HT; i < SCREEN_HEIGHT; i++)
    {
        lcd_moveto(i, 0);
        temp_ind = (*menu_ind+(i-INSTRN_SCRN_HT)) % menu_len;
        lcd_stringout(menu[temp_ind]);
    }
}

/* check_input - returns whether or not the button in register reg
    bit bit has been pushed
    Note:
        reg = 0 <== PINB
        reg = 1 <== PINC
        reg = 2 <== PIND
*/
unsigned char check_input(unsigned char bit)
{
    if((PINB & (1 << bit)) != 0)
    {
        return 0;
    }
    // Debouncing
    _delay_ms(5);
    while((PINB & (1 << bit)) == 0);
    _delay_ms(5);
    return 1;
}

/* get_samples - populates the Data structure with most recent
    sensor readings
*/
void get_samples()
{
    system_data.uv = get_uv_sample();
    // If the program has runtime blowup, probably here or in create_readings_menu
    free(temp_humid_sample);
    temp_humid_sample = get_temp_humid_sample();
    system_data.humidity_int = temp_humid_sample[0];
    system_data.humidity_dec = temp_humid_sample[1];
    system_data.temperature_int = temp_humid_sample[2];
    system_data.temperature_dec = temp_humid_sample[3];
    system_data.weight = HX711_read_average(10);
    /* The accessor functions below need to be implemented */
    // system_data.weight = get_weight_sample();
}

/* create_readings_menu - creates the char array menu with the
    most recent sensor readings
*/
void create_readings_menu()
{
    int i;
    /* Reduce memory leaks by freeing previous reading menu strings */
    for(i = 0; i < READ_MENU_LEN; i++)
    {
        free(reading_menu[i]);
    }
    char buf1[SCREEN_WIDTH+1],buf2[SCREEN_WIDTH+1],buf3[SCREEN_WIDTH+1],buf4[SCREEN_WIDTH+1];
    /* Setup UV index string */
    sprintf(buf1, "UV Index:%11d", system_data.uv);
    reading_menu[0] = malloc(strlen(buf1)+1);
    strcpy(reading_menu[0],buf1);
    /* Setup Temperature string */
    sprintf(buf2, "Temperature:%4d.%d F", system_data.temperature_int, system_data.temperature_dec);
    reading_menu[1] = malloc(strlen(buf2)+1);
    strcpy(reading_menu[1], buf2);
    /* Setup Humidity String */
    sprintf(buf3, "Humidity:%7d.%d %%", system_data.humidity_int, system_data.humidity_dec);
    reading_menu[2] = malloc(strlen(buf3)+1);
    strcpy(reading_menu[2], buf3);
    /* Setup Weight string */
    sprintf(buf4, "Weight:%9d lbs", system_data.weight);
    reading_menu[3] = malloc(strlen(buf4)+1);
    strcpy(reading_menu[3], buf4);
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

void save_data_to_eeprom()
{
    char buf[4];

    sprintf(buf, "%3d", system_data.uv);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.temperature_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.temperature_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.humidity_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.humidity_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.weight);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
}

void init_timer1()
{
    TCCR1B |= (1 << WGM12);                 // set for clear timer on compare
    TIMSK1 |= (1 << OCIE1A);                // Enable output compare A Match Interrupt
    OCR1A = 35999;                          // Interrupt every 5 seconds
    TCCR1B |= ((1 << CS12) | (1<< CS10));   // ^
}

ISR(TIMER1_COMPA_vect)
{
    eeprom_timer_count++;
}