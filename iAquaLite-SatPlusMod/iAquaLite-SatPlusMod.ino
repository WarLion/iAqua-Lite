// iAqua Lite - Aquarium Lighting Controller for the Ecoxotic E-Series
// Written by Dan Cunningham, aka AnotherHobby @ plantedtank.net
//
// The full thread for this code can be foudn at:
//              http://www.plantedtank.net/forums/showthread.php?t=783426
//
// Much of the code was taken from my iAqua full touch screen aquarium controller:
//              http://www.plantedtank.net/forums/showthread.php?t=677265
//
// Some code was swiped, modified, and integrated or otherwise inspired from other public works
// All code is public domain, feel free to use, abuse, edit, and share
// Written for Arduino Mega 2560
//
// VERSION:  1.0
// - initial version, completed November 16, 2014.
//
// NEW VERSION: 1.0b : KMAN - FIXES AND CHANGES MADE BY kman PER scaLLas AND xtremebassist CODE-CHECK FIXES AS OF MARCH 17, 2015
// NEW VERSION: 1.0c : KMAN - FIXES AND CHANGES MADE BY kman PER scaLLas AND xtremebassist CODE-CHECK FIXES AS OF MARCH 17, 2015 - CONVERT to SAT+ Codes
// 
// REMINDER: IRRemote Library: IRremoteInt.h: Change to use Mega pin 46:
// #define IR_USE_TIMER5 // tx = pin 46
// 
// Prerequisites: Program your light values into the stored memory positions on your remote.
// E-Series: Moon (lowest light), then M1 (next level up), then M2 (next level up), then Daylight.
// WRGB Values for each color range from 0-100%
//
// Sat+: Moon=M4 (lowest light), then M3 (next level up), then M2 (next level up), then Daylight=M1.
// WRGB Values for each color range from 0-42.


/*--------------------------------------------------------------------------------------
 Includes
 --------------------------------------------------------------------------------------*/
#include <EEPROM.h>  // used to store and retrieve settings from memory
#include <DS1302RTC.h> // library for the DS1302 RTC
#include <Time.h>  // gives us all utilities to manipulate date/time
#include <TimeAlarms.h>  // allows us to easily set timers (scheduling)
#include <IRremote.h> // gives the ability to control an IR LED
#include <LiquidCrystal.h>   // controls the LCD display
#include <MenuBackend.h>  // helps with menu system to navigate the display

/*--------------------------------------------------------------------------------------
 Defines
 --------------------------------------------------------------------------------------*/
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         10  // D10 controls LCD backlight

//BEGIN KMAN CODE CHANGE per xtremebassist
#define IR_TRANS_5V_PIN           44 // Power to IR transmitter
#define IR_TRANS_GND_PIN          42 // GND for IR transmitter 
//END KMAN CODE CHANGE per xtremebassist

// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          505  // left
#define SELECT_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if ( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }

// define pins for DS1302 clock
// Set pins:  CE, IO,CLK
DS1302RTC RTC(27, 29, 31);
#define DS1302_GND_PIN 33
#define DS1302_VCC_PIN 35


// these are used to keep track of every single editable field while editing
#define EDIT_NOTHING			0
#define EDIT_BRIGHTNESS			1
#define EDIT_DAYWHITE			2
#define EDIT_DAYRED			3
#define EDIT_DAYGREEN			4
#define EDIT_DAYBLUE			5
#define EDIT_MOONWHITE			6
#define EDIT_MOONRED			7
#define EDIT_MOONGREEN			8
#define EDIT_MOONBLUE			9
#define EDIT_M1WHITE			10
#define EDIT_M1RED			11
#define EDIT_M1GREEN			12
#define EDIT_M1BLUE			13
#define EDIT_M2WHITE			14
#define EDIT_M2RED			15
#define EDIT_M2GREEN			16
#define EDIT_M2BLUE			17
#define EDIT_THOUR			18
#define EDIT_TMIN			19
#define EDIT_TSEC			20
#define EDIT_TMONTH			21
#define EDIT_TDAY			22
#define EDIT_TYEAR			23
#define EDIT_12HR			24
#define EDIT_DATE_FMT			25
#define EDIT_F1_ONHOUR			26
#define EDIT_F1_ONMINUTE		27
#define EDIT_F1_DURATIONHOUR		28
#define EDIT_F1_DURATIONMIN		29
#define EDIT_F2_ONHOUR			30
#define EDIT_F2_ONMINUTE		31
#define EDIT_F2_DURATIONHOUR		32
#define EDIT_F2_DURATIONMIN		33
#define EDIT_F3_ONHOUR			34
#define EDIT_F3_ONMINUTE		35
#define EDIT_F3_DURATIONHOUR		36
#define EDIT_F3_DURATIONMIN		37
#define EDIT_F4_ONHOUR			38
#define EDIT_F4_ONMINUTE		39
#define EDIT_F4_DURATIONHOUR		40
#define EDIT_F4_DURATIONMIN		41
#define EDIT_F5_ONHOUR			42
#define EDIT_F5_ONMINUTE		43
#define EDIT_F5_DURATIONHOUR		44
#define EDIT_F5_DURATIONMIN		45
#define EDIT_F6_ONHOUR			46
#define EDIT_F6_ONMINUTE		47
#define EDIT_F6_DURATIONHOUR		48
#define EDIT_F6_DURATIONMIN		49
#define EDIT_L_ONHOUR			50
#define EDIT_L_ONMINUTE			51
#define EDIT_L_OFFHOUR			52
#define EDIT_L_OFFMINUTE		53

int EDIT_FIELD=0;  // edit field starts at nothing

