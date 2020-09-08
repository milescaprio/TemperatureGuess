//Version 1.0.1, last updated 9/8/2020

//* DON'T FORGET TO DOWNLOAD AND INSTALL THE DHT_nonblocking, Servo, LiquidCrystal, and Keypad LIBRARIES *
//  JUST EXTRACT THEM IN ARDUINO LIBRARIES FOLDER
//  Get them here: https://github.com/PunkyMunky64/TemperatureGuess/tree/master

#include <dht_nonblocking.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

//pins, easy for ya guys to change right here:)
#define SERVOPIN A4
#define ONBOARDLED 13
#define DHTPIN 10
#define COL_0_PIN 9
#define COL_1_PIN 8
#define COL_2_PIN 7
#define COL_3_PIN 6
#define ROW_0_PIN A0
#define ROW_1_PIN A1
#define ROW_2_PIN A2
#define ROW_3_PIN A3

#define DHT_SENSOR_TYPE DHT_TYPE_11
//#define DHT_SENSOR_TYPE DHT_TYPE_21
//#define DHT_SENSOR_TYPE DHT_TYPE_22
//^^choose your DHT SENSOR TYPE^^
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo myservo;

//internal game settings
const float DIFF_TOL_C[4] = {2, 1.5, 1, 0.5}; //How far you have to be off to get a win, depending on difficulty. Based on temp unit. Easy to reprogram
const float DIFF_TOL_F[4] = {3.6 , 2.7, 1.8, 0.9};
const uint8_t ACCURACY = 3; //How many times to measure temp at the beginning. Takes longer to do more measures. Up to 255

//Game setting vars
uint8_t programLocation = 0;
uint8_t difficulty = 0;//0 is easy, 1 is medium, 2 is hard, 3 is epic
bool isFahrenheit = 0; //Temperature measuring unit, 1=Fahrenheit 0=Celsius

//keypad reading and interpreting vars
uint8_t digitHundredths = 0;
uint8_t digitTenths = 0;
uint8_t digitOnes = 0;
uint8_t digitTens = 0;
uint8_t digitHundreds = 0;
uint8_t index = 0;
char inputTemperature[16] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'}; //a list of button pressed on the keypad for the input temperature
int8_t asteriskIn;
float guessedTemp;
bool win;

//Keypad vars
char gotKey;
const byte ROWS = 4; // Four rows
const byte COLS = 4; // columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {ROW_0_PIN, ROW_1_PIN, ROW_2_PIN, ROW_3_PIN};// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte colPins[COLS] = {COL_0_PIN, COL_1_PIN, COL_2_PIN, COL_3_PIN};// Connect keypad COL0, COL1, COL2 and COL3 to these Arduino pins.
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS ); //Make keypad

//DHT sensor vars
float temperature;
float humidity;
byte data[40] = {0};
float tempMeasure = 0;
uint8_t measureCount = 0; //A count of how many times the temperature has been measure so far. Goes up to ACCURACY
float tempCelsius;
float tempFahrenheit;
DHT_nonblocking dht_sensor(10, DHT_SENSOR_TYPE);

int in_array_char(const char store[], const int storeSize, const char query) {
  if (query == store[0]) {
    return 0;
  }
  if (query == store[1]) {
    return 1;
  }
  if (query == store[2]) {
    return 2;
  }
  if (query == store[3]) {
    return 3;
  }
  if (query == store[4]) {
    return 4;
  }
  if (query == store[5]) {
    return 5;
  }
  if (query == store[6]) {
    return 6;
  }
  if (query == store[7]) {
    return 7;
  }
  return -1;
} //A really stupid way of seeing if something exists in an array; my other function was getting too much errors so I gave up D:
  //If you add more digits to the maximum, add more query-stores.
  
