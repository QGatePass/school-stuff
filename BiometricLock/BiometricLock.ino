/*
  Author:  Nwora Chidubem
  Date Initialized:  20/9/2024
  Date Completed:  TBD
  ***************************************************
  This sketch is for Final Year Project[Biometric Lock System]
      - Show the Date and Time
      - Unlock the door using Fingerprint scanner
      - Unlock the door using PIN code
      - Enroll new fingerprints
      - Delete fingerprints[All fingerprints will be deleted]
      - Change PIN code
      OUTPUT DEVICES
          - 20 x 4 LCD with I2C backpack
          - Solenoid Lock
          - Light Emitting Diodes
          - Buzzer
      INPUT DEVICES
          - 4 x 4 Matrix Keypad[Membrane]
          - AS608 fingerprint scanner
          - DS1307 RTC
      POWER SUPPLY
          - 3x 18650 batteries(12V)
*/

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <EEPROM.h>
#include <RtcDS1302.h>

ThreeWire myWire(12, 13, A2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);


SoftwareSerial myserial(2, 3);
LiquidCrystal_I2C lcd(0x27, 20, 4);


char code[5] = "8357";
char entry[] = {};


int green = A4, red = 1, buzzer = A0, lock = A1;

const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = {11,10,9,8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7,6,5,4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad and Fingerprint sensor
Keypad keys = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&myserial);
int templateCount;

void setup() {
  Serial.begin(9600);
  Rtc.Begin();
  pinMode(green, 1);
  pinMode(red, 1);
  pinMode(buzzer, 1);
  pinMode(lock, 1);
  lcd.init();
  lcd.noBacklight();
  lcd.clear();
  finger.begin(57600);
  if (finger.verifyPassword()) {
    //pass if fingerprint scanner found
  }
  // Print a warning if sensor not found
  else {
    lcd.print("Did not find");
    lcd.setCursor(0, 1);
    lcd.print("fingerprint sensor");
    delay(1500);
  }
}

void loop() {
  char select = main_menu();
  print_time();
  delay(3000);
  switch (select) {
    case '1':
      finger_unlock();
      break;
    case '2':
      pin_unlock();
      break;
    case '3':
      options();
      break;
    default:
      lcd.clear();
      print_time();
      lcd.setCursor(0, 1);
      lcd.print("Unrecognized Choice");
      lcd.setCursor(0,2);
      lcd.print(select);
      pin_unlock();
      delay(700);
      break;
  }
}

char main_menu(void) {
  /* The main interface of the device
     @returns the choice made by the user by pressing the keypad
     No enter key required  
  */
  lcd.clear();
  print_time();
  lcd.setCursor(0, 0);
  lcd.print("1. Biometric Unlock");
  lcd.setCursor(0, 1);
  lcd.print("2. Unlock with PIN");
  lcd.setCursor(0, 2);
  lcd.print("3. Options");

  char choice = 0; // Initialize choice

  while (choice == 0) { // Loop until a valid key is pressed
    choice = keys.getKey(); // Get key press
  }

  return choice; // Return the valid choice
}



void print_time() {   //This function MUST be called every time "lcd.clear" is called!
  RtcDateTime tm = Rtc.GetDateTime();
  lcd.setCursor(0, 3);
  lcd.print(tm.Day());
  lcd.print('/');
  lcd.print(tm.Month());
  lcd.print('/');
  lcd.print(tm.Year());
  lcd.setCursor(15, 3);
  lcd.print(tm.Hour());
  lcd.print(':');
  lcd.print(tm.Minute());
  lcd.setCursor(0, 0);
  return;
}


void finger_unlock(){   //Check for valid registered print and unlock the door
  lcd.clear();
  print_time();
  lcd.setCursor(0,0);
  lcd.print("Scanning Fingerprint");
 int result = finger.getImage();
  
  // If a finger is detected
  while (result != FINGERPRINT_OK){
    result = finger.getImage();
    lcd.setCursor(0,2);
    lcd.print("No Finger!");

  }
  if (result == FINGERPRINT_OK) {
    Serial.println("Finger detected!");
    
    // Convert the image to a template
    result = finger.image2Tz();
    if (result != FINGERPRINT_OK) {
      Serial.print("Error converting image to template: ");
      Serial.println(result);
      return;
    }

    // Search for the finger in the database
    result = finger.fingerSearch();
    if (result == FINGERPRINT_OK) {
      Serial.print("Finger matched! ID: ");
      Serial.println(finger.fingerID);
       lcd.setCursor(0, 2);
    lcd.print("Fingerprint Matched!");
    digitalWrite(green, 1);
    digitalWrite(lock, 1);
    digitalWrite(buzzer, 1);
    delay(500);
    digitalWrite(buzzer, 0);
    delay(1000);
    digitalWrite(green, 0);
    digitalWrite(lock, 0);
    } else if (result == FINGERPRINT_NOTFOUND) {
      Serial.println("Finger not recognized.");
      lcd.setCursor(0,2);
    lcd.print("Unmatched Finger!");
    digitalWrite(red, 1);
    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);
    delay(100);
    digitalWrite(buzzer, 1);
    delay(300);
    digitalWrite(buzzer, 0);
    digitalWrite(red, 0);
    } else {
      Serial.print("Error searching for finger: ");
      Serial.println(result);
    }
  } else {
    Serial.println("No finger detected. Please try again.");
      lcd.setCursor(0,2);
    lcd.print("No Finger!");
  }
  
  delay(1000); // Wait before the next scan[]
  return;
}


