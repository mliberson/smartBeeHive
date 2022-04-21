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
#define READ_MENU_LEN   6
#define SET_MENU_LEN    1
#define INSTRN_SCRN_HT  2   // Refers to number of rows in home and reading screen

#define ENTER_BTN       PB1
#define HEAT_OUT1       PD5
#define HEAT_OUT2       PB7
#define FAN_OUT1        PD6
#define FAN_OUT2        PB7
#define TEMP_OUT_PIN    PD3
#define TEMP_IN_PIN     PD2

#define TEMP_IN_INIT    20
#define MAX_TEMP_VAL    30
#define MIN_TEMP_VAL    15
#define DELTA_TEMP      3

#define EEPROM_INIT     50

char *splash_screen[SCREEN_HEIGHT] = {"                    ",
                                      "      WELCOME       ",
                                      "    TO THE HIVE     ",
                                      "                    "};
char *options_menu[OPT_MENU_LEN] =   {"- See Sensor Values ",       // If changing the order of options menu
                                      "- Adjust Settings   ",
                                      "- Best Conditions   ",
                                      "- See Warnings      "};      // be sure to change order in state transitions below
char *home_screen[INSTRN_SCRN_HT] =  {"   PRESS TO SELECT  ",
                                      " SCROLL TO SEE MORE "};
char *reading_screen[INSTRN_SCRN_HT]={"  PRESS TO GO BACK  ",
                                      " SCROLL TO SEE MORE "};
char *reading_menu[READ_MENU_LEN];                                  // created and updated in create_readings_menu() func
char *set_menu[SET_MENU_LEN] =       {"- Set Temperature   "};
char *set_screen[INSTRN_SCRN_HT] =   {"SCROLL TO CHANGE VAL",
                                      "PRESS TO SELECT     "};

enum states {HOME, DISP_READINGS, SET_PARAMS};
unsigned char state;
unsigned char options_menu_ind = 0, reading_menu_ind = 0, set_menu_ind = 0;
unsigned char set_temp_val = TEMP_IN_INIT;
unsigned char set_flag = 0;
unsigned int eeprom_internal_addr = EEPROM_INIT, eeprom_timer_count = 0;
char *temp_humid_in_sample = NULL, *temp_humid_out_sample = NULL;
struct Data system_data;
struct Data op_conditions;

