//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "LCDi2c.h"

 //=====[Declaration and initialization of public global objects]===============
AnalogIn potentiometer(A0);
AnalogIn lm35(A1); // 10 mV/\xB0 C
PwmOut buzzer(D5);
AnalogIn mq2(A2);
DigitalOut led(LED3);
#define NUMBER_OF_KEYS                           4
#define BLINKING_TIME_GAS_ALARM               1000
#define BLINKING_TIME_OVER_TEMP_ALARM          500
#define BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM  100
#define NUMBER_OF_AVG_SAMPLES                   100
#define OVER_TEMP_LEVEL                         50
#define TIME_INCREMENT_MS                       10
#define DEBOUNCE_KEY_TIME_MS                    40
#define KEYPAD_NUMBER_OF_ROWS                    4
#define KEYPAD_NUMBER_OF_COLS                    4
#define EVENT_MAX_STORAGE                      100
#define EVENT_NAME_MAX_LENGTH                   14
LCDi2c lcd(LCD20x4);
 UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
//=====[Declaration and initialization of public global variables]=============
typedef enum {
    MATRIX_KEYPAD_SCANNING,
    MATRIX_KEYPAD_DEBOUNCE,
    MATRIX_KEYPAD_KEY_HOLD_PRESSED
} matrixKeypadState_t;
char receivedChar = '\0';
char str[100] = "";
char alrm1[100] = "";
char alrm2[100] = "";
char alrm3[100] = "";
int yo = 0;
char alrm4[100] = "";
char alrm5[100] = "";
char codeSequence[NUMBER_OF_KEYS]   = { '1', '5', '9', '0' };
char keyPressed[NUMBER_OF_KEYS] = { '0', '0', '0', '0' };
typedef struct systemEvent {
    time_t seconds;
    char typeOfEvent[EVENT_NAME_MAX_LENGTH];
} systemEvent_t;

DigitalOut keypadRowPins[KEYPAD_NUMBER_OF_ROWS] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn keypadColPins[KEYPAD_NUMBER_OF_COLS]  = {PB_12, PB_13, PB_15, PC_6};

 bool quit = false;
     int cony = 0;
     int pas = 0;

float lm35Reading = 0.0; // Raw ADC input A0 value
float lm35TempC = 0.0;   // Temperature in Celsius degrees [\xB0 C]
float lm35TempF = 0.0;   // Temperature in Fahrenheit degrees [\xB0 F]
int numberOfHashKeyReleasedEvents = 0;
float gasread = 0.0;
float tempread = 0.0;
float potentiometerReading = 0.0;   // Raw ADC input A1 value
float potentiometerScaledToC = 0.0; // Potentiometer value scaled to Celsius degrees [\xB0 C]
float potentiometerScaledToF = 0.0; // Potentiometer value scaled to Fahrenheit degrees [\xB0 F]
int accumulatedDebounceMatrixKeypadTime = 0;
int matrixKeypadCodeIndex = 0;
int code = 0;
char matrixKeypadLastKeyPressed = '\0';
char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D',
};
matrixKeypadState_t matrixKeypadState;

int eventsIndex            = 0;
systemEvent_t arrayOfStoredEvents[EVENT_MAX_STORAGE];

bool showKeypadInUart = true;
struct tm rtcTime;
int flagg = 0;
//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
 void uartTask();
bool alarmG;
bool alarmT;
void matrixKeypadInit();
char matrixKeypadUpdate();
void keypadToUart();
void eventLogUpdate();
char matrixKeypadScan();
void systemElementStateUpdate( bool lastState,
                               bool currentState,
                               const char* elementName );
 float analogReadingScaledWithTheLM35Formula( float analogReading );;
 float potentiometerScaledToCelsius( float analogValue );
void pcSerialComStringWrite( const char* str );
char pcSerialComCharRead();
void pcSerialComStringRead( char* str, int strLength );
void pcSerialComCharWrite( char chr );

int flag = 0;
int main()
{   
    int n = 0; 
    inputsInit();
    while( true ) { 
       lcd.display(CURSOR_OFF);
       lcd.display(BLINK_OFF);
                char str[100] = "";
    	        lcd.locate(0,0);
                lm35Reading = lm35.read();
                if(lm35TempC >= 30){
                    alarmT = HIGH;
                    lcd.locate(13,0);
                    lcd.printf("<\\\\>");
                } else {
                    lcd.locate(13,0);
                    lcd.printf("     ");
                }      
                lcd.locate(0,0);          
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                str[0] = '\0';
            sprintf ( str, "LM35:%.2f C", lm35TempC);
                lcd.printf("%s ", str);
                lcd.locate(10,0);
                lcd.printf("%c",(char)223);
                str[0] = '\0';
    gasread = mq2.read();
       	        lcd.locate(0,1);
   if(gasread >=0.2){
       if(flag == 0){
           flagg = 0;
 lcd.printf("Gas:detected!   ");
 alarmG = HIGH;
 lcd.printf("%c%c", (char)179, (char)179);
 flag = 1;
       }
   } else if(flagg == 0){
       flag = 0;
        lcd.printf("Gas:not detected  ");
        flagg = 1;
   } 
   if(alarmG == HIGH || alarmT == HIGH){   
       lcd.locate(0,2);        
   lcd.printf("Alarm: Alert");  
     lcd.locate(0,3);
   lcd.printf("Enter code:");    
   while(pas <= 3){
       keypadToUart();
       delay(10);
       if (pas == 4){
          pas = 0;
   alarmG = LOW;
   alarmT = LOW;
   lcd.locate(0,3);
   lcd.printf("             ");
   break;
       }
   }
   
   }else{
       lcd.locate(0,2);
          lcd.printf("Alarm: Safe ");    
   }
                delay(1000);  
                }                 
     }