// defines for some custom LCD symbols
byte upArrow[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte dnArrow[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

byte rtArrow[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
};

IRsend irsend;

/*--------------------------------------------------------------------------------------
 Variables
 --------------------------------------------------------------------------------------*/

unsigned long prevMillis5sec = 0; // track 5 seconds for refreshing clock and temp
unsigned long currentMillis = 0; // track current millis
unsigned long prevMillisHalfsec = 0; // track current millis

boolean editingActive = false; // track if we are actively editing a value
boolean editingAvailable = false; // track if we click into a menu where editing is available

// these are used for storing some strings in progmem
static FILE uartout = {
  0};   // UART FILE structure
static FILE lcdout = {
  0};   // LCD FILE structure

// track if we are in a sub menu
boolean subMenu = false;

// load the saved backLight value from eeprom
byte backLight=EEPROM.read(31);

// used to display month names on the LCD
char *Mon[] = {
  "","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

boolean _24hr = false; // holds the setting for a 24hr clock
boolean dateMonthFirst = true; // holds the setting for dd/mm/yyyy or mm/dd/yyyy


struct RTC_T  // used for time
{  
  int tHour;
  int tMinute;
  int tSecond;
  boolean tAM;
  int tDow;
  int tDay;
  int tMonth;
  int tYear;
} 
prevRTC, saveRTC;

struct WRGB  // for storing light intensity values
{
  int White;
  int Red;
  int Green;
  int Blue;
};
typedef struct WRGB LightColor;

LightColor currentColor = {
  0,0,0,0}; // The current color of the light (used for fading)
LightColor lastColor = {
  0,0,0,0};   // The previous color of the light (used for fading)
LightColor targetColor = {
  0,0,0,0}; // The target color of the light (used for fading)

// these will store the 4 saved light modes using the WRGB struct above
LightColor lightDaylight;
LightColor lightMoon;
LightColor lightM1;
LightColor lightM2;

byte currentLightMode=0;  //0=daylight, 1=sunrise or sunset, 2=dawn or dusk, 3=moon, 4=transition, 5=off
byte lastFade = 0; // track which fade was last

// holds the schedule for each light fade and the schedule for power on/off
struct SCHED
{  
  byte onHour;
  byte onMinute;
  byte offHour; // only used for light power
  byte offMinute; // only used for light power
  byte durationHours; // only used for fades
  byte durationMinutes; // only used for fades
} 
fadeT, fade1, fade2, fade3, fade4, fade5, fade6, lightPower, lightPowerT; 
// fadeT and lightPowerT are for temporarily storing values during edits

// this stores the alarm ID's so we can delete them and recreate them
byte fade1alarmID;
byte fade2alarmID;
byte fade3alarmID;
byte fade4alarmID;
byte fade5alarmID;
byte fade6alarmID;
byte lightPowerOnAlarmID;
byte lightPowerOffAlarmID;

// the next 6 variables will be set at the start of a lighting fade and used to track it
int fadeDurationSeconds = 0; 
unsigned long fadeStartingSeconds = 0;
unsigned long fadeTimeLeft;
boolean fadeInProgress = false;
byte fadeFromMode = 0;  //0=daylight, 1=dawn or dusk, 2=sunrise or sunset, 3=moon
byte fadeToMode = 0;  //0=daylight, 1=dawn or dusk, 2=sunrise or sunset, 3=moon

//this starts a MenuBackend instance that controls the menu and the event generation
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);

/*--------------------------------------------------------------------------------------
 Menu items below are delcared first, and will be "built" and organized later
 --------------------------------------------------------------------------------------*/

// top menu items
MenuItem miHOME = MenuItem("HOME");
MenuItem miFSCHED = MenuItem("FADE SCHEDULES");
MenuItem miPSCHED = MenuItem("POWER SCHEDULE");
MenuItem miWRGB = MenuItem("WRGB VALUES");
MenuItem miTIDA = MenuItem("SET DATE/TIME");
MenuItem miSC = MenuItem("SCREEN SETUP");

// menus for saved colors
MenuItem miDAYLIGHT = MenuItem("WRGB DAYLIGHT");
MenuItem miMOON = MenuItem("WRGB MOON");
MenuItem miM1 = MenuItem("WRGB M3 DWN/DSK"); //KMAN- adjust for Sat+
MenuItem miM2 = MenuItem("WRGB M2 RIS/SET"); //KMAN- adjust for Sat+

// KMAN CHANGE: Change M1 (for E-Series) to M3 (for Sat+ below)
// fade menu for fade schedules
MenuItem miF1 = MenuItem("MOON > M3 DAWN");
MenuItem miF2 = MenuItem("M3 DWN > M2 RSE");
MenuItem miF3 = MenuItem("M2 RISE > DAY");
MenuItem miF4 = MenuItem("DAY > M2 SET");
MenuItem miF5 = MenuItem("M2 SET > M3 DSK");
MenuItem miF6 = MenuItem("M3 DUSK > MOON");

// power schedule menu
MenuItem miPOWER = MenuItem("LIGHT ON / OFF");

// set date and time menu
MenuItem miTIME = MenuItem("SET TIME");
MenuItem miDATE = MenuItem("SET DATE");
MenuItem mi12HR = MenuItem("TIME FORMAT");
MenuItem miDATEFMT = MenuItem("DATE FORMAT");

// SCREEN MENU
MenuItem miSCBR = MenuItem("BRIGHTNESS");


// Init the LCD library with the LCD pins to be used (this is for the LinkSprite LCD)
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );  

/*
// REMOTE CONTROL CODES FOR ECOXOTIC E-SERIES USED IN THIS SKETCH
const unsigned long POWER = 0x20DF02FD;
const unsigned long REDUP = 0x20DF0AF5;
const unsigned long REDDOWN = 0x20DF38C7;
const unsigned long GREENUP = 0x20DF8A75;
const unsigned long GREENDOWN = 0x20DFB847;
const unsigned long BLUEUP = 0x20DFB24D;
const unsigned long BLUEDOWN = 0x20DF7887;
const unsigned long WHITEUP = 0x20DF32CD;
const unsigned long WHITEDOWN = 0x20DFF807;
const unsigned long DAYLIGHT = 0x20DF58A7;
const unsigned long M2 = 0x20DF9867;
const unsigned long M1 = 0x20DF18E7;
const unsigned long MOON = 0x20DFD827;

// REMOTE CONTROL CODES FOR ECOXOTIC E-SERIES NOT USED IN THIS SKETCH (KEPT FOR FUTURE USE)
const unsigned long SETCLOCK = 0x20DF3AC5;
const unsigned long ONTIME = 0x20DFBA45;
const unsigned long OFFTIME = 0x20DF827D;
const unsigned long HOURUP = 0x20DF1AE5;
const unsigned long MINUTEDOWN = 0x20DF9A65;
const unsigned long ENTER = 0x20DFA25D;
const unsigned long RESUME = 0x20DF22DD;
const unsigned long SUNLIGHT = 0x20DF2AD5;
const unsigned long CRISPBLUE = 0x20DF926D;
const unsigned long FULLSPECTRUM = 0x20DFAA55;
const unsigned long DEEPWATER = 0x20DF12ED;
const unsigned long DYNAMICMOON = 0x20DF28D7;
const unsigned long DYNAMICLIGHTNING = 0x20DFA857;
const unsigned long DYNAMICCLOUD = 0x20DF6897;
const unsigned long DYNAMICFADE = 0x20DFE817;
*/

// REMOTE CONTROL CODES FOR CURRENT SATELLITE PLUS

/* // KMAN SAT+ CODE MOD:
NOTE: E-Series and Sat+ use different terminology.  Use 4 memory positions of Sat+ to emulate presets in E-Series, changing as little code as possible.

E-Series Daylight should map to Sat+ M1
E-Series M2       should map to Sat+ M2
E-Series M1       should map to Sat+ M3
E-Series Moon     should map to Sat+ M4

*/

const unsigned long POWER = 0x20DF02FD;  // SAME AS E-Series
const unsigned long REDUP = 0x20DF2AD5;   // E-Series = SUNLIGHT
const unsigned long REDDOWN = 0x20DF0AF5; // E-Series = REDUP
const unsigned long GREENUP = 0x20DFAA55; // E-Series = FULLSPECTRUM
const unsigned long GREENDOWN = 0x20DF8A75; // E-Series = GREENUP
const unsigned long BLUEUP = 0x20DF926D; // E-Series = CRISPBLUE
const unsigned long BLUEDOWN = 0x20DFB24D; // E-Series = BLUEUP
const unsigned long WHITEUP = 0x20DF12ED; // E-Series = DEEPWATER
const unsigned long WHITEDOWN = 0x20DF32CD; // E-Series = WHITEUP
const unsigned long DAYLIGHT = 0x20DF38C7; // Sat+ M1 maps to iAquaLite Daylight // E-Series = REDDOWN
const unsigned long M2 = 0x20DFB847;       // Sat+ M2 maps to iAquaLite M2 (same term, different light position) // E-Series = GREENDOWN
const unsigned long M1 = 0x20DF7887;       // Sat+ M3 maps to iAquaLite M1 // E-Series = BLUEDOWN
const unsigned long MOON = 0x20DFF807;     // Sat+ M4 maps to iAquaLite Moon // E-Series = WHITEDOWN

// SAT+ CODES NOT USED IN THIS SKETCH (KEPT FOR FUTURE USE)
const unsigned long FULLORANGE = 0x20DF3AC5; // E-Series = SETCLOCK
const unsigned long FULLLIGHTBLUE = 0x20DFBA45; // E-Series = ONTIME
const unsigned long FULLPURPLE = 0x20DF827D; // E-Series = OFFTIME
const unsigned long FULLWHITE = 0x20DF1AE5; // E-Series = HOURUP
const unsigned long FULLYELLOW = 0x20DF9A65; // E-Series = MINUTEDOWN
const unsigned long FULLBLUE = 0x20DFA25D; // E-Series = ENTER
const unsigned long MOONLIGHT = 0x20DF18E7; // E-Series = M1 
const unsigned long MOONDARK = 0x20DF9867; // E-Series = M2
const unsigned long MOONCLOUDS = 0x20DF58A7; // E-Series = DAYLIGHT
const unsigned long SUNRISE = 0x20DFD827; // E-Series = MOON
const unsigned long CLOUDS1 = 0x20DF28D7; // E-Series = DYNAMICMOON
const unsigned long CLOUDS2 = 0x20DFA857; // E-Series = DYNAMICLIGHTNING
const unsigned long CLOUDS3 = 0x20DF6897; // E-Series = DYNAMICCLOUD
const unsigned long CLOUDS4 = 0x20DFE817; // E-Series = DYNAMICFADE
const unsigned long CLOUDS5 = 0x20DFC837;
const unsigned long STORM1 = 0x20DF08F7;
const unsigned long STORM2 = 0x20DF8877;
const unsigned long STORM3 = 0x20DF48B7;

void setup()
{

  //lcd backlight control
  pinMode( LCD_BACKLIGHT_PIN, OUTPUT );

  // Activate RTC module
  digitalWrite(DS1302_GND_PIN, LOW);
  pinMode(DS1302_GND_PIN, OUTPUT);

  digitalWrite(DS1302_VCC_PIN, HIGH);
  pinMode(DS1302_VCC_PIN, OUTPUT);
  
  //BEGIN KMAN CODE CHANGE - code change per xtremebassist
  // Activate IR emmitter
  digitalWrite(IR_TRANS_GND_PIN, LOW);
  pinMode(IR_TRANS_GND_PIN, OUTPUT);

  digitalWrite(IR_TRANS_5V_PIN, HIGH);
  pinMode(IR_TRANS_5V_PIN, OUTPUT); 
  //END KMAN CODE CHANGE

  // this starts the backlight off for a second, allows for the startup fade in
  analogWrite(LCD_BACKLIGHT_PIN, 0);
  delay(1000); // only here for visual effect, if you prefer you can comment this line out and you'll start up 1 second faster

  // if the backlight value is out of scope (first startup or not set), then we set it to 10
  if ((backLight > 10)||(backLight < 1)||(backLight == NULL)) {
    backLight=10;
    EEPROM.write(31,10);
  }

  // create the 3 custom icons for the CLD
  lcd.createChar(0, upArrow);
  lcd.createChar(1, dnArrow);
  lcd.createChar(2, rtArrow);

  // start some necessary stuff
  Serial.begin(9600);
  //Wire.begin();
  RTC.get();
  lcd.begin(16, 2);  

  // sync the RTC to the Arduino
  setSyncProvider(syncProvider);

  // for troubleshooting, output to serial if the RTC is running or not
  if (! RTC.haltRTC()) 
  {
    // If no RTC is installed, alert on serial
    Serial.println("RTC is stopped!\n");  // Store this string in PROGMEM
  }
  else Serial.println("RTC is running!\n");  // Store this string in PROGMEM

   //Serial.println(TIMER_PWM_PIN);
  //button adc input - all buttons are on 1 pin
  pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0

  // fill in the UART file descriptor with pointer to writer, this is for storing and recalling strings in progmem
  fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;  // Output stdout to UART

  // fill in lcd file descriptor (we'll use fprintf to output to it)  
  fdev_setup_stream (&lcdout, lcd_putchar, NULL, _FDEV_SETUP_WRITE);

  // startup screen
  lcd.setCursor(3,0);
  fprintf(&lcdout, "iAqua Lite");  // Print the time HH:MM to the lcd
  lcd.setCursor(0,1);
  fprintf(&lcdout, "Smart Startup...");  // Print the time HH:MM to the lcd

  byte x;

  // do a nice backlight fade in for the startup screen
  // it's only here for visual effect, if you prefer you can comment this block out and you'll start up faster
  // but you won't have a cool startup screen. :)
  x=0;
  while (x<200) 
  {
    analogWrite(LCD_BACKLIGHT_PIN, x);
    x++;
    delay(15);
  }


  // set up the time display as 12 or 24 hour
  byte timeFormat=(EEPROM.read(50)); // read from EEPROM
  if (timeFormat==0) _24hr=false; // 0 means 12 hour clock
  else if (timeFormat==1) _24hr=true; // 1 means 24 hour clock
  else {
    _24hr=false; // if it's not 0 or 1, it hasn't been set right, so we default to 12
    EEPROM.write(50,0); // save default to EEPROM
  }

  // set up the date format to be day first or month first
  byte dateFormat=(EEPROM.read(51)); // read from EEPROM
  if (dateFormat==0) dateMonthFirst=true; // 0 means MM/DD/YYYY
  else if (dateFormat==1) dateMonthFirst=false; // 1 mean DD/MM/YYYY
  else {
    dateMonthFirst=true; // if it's not 0 or 1, it hasn't been set right, so we default to US format of MM/DD/YYYY
    EEPROM.write(51,0); // save default to EEPROM
  }

  // create LCD menu structure
  menuSetup();

  // load in all of the fade schedules and create alarm events for them
  loadFade1();
  loadFade2();
  loadFade3();
  loadFade4();
  loadFade5();
  loadFade6();

  // load up all of the WRGB values for the stored light settings
  loadLightWRGB();

  // update the schedule for powering the lghts on and off
  loadPowerSchedule();

  // start up the lights in the correct mode based on time of day and fade schedule
  smartStartup();

  menu.moveDown(); // this starts the menu at "home"

  drawScreen(); // display the home screen

    //fade back down to the preferred user defined level
  x=255;
  while (x>backLight*25) 
  {
    analogWrite(LCD_BACKLIGHT_PIN, x);
    x--;
    delay(15);
  }

  // Print available SRAM for debugging, comment out if you want
  printf_P(PSTR("SRAM: %d\n"), freeRam());

}

void loop()
{

  // Get the UNIX time in seconds (since 1970), using UNIX time because the calculations are MUCH easier
  unsigned long rightNow = now();

  currentMillis = millis(); // get current millis

    processButtonActions(); // handle any button presses

    // update home screen every 5 seconds
  if (currentMillis - prevMillis5sec > 5000) {
    if (menu.getCurrent().getName() == "HOME") drawScreen();
    prevMillis5sec=millis();
  }

  // update time setting screen every .5 seconds so that the clock is live
  if (menu.getCurrent().getName() == "SET TIME") {
    if (currentMillis - prevMillisHalfsec > 500) {
      drawScreen();
      prevMillisHalfsec=millis();
    }
  }

  // check on the fading of the lights  
  if (fadeInProgress==true)
  { 
    if (rightNow > fadeStartingSeconds + fadeDurationSeconds)   // if we have just finished the fade...
    {
      fadeInProgress = false;
      currentLightMode=fadeToMode;  // If a color fade has been completed, set the final mode

        // we send a hard code to lock in the setting and avoid drift
      if (currentLightMode == 0) irsend.sendNEC(DAYLIGHT,32);
      else if (currentLightMode == 1) irsend.sendNEC(M2,32);
      else if (currentLightMode == 2) irsend.sendNEC(M1,32);
      else if (currentLightMode == 3) irsend.sendNEC(MOON,32);
    }
    else   // if the fade is still running
    {
      // If there is a fade in progress, check if there are any IR commands that should be sent
      checkLightFade(rightNow - fadeStartingSeconds, fadeDurationSeconds);
    }
  }

  Alarm.delay(10);   // Service alarms & wait (msec), we loop very quickly so we don't miss button presses

}

void menuSetup()
{

  fprintf_P(&lcdout, PSTR("Setting up menu..."));

  // create main menu
  menu.getRoot().add(miHOME);

  // add it all top level menus
  miHOME.addRight(miFSCHED).addRight(miPSCHED).addRight(miWRGB).addRight(miTIDA).addRight(miSC);

  // add fade schedule sub menus to the fade menu
  miFSCHED.add(miF1).addRight(miF2).addRight(miF3).addRight(miF4).addRight(miF5).addRight(miF6);

  // add power schedule sub menu
  miPSCHED.add(miPOWER);

  // add saved lighting value sub menus
  miWRGB.add(miMOON).addRight(miM1).addRight(miM2).addRight(miDAYLIGHT);

  // add date/time sub menus
  miTIDA.add(miTIME).addRight(mi12HR).addRight(miDATEFMT).addRight(miDATE);

  // add screen sub menu
  miSC.add(miSCBR);

}

time_t syncProvider()
{
  return RTC.get();
}

// Output function for stdout redirect for saving strings in progmem
static int uart_putchar (char c, FILE *stream)
{
  // Serial write fumction
  Serial.write(c);
  return 0;
}

// Output function for lcd output for saving strings in progmem
static int lcd_putchar(char ch, FILE* stream)
{
  // lcd write function
  lcd.write(ch);
  return (0);
}


void updatePowerSchedule() // called when the users saves the power schedule
{
  EEPROM.write(101, lightPowerT.onHour);
  EEPROM.write(102, lightPowerT.onMinute);
  EEPROM.write(103, lightPowerT.offHour);
  EEPROM.write(104, lightPowerT.offMinute);
  loadPowerSchedule(); // activates the new schedule
}


void loadPowerSchedule()
{

  lightPower.onHour = EEPROM.read(101); //load the schedule from EEPROM
  // if value is out of scope (first startup or not set), set it to 0
  if (lightPower.onHour > 23) {
    lightPower.onHour = 0;
    EEPROM.write(101, lightPower.onHour);
  }

  lightPower.onMinute = EEPROM.read(102); //load the schedule from EEPROM
  // if value is out of scope (first startup or not set), set it to 0
  if (lightPower.onMinute > 59) {
    lightPower.onMinute = 0;
    EEPROM.write(102, lightPower.onMinute);
  }

  lightPower.offHour = EEPROM.read(103); //load the schedule from EEPROM
  // if value is out of scope (first startup or not set), set it to 0
  if (lightPower.offHour > 23) {
    lightPower.offHour = 0;
    EEPROM.write(103, lightPower.offHour);
  }

  //BEGIN KMAN CHANGE CODE
//  lightPower.offHour = EEPROM.read(104); //load the schedule from EEPROM - BAD CODE per scaLLas
  lightPower.offMinute = EEPROM.read(104); //load the schedule from EEPROM - CORRECTED CODE per scaLLas
  //END KMAN CHANGE CODE

  // if value is out of scope (first startup or not set), set it to 0
  if (lightPower.offMinute > 59) {
    lightPower.offMinute = 0;
    EEPROM.write(104, lightPower.offMinute);
  }

  // remove alarms first if they already exist, this would be if a user modifies the schedule
  if (lightPowerOnAlarmID != NULL) Alarm.free(lightPowerOnAlarmID);
  if (lightPowerOffAlarmID != NULL) Alarm.free(lightPowerOffAlarmID);

  if ((lightPower.onHour==lightPower.offHour)&&(lightPower.onMinute==lightPower.offMinute)) {
    // nothing happens here because...
    // we do NOT load the power schedules if the on/off times are set the same
    // this effectively allows the user to not power the lights on/off
  }
  else {

    // alarm to turn the lights on
    lightPowerOnAlarmID = Alarm.alarmRepeat(lightPower.onHour,lightPower.onMinute,0, AlarmLightPower);

    // alarm to turn the lights off
    lightPowerOffAlarmID = Alarm.alarmRepeat(lightPower.offHour,lightPower.offMinute,0, AlarmLightPower);

  }
}

void updateFade1() // for saving new fade values
{

  EEPROM.write(220, fadeT.onHour);
  EEPROM.write(221, fadeT.onMinute);
  EEPROM.write(222, fadeT.durationHours);
  EEPROM.write(223, fadeT.durationMinutes);
  loadFade1();
}


void updateFade2() // for saving new fade values
{
  EEPROM.write(224, fadeT.onHour);
  EEPROM.write(225, fadeT.onMinute);
  EEPROM.write(226, fadeT.durationHours);
  EEPROM.write(227, fadeT.durationMinutes);
  loadFade2();
}

void updateFade3() // for saving new fade values
{
  EEPROM.write(228, fadeT.onHour);
  EEPROM.write(229, fadeT.onMinute);
  EEPROM.write(230, fadeT.durationHours);
  EEPROM.write(231, fadeT.durationMinutes);
  loadFade3();
}

void updateFade4() // for saving new fade values
{
  EEPROM.write(232, fadeT.onHour);
  EEPROM.write(233, fadeT.onMinute);
  EEPROM.write(234, fadeT.durationHours);
  EEPROM.write(235, fadeT.durationMinutes);
  loadFade4();
}

void updateFade5() // for saving new fade values
{
  EEPROM.write(236, fadeT.onHour);
  EEPROM.write(237, fadeT.onMinute);
  EEPROM.write(238, fadeT.durationHours);
  EEPROM.write(239, fadeT.durationMinutes);
  loadFade5();
}

void updateFade6() // for saving new fade values
{
  EEPROM.write(240, fadeT.onHour);
  EEPROM.write(241, fadeT.onMinute);
  EEPROM.write(242, fadeT.durationHours);
  EEPROM.write(243, fadeT.durationMinutes);
  loadFade6();
}


void loadFade1()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade1.onHour = EEPROM.read(220);
  if (fade1.onHour > 23) {
    fade1.onHour = 6;
    EEPROM.write(220, fade1.onHour);
  }
  fade1.onMinute = EEPROM.read(221);
  if (fade1.onMinute > 59) {
    fade1.onMinute = 0;
    EEPROM.write(221, fade1.onMinute);
  }
  fade1.durationHours = EEPROM.read(222);
  if (fade1.durationHours > 23) {
    fade1.durationHours = 1;
    EEPROM.write(222, fade1.durationHours);
  }
  fade1.durationMinutes = EEPROM.read(223);
  if (fade1.durationMinutes > 59) {
    fade1.durationMinutes = 59;
    EEPROM.write(223, fade1.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade1alarmID != NULL) Alarm.free(fade1alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade1alarmID = Alarm.alarmRepeat(fade1.onHour,fade1.onMinute,0,AlarmFade1);

} 

void loadFade2()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade2.onHour = EEPROM.read(224);
  if (fade2.onHour > 23) {
    fade2.onHour = 8;
    EEPROM.write(224, fade2.onHour);
  }
  fade2.onMinute = EEPROM.read(225);
  if (fade2.onMinute > 59) {
    fade2.onMinute = 0;
    EEPROM.write(225, fade2.onMinute);
  }
  fade2.durationHours = EEPROM.read(226);
  if (fade2.durationHours > 23) {
    fade2.durationHours = 2;
    EEPROM.write(226, fade2.durationHours);
  }
  fade2.durationMinutes = EEPROM.read(227);
  if (fade2.durationMinutes > 59) {
    fade2.durationMinutes = 0;
    EEPROM.write(227, fade2.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade2alarmID != NULL) Alarm.free(fade2alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade2alarmID = Alarm.alarmRepeat(fade2.onHour,fade2.onMinute,0,AlarmFade2);

}

void loadFade3()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade3.onHour = EEPROM.read(228);
  if (fade3.onHour > 23) {
    fade3.onHour = 11;
    EEPROM.write(228, fade3.onHour);
  }
  fade3.onMinute = EEPROM.read(229);
  if (fade3.onMinute > 59) {
    fade3.onMinute = 30;
    EEPROM.write(229, fade3.onMinute);
  }
  fade3.durationHours = EEPROM.read(230);
  if (fade3.durationHours > 23) {
    fade3.durationHours = 1;
    EEPROM.write(230, fade3.durationHours);
  }
  fade3.durationMinutes = EEPROM.read(231);
  if (fade3.durationMinutes > 59) {
    fade3.durationMinutes = 0;
    EEPROM.write(231, fade3.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade3alarmID != NULL) Alarm.free(fade3alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade3alarmID = Alarm.alarmRepeat(fade3.onHour,fade3.onMinute,0,AlarmFade3);


}

void doNothing()
{
}

void loadFade4()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade4.onHour = EEPROM.read(232);
  if (fade4.onHour > 23) {
    fade4.onHour = 20;
    EEPROM.write(232, fade4.onHour);
  }
  fade4.onMinute = EEPROM.read(233);
  if (fade4.onMinute > 59) {
    fade4.onMinute = 0;
    EEPROM.write(233, fade4.onMinute);
  }
  fade4.durationHours = EEPROM.read(234);
  if (fade4.durationHours > 23) {
    fade4.durationHours = 0;
    EEPROM.write(234, fade4.durationHours);
  }
  fade4.durationMinutes = EEPROM.read(235);
  if (fade4.durationMinutes > 59) {
    fade4.durationMinutes = 59;
    EEPROM.write(235, fade4.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade4alarmID != NULL) Alarm.free(fade4alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade4alarmID = Alarm.alarmRepeat(fade4.onHour,fade4.onMinute,0,AlarmFade4);

}

void loadFade5()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade5.onHour = EEPROM.read(236);
  if (fade5.onHour > 23) {
    fade5.onHour = 21;
    EEPROM.write(236, fade5.onHour);
  }
  fade5.onMinute = EEPROM.read(237);
  if (fade5.onMinute > 59) {
    fade5.onMinute = 0;
    EEPROM.write(237, fade5.onMinute);
  }
  fade5.durationHours = EEPROM.read(238);
  if (fade5.durationHours > 23) {
    fade5.durationHours = 0;
    EEPROM.write(238, fade5.durationHours);
  }
  fade5.durationMinutes = EEPROM.read(239);
  if (fade5.durationMinutes > 59) {
    fade5.durationMinutes = 59;
    EEPROM.write(239, fade5.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade5alarmID != NULL) Alarm.free(fade5alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade5alarmID = Alarm.alarmRepeat(fade5.onHour,fade5.onMinute,0,AlarmFade5);

}

void loadFade6()
{
  // if values loaded from memory below are out of scope (first startup or not set), 
  // it will set it to a suggested time

  fade6.onHour = EEPROM.read(240);
  if (fade6.onHour > 23) {
    fade6.onHour = 22;
    EEPROM.write(240, fade6.onHour);
  }
  fade6.onMinute = EEPROM.read(241);
  if (fade6.onMinute > 59) {
    fade6.onMinute = 0;
    EEPROM.write(241, fade6.onMinute);
  }
  fade6.durationHours = EEPROM.read(242);
  if (fade6.durationHours > 23) {
    fade6.durationHours = 1;
    EEPROM.write(242, fade6.durationHours);
  }
  fade6.durationMinutes = EEPROM.read(243);
  if (fade6.durationMinutes > 59) {
    fade6.durationMinutes = 0;
    EEPROM.write(243, fade6.durationMinutes);
  }

  // remove alarm first if already exists
  if (fade6alarmID != NULL) Alarm.free(fade6alarmID);

  //////// CREATE fade ALARM BASED ON SCHEDULE ////////
  fade6alarmID = Alarm.alarmRepeat(fade6.onHour,fade6.onMinute,0,AlarmFade6);

}


/*--------------------------------------------------------------------------------------
 processButtonActions()
 
 Here we process all of the button clicks and pass them off to other handlers depending on context.
 The most commmon context is what menu we are actively in.
 If we are editing, pass it off to the handler for editing.
 If we are in a menu and not editing, move through the menu as expected.
 --------------------------------------------------------------------------------------*/

void processButtonActions()
{

  byte button;

  //get the latest button pressed
  button = ReadButtons();

  // respond to each button differently
  switch( button )
  {
  case BUTTON_NONE:
    {
      break;
    }
  case BUTTON_RIGHT:
    {
      if (editingActive == true) editButtons(BUTTON_RIGHT);
      else {
        menu.moveDown();
        drawScreen();
        delay(250);
      }
      break;
    }
  case BUTTON_UP:
    {
      if (editingActive == true) editButtons(BUTTON_UP);
      else {
        menu.moveLeft(); 
        drawScreen();
        delay(250);
      }
      break;
    }
  case BUTTON_DOWN:
    {
      if (editingActive == true) editButtons(BUTTON_DOWN);
      else {
        menu.moveRight();
        drawScreen();
        delay(250);
      }
      break;
    }
  case BUTTON_LEFT:
    {
      if (editingActive == true) editButtons(BUTTON_LEFT);
      else {

        if (menu.getCurrent().getName() != "HOME") { // don't move up from home, this is a nuance of the menubackend library

          /// this code makes it so that if you click left in a top menu other than home, it'll bring you home
          boolean dontMoveUp=false;
          if ((menu.getCurrent().getName() == "FADE SCHEDULES")||
            (menu.getCurrent().getName() == "WRGB VALUES")||
            (menu.getCurrent().getName() == "SET DATE/TIME")||
            (menu.getCurrent().getName() == "SCREEN SETUP")||
            (menu.getCurrent().getName() == "POWER SCHEDULE")) {
            while (menu.getCurrent().getName() != "HOME") {
              delay(1); // for some reason it needs a tiny delay or it won't execute
              menu.moveLeft();
            }
            dontMoveUp=true; // don't move up any further
            drawScreen();
          }

          // A nuance of the menubackend library is that it won't go back until you are at the top of a menu tree.
          // Below is a loop routine to make the system return to the previous menu when you hit "left" no matter 
          // where you are in the menu so you don't have to go up to the "top" menu item.

          else if (subMenu == true) { // if we are in a sub menu, we need to move left until we get to the beginning
            while (menu.getCurrent().getBefore()->getName() == NULL) { // this will move left until we hit the far left
              delay(1); // for some reason it needs a tiny delay or it won't execute
              menu.moveLeft();
            }
            subMenu=false;
          }
          if (dontMoveUp==false) {
            menu.moveUp();
            drawScreen();
          }
          delay(250);   
          //}
        }
      }
      break;
    }
  case BUTTON_SELECT:
    {
      // if the select button is pressed in the time or date menu, and we are editing, set the RTC immediately
      if (menu.getCurrent().getName() == "SET TIME") {
        if ( editingActive == true ) {
          if (_24hr==false) {
            if (saveRTC.tAM==false) {
              if (saveRTC.tHour!=12)
                saveRTC.tHour=saveRTC.tHour+12;
            } 
          }

          time_t t;
          tmElements_t tm;
          tm.Year = CalendarYrToTm(year());
          tm.Month = month();
          tm.Day = day();
          tm.Hour = saveRTC.tHour;
          tm.Minute = saveRTC.tMinute;
          tm.Second = saveRTC.tSecond;
          t = makeTime(tm);

          if(RTC.set(t) == 0) setTime(t); // set RTC if successful

          setSyncProvider(syncProvider);
        }
      }
      
      if (menu.getCurrent().getName() == "SET DATE") {
        if ( editingActive == true ) {
          
          time_t t;
          tmElements_t tm;
          tm.Year = CalendarYrToTm(saveRTC.tYear);
          tm.Month = saveRTC.tMonth;
          tm.Day = saveRTC.tDay;
          tm.Hour = hour();
          tm.Minute = minute();
          tm.Second = second();
          t = makeTime(tm);
          //BEGIN KMAN CHANGE CODE - code change per xtremebassist
          if(RTC.set(t) == 0) setTime(t); // set RTC if successful
          //END KMAN CHANGE CODE
          
          setSyncProvider(syncProvider);
        }
      }

      // SAVE POWER SCHEDULE
      if (menu.getCurrent().getName() == "LIGHT ON / OFF") updatePowerSchedule();

// KMAN CHANGE: Rename M1 (E-Series) to M3 (Sat+)
      // SAVE RAMP SCHEDULES
      if (menu.getCurrent().getName() == "MOON > M3 DAWN") updateFade1();
      if (menu.getCurrent().getName() == "M3 DWN > M2 RSE") updateFade2();
      if (menu.getCurrent().getName() == "M2 RISE > DAY") updateFade3();
      if (menu.getCurrent().getName() == "DAY > M2 SET") updateFade4();
      if (menu.getCurrent().getName() == "M2 SET > M3 DSK") updateFade5();
      if (menu.getCurrent().getName() == "M3 DUSK > MOON") updateFade6();

      //this is for debugging, simply print free SRAM to serial out when the select button is pushed on the home screen
      if (menu.getCurrent().getName() == "HOME") printf_P(PSTR("SRAM: %d\n"), freeRam());

      // the block below is all for drawing the arrows in the menu's
      // when switching in/out of edit mode, we make the up/down arrows disappear or appear
      // and make the up/down arrows dissappear based on where we are in the menu when not editing
      else if ( editingAvailable == true ) {
        if ( editingActive == false ) {
          editingActive=true;
          lcd.setCursor(15,0);
          fprintf_P(&lcdout, PSTR(" "));
          lcd.setCursor(15,1);
          fprintf_P(&lcdout, PSTR(" "));
        }
        else if ( editingActive == true ) {
          editingActive=false;
          // for items at the top of a menu
          if ((EDIT_FIELD >= EDIT_MOONWHITE)&&(EDIT_FIELD <= EDIT_MOONBLUE)); // don't draw up arrow in daylight WRGB menu
          else if ((EDIT_FIELD >= EDIT_F1_ONHOUR)&&(EDIT_FIELD <= EDIT_F1_DURATIONMIN)); // don't draw up arrow edit Fade 1 menu
          else if ((EDIT_FIELD >= EDIT_L_ONHOUR)&&(EDIT_FIELD <= EDIT_L_OFFMINUTE)); // don't draw up arrow edit power schedule menu
          else if (EDIT_FIELD == EDIT_BRIGHTNESS); // don't draw up arrow in edit brightness menu
          else if ((EDIT_FIELD >= EDIT_THOUR)&&(EDIT_FIELD <= EDIT_TSEC)); // don't draw up arrow in set time menu
          else { // draw up arrow
            lcd.setCursor(15,0);
            lcd.write(byte(0));
          }
          // for items at the bottom of a menu
          if ((EDIT_FIELD >= EDIT_DAYWHITE)&&(EDIT_FIELD <= EDIT_DAYBLUE)); // don't draw down arrow in M2 WRGB menu
          else if ((EDIT_FIELD >= EDIT_F6_ONHOUR)&&(EDIT_FIELD <= EDIT_F6_DURATIONMIN)); // don't draw down arrow in fade 6 menu
          else if ((EDIT_FIELD >= EDIT_L_ONHOUR)&&(EDIT_FIELD <= EDIT_L_OFFMINUTE)); // don't draw down arrow edit power schedule menu
          else if ((EDIT_FIELD >= EDIT_TMONTH)&&(EDIT_FIELD <= EDIT_TYEAR)); // don't draw down arrow in date setting
          else if (EDIT_FIELD == EDIT_BRIGHTNESS); // don't draw down arrow in brightness menu
          else { // draw down arrow
            lcd.setCursor(15,1);
            lcd.write(byte(1));
          }
        }
      }
      // if we aren't editing, just move into the menu
      else if ( editingActive == false ) menu.moveDown();
      else menu.use();
      drawScreen();
      delay(250);
      break;
    }
  default:
    {
      break;
    }
  }
}


/*--------------------------------------------------------------------------------------
 ReadButtons()
 This actually reads the voltage from analog 0 and processes what each click is
 If you are using a different LCD, you may need to adjust this, otherwise leave it alone
 --------------------------------------------------------------------------------------*/
byte ReadButtons()
{
  unsigned int buttonVoltage;
  byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn

  //read the button ADC pin voltage
  buttonVoltage = analogRead( BUTTON_ADC_PIN );
  //sense if the voltage falls within valid voltage windows
  if ( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_RIGHT;
  }
  else if (   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_UP;
  }
  else if (   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_DOWN;
  }
  else if (   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_LEFT;
  }
  else if (   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
    && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
  {
    button = BUTTON_SELECT;
  }
  return( button );
}

void menuUseEvent(MenuUseEvent used) // we don't really use this, it's left in for potential future use
{

  fprintf_P(&lcdout, PSTR("Menu use "));
  Serial.println(used.item.getName());
  if (used.item == "HOME") //comparison using a string literal
  {
    // Print available SRAM for debugging, comment out if you want
    printf_P(PSTR("SRAM: %d\n"), freeRam());
  }
  /*
  if (used.item == miDAYLIGHT) //comparison agains a known item
   {
   //Serial.println("menuUseEvent found miDAYLIGHT (V)");
   fprintf_P(&lcdout, PSTR("menuUseEvent found miDAYLIGHT (V)"));
   }
   */
  delay(250);
}

/*----------------------------------------------------------------
 menuChangeEvent()
 Here is where we get a notification whenever the user changes the menu
 It is not called directly, but rather by menubacked when we do call menu.move() commands
 ----------------------------------------------------------------*/
void menuChangeEvent(MenuChangeEvent changed)
{

  // next two lines are for testing and debugging, they output to serial which menu we moved into
  //fprintf_P(&lcdout, PSTR("Menu changed to: ")); 
  //Serial.println(changed.to.getName());

  editingActive=false;

  // we start by clearing and resetting the LCD every time the menu changes
  lcd.clear();
  lcd.noCursor();
  lcd.setCursor(0,0);
  lcd.print(changed.to.getName());

  // the next two if/else statements are just to draw up/down arrows 
  // based on if there are items above or below each item in the menu
  if ((changed.to == miHOME)||(changed.to == miF1)||(changed.to == miPOWER)||(changed.to == miMOON)||(changed.to == miTIME)||(changed.to == miSCBR)); // don't draw up arrow
  else { 
    lcd.setCursor(15,0);
    lcd.write(byte(0));
  }
  delay(10);
  if ((changed.to == miSC)||(changed.to == miF6)||(changed.to == miPOWER)||(changed.to == miDAYLIGHT)||(changed.to == miDATE)||(changed.to == miSCBR)); // don't draw down arrow
  else { 
    lcd.setCursor(15,1);
    lcd.write(byte(1));
  }

  // don't allow editing mode in the following menus
  if ((changed.to == miHOME)||(changed.to == miFSCHED)||(changed.to == miPSCHED)||(changed.to == miWRGB)||(changed.to == miTIDA)||(changed.to == miSC)) editingAvailable=false;

  // set the first field to edit as we change to different menus
  if (changed.to == miDAYLIGHT) EDIT_FIELD=EDIT_DAYWHITE;
  else if (changed.to == miMOON) EDIT_FIELD=EDIT_MOONWHITE;
  else if (changed.to == miM1) EDIT_FIELD=EDIT_M1WHITE;
  else if (changed.to == miM2) EDIT_FIELD=EDIT_M2WHITE;
  else if (changed.to == miTIME) EDIT_FIELD=EDIT_THOUR;
  else if (changed.to == miDATE) EDIT_FIELD=EDIT_TMONTH;
  else if (changed.to == miF1) EDIT_FIELD=EDIT_F1_ONHOUR;
  else if (changed.to == miF2) EDIT_FIELD=EDIT_F2_ONHOUR;
  else if (changed.to == miF3) EDIT_FIELD=EDIT_F3_ONHOUR;
  else if (changed.to == miF4) EDIT_FIELD=EDIT_F4_ONHOUR;
  else if (changed.to == miF5) EDIT_FIELD=EDIT_F5_ONHOUR;
  else if (changed.to == miF6) EDIT_FIELD=EDIT_F6_ONHOUR;
  else if (changed.to == miPSCHED) EDIT_FIELD=EDIT_L_ONHOUR;

  // tracks if we drop into a sub menu, this lets us back out with the left button
  if ((changed.from == miFSCHED)&&(changed.to == miF1)) subMenu=true; 
  else if ((changed.from == miWRGB)&&(changed.to == miMOON)) subMenu=true;
  else if ((changed.from == miTIDA)&&(changed.to == miTIME)) subMenu=true;

}

/*-------------------------------------------------------------------------
 drawScreen()
 This handler is called every time the screen/menu changes, it looks at what screen/menu the user is on
 and what is going on contextually, and writes everything to the LCD as needed
 -------------------------------------------------------------------------*/

void drawScreen()
{

  // Get the time in seconds (since 1970)
  unsigned long rightNow = now();

  // home is a lot of code because we have to watch the lighting fades, date, and time, and keep the screen updated
  if (menu.getCurrent().getName() == "HOME") {

    lcd.clear();

    // PRINT DATE
    lcd.setCursor(0,0);
    if (dateMonthFirst==true) {
      lcd.print(Mon[month()]);  
      fprintf(&lcdout, " ");
      lcd.print(day());  
    }
    else {
      lcd.print(day());  
      fprintf(&lcdout, " ");
      lcd.print(Mon[month()]);  
    }

    // PRINT HOURS
    if (_24hr==false) {
      lcd.setCursor(8,0);
      if (hourFormat12() < 10) fprintf(&lcdout, " ");
      lcd.print(hourFormat12());
    }
    else {
      lcd.setCursor(11,0);
      if (hour() < 10) fprintf(&lcdout, " ");
      lcd.print(hour());
    }

    // PRINT Minutes
    fprintf(&lcdout, ":");
    if (minute() < 10) lcd.print(0);
    lcd.print(minute());
    fprintf(&lcdout, " ");

    // PRINT AM/PM
    if (_24hr==false) {
      if (isAM() == true) fprintf(&lcdout, "AM");
      else fprintf(&lcdout, "PM");
    }

    // prepare to print current lighting situation
    lcd.setCursor(0,1);
    int minsLeft=0;

    // get the current time in UNIX time, which is FAR easier for time calculations and comparisons
    time_t currentTime = tmConvert_t(year(),month(),day(),hour(),minute(),second());

    unsigned long timeLeft=0;  // used later to track how much time is left in the current lighting context

    // from here down in the home menu is just keeping track of lighting modes and how much time is left

    if (currentLightMode == 5) fprintf_P(&lcdout, PSTR("LIGHT OFF"));
    else if (currentLightMode == 0) fprintf_P(&lcdout, PSTR("DAYLIGHT"));
    else if (currentLightMode == 1) {
      // as a reference point, check to see if we are before or after the fade into daylight
      time_t fade3Start = tmConvert_t(year(),month(),day(),fade3.onHour,fade3.onMinute,0); // get a unix time stamp for the fade start
      if (fade3Start > currentTime) fprintf_P(&lcdout, PSTR("SUNRISE M2"));
      else fprintf_P(&lcdout, PSTR("SUNSET M2"));
    }
    else if (currentLightMode == 2) {
      // as a reference point, check to see if we are before or after the fade into daylight
      //KMAN CHANGE: For consistency, change Dawn/Dusk M1 to M3 (for Sat+)
      time_t fade3Start = tmConvert_t(year(),month(),day(),fade3.onHour,fade3.onMinute,0); // get a unix time stamp for the fade start
      if (fade3Start > currentTime) fprintf_P(&lcdout, PSTR("DAWN M3"));
      else fprintf_P(&lcdout, PSTR("DUSK M3"));
    }
    else if (currentLightMode == 3) fprintf_P(&lcdout, PSTR("MOON"));

    // below is for lighting transitions
    // for reference, fadeFromMode:  0=daylight, 1=dawn or dusk, 2=sunrise or sunset, 3=moon

    else if (currentLightMode == 4) // we are in a transition
    {
      if (fadeFromMode == 0) fprintf_P(&lcdout, PSTR("DAY"));

      else if (fadeFromMode == 1) {
        if (fadeToMode == 0) fprintf_P(&lcdout, PSTR("RISE"));
        else fprintf_P(&lcdout, PSTR("SET"));
      }

      else if (fadeFromMode == 2) {
        if (fadeToMode == 1) fprintf_P(&lcdout, PSTR("DAWN"));
        else fprintf_P(&lcdout, PSTR("DUSK"));
      }

      else if (fadeFromMode == 3) fprintf_P(&lcdout, PSTR("MOON"));

      fprintf_P(&lcdout, PSTR(">"));

      if (fadeToMode == 0) fprintf_P(&lcdout, PSTR("DAY"));

      else if (fadeToMode == 1) {
        if (fadeFromMode == 2) fprintf_P(&lcdout, PSTR("RISE"));
        else fprintf_P(&lcdout, PSTR("SET"));
      }

      else if (fadeToMode == 2) {
        if (fadeFromMode == 3) fprintf_P(&lcdout, PSTR("DAWN"));
        else fprintf_P(&lcdout, PSTR("DUSK"));
      }

      else if (fadeToMode == 3) fprintf_P(&lcdout, PSTR("MOON"));

      // calculate how much time is left in the fade in minutes
      // this will be used to track how much time is left
      timeLeft=((fadeStartingSeconds + fadeDurationSeconds)-rightNow)/60;

    }

    // if we are not in a transition, cacluate the amount of time until the next transition starts
    if (currentLightMode < 4)
    {
      byte startHour, startMinute;

      if (lastFade==6) startHour=fade1.onHour, startMinute=fade1.onMinute;
      else if (lastFade==1) startHour=fade2.onHour, startMinute=fade2.onMinute;
      else if (lastFade==2) startHour=fade3.onHour, startMinute=fade3.onMinute;
      else if (lastFade==3) startHour=fade4.onHour, startMinute=fade4.onMinute;
      else if (lastFade==4) startHour=fade5.onHour, startMinute=fade5.onMinute;
      else if (lastFade==5) startHour=fade6.onHour, startMinute=fade6.onMinute;

      time_t fadeStart = tmConvert_t(year(),month(),day(),startHour,startMinute,0); // get a unix time stamp for the fade start

      // if fadeStart is less than now, then we need to add a day's worth of seconds
      // the only reason fadeStart would be earlier is if it's actually going to happen the next day
      if (rightNow>fadeStart) fadeStart=fadeStart+86400;

      // finally, calcuate how long in minutes
      timeLeft=(fadeStart-rightNow)/60;
    }

    // split out hours and minutes and send it to the lcd
    lcd.setCursor(11,1);
    minsLeft=timeLeft;
    int hoursLeft = (minsLeft/60);
    minsLeft = (minsLeft-(hoursLeft*60));
    fprintf(&lcdout, "<%1d:%02d", hoursLeft, minsLeft);

  }
  else if (menu.getCurrent().getName() == "WRGB DAYLIGHT") {

    editingAvailable = true;

    // drawColorVal is reused to allow reusing a bunch of code, send it a position and a value and it updates the screen
    drawColorVal(0, lightDaylight.White);
    drawColorVal(1, lightDaylight.Red);
    drawColorVal(2, lightDaylight.Green);
    drawColorVal(3, lightDaylight.Blue);

    // if we are editing, we draw the cursor so the user knows we are editing
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_DAYWHITE) lcd.setCursor(2,1);
      else if (EDIT_FIELD==EDIT_DAYRED) lcd.setCursor(6,1);
      else if (EDIT_FIELD==EDIT_DAYGREEN) lcd.setCursor(10,1);
      else if (EDIT_FIELD==EDIT_DAYBLUE) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }
  else if (menu.getCurrent().getName() == "WRGB MOON") {

    editingAvailable = true;

    // drawColorVal is reused to allow reusing a bunch of code, send it a position and a value and it updates the screen
    drawColorVal(0, lightMoon.White);
    drawColorVal(1, lightMoon.Red);
    drawColorVal(2, lightMoon.Green);
    drawColorVal(3, lightMoon.Blue);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_MOONWHITE) lcd.setCursor(2,1);
      else if (EDIT_FIELD==EDIT_MOONRED) lcd.setCursor(6,1);
      else if (EDIT_FIELD==EDIT_MOONGREEN) lcd.setCursor(10,1);
      else if (EDIT_FIELD==EDIT_MOONBLUE) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }
  //KMAN Change: Change M1 to M3 for Sat+
  else if (menu.getCurrent().getName() == "WRGB M3 DWN/DSK") {

    editingAvailable = true;

    // drawColorVal is reused to allow reusing a bunch of code, send it a position and a value and it updates the screen
    drawColorVal(0, lightM1.White);
    drawColorVal(1, lightM1.Red);
    drawColorVal(2, lightM1.Green);
    drawColorVal(3, lightM1.Blue);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_M1WHITE) lcd.setCursor(2,1);
      else if (EDIT_FIELD==EDIT_M1RED) lcd.setCursor(6,1);
      else if (EDIT_FIELD==EDIT_M1GREEN) lcd.setCursor(10,1);
      else if (EDIT_FIELD==EDIT_M1BLUE) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }
  else if (menu.getCurrent().getName() == "WRGB M2 RIS/SET") {

    editingAvailable = true;

    // drawColorVal is reused to allow reusing a bunch of code, send it a position and a value and it updates the screen
    drawColorVal(0, lightM2.White);
    drawColorVal(1, lightM2.Red);
    drawColorVal(2, lightM2.Green);
    drawColorVal(3, lightM2.Blue);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_M2WHITE) lcd.setCursor(2,1);
      else if (EDIT_FIELD==EDIT_M2RED) lcd.setCursor(6,1);
      else if (EDIT_FIELD==EDIT_M2GREEN) lcd.setCursor(10,1);
      else if (EDIT_FIELD==EDIT_M2BLUE) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }
  // KMAN CHANGE: Edit M1 Dawn to M3 Dawn for Sat+
  else if (menu.getCurrent().getName() == "MOON > M3 DAWN") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) {
      fadeT.onHour=fade1.onHour;
      fadeT.onMinute=fade1.onMinute;
      //BEGIN KMAN CHANGES: - code change per scaLLas
      //lightPowerT.durationHours=lightPower.offHour;   //AH bad code
      //lightPowerT.durationMinutes=lightPower.offMinute;  //AH bad code
      lightPowerT.offHour=lightPower.offHour;   //scaLLas corrected code
      lightPowerT.offMinute=lightPower.offMinute;    //scaLLas corrected code
      //END KMAN CHANGES
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F1_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F1_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F1_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F1_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

//KMAN CHANGE: M1 Dwn to M3 Dwn for Sat+
  else if (menu.getCurrent().getName() == "M3 DWN > M2 RSE") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) { 
      fadeT.onHour=fade2.onHour;
      fadeT.onMinute=fade2.onMinute;
      fadeT.durationHours=fade2.durationHours;
      fadeT.durationMinutes=fade2.durationMinutes;
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F2_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F2_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F2_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F2_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

  else if (menu.getCurrent().getName() == "M2 RISE > DAY") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) { 
      fadeT.onHour=fade3.onHour;
      fadeT.onMinute=fade3.onMinute;
      fadeT.durationHours=fade3.durationHours;
      fadeT.durationMinutes=fade3.durationMinutes;
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F3_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F3_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F3_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F3_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

  else if (menu.getCurrent().getName() == "DAY > M2 SET") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) {
      fadeT.onHour=fade4.onHour;
      fadeT.onMinute=fade4.onMinute;
      fadeT.durationHours=fade4.durationHours;
      fadeT.durationMinutes=fade4.durationMinutes;
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F4_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F4_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F4_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F4_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

//KMAN Change: M1 Dsk to M3 Dsk for Sat+
  else if (menu.getCurrent().getName() == "M2 SET > M3 DSK") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) { 
      fadeT.onHour=fade5.onHour;
      fadeT.onMinute=fade5.onMinute;
      fadeT.durationHours=fade5.durationHours;
      fadeT.durationMinutes=fade5.durationMinutes;
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F5_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F5_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F5_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F5_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

//KMAN Change: M1 Dusk to M3 Dusk for Sat+
  else if (menu.getCurrent().getName() == "M3 DUSK > MOON") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) { 
      fadeT.onHour=fade6.onHour;
      fadeT.onMinute=fade6.onMinute;
      fadeT.durationHours=fade6.durationHours;
      fadeT.durationMinutes=fade6.durationMinutes;
    }

    drawFadeSchedule(fadeT.onHour, fadeT.onMinute, fadeT.durationHours, fadeT.durationMinutes);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_F6_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_F6_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_F6_DURATIONHOUR) lcd.setCursor(11,1);
      else if (EDIT_FIELD==EDIT_F6_DURATIONMIN) lcd.setCursor(14,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }

  else if (menu.getCurrent().getName() == "LIGHT ON / OFF") {

    editingAvailable=true;

    // for editing fades, we use fadeT to temporarily hold the edited values, this way we don't lose the original values
    // here we set them to the appropriate fade values to be sent to the screen
    if (editingActive==false) { 
      lightPowerT.onHour=lightPower.onHour;
      lightPowerT.onMinute=lightPower.onMinute;
//      lightPowerT.offHour=lightPower.offHour;      //AH Bad Code
//      lightPowerT.offMinute=lightPower.offMinute;  //AH Bad Code
      lightPowerT.durationHours=lightPower.offHour;     //scaLLas corrected code
      lightPowerT.durationMinutes=lightPower.offMinute; //scaLLas corrected code
    }

    // if we are editing, we draw the cursor so the user knows we are editing
    drawLightSchedule(lightPowerT.onHour, lightPowerT.onMinute, lightPowerT.offHour, lightPowerT.offMinute);

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_L_ONHOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_L_ONMINUTE) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_L_OFFHOUR) lcd.setCursor(10,1);
      else if (EDIT_FIELD==EDIT_L_OFFMINUTE) lcd.setCursor(13,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 
  }


  else if (menu.getCurrent().getName() == "SET TIME") {

    editingAvailable = true;

    // we continually update the saveRTC values, so that as soon as we hit edit, we start editing at the current time
    if (editingActive==false) {
      if (_24hr==false) saveRTC.tHour=hourFormat12();
      else saveRTC.tHour=hour();
      saveRTC.tMinute=minute(); 
      saveRTC.tSecond=second();
      saveRTC.tAM=isAM(); // true if AM, false if PM
    }

    // PRINT HOURS to screen with proper formatting, and taking 12/24 hour clock into consideration
    lcd.setCursor(0,1);
    if (saveRTC.tHour < 10) {
      if (_24hr==false) fprintf(&lcdout, " ");
      else fprintf(&lcdout, "0");
    }
    lcd.print(saveRTC.tHour);

    fprintf(&lcdout, ":");

    // PRINT MINS to screen with proper formatting, and taking 12/24 hour clock into consideration
    if (saveRTC.tMinute < 10) lcd.print(0);
    lcd.print(saveRTC.tMinute);
    fprintf(&lcdout, ":");

    if (saveRTC.tSecond < 10) lcd.print(0);
    lcd.print(saveRTC.tSecond);

    if (_24hr==false) {
      if (saveRTC.tAM == true) fprintf(&lcdout, " AM");
      else fprintf(&lcdout, " PM");
    }

    // if we are editing, we need to place the cursor in the correct spot
    // at the same time we set the edit field
    if (editingActive == true) {
      if (EDIT_FIELD==EDIT_THOUR) lcd.setCursor(1,1);
      else if (EDIT_FIELD==EDIT_TMIN) lcd.setCursor(4,1);
      else if (EDIT_FIELD==EDIT_TSEC) lcd.setCursor(7,1);
      lcd.cursor(); 
    }
    else lcd.noCursor(); 

  }
  else if (menu.getCurrent().getName() == "SET DATE") {

    editingAvailable = true;

    // we continually update the saveRTC values, so that as soon as we hit edit, we start editing at the current date
    if (editingActive==false) {
      saveRTC.tMonth=month();
      saveRTC.tDay=day();
      saveRTC.tYear=year();
    }

    // PRINT DATE taking into consideration formatting
    lcd.setCursor(0,1);
    if (dateMonthFirst==true) {
      if (saveRTC.tMonth < 10) lcd.print(0);
      lcd.print(saveRTC.tMonth);  
      fprintf(&lcdout, "/");
      if (saveRTC.tDay < 10) lcd.print(0);
      lcd.print(saveRTC.tDay);  
    }
    else {
      if (saveRTC.tDay < 10) lcd.print(0);
      lcd.print(saveRTC.tDay);      
      fprintf(&lcdout, "/");
      if (saveRTC.tMonth < 10) lcd.print(0);
      lcd.print(saveRTC.tMonth);  
    }
    fprintf(&lcdout, "/");
    lcd.print(saveRTC.tYear);  

    // if we are editing, we need to place the cursor in the correct spot, depending on user's date format
    // at the same time we set the edit field
    if (editingActive == true) {
      if (dateMonthFirst==true) {
        if (EDIT_FIELD==EDIT_TMONTH) lcd.setCursor(1,1);
        else if (EDIT_FIELD==EDIT_TDAY) lcd.setCursor(4,1);
        else if (EDIT_FIELD==EDIT_TYEAR) lcd.setCursor(9,1);
      }
      else {
        if (EDIT_FIELD==EDIT_TDAY) lcd.setCursor(1,1);
        else if (EDIT_FIELD==EDIT_TMONTH) lcd.setCursor(4,1);
        else if (EDIT_FIELD==EDIT_TYEAR) lcd.setCursor(9,1);
      }
      lcd.cursor(); 
    }
    else lcd.noCursor(); 

  }
  else if (menu.getCurrent().getName() == "TIME FORMAT") {

    editingAvailable = true;

    // set the eidt field
    EDIT_FIELD=EDIT_12HR;

    // send the current setting to the lcd
    lcd.setCursor(0,1);
    if (_24hr==false) lcd.print("12 HOUR CLOCK");  
    else lcd.print("24 HOUR CLOCK");

    // if editing is active, display the cursor
    lcd.setCursor(12,1);
    if (editingActive == true) lcd.cursor();
    else lcd.noCursor(); 
  }

  else if (menu.getCurrent().getName() == "DATE FORMAT") {

    editingAvailable = true;

    // set the eidt field
    EDIT_FIELD=EDIT_DATE_FMT;

    // send the current setting to the lcd
    lcd.setCursor(0,1);
    if (dateMonthFirst==true) lcd.print("MM/DD/YYYY");  
    else lcd.print("DD/MM/YYYY");

    // if editing is active, display the cursor
    lcd.setCursor(9,1);
    if (editingActive == true) lcd.cursor();
    else lcd.noCursor(); 
  }


  else if (menu.getCurrent().getName() == "BRIGHTNESS") {

    editingAvailable = true;

    // set the eidt field
    EDIT_FIELD=EDIT_BRIGHTNESS;

    // send the current setting to the lcd
    lcd.setCursor(0,1);
    fprintf(&lcdout, "(0-10): ");
    if (backLight < 10) fprintf(&lcdout, " "); // add a space if less than 10
    lcd.print(backLight);

    // if editing is active, display the cursor
    lcd.setCursor(9,1);
    if (editingActive == true) lcd.cursor();   
    else lcd.noCursor(); 
  }

}