void init_system(void);
void update_menu(char** menu, unsigned char *menu_ind, const unsigned char menu_len, char select_flag);
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
    _delay_ms(3000);            // Ensure everything is loaded properly

    DDRB |= ((1 << FAN_OUT2) | (1 << HEAT_OUT2));   // Set heaters and fans as outputs
    DDRD |= ((1 << FAN_OUT1) | (1 << HEAT_OUT1));   // ^

    /* Initialize to HOME state */
    state = HOME;
    lcd_clear();
    lcd_screen(home_screen, INSTRN_SCRN_HT);
    _delay_ms(3000);
    update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN, 1);

    /* Fill system_data with most recent data */
    get_samples();

    op_conditions.temp_in_int = set_temp_val;

    while(1)
    {
        /*************************************************
                         STATE MACHINE
         *************************************************/
        if(state == HOME)
        {
            /* DISPLAY UPDATES */
            if(rot_changed)
            {
                update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN, 1);
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
                    _delay_ms(3000);
                    update_menu(reading_menu, &reading_menu_ind, READ_MENU_LEN, 0);
                }
                else if(options_menu_ind == 1)
                {
                    state = SET_PARAMS;
                    lcd_clear();
                    lcd_screen(home_screen, INSTRN_SCRN_HT);
                    _delay_ms(3000);
                    update_menu(set_menu, &set_menu_ind, SET_MENU_LEN, 1);
                }
                else if(options_menu_ind == 2)
                {
                    /* Best conditions info */
                }
                else if(options_menu_ind == 3)
                {
                    /* Wanring info */
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
                update_menu(reading_menu, &reading_menu_ind, READ_MENU_LEN, 0);
                rot_changed = 0;
            }

            /* STATE TRANSITIONS */
            if(check_input(ENTER_BTN))
            {
                state = HOME;
                lcd_clear();
                lcd_screen(home_screen, INSTRN_SCRN_HT);
                _delay_ms(3000);
                update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN, 1);
            }

        }
        else if(state == SET_PARAMS)
        {
            /* Scroll functionality */
            if(rot_changed && !set_flag)
            {
                update_menu(set_menu, &set_menu_ind, SET_MENU_LEN, 1);
                rot_changed = 0;
            }

            /* Denote which paramter to change */
            if(check_input(ENTER_BTN))
            {
                set_flag = 1;
                lcd_clear();
                lcd_screen(set_screen, INSTRN_SCRN_HT);
                _delay_ms(3000);
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
                    op_conditions.temp_in_int = set_temp_val;   // will need to be made more generic for other conditions
                    set_flag = 0;
                    state = HOME;
                    lcd_clear();
                    lcd_moveto(1,0);
                    lcd_stringout("   PARAMETER SET!   ");
                    _delay_ms(1000);
                    lcd_clear();
                    lcd_screen(home_screen, INSTRN_SCRN_HT);
                    update_menu(options_menu, &options_menu_ind, OPT_MENU_LEN, 1);
                }
            }
        }

        /*******************
            OUTPUT LOGIC
         *******************/
        /* turn on heaters or fans to change internal temperataure */
        if(system_data.temp_in_int < op_conditions.temp_in_int - DELTA_TEMP)
        {
            // turn on heaters
            PORTD |= (1 << HEAT_OUT1);
            PORTB |= (1 << HEAT_OUT2);
        }
        else if(system_data.temp_in_int > op_conditions.temp_in_int + DELTA_TEMP)
        {
            // turn on fans
            // maybe make this pwm? idk
            PORTD |= (1 << FAN_OUT1);
            PORTD |= (1 << FAN_OUT2);
        }
        else 
        {
            // Turn it all off
            PORTD &= ~((1 << FAN_OUT1) | (1 << HEAT_OUT1));
            PORTB &= ~((1 << FAN_OUT2) | (1 << HEAT_OUT2));
        }

        if((eeprom_timer_count % 20) == 0)
        {
            get_samples();
        }

        /*Save to EEPROM every hour */
        if(eeprom_timer_count == 720)
        {
            eeprom_timer_count = 0;
            // Save to EEPROM
            get_samples();
            save_data_to_eeprom();
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
    init_timer1();          // Initialize timer
    sei();                  // Turn on all Interrupts
}

/* update_menu - Creates the scrolling effect for any given menu on the LCD 
    Parameters:
        - menu --> the array of strings to be scrolled through
        - menu_ind --> the integer denoting which index of the array is selected
        - menu_len --> the length of the array of strings denoted by 'menu'
        - menu_flag --> '1' if the menu has selectable content, '0' otherwise
*/
void update_menu(char** menu, unsigned char *menu_ind, const unsigned char menu_len, char select_flag)
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
    for(i = 0; i < SCREEN_HEIGHT; i++)
    {
        lcd_moveto(i, 0);
        temp_ind = (*menu_ind+i) % menu_len;
        lcd_stringout(menu[temp_ind]);
    }

    if (select_flag)
    {
        lcd_moveto(0,0);
        lcd_stringout("> ");
    }
}