float charlistToTemp(char charlist[]) {
  asteriskIn = in_array_char(charlist, sizeof(charlist), '*');
  Serial.println("Asterisk/Decimal Point Location (asteriskIn):");
  Serial.println(asteriskIn);
  Serial.println("Length of Keypad String (index):");
  Serial.println(index);

  if (charlist[0] != '#' && asteriskIn == -1) {
    if (index == 3) {
      digitHundreds = charlist[0] - '0';
      digitTens = charlist[1] - '0';
      digitOnes = charlist[2] - '0';
    }

    if (index == 2) {
      digitTens = charlist[0] - '0';
      digitOnes = charlist[1] - '0';
    }

    if (index == 1) {
      digitOnes = charlist[0] - '0';
    }
  }

  if (charlist[0] != '#' && asteriskIn != -1) {
    if (asteriskIn == 3) {
      digitHundreds = charlist[0] - '0';
      digitTens = charlist[1] - '0';
      digitOnes = charlist[2] - '0';
    }

    if (asteriskIn == 2) {
      digitTens = charlist[0] - '0';
      digitOnes = charlist[1] - '0';
    }

    if (asteriskIn == 1) {
      digitOnes = charlist[0] - '0';
    }
    if (index - asteriskIn == 2) { //index is like a length variable, but asteriskIn is like an index variable (yes that is confusing) so it is only checking for one digit in between,
      //but the number is 2 because of the zero-starting property in arrays and the way cpp (and many other languages) take lengths. It also matters how the rest of my code is formatted to output "index".
      digitTenths = charlist[asteriskIn + 1] - '0';
    }
    if (index - asteriskIn >= 3) { //same rule, I am stopping here, making the max you can put in 2 digits
      digitTenths = charlist[asteriskIn + 1] - '0';
      digitHundredths = charlist[asteriskIn + 2] - '0';
    }
  }
  //copying all this but with the "negative" symbol
  if (charlist[0] == '#' && asteriskIn == -1) {
    if (index == 4) { //increase index by one to factor in the # symbol
      digitHundreds = charlist[1] - '0';
      digitTens = charlist[2] - '0';
      digitOnes = charlist[3] - '0';
    }

    if (index == 3) {
      digitTens = charlist[1] - '0';
      digitOnes = charlist[2] - '0';
    }

    if (index == 2) {
      digitOnes = charlist[1] - '0';
    }
  }

  if (charlist[0] == '#' && asteriskIn != -1) {
    if (asteriskIn == 4) {
      digitHundreds = charlist[1] - '0';
      digitTens = charlist[2] - '0';
      digitOnes = charlist[3] - '0';
    }

    if (asteriskIn == 3) {
      digitTens = charlist[1] - '0';
      digitOnes = charlist[2] - '0';
    }

    if (asteriskIn == 2) {
      digitOnes = charlist[1] - '0';
    }

    if (index - asteriskIn == 2) { //index is like a length variable, but asteriskIn is like an index variable (yes that is confusing) so it is only checking for one digit in between,
      //but the number is 2 because of the zero-starting property in arrays and the way cpp (and many other languages) take lengths. It also matters how the rest of my code is formatted to output "index".
      digitTenths = charlist[asteriskIn + 1] - '0';
    }
    if (index - asteriskIn >= 3) { //same rule, I am stopping here, making the max you can put in 2 digits
      digitTenths = charlist[asteriskIn + 1] - '0';
      digitHundredths = charlist[asteriskIn + 2] - '0';
    }
  }
  int8_t negative = 1;
  if (charlist[0] == '#') {
    negative = -1;
  }
  Serial.println("Digits:");
  if (negative) {
    Serial.println("-");
  }
  Serial.println(digitHundreds);
  Serial.println(digitTens);
  Serial.println(digitOnes);
  Serial.println(".");
  Serial.println(digitTenths);
  Serial.println(digitHundredths);
  float output = (digitHundreds * 100 + digitTens * 10 + digitOnes + digitTenths * 0.1 + digitHundredths * 0.01) * negative;
  return output;
}

void checkTemperature() {
  guessedTemp = charlistToTemp(inputTemperature);
  Serial.println("Guessed Temperature (guessedTemp):");
  Serial.println(guessedTemp);
  float Fdiff = abs(guessedTemp - tempFahrenheit);
  float Cdiff = abs(guessedTemp - tempCelsius);
  if (isFahrenheit){
    Serial.println("Guessed Temperature Difference (Fdiff):");
    Serial.println(Fdiff);
  } else {
    Serial.println("Guessed Temperature Difference (Cdiff):");
    Serial.println(Cdiff);
  }
  if (isFahrenheit) {
    win = (abs(guessedTemp - tempFahrenheit) <= DIFF_TOL_F[difficulty]); //win is a boolean, it takes the condition of (abs(..., so it is like a shortened if loop.
  } else {
    win = (abs(guessedTemp - tempCelsius) <= DIFF_TOL_C[difficulty]);
    //I could have shortened this further by putting the isFahrenheit all in with the win statement by multiplying it with
    //the isFahrenheit for the fahrenheit part and !isFahrenheit for the celsius part, but that felt like overobfuscating
    //like this
    //win = (abs(guessedTemp - tempFahrenheit) <= DIFF_TOL_F[difficulty])*isFahrenheit + (abs(guessedTemp - tempCelsius) <= DIFF_TOL_C[difficulty])*(!isFahrenheit)
  }
}