/*----------------------------------------------------------------------------
 drawColorVal()
 This sends the color values to the LCD with proper formatting.
 It is called from the drawScreen() routine above when editing the RGBW values
 ----------------------------------------------------------------------------*/

void drawColorVal(byte colChannel, byte colVal)
{
  // colChannel 0 = white, 1 = red, 2 = blue, 3 = green
  byte x=(colChannel*4); // move the cursor over 4 chars per color

  // set the cursor to write the color
  lcd.setCursor(x,1);
  // if the color value is less than 100, insert a space and move the cursor over 1
  if (colVal < 100) {
    fprintf(&lcdout, "0");
    lcd.setCursor(x+1,1);
  }
  if (colVal < 10) {
    fprintf(&lcdout, "0");
    lcd.setCursor(x+2,1);
  }
  // print the value
  lcd.print(colVal);
}


/*----------------------------------------------------------------------------
 editButtons()
 
 This is a giant routine for processing the buttons during editing based on what field you are editing.
 
 The EDIT_FIELD's make this very readable, otherwise it'd be terribly confusing to work with.
 
 I'm not going to comment this block. I think it's pretty easy to see that it's basically incrementing
 values up and down, and wrapping them over the top value to 0 or under 0 to the top value.
 
 With a some of the edits, it's writing the values directly and immediately to EEPROM.
 
 When it's done, it redraws the screen.
 ----------------------------------------------------------------------------*/