/* check_input - returns whether or not the button in register reg
    bit bit has been pushed
    Note:
        Only setup for PINB b/c that's what button is on
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
    free(temp_humid_in_sample);                            // I'm pretty sure this will prevent memory leaks
    temp_humid_in_sample = get_temp_humid_sample(TEMP_IN_PIN);
    system_data.hum_in_int = temp_humid_in_sample[0];
    system_data.hum_in_dec = temp_humid_in_sample[1];
    system_data.temp_in_int = temp_humid_in_sample[2];
    system_data.temp_in_dec = temp_humid_in_sample[3];
    free(temp_humid_out_sample);
    temp_humid_out_sample = get_temp_humid_sample(TEMP_OUT_PIN);
    system_data.hum_out_int = temp_humid_out_sample[0];
    system_data.hum_out_dec = temp_humid_out_sample[1];
    system_data.temp_out_int = temp_humid_out_sample[2];
    system_data.temp_out_dec = temp_humid_out_sample[3];
    system_data.weight = HX711_read_average(10);
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
    char buf1[SCREEN_WIDTH+1],buf2[SCREEN_WIDTH+1],buf3[SCREEN_WIDTH+1],buf4[SCREEN_WIDTH+1],buf5[SCREEN_WIDTH+1],buf6[SCREEN_WIDTH+1];
    /* Setup UV index string */
    sprintf(buf1, "UV Index:%11d", system_data.uv);
    reading_menu[0] = malloc(strlen(buf1)+1);
    strcpy(reading_menu[0],buf1);
    /* Setup Internal Temperature string */
    sprintf(buf2, "Inside Temp:%4d.%d C", system_data.temp_in_int, system_data.temp_in_dec);
    reading_menu[1] = malloc(strlen(buf2)+1);
    strcpy(reading_menu[1], buf2);
    /* Setup External Temperature string */
    sprintf(buf3, "Outside Temp:%3d.%d C", system_data.temp_out_int, system_data.temp_out_dec);
    reading_menu[2] = malloc(strlen(buf3)+1);
    strcpy(reading_menu[2], buf3);
    /* Setup Internal Humidity String */
    sprintf(buf4, "Inside RH:%6d.%d %%", system_data.hum_in_int, system_data.hum_in_dec);
    reading_menu[3] = malloc(strlen(buf4)+1);
    strcpy(reading_menu[3], buf4);
    /* Setup External Humidity String */
    sprintf(buf5, "Outside RH:%5d.%d %%", system_data.hum_out_int, system_data.hum_out_dec);
    reading_menu[4] = malloc(strlen(buf5)+1);
    strcpy(reading_menu[4], buf5);
    /* Setup Weight string */
    sprintf(buf6, "Weight:%9d lbs", system_data.weight);
    reading_menu[5] = malloc(strlen(buf6)+1);
    strcpy(reading_menu[5], buf6);
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

/* save_data_to_eeprom - writes all of the current sensor data
    to the eeprom in 4-byte chunks (3 bytes for the digits and
    one byte for the null)
    NOTE:
        Each write goes for 40 bytes
*/
void save_data_to_eeprom()
{
    char buf[4];

    sprintf(buf, "%3d", system_data.temp_in_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.temp_in_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.temp_out_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.temp_out_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.uv);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.hum_in_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.hum_in_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.hum_out_int);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.hum_out_dec);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;
    sprintf(buf, "%3d", system_data.weight);
    write_eeprom(buf, eeprom_internal_addr);
    eeprom_internal_addr += 4;

    /* Handle case of writing past 32KB boundary */
    if(eeprom_internal_addr > 32000)
    {
        eeprom_internal_addr = EEPROM_INIT;
    }
}

/* init_timer1 - initializes timer one to be used as a
    countdown for the eeprom writes
    Note:
        - currently setup to increment counter every 5 seconds (OCR1A=35999)

*/
void init_timer1()
{
    TCCR1B |= (1 << WGM12);                 // set for clear timer on compare
    TIMSK1 |= (1 << OCIE1A);                // Enable output compare A Match Interrupt
    OCR1A = 35999;                          // Interrupt every 5 seconds
    TCCR1B |= ((1 << CS12) | (1<< CS10));   // ^
}

/* Entered every 5 seconds */
ISR(TIMER1_COMPA_vect)
{
    eeprom_timer_count++;
}