void setup() {
  pinMode(ONBOARDLED, OUTPUT);
  myservo.attach(SERVOPIN);
  digitalWrite(ONBOARDLED, LOW); // Turn the LED off
  myservo.write(0);
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.setCursor(0, 0);
  lcd.print("Measuring Temp");
  lcd.setCursor(0, 1);
  lcd.print("Most Fun Outside");
  while (measureCount < ACCURACY) {
    if ((dht_sensor.measure(&temperature, &humidity))) {
      tempMeasure += temperature;
      Serial.println("Measured");
      //Serial.println(temperature);
      //Uncomment the above line for debugging or to cheat:)
      measureCount++;
    }
  }
  lcd.clear();
  tempCelsius = tempMeasure / ACCURACY;
  tempFahrenheit = tempCelsius * 1.8 + 32;
}

void loop() {
  gotKey = keypad.getKey(); //take keypad key
  if (programLocation == 0) { //programLocation 0 is for changing the difficulty
    lcd.setCursor(0, 0);
    lcd.print("Difficulty A:OK");
    lcd.setCursor(0, 1);
    switch (difficulty) {
      case 0: lcd.print("EASY         C->"); break;
      case 1: lcd.print("MEDIUM   <-B C->"); break;
      case 2: lcd.print("HARD     <-B C->"); break;
      case 3: lcd.print("EPIC     <-B    "); break;
    }

    if (gotKey == 'B' && difficulty > 0) { //second condition to restrain rollunder in the menu
      difficulty--;
    }

    if (gotKey == 'C' && difficulty < 3) { //second condition to restrain rollover in the menu
      difficulty++;
    }

    if (gotKey == 'A') {
      programLocation = 1;
      Serial.println("Difficulty 0 - 3 (difficulty):");
      Serial.println(difficulty);
      lcd.clear();
      gotKey = keypad.getKey();
    }
  }

  if (programLocation == 1) {//Program Location for Fahrenheit/Celsius choice
    lcd.setCursor(0, 0);
    lcd.print("Temp Unit  A:OK");
    lcd.setCursor(0, 1);
    switch (isFahrenheit) {
      case 1: lcd.print("FAHRENHEIT   B->"); break;
      case 0: lcd.print("CELSIUS      B->"); break;
    }

    if (gotKey == 'B') {
      isFahrenheit = !isFahrenheit;
    }

    if (gotKey == 'A') {
      programLocation = 2;
      gotKey = keypad.getKey();
      lcd.clear();
      lcd.setCursor(0, 1);
      Serial.println("Temperature Unit (isFahrenheit):");
      Serial.println(isFahrenheit);
    }


  }
  if (programLocation == 2) {//Program Location for keypad input!
    lcd.setCursor(0, 0);
    lcd.print("Guess the temp");
    if (gotKey) {
      lcd.setCursor(index, 1);
      switch (gotKey) {
        case 'A': checkTemperature(); programLocation = 3; lcd.clear(); break; // confirm
        case 'C': index = 0; lcd.setCursor(0, 1); lcd.print("                "); break; //clear it
        case NO_KEY: break;
        default: inputTemperature[index] = gotKey; lcd.print(gotKey); index++;//add to list of chars and print
      }
    }
  }

  if (programLocation == 3) {//Program Location for Output Screen: Press reset to start over.
    lcd.setCursor(0, 0);
    if (win)  {
      Serial.println("You Win!");
      lcd.print("You Win!");
      myservo.write(0);
      digitalWrite(ONBOARDLED, HIGH);
    } else {
      Serial.println("SLAP!");
      lcd.print("SLAP!");
      myservo.write(90);
      digitalWrite(ONBOARDLED, LOW);
    }
    lcd.setCursor(0, 1);
    lcd.print("Temp Was:");
    lcd.print((isFahrenheit * tempFahrenheit) + ((!isFahrenheit) * tempCelsius));
    programLocation = 4; //Don't repeatedly execute Program Location 3
  }
}