/*  KMAN CHANGES:
Change lines such as:
if (lightDaylight.White<100)
to
if (lightDaylight.White<42)
... to correct max value of E-Series (100 max) to Sat+ (42 max)
*/

void editButtons(byte buttons)
{
  byte x;
  byte y;
  switch( buttons ) // need to respond differently to each button
  {
  case BUTTON_UP:
    {
      if (EDIT_FIELD==EDIT_BRIGHTNESS) {
        if (backLight<10) backLight++;
        analogWrite(LCD_BACKLIGHT_PIN, backLight*25);
        EEPROM.write(31,backLight);
      }
      else if (EDIT_FIELD==EDIT_DAYWHITE) {
        if (lightDaylight.White<42) lightDaylight.White++;
        else lightDaylight.White=0;
        EEPROM.write(203,lightDaylight.White);
      }
      else if (EDIT_FIELD==EDIT_DAYRED) {
        if (lightDaylight.Red<42) lightDaylight.Red++;
        else lightDaylight.Red=0;
        EEPROM.write(200,lightDaylight.Red);
      }
      else if (EDIT_FIELD==EDIT_DAYGREEN) {
        if (lightDaylight.Green<42) lightDaylight.Green++;
        else lightDaylight.Green=0;
        EEPROM.write(201,lightDaylight.Green);
      }
      else if (EDIT_FIELD==EDIT_DAYBLUE) {
        if (lightDaylight.Blue<42) lightDaylight.Blue++;
        else lightDaylight.Blue=0;
        EEPROM.write(202,lightDaylight.Blue);
      }
      else if (EDIT_FIELD==EDIT_MOONWHITE) {
        if (lightMoon.White<42) lightMoon.White++;
        else lightMoon.White=0;
        EEPROM.write(215,lightMoon.White);
      }
      else if (EDIT_FIELD==EDIT_MOONRED) {
        if (lightMoon.Red<42) lightMoon.Red++;
        else lightMoon.Red=0;
        EEPROM.write(212,lightMoon.Red);
      }
      else if (EDIT_FIELD==EDIT_MOONGREEN) {
        if (lightMoon.Green<42) lightMoon.Green++;
        else lightMoon.Green=0;
        EEPROM.write(213,lightMoon.Green);
      }
      else if (EDIT_FIELD==EDIT_MOONBLUE) {
        if (lightMoon.Blue<42) lightMoon.Blue++;
        else lightMoon.Blue=0;
        EEPROM.write(214,lightMoon.Blue);
      }
      else if (EDIT_FIELD==EDIT_M1WHITE) {
        if (lightM1.White<42) lightM1.White++;
        else lightM1.White=0;
        EEPROM.write(211,lightM1.White);
      }
      else if (EDIT_FIELD==EDIT_M1RED) {
        if (lightM1.Red<42) lightM1.Red++;
        else lightM1.Red=0;
        EEPROM.write(208,lightM1.Red);
      }
      else if (EDIT_FIELD==EDIT_M1GREEN) {
        if (lightM1.Green<42) lightM1.Green++;
        else lightM1.Green=0;
        EEPROM.write(209,lightM1.Green);
      }
      else if (EDIT_FIELD==EDIT_M1BLUE) {
        if (lightM1.Blue<42) lightM1.Blue++;
        else lightM1.Blue=0;
        EEPROM.write(210,lightM1.Blue);
      }
      else if (EDIT_FIELD==EDIT_M2WHITE) {
        if (lightM2.White<42) lightM2.White++;
        else lightM2.White=0;
        EEPROM.write(207,lightM2.White);
      }
      else if (EDIT_FIELD==EDIT_M2RED) {
        if (lightM2.Red<42) lightM2.Red++;
        else lightM2.Red=0;
        EEPROM.write(204,lightM2.Red);
      }
      else if (EDIT_FIELD==EDIT_M2GREEN) {
        if (lightM2.Green<42) lightM2.Green++;
        else lightM2.Green=0;
        EEPROM.write(205,lightM2.Green);
      }
      else if (EDIT_FIELD==EDIT_M2BLUE) {
        if (lightM2.Blue<42) lightM2.Blue++;
        else lightM2.Blue=0;
        EEPROM.write(206,lightM2.Blue);
      }
      
      // END KMAN CODE CHANGES RE MAX VALUE Sat+ of 42 vs E-Series 100
      
      else if (EDIT_FIELD==EDIT_THOUR) {
        if (_24hr==false) {
          if (saveRTC.tHour==12) saveRTC.tHour=1;
          else if (saveRTC.tHour==11) {
            saveRTC.tHour++;
            if (saveRTC.tAM==true) saveRTC.tAM=false;
            else saveRTC.tAM=true;
          }
          else saveRTC.tHour++;
        }
        else {
          if (saveRTC.tHour==23) saveRTC.tHour=0;
          else saveRTC.tHour++;
        }
      }
      else if (EDIT_FIELD==EDIT_TMIN) {
        if (saveRTC.tMinute==59) saveRTC.tMinute=0;
        else saveRTC.tMinute++;
      }
      else if (EDIT_FIELD==EDIT_TSEC) {
        if (saveRTC.tSecond==59) saveRTC.tSecond=0;
        else saveRTC.tSecond++;
      }
      else if (EDIT_FIELD==EDIT_TMONTH) {
        if (saveRTC.tMonth==12) saveRTC.tMonth=1;
        else saveRTC.tMonth++;
      }
      else if (EDIT_FIELD==EDIT_TDAY) {
        Serial.print(saveRTC.tDay);
        if ((saveRTC.tMonth==1)||(saveRTC.tMonth==3)||(saveRTC.tMonth==5)||(saveRTC.tMonth==7)||(saveRTC.tMonth==8)||(saveRTC.tMonth==10)||(saveRTC.tMonth==12)) {
          if (saveRTC.tDay==31) saveRTC.tDay=1;
          else saveRTC.tDay++;
        }
        else if ((saveRTC.tMonth==4)||(saveRTC.tMonth==6)||(saveRTC.tMonth==9)||(saveRTC.tMonth==11)) {
          if (saveRTC.tDay==30) saveRTC.tDay=1;
          else saveRTC.tDay++;
        }
        else if (saveRTC.tMonth==2) {
          if (saveRTC.tDay==28) saveRTC.tDay=1;
          else saveRTC.tDay++;
        }
        Serial.println(saveRTC.tDay);
      }
      else if (EDIT_FIELD==EDIT_TYEAR) {
        saveRTC.tYear++;
      }
      else if (EDIT_FIELD==EDIT_F1_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F1_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_F2_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F2_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_F3_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F3_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_F4_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F4_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_F5_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F5_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_F6_ONHOUR) {
        if (fadeT.onHour==23) fadeT.onHour=0;
        else fadeT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_F6_ONMINUTE) {
        if (fadeT.onMinute==59) fadeT.onMinute=0;
        else fadeT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONHOUR) {
        if (fadeT.durationHours==9) fadeT.durationHours=0;
        else fadeT.durationHours++;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONMIN) {
        if (fadeT.durationMinutes==59) fadeT.durationMinutes=0;
        else fadeT.durationMinutes++;
      }
      else if (EDIT_FIELD==EDIT_12HR) {
        if (_24hr==false) {
          _24hr=true;
          EEPROM.write(50,1);
        }
        else {
          _24hr=false;
          EEPROM.write(50,0);
        }
      }
      else if (EDIT_FIELD==EDIT_DATE_FMT) {
        if (dateMonthFirst==true) {
          dateMonthFirst=false;
          EEPROM.write(51,1);
        }
        else {
          dateMonthFirst=true;
          EEPROM.write(51,0);
        }
      }
      else if (EDIT_FIELD==EDIT_L_ONHOUR) {
        if (lightPowerT.onHour==23) lightPowerT.onHour=0;
        else lightPowerT.onHour++;
      }
      else if (EDIT_FIELD==EDIT_L_ONMINUTE) {
        if (lightPowerT.onMinute==59) lightPowerT.onMinute=0;
        else lightPowerT.onMinute++;
      }
      else if (EDIT_FIELD==EDIT_L_OFFHOUR) {
        if (lightPowerT.offHour==23) lightPowerT.offHour=0;
        else lightPowerT.offHour++;
      }
      else if (EDIT_FIELD==EDIT_L_OFFMINUTE) {
        if (lightPowerT.offMinute==59) lightPowerT.offMinute=0;
        else lightPowerT.offMinute++;
      }
      break;
    }

    // BEGIN KMAN SAT+ CODE CHANGES: change lightDaylight.White=100 to lightDaylight.White=42 and so on for max value 42 of Sat+

  case BUTTON_DOWN:
    {
      if (EDIT_FIELD==EDIT_BRIGHTNESS) {
        if (backLight>1) backLight--;
        analogWrite(LCD_BACKLIGHT_PIN, backLight*25);
        EEPROM.write(31,backLight);
      }
      else if (EDIT_FIELD==EDIT_DAYWHITE)
      {
        if (lightDaylight.White>0) lightDaylight.White--;
        else lightDaylight.White=42;
        EEPROM.write(203,lightDaylight.White);
      }
      else if (EDIT_FIELD==EDIT_DAYRED)
      {
        if (lightDaylight.Red>0) lightDaylight.Red--;
        else lightDaylight.Red=42;
        EEPROM.write(200,lightDaylight.Red);
      }
      else if (EDIT_FIELD==EDIT_DAYGREEN)
      {
        if (lightDaylight.Green>0) lightDaylight.Green--;
        else lightDaylight.Green=42;
        EEPROM.write(201,lightDaylight.Green);
      }
      if (EDIT_FIELD==EDIT_DAYBLUE)
      {
        if (lightDaylight.Blue>0) lightDaylight.Blue--;
        else lightDaylight.Blue=42;
        EEPROM.write(202,lightDaylight.Blue);
      }
      else if (EDIT_FIELD==EDIT_MOONWHITE) {
        if (lightMoon.White>0) lightMoon.White--;
        else lightMoon.White=42;
        EEPROM.write(215,lightMoon.White);
      }
      else if (EDIT_FIELD==EDIT_MOONRED) {
        if (lightMoon.Red>0) lightMoon.Red--;
        else lightMoon.Red=42;
        EEPROM.write(212,lightMoon.Red);
      }
      else if (EDIT_FIELD==EDIT_MOONGREEN) {
        if (lightMoon.Green>0) lightMoon.Green--;
        else lightMoon.Green=42;
        EEPROM.write(213,lightMoon.Green);
      }
      else if (EDIT_FIELD==EDIT_MOONBLUE) {
        if (lightMoon.Blue>0) lightMoon.Blue--;
        else lightMoon.Blue=42;
        EEPROM.write(214,lightMoon.Blue);
      }
      else if (EDIT_FIELD==EDIT_M1WHITE) {
        if (lightM1.White>0) lightM1.White--;
        else lightM1.White=42;
        EEPROM.write(211,lightM1.White);
      }
      else if (EDIT_FIELD==EDIT_M1RED) {
        if (lightM1.Red>0) lightM1.Red--;
        else lightM1.Red=42;
        EEPROM.write(208,lightM1.Red);
      }
      else if (EDIT_FIELD==EDIT_M1GREEN) {
        if (lightM1.Green>0) lightM1.Green--;
        else lightM1.Green=42;
        EEPROM.write(209,lightM1.Green);
      }
      else if (EDIT_FIELD==EDIT_M1BLUE) {
        if (lightM1.Blue>0) lightM1.Blue--;
        else lightM1.Blue=42;
        EEPROM.write(210,lightM1.Blue);
      }
      else if (EDIT_FIELD==EDIT_M2WHITE) {
        if (lightM2.White>0) lightM2.White--;
        else lightM2.White=42;
        EEPROM.write(207,lightM2.White);
      }
      else if (EDIT_FIELD==EDIT_M2RED) {
        if (lightM2.Red>0) lightM2.Red--;
        else lightM2.Red=42;
        EEPROM.write(204,lightM2.Red);
      }
      else if (EDIT_FIELD==EDIT_M2GREEN) {
        if (lightM2.Green>0) lightM2.Green--;
        else lightM2.Green=42;
        EEPROM.write(205,lightM2.Green);
      }
      else if (EDIT_FIELD==EDIT_M2BLUE) {
        if (lightM2.Blue>0) lightM2.Blue--;
        else lightM2.Blue=42;
        EEPROM.write(206,lightM2.Blue);

       // END KMAN SAT+ CODE CHANGES RE lightxxxx.White=100 to 42
       
      }
      else if (EDIT_FIELD==EDIT_THOUR) {
        if (_24hr==false) {
          if (saveRTC.tHour==1) saveRTC.tHour=12;
          else if (saveRTC.tHour==12) {
            saveRTC.tHour--;
            if (saveRTC.tAM==true) saveRTC.tAM=false;
            else saveRTC.tAM=true;
          }
          else saveRTC.tHour--;
        }
        else {
          if (saveRTC.tHour==0) saveRTC.tHour=23;
          else saveRTC.tHour--;
        }
      }
      else if (EDIT_FIELD==EDIT_TMIN) {
        if (saveRTC.tMinute==0) saveRTC.tMinute=59;
        else saveRTC.tMinute--;
      }
      else if (EDIT_FIELD==EDIT_TSEC) {
        if (saveRTC.tSecond==0) saveRTC.tSecond=59;
        else saveRTC.tSecond--;
      }
      else if (EDIT_FIELD==EDIT_TMONTH) {
        if (saveRTC.tMonth==1) saveRTC.tMonth=12;
        else saveRTC.tMonth--;
      }
      else if (EDIT_FIELD==EDIT_TDAY) {
        if ((saveRTC.tMonth==1)||(saveRTC.tMonth==3)||(saveRTC.tMonth==5)||(saveRTC.tMonth==7)||(saveRTC.tMonth==8)||(saveRTC.tMonth==10)||(saveRTC.tMonth==12)) {
          if (saveRTC.tDay==1) saveRTC.tDay=31;
          else saveRTC.tDay--;
        }
        else if ((saveRTC.tMonth==4)||(saveRTC.tMonth==6)||(saveRTC.tMonth==9)||(saveRTC.tMonth==11)) {
          if (saveRTC.tDay==1) saveRTC.tDay=30;
          else saveRTC.tDay--;
        }
        else if (saveRTC.tMonth==2) {
          if (saveRTC.tDay==1) saveRTC.tDay=28;
          else saveRTC.tDay--;
        }
      }
      else if (EDIT_FIELD==EDIT_TYEAR) {
        saveRTC.tYear--;
      }
      else if (EDIT_FIELD==EDIT_F1_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F1_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_F2_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F2_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_F3_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F3_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_F4_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F4_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_F5_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F5_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_F6_ONHOUR) {
        if (fadeT.onHour==0) fadeT.onHour=23;
        else fadeT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_F6_ONMINUTE) {
        if (fadeT.onMinute==0) fadeT.onMinute=59;
        else fadeT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONHOUR) {
        if (fadeT.durationHours==0) fadeT.durationHours=9;
        else fadeT.durationHours--;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONMIN) {
        if (fadeT.durationMinutes==0) fadeT.durationMinutes=59;
        else fadeT.durationMinutes--;
      }
      else if (EDIT_FIELD==EDIT_L_ONHOUR) {
        if (lightPowerT.onHour==0) lightPowerT.onHour=23;
        else lightPowerT.onHour--;
      }
      else if (EDIT_FIELD==EDIT_L_ONMINUTE) {
        if (lightPowerT.onMinute==0) lightPowerT.onMinute=59;
        else lightPowerT.onMinute--;
      }
      else if (EDIT_FIELD==EDIT_L_OFFHOUR) {
        if (lightPowerT.offHour==0) lightPowerT.offHour=23;
        else lightPowerT.offHour--;
      }
      else if (EDIT_FIELD==EDIT_L_OFFMINUTE) {
        if (lightPowerT.offMinute==0) lightPowerT.offMinute=59;
        else lightPowerT.offMinute--;
      }
      else if (EDIT_FIELD==EDIT_12HR) {
        if (_24hr==false) {
          _24hr=true;
          EEPROM.write(50,1);
        }
        else {
          _24hr=false;
          EEPROM.write(50,0);
        }
      }
      else if (EDIT_FIELD==EDIT_DATE_FMT) {
        if (dateMonthFirst==true) {
          dateMonthFirst=false;
          EEPROM.write(51,1);
        }
        else {
          dateMonthFirst=true;
          EEPROM.write(51,0);
        }
      }
      break;
    }
  case BUTTON_RIGHT:
    {
      if (EDIT_FIELD==EDIT_DAYWHITE) {
        lcd.setCursor(6,1);
        EDIT_FIELD=EDIT_DAYRED;
      }
      else if (EDIT_FIELD==EDIT_DAYRED) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_DAYGREEN;
      }
      else if (EDIT_FIELD==EDIT_DAYGREEN) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_DAYBLUE;
      }
      else if (EDIT_FIELD==EDIT_MOONWHITE) {
        lcd.setCursor(6,1);
        EDIT_FIELD=EDIT_MOONRED;
      }
      else if (EDIT_FIELD==EDIT_MOONRED) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_MOONGREEN;
      }
      else if (EDIT_FIELD==EDIT_MOONGREEN) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_MOONBLUE;
      }
      else if (EDIT_FIELD==EDIT_M1WHITE) {
        lcd.setCursor(6,1);
        EDIT_FIELD=EDIT_M1RED;
      }
      else if (EDIT_FIELD==EDIT_M1RED) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_M1GREEN;
      }
      else if (EDIT_FIELD==EDIT_M1GREEN) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_M1BLUE;
      }
      else if (EDIT_FIELD==EDIT_M2WHITE) {
        lcd.setCursor(6,1);
        EDIT_FIELD=EDIT_M2RED;
      }
      else if (EDIT_FIELD==EDIT_M2RED) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_M2GREEN;
      }
      else if (EDIT_FIELD==EDIT_M2GREEN) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_M2BLUE;
      }
      else if (EDIT_FIELD==EDIT_THOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_TMIN;
      }
      else if (EDIT_FIELD==EDIT_TMIN) {
        lcd.setCursor(7,1);
        EDIT_FIELD=EDIT_TSEC;
      }
      else if (EDIT_FIELD==EDIT_TMONTH) {
        if (dateMonthFirst==true) {
          lcd.setCursor(4,1);
          EDIT_FIELD=EDIT_TDAY;
        }
        else {
          lcd.setCursor(9,1);
          EDIT_FIELD=EDIT_TYEAR;         
        }
      }
      else if (EDIT_FIELD==EDIT_TDAY) {
        if (dateMonthFirst==true) {
          lcd.setCursor(9,1);
          EDIT_FIELD=EDIT_TYEAR;
        }
        else {
          lcd.setCursor(4,1);
          EDIT_FIELD=EDIT_TMONTH;
        }
      }
      else if (EDIT_FIELD==EDIT_F1_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F1_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F1_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F1_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F1_DURATIONMIN;
      }
      else if (EDIT_FIELD==EDIT_F2_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F2_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F2_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F2_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F2_DURATIONMIN;
      }
      else if (EDIT_FIELD==EDIT_F3_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F3_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F3_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F3_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F3_DURATIONMIN;
      }
      else if (EDIT_FIELD==EDIT_F4_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F4_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F4_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F4_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F4_DURATIONMIN;
      }
      else if (EDIT_FIELD==EDIT_F5_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F5_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F5_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F5_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F5_DURATIONMIN;
      }
      else if (EDIT_FIELD==EDIT_F6_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F6_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F6_ONMINUTE) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F6_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONHOUR) {
        lcd.setCursor(14,1);
        EDIT_FIELD=EDIT_F6_DURATIONMIN;
      }

      else if (EDIT_FIELD==EDIT_L_ONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_L_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_L_ONMINUTE) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_L_OFFHOUR;
      }
      else if (EDIT_FIELD==EDIT_L_OFFHOUR) {
        lcd.setCursor(13,1);
        EDIT_FIELD=EDIT_L_OFFMINUTE;
      }

      break;
    }
  case BUTTON_LEFT:
    {
      if (EDIT_FIELD==EDIT_DAYRED) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_DAYWHITE;
      }
      else if (EDIT_FIELD==EDIT_DAYGREEN) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_DAYRED;
      }
      else if (EDIT_FIELD==EDIT_DAYBLUE) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_DAYGREEN;
      }
      else if (EDIT_FIELD==EDIT_MOONRED) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_MOONWHITE;
      }
      else if (EDIT_FIELD==EDIT_MOONGREEN) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_MOONRED;
      }
      else if (EDIT_FIELD==EDIT_MOONBLUE) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_MOONGREEN;
      }
      else if (EDIT_FIELD==EDIT_M1RED) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M1WHITE;
      }
      else if (EDIT_FIELD==EDIT_M1GREEN) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M1RED;
      }
      else if (EDIT_FIELD==EDIT_M1BLUE) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M1GREEN;
      }
      else if (EDIT_FIELD==EDIT_M2RED) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M2WHITE;
      }
      else if (EDIT_FIELD==EDIT_M2GREEN) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M2RED;
      }
      else if (EDIT_FIELD==EDIT_M2BLUE) {
        lcd.setCursor(2,1);
        EDIT_FIELD=EDIT_M2GREEN;
      }
      else if (EDIT_FIELD==EDIT_TSEC) {
        lcd.setCursor(7,1);
        EDIT_FIELD=EDIT_TMIN;
      }
      else if (EDIT_FIELD==EDIT_TMIN) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_THOUR;
      }
      else if (EDIT_FIELD==EDIT_TMONTH) {
        if (dateMonthFirst==false) {
          lcd.setCursor(1,1);
          EDIT_FIELD=EDIT_TDAY;
        }
      }
      else if (EDIT_FIELD==EDIT_TDAY) {
        if (dateMonthFirst==true) {
          lcd.setCursor(1,1);
          EDIT_FIELD=EDIT_TMONTH;
        }
      }
      else if (EDIT_FIELD==EDIT_TYEAR) {
        if (dateMonthFirst==true) {
          lcd.setCursor(4,1);
          EDIT_FIELD=EDIT_TDAY;
        }
        else {
          lcd.setCursor(4,1);
          EDIT_FIELD=EDIT_TMONTH;
        }
      }
      else if (EDIT_FIELD==EDIT_F1_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F1_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F1_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F1_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F1_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F2_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F2_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F2_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F2_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F2_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F3_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F3_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F3_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F3_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F3_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F4_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F4_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F4_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F4_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F4_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F5_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F5_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F5_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F5_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F5_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F6_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_F6_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_F6_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_F6_DURATIONMIN) {
        lcd.setCursor(11,1);
        EDIT_FIELD=EDIT_F6_DURATIONHOUR;
      }
      else if (EDIT_FIELD==EDIT_L_ONMINUTE) {
        lcd.setCursor(1,1);
        EDIT_FIELD=EDIT_L_ONHOUR;
      }
      else if (EDIT_FIELD==EDIT_L_OFFHOUR) {
        lcd.setCursor(4,1);
        EDIT_FIELD=EDIT_L_ONMINUTE;
      }
      else if (EDIT_FIELD==EDIT_L_OFFMINUTE) {
        lcd.setCursor(10,1);
        EDIT_FIELD=EDIT_L_OFFHOUR;
      }
      break;
    }
  }
  // update the value on the LCD
  drawScreen();

  // this delay makes it so the values don't edit to fast. A delay of 250 allows it to change 4 times per second.
  // you can speed up editing by lowering this value, but it makes it hard not to get double clicks
  delay(250);
}