void pin_unlock() {   // Enter PIN to unlock
  int i = 0;
  lcd.clear();
  print_time();
  lcd.setCursor(0, 0);
  lcd.print("Enter PIN:");
  lcd.setCursor(0, 1);
  char entry[5] = ""; 
  while (i < 4) {
    char thisKey = keys.getKey();
    while (!thisKey) {
      thisKey = keys.getKey();
    }
    entry[i] = thisKey;
    entry[i + 1] = '\0'; // Null-terminate the string for display
    lcd.setCursor(0, 1);
    lcd.print(entry);
    Serial.println(entry);
    i++; // Increment after processing the key
  }
  
  if (strcmp(entry, code) == 0) { // Compare strings correctly
    lcd.setCursor(0, 2);
    lcd.print("PIN matched!");
    digitalWrite(green, HIGH);
    digitalWrite(lock, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(1000);
    digitalWrite(green, LOW);
    digitalWrite(lock, LOW);
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Wrong PIN!");
    digitalWrite(red, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(100);
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    digitalWrite(red, LOW);
  }
}

void options(){
  lcd.clear();
  print_time();
  lcd.setCursor(0,0);
  lcd.print("1. Enroll New Print");
  lcd.setCursor(0, 1);
  lcd.print("2. Delete Fingerprint");
  lcd.setCursor(0, 2);
  lcd.print("3. Change Pin");
  char choice = 0; // Initialize choice

  while (choice == 0) { // Loop until a valid key is pressed
    choice = keys.getKey(); // Get key press
  }
  if (choice == 'D'){
    return main_menu();
  }
  switch (choice) {
    case '1' :
    enroll_finger();
    break;
    // case '2':
    // delete_finger();
    // break;
    // case '3':
    // pin_change();
    // break;
    default:
    lcd.clear();
      print_time();
      lcd.setCursor(0, 0);
      lcd.print("Unrecognized Choice");
      delay(700);
      break;
  }
return;
}


void enroll_finger() {
  int p = -1;

  // Get the current number of stored templates
  finger.getTemplateCount();
  if (finger.templateCount < 0) {
    lcd.clear();
    lcd.print("Error reading");
    delay(2000);
    return;
  }
  
  // Check if there is space for more fingerprints
  if (finger.templateCount >= 127) { // Assuming max is 127
    lcd.clear();
    lcd.print("Storage full!");
    delay(2000);
    return;
  }

  // Determine the next available ID
  int id = finger.templateCount + 1; // Next available ID

  lcd.clear();
  lcd.print("Place your finger...");
  
  // Wait for the finger to be detected
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      lcd.clear();
      lcd.print("Image taken!");
    } else if (p == FINGERPRINT_NOFINGER) {
      lcd.setCursor(0, 1);
      lcd.print("No finger detected.");
    }
  }

  // Convert the image to a template
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Error: Img to Tz");
    delay(2000);
    return;
  }
  lcd.clear();
  lcd.print("Img to Tz done!");

  // Ask for the second finger scan
  lcd.clear();
  lcd.print("Same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      lcd.clear();
      lcd.print("Image taken!");
    } else if (p == FINGERPRINT_NOFINGER) {
      lcd.setCursor(0, 1);
      lcd.print("No finger detected.");
    }
  }

  // Convert the second image to a template
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Error: Img to Tz");
    delay(2000);
    return;
  }
  lcd.clear();
  lcd.print("Second Img done!");

  // Create a new fingerprint by merging the two templates
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Error: Create Model");
    delay(2000);
    return;
  }
  lcd.clear();
  lcd.print("Model created!");

  // Store the model in the next available ID
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Error: Store ID");
    delay(2000);
    return;
  }
  lcd.clear();
  lcd.print("Stored ID: ");
  lcd.print(id);
  delay(2000);
  return;
}