float analogReadingScaledWithTheLM35Formula( float analogReading )
{
    return analogReading * 330.0;
}

float celsiusToFahrenheit( float tempInCelsiusDegrees )
{
    return 9.0/5.0 * tempInCelsiusDegrees + 32.0;
}

float potentiometerScaledToCelsius( float analogValue )
{
    return 148.0 * analogValue + 2.0;
}

float potentiometerScaledToFahrenheit( float analogValue )
{
    return celsiusToFahrenheit( potentiometerScaledToCelsius(analogValue) );
}

void inputsInit()
{
    matrixKeypadInit();
}
void pcSerialComStringWrite( const char* str )
{
    uartUsb.write( str, strlen(str) );
}

char pcSerialComCharRead()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
    }
    return receivedChar;
}
void matrixKeypadInit()
{
    matrixKeypadState = MATRIX_KEYPAD_SCANNING;
    int pinIndex = 0;
    for( pinIndex=0; pinIndex<KEYPAD_NUMBER_OF_COLS; pinIndex++ ) {
        (keypadColPins[pinIndex]).mode(PullUp);
    }
}

char matrixKeypadScan()
{
    int r = 0;
    int c = 0;
    int i = 0;

    for( r=0; r<KEYPAD_NUMBER_OF_ROWS; r++ ) {

        for( i=0; i<KEYPAD_NUMBER_OF_ROWS; i++ ) {
            keypadRowPins[i] = ON;
        }

        keypadRowPins[r] = OFF;

        for( c=0; c<KEYPAD_NUMBER_OF_COLS; c++ ) {
            if( keypadColPins[c] == OFF ) {
                return matrixKeypadIndexToCharArray[r*KEYPAD_NUMBER_OF_ROWS + c];
            }
        }
    }
    return '\0';
}

char matrixKeypadUpdate()
{
    char keyDetected = '\0';
    char keyReleased = '\0';

    switch( matrixKeypadState ) {

    case MATRIX_KEYPAD_SCANNING:
        keyDetected = matrixKeypadScan();
        if( keyDetected != '\0' ) {
            matrixKeypadLastKeyPressed = keyDetected;
            accumulatedDebounceMatrixKeypadTime = 0;
            matrixKeypadState = MATRIX_KEYPAD_DEBOUNCE;
        }
        break;

    case MATRIX_KEYPAD_DEBOUNCE:
        if( accumulatedDebounceMatrixKeypadTime >=
            DEBOUNCE_KEY_TIME_MS ) {
            keyDetected = matrixKeypadScan();
            if( keyDetected == matrixKeypadLastKeyPressed ) {
                matrixKeypadState = MATRIX_KEYPAD_KEY_HOLD_PRESSED;
            } else {
                matrixKeypadState = MATRIX_KEYPAD_SCANNING;
            }
        }
        accumulatedDebounceMatrixKeypadTime =
            accumulatedDebounceMatrixKeypadTime + TIME_INCREMENT_MS;
        break;

    case MATRIX_KEYPAD_KEY_HOLD_PRESSED:
        keyDetected = matrixKeypadScan();
        if( keyDetected != matrixKeypadLastKeyPressed ) {
            if( keyDetected == '\0' ) {
                keyReleased = matrixKeypadLastKeyPressed;
            }
            matrixKeypadState = MATRIX_KEYPAD_SCANNING;
        }
        break;

    default:
        matrixKeypadInit();
        break;
    }
    return keyReleased;
}


void pcSerialComStringRead( char* str, int strLength )
{
    int strIndex;
    for ( strIndex = 0; strIndex < strLength; strIndex++) {
        uartUsb.read( &str[strIndex] , 1 );
        uartUsb.write( &str[strIndex] ,1 );
    }
    str[strLength]='\0';
}

void pcSerialComCharWrite( char chr )
{
    char str[2] = "";
    sprintf (str, "%c", chr);
    uartUsb.write( str, strlen(str) );
}

    int n = 11;
void keypadToUart()
{
    int keyPressed;
    int i;
yo = 0;
    if ( showKeypadInUart ) {
        keyPressed = matrixKeypadUpdate();
        if ( keyPressed != 0 ) {
            lcd.locate(n,3);
            lcd.printf("*");
            cony++;
            if (keyPressed == codeSequence[cony - 1]){
               pas++;
            } else{
                pas = 0;
            }               
            n++;
            if (n == 15){
                n = 11;
                lcd.locate(n,3);
                delay(1000);
                lcd.printf("    ");
                if(pas != 4){
                    lcd.locate(0,3);
                    lcd.printf("Wrong code!");
                    delay(1000);
                    lcd.locate(0,3);
                       lcd.printf("Enter code:");
                }
            }
            if(cony == 4){
                cony = 0;
            }

        }
    }
}