// this draws a fade schedule to the screen with proper formatting
void drawFadeSchedule(byte onHour, byte onMinute, byte durationHours, byte durationMinutes)
{

  // set the cursor to write the time
  lcd.setCursor(0,1);
  boolean pm=false;

  // consider if the user prefers 12 or 24 hour time
  if (_24hr==false) {
    if (onHour >= 12) pm=true;
    if (onHour > 12) onHour=onHour-12;
    if (onHour == 0) onHour=12;
    if (onHour < 10) lcd.print(" ");
  }
  else {
    if (onHour < 10) lcd.print("0");
  }
  lcd.print(onHour);

  fprintf(&lcdout, ":");  
  if (onMinute < 10) lcd.print(0);
  lcd.print(onMinute);

  if (_24hr==false) {
    if (pm == true) fprintf(&lcdout, " PM");
    else fprintf(&lcdout, " AM");
    fprintf(&lcdout, "  ");
  }
  else {
    fprintf(&lcdout, "     ");
  }

  // from here down draws the duration
  lcd.write(byte(2)); // draw the right -> arrow
  lcd.print(durationHours);
  fprintf(&lcdout, ":");  
  if (durationMinutes < 10) lcd.print(0);
  lcd.print(durationMinutes);

}

// this draws the power schedule for the lights to the LCD with proper formatting
void drawLightSchedule(byte onHour, byte onMinute, byte offHour, byte offMinute)
{

  // set the cursor to write the lights on time
  lcd.setCursor(0,1);
  boolean pm=false;

  if (_24hr==false) {
    if (onHour >= 12) pm=true;
    if (onHour > 12) onHour=onHour-12;
    if (onHour == 0) onHour=12;
    if (onHour < 10) lcd.print(" ");
  }
  else {
    if (onHour < 10) lcd.print("0");
  }
  lcd.print(onHour);

  fprintf(&lcdout, ":");  
  if (onMinute < 10) lcd.print(0);
  lcd.print(onMinute);

  if (_24hr==false) {
    if (pm == true) fprintf(&lcdout, "PM");
    else fprintf(&lcdout, "AM");
    fprintf(&lcdout, "  ");
  }
  else {
    fprintf(&lcdout, "    ");
  }

  // write the lights off time

  if (_24hr==false) {
    if (offHour >= 12) pm=true;
    if (offHour > 12) offHour=offHour-12;
    if (offHour == 0) offHour=12;
    if (offHour < 10) lcd.print(" ");
  }
  else {
    if (offHour < 10) lcd.print("0");
  }
  lcd.print(offHour);

  fprintf(&lcdout, ":");  
  if (offMinute < 10) lcd.print(0);
  lcd.print(offMinute);

  if (_24hr==false) {
    if (pm == true) fprintf(&lcdout, "PM");
    else fprintf(&lcdout, "AM");
  }

}

// called by the power alarm, just toggles light power via IR
void AlarmLightPower()
{
  irsend.sendNEC(POWER,32);
}

// this actually starts the fading process when the alarm calls it
void AlarmFade1()
{

  fadeFromMode = 3;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 2;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is low sun
  targetColor.White = lightM1.White;
  targetColor.Red = lightM1.Red;
  targetColor.Green = lightM1.Green;
  targetColor.Blue = lightM1.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade1.durationHours;
  int fadeMins = fade1.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=1;
}

// this actually starts the fading process when the alarm calls it
void AlarmFade2()
{
  fadeFromMode = 2;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 1;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is mid sun
  targetColor.White = lightM2.White;
  targetColor.Red = lightM2.Red;
  targetColor.Green = lightM2.Green;
  targetColor.Blue = lightM2.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade2.durationHours;
  int fadeMins = fade2.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=2;

}

// this actually starts the fading process when the alarm calls it
void AlarmFade3()
{

  fadeFromMode = 1;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 0;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is high sun
  targetColor.White = lightDaylight.White;
  targetColor.Red = lightDaylight.Red;
  targetColor.Green = lightDaylight.Green;
  targetColor.Blue = lightDaylight.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade3.durationHours;
  int fadeMins = fade3.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=3;

}

// this actually starts the fading process when the alarm calls it
void AlarmFade4()
{

  fadeFromMode = 0;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 1;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is mid sun
  targetColor.White = lightM2.White;
  targetColor.Red = lightM2.Red;
  targetColor.Green = lightM2.Green;
  targetColor.Blue = lightM2.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade4.durationHours;
  int fadeMins = fade4.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=4;

}

// this actually starts the fading process when the alarm calls it
void AlarmFade5()
{
  fadeFromMode = 1;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 2;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is low sun
  targetColor.White = lightM1.White;
  targetColor.Red = lightM1.Red;
  targetColor.Green = lightM1.Green;
  targetColor.Blue = lightM1.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade5.durationHours;
  int fadeMins = fade5.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=5;

}

// this actually starts the fading process when the alarm calls it
void AlarmFade6()
{
  fadeFromMode = 2;  //0=high sun, 1=mid sun, 2=low sun, 3=moon
  fadeToMode = 3;  //0=high sun, 1=mid sun, 2=low sun, 3=moon

  // last color is the starting point of the fade
  lastColor.White = currentColor.White;
  lastColor.Red = currentColor.Red;
  lastColor.Green = currentColor.Green;
  lastColor.Blue = currentColor.Blue;

  // target color is moonlight
  targetColor.White = lightMoon.White;
  targetColor.Red = lightMoon.Red;
  targetColor.Green = lightMoon.Green;
  targetColor.Blue = lightMoon.Blue;

  // calculate how long to run the fade for
  int fadeHours = fade6.durationHours;
  int fadeMins = fade6.durationMinutes;  
  fadeDurationSeconds = ((fadeHours*60*60)+(fadeMins*60));

  fadeStartingSeconds = now();
  fadeInProgress = true;
  currentLightMode=4;
  lastFade=6;

}

// this just calculates free SRAM
int freeRam ()
{
  // Returns available SRAM
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


/*-------------------------------------------------------------------------
 checkLightFade()
 
 This is the actual cross fading code that blends from each lighting mode to another.
 It has been slightly modified, but was posted by Curt_Planted at The Planted Tank forums.
 Every time it's called, it checks if it should be sending an IR command to continue the fade.
 It checks each color channel individually.
 -------------------------------------------------------------------------*/

void checkLightFade(int secondsElapsed, int durationInSeconds)
{  

  // RED /////////////////////////////////////////////////////////
  if (targetColor.Red != lastColor.Red)
  {
    // Get the change per second for the current color to the target color
    float RTick = (float)durationInSeconds / (float)(targetColor.Red - lastColor.Red);

    // Get the expected change for the time elapsed
    int RValue = round(secondsElapsed / RTick); 

    if (RTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if (lastColor.Red + RValue > currentColor.Red) 
      {
        // And, for protection, the current color does not exceed the target currentColor.
        if (currentColor.Red < targetColor.Red)
        {
          // Increase the current color
          currentColor.Red++;

          //Serial.print("red up\n");
          irsend.sendNEC(REDUP,32);
          delay(333);

        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if ((lastColor.Red + RValue) < currentColor.Red)
      {
        // And, for protection, the current color is not less than the target currentColor.
        if (currentColor.Red > targetColor.Red)
        {
          // Decrease the current color
          currentColor.Red--;
          //Serial.print("red down\n");
          irsend.sendNEC(REDDOWN,32);
          delay(333);
        }
      } 
    }
  }

  // GREEN /////////////////////////////////////////////////////////
  if (targetColor.Green != lastColor.Green)
  {
    // Get the change per second for the current color to the target color
    float GTick = (float)durationInSeconds / (float)(targetColor.Green - lastColor.Green);

    // Get the expected change for the time elapsed
    int GValue = round(secondsElapsed / GTick);

    if (GTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if (lastColor.Green + GValue > currentColor.Green)
      {
        // And, for protection, the current color does not exceed the target currentColor.
        if (currentColor.Green < targetColor.Green)
        {
          // Increase the current color
          currentColor.Green++;
          //Serial.print("green up\n");
          irsend.sendNEC(GREENUP,32);
          delay(333);
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if ((lastColor.Green + GValue) < currentColor.Green)
      {        
        // And, for protection, the current color is not less than the target currentColor.
        if (currentColor.Green > targetColor.Green)
        {
          // Decrease the current color
          currentColor.Green--;
          //Serial.print("green down\n");
          irsend.sendNEC(GREENDOWN,32);
          delay(333);
        }
      } 
    }
  }

  // BLUE /////////////////////////////////////////////////////////
  if (targetColor.Blue != lastColor.Blue)
  {
    // Get the change per second for the current color to the target color
    float BTick = (float)durationInSeconds / (float)(targetColor.Blue - lastColor.Blue);

    // Get the expected change for the time elapsed
    int BValue = round(secondsElapsed / BTick);

    if (BTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if (lastColor.Blue + BValue > currentColor.Blue)
      {
        // And, for protection, the current color does not exceed the target currentColor.
        if (currentColor.Blue < targetColor.Blue)
        {
          // Increase the current color
          currentColor.Blue++;
          //Serial.print("blue up\n");
          irsend.sendNEC(BLUEUP,32);
          delay(333);
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if ((lastColor.Blue + BValue) < currentColor.Blue)
      {
        // And, for protection, the current color is not less than the target currentColor.
        if (currentColor.Blue > targetColor.Blue)
        {
          // Decrease the current color
          currentColor.Blue--;
          //Serial.print("blue down\n");
          irsend.sendNEC(BLUEDOWN,32);
          delay(333);
        }
      }
    }
  }

  // WHITE /////////////////////////////////////////////////////////
  if (targetColor.White != lastColor.White)
  {
    // Get the change per second for the current color to the target color
    float WTick = (float)durationInSeconds / (float)(targetColor.White - lastColor.White);

    // Get the expected change for the time elapsed
    int WValue = round(secondsElapsed / WTick);

    if (WTick > 0) // If the change is positive
    {
      // And if the current color does not match the expected color
      if (lastColor.White + WValue > currentColor.White)
      {
        // And, for protection, the current color does not exceed the target currentColor.
        if (currentColor.White < targetColor.White)
        {
          // Increase the current color
          currentColor.White++;
          //Serial.print("white up\n");
          irsend.sendNEC(WHITEUP,32);
          delay(333);
        }
      }
    }
    else // If the change is negative
    {
      // And if the current color does not match the expected color
      if ((lastColor.White + WValue) < currentColor.White)
      {
        // And, for protection, the current color is not less than the target currentColor.
        if (currentColor.White > targetColor.White)
        {
          // Decrease the current color
          currentColor.White--;
          //Serial.print("white down\n");
          irsend.sendNEC(WHITEDOWN, 32);
          delay(333);
        }
      } 
    }
  }
}

/*----------------------------------------------------------------------------
 smartStartup()
 
 This routine starts up the lights in the correct mode (or the closest possible thing).
 It does this by looking at the fade schedules and determining which lighting mode we should be in.
 It cannot resume a mid-fade, so it chooses the closest lighting mode, where it will stay until the next fade resumes.
 ----------------------------------------------------------------------------*/

void smartStartup()
{

  RTC.get();

  int onHour, offHour;

  // get the current time in UNIX time, which is FAR easier for time calculations and comparisons
  time_t currentTime = tmConvert_t(year(),month(),day(),hour(),minute(),second());

  Serial.print("currentTime: ");
  Serial.print(currentTime);
  Serial.print("\n");

  // first check to see if we are in fade 1
  time_t fadeStart = tmConvert_t(year(),month(),day(),fade1.onHour,fade1.onMinute,0); // get a unix time stamp for the fade start

  // first we need to check if we are before fade 1
  if (fadeStart>currentTime) // if we are before fade 1
  {
    Serial.println("before fade 1");
    irsend.sendNEC(MOON,32); // flip to moon
    currentLightMode=3;
    lastFade=6;
  }
  else if (currentTime>=fadeStart) // if we are past the start of  fade 1
  {
    //Serial.println("past fade 1 start");
    unsigned long fadeEnd=((fade1.durationHours*60)+(fade1.durationMinutes))*60; // calculate how many seconds long the fade is
    fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
    if (currentTime<fadeEnd) // if we are before the end of the fade
    {
      //Serial.println("in fade 1");
      int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
      int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
      if (secondsBackToStart>secondsToEnd) 
      {
        irsend.sendNEC(MOON,32); // flip to moonlight
        currentLightMode=3;
        lastFade=1;
      }
      else 
      {
        irsend.sendNEC(M1,32); // flip to low sun
        currentLightMode=2;
        lastFade=1;
      }
    }

    else // move on to fade 2
    {
      // Serial.println("checking fade 2");
      fadeStart = tmConvert_t(year(),month(),day(),fade2.onHour,fade2.onMinute,0); // get a unix time stamp for the fade start

      // first we need to check if we are between the previous fade and this fade
      if (fadeStart>currentTime) // if we are not in the previous fade, but are still before this one
      {
        //Serial.println("in before fade 2");
        irsend.sendNEC(M1,32); // flip to low sun
        currentLightMode=2;
        lastFade=1;
      }
      else if (currentTime>=fadeStart) // if we are past the start of  fade 2
      {
        //Serial.println("past fade 2 start");
        unsigned long fadeEnd=((fade2.durationHours*60)+(fade2.durationMinutes))*60; // calculate how many seconds long the fade is
        fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
        if (currentTime<fadeEnd) // if we are before the end of the fade
        {
          //Serial.println("in fade 2");
          int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
          int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
          if (secondsBackToStart>secondsToEnd) 
          {
            irsend.sendNEC(M1,32); // flip to low sun
            currentLightMode=2;
            lastFade=2;
          }
          else 
          {
            irsend.sendNEC(M1,32); // flip to mid sun
            currentLightMode=1;
            lastFade=2;
          }
        }

        else // move on to fade 3
        {
          //Serial.println("checking fade 3");
          fadeStart = tmConvert_t(year(),month(),day(),fade3.onHour,fade3.onMinute,0); // get a unix time stamp for the fade start
          // first we need to check if we are between the previous fade and this fade
          if (fadeStart>currentTime) // if we are not in the previous fade, but are still before this one
          {
            //Serial.println("before fade 3");
            irsend.sendNEC(M2,32); // flip to mid sun
            currentLightMode=1;
            lastFade=2;
          }
          else if (currentTime>=fadeStart) // if we are past the start of  fade 3
          {
            //Serial.println("past fade 3 start");
            unsigned long fadeEnd=((fade3.durationHours*60)+(fade3.durationMinutes))*60; // calculate how many seconds long the fade is
            fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
            if (currentTime<fadeEnd) // if we are before the end of the fade
            {
              //Serial.println("in fade 3");
              int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
              int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
              if (secondsBackToStart>secondsToEnd) 
              {
                irsend.sendNEC(M2,32); // flip to mid sun
                currentLightMode=1;
                lastFade=3;
              }
              else 
              {
                irsend.sendNEC(DAYLIGHT,32); // flip to high sun
                currentLightMode=0;
                lastFade=3;
              }
            }

            else // move on to fade 4
            //Serial.println("checking fade 4");
            {
              fadeStart = tmConvert_t(year(),month(),day(),fade4.onHour,fade4.onMinute,0); // get a unix time stamp for the fade start
              // first we need to check if we are between the previous fade and this fade
              if (fadeStart>currentTime) // if we are not in the previous fade, but are still before this one
              {
                //Serial.println("before fade 4");
                irsend.sendNEC(DAYLIGHT,32); // flip to high sun
                currentLightMode=0;
                lastFade=3;
              }
              else if (currentTime>=fadeStart) // if we are past the start of  fade 4
              {
                //Serial.println("past fade 4 start");
                unsigned long fadeEnd=((fade4.durationHours*60)+(fade4.durationMinutes))*60; // calculate how many seconds long the fade is
                fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
                if (currentTime<fadeEnd) // if we are before the end of the fade
                {
                  //Serial.println("in fade 4");
                  int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
                  int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
                  if (secondsBackToStart>secondsToEnd) 
                  {
                    irsend.sendNEC(DAYLIGHT,32); // flip to high sun
                    currentLightMode=0;
                    lastFade=4;
                  }
                  else 
                  {
                    irsend.sendNEC(M2,32); // flip to mid sun
                    currentLightMode=1;
                    lastFade=4;
                  }
                }

                else // move on to fade 5
                {    
                  // Serial.println("checking fade 5");
                  fadeStart = tmConvert_t(year(),month(),day(),fade5.onHour,fade5.onMinute,0); // get a unix time stamp for the fade start
                  // first we need to check if we are between the previous fade and this fade
                  if (fadeStart>currentTime) // if we are not in the previous fade, but are still before this one
                  {
                    //Serial.println("before fade 5");
                    irsend.sendNEC(M2,32); // flip to mid sun
                    currentLightMode=1;
                    lastFade=4;
                  }
                  else if (currentTime>=fadeStart) // if we are past the start of  fade 5
                  {
                    //Serial.println("past fade 5 start");
                    unsigned long fadeEnd=((fade5.durationHours*60)+(fade5.durationMinutes))*60; // calculate how many seconds long the fade is
                    fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
                    if (currentTime<fadeEnd) // if we are before the end of the fade
                    {
                      //Serial.println("in fade 5");
                      int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
                      int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
                      if (secondsBackToStart>secondsToEnd) 
                      {
                        irsend.sendNEC(M2,32); // flip to mid sun
                        currentLightMode=1;
                        lastFade=5;
                      }
                      else 
                      {
                        irsend.sendNEC(M1,32); // flip to low sun
                        currentLightMode=2;
                        lastFade=5;
                      }
                    }

                    else // move on to fade 6
                    {
                      //Serial.println("checking fade 6");
                      fadeStart = tmConvert_t(year(),month(),day(),fade6.onHour,fade6.onMinute,0); // get a unix time stamp for the fade start
                      // first we need to check if we are between the previous fade and this fade
                      if (fadeStart>currentTime) // if we are not in the previous fade, but are still before this one
                      {
                        //Serial.println("before fade 6");
                        irsend.sendNEC(M1,32); // flip to low sun
                        currentLightMode=2;
                        lastFade=5;
                      }
                      else if (currentTime>=fadeStart) // if we are past the start of  fade 6
                      {
                        //Serial.println("past fade 6 start");
                        unsigned long fadeEnd=((fade6.durationHours*60)+(fade6.durationMinutes))*60; // calculate how many seconds long the fade is
                        fadeEnd=(fadeStart+fadeEnd); // add the fade seconds to the fade start to get the end time of the fade
                        if (currentTime<fadeEnd) // if we are before the end of the fade
                        {
                          //Serial.println("in fade 6");
                          int secondsBackToStart=(currentTime-fadeStart); // calculate how long since it started
                          int secondsToEnd=(fadeEnd-currentTime); // calculate how many seconds until the end
                          if (secondsBackToStart>secondsToEnd) 
                          {
                            irsend.sendNEC(M1,32); // flip to low sun
                            currentLightMode=2;
                            lastFade=6;
                          }
                          else 
                          {
                            irsend.sendNEC(MOON,32); // flip to moonlight
                            currentLightMode=3;
                            lastFade=6;
                          }
                        }
                        else // if we are after everything else
                        {
                          //Serial.println("past fade 6 end");
                          irsend.sendNEC(MOON,32); // flip to moonlight
                          currentLightMode=3;
                          lastFade=6;
                        }

                      }
                    }
                  }
                }
              }
            }
          }
        }
      }    
    }
  }
}

// this is used to convert time, very useful. :)
time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet);         //convert to time_t
}

// This is ran only at startup, it loads all of the WRGB values into memory from EEPROM.
// If the values are not correct, it'll be automatically reset to 0, which would happen with a new arduino.
void loadLightWRGB() 
{

  // read in stored RGBW values for each mode from memory
  lightDaylight.Red=EEPROM.read(200);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightDaylight.Red <= 42) && (lightDaylight.Red >= 0)) ;
  else {
    lightDaylight.Red=0;
    EEPROM.write(200,0);
  }
  lightDaylight.Green=EEPROM.read(201);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightDaylight.Green <= 42) && (lightDaylight.Green >= 0))  ;
  else {
    lightDaylight.Green=0;
    EEPROM.write(201,0);
  }
  lightDaylight.Blue=EEPROM.read(202);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightDaylight.Blue <= 42) && (lightDaylight.Blue >= 0))  ;
  else {
    lightDaylight.Blue=0;
    EEPROM.write(202,0);
  }
  lightDaylight.White=EEPROM.read(203);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightDaylight.White <= 42) && (lightDaylight.White >= 0))  ;
  else {
    lightDaylight.White=0;
    EEPROM.write(203,0);
  }

  lightM2.Red=EEPROM.read(204);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM2.Red <= 42) && (lightM2.Red >= 0))  ;
  else {
    lightM2.Red=0;
    EEPROM.write(204,0);
  }
  lightM2.Green=EEPROM.read(205);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM2.Green <= 42) && (lightM2.Green >= 0))  ;
  else {
    lightM2.Green=0;
    EEPROM.write(205,0);
  }
  lightM2.Blue=EEPROM.read(206);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM2.Blue <= 42) && (lightM2.Blue >= 0))  ;
  else {
    lightM2.Blue=0;
    EEPROM.write(206,0);
  }
  lightM2.White=EEPROM.read(207);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM2.White <= 42) && (lightM2.White >= 0))  ;
  else {
    lightM2.White=0;
    EEPROM.write(207,0);
  }

  lightM1.Red=EEPROM.read(208);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM1.Red <= 42) && (lightM1.Red >= 0))  ;
  else {
    lightM1.Red=0;
    EEPROM.write(208,0);
  }
  lightM1.Green=EEPROM.read(209);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM1.Green <= 42) && (lightM1.Green >= 0))  ;
  else {
    lightM1.Green=0;
    EEPROM.write(209,0);
  }
  lightM1.Blue=EEPROM.read(210);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM1.Blue <= 42) && (lightM1.Blue >= 0))  ;
  else {
    lightM1.Blue=0;
    EEPROM.write(210,0);
  }
  lightM1.White=EEPROM.read(211);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightM1.White <= 42) && (lightM1.White >= 0))  ;
  else {
    lightM1.White=0;
    EEPROM.write(211,0);
  }

  lightMoon.Red=EEPROM.read(212);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightMoon.Red <= 42) && (lightMoon.Red >= 0))  ;
  else {
    lightMoon.Red=0;
    EEPROM.write(212,0);
  }
  lightMoon.Green=EEPROM.read(213);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightMoon.Green <= 42) && (lightMoon.Green >= 0))  ;
  else {
    lightMoon.Green=0;
    EEPROM.write(213,0);
  }
  lightMoon.Blue=EEPROM.read(214);
  // if the value is out of range, set it to 0 and re-save it
  if ((lightMoon.Blue <= 42) && (lightMoon.Blue >= 0))  ;
  else {
    lightMoon.Blue=0;
    EEPROM.write(214,0);
  }
  lightMoon.White=EEPROM.read(215);  
  // if the value is out of range, set it to 0 and re-save it
  if ((lightMoon.White <= 42) && (lightMoon.White >= 0))  ;
  else {
    lightMoon.White=0;
    EEPROM.write(215,0);

// END KMAN SAT+ CODE CHANGES, SMARTSTART MAX VALUE MODIFICATIONS void loadLightWRGB() from 100 to 42

  }

}  
