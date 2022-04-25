/*
  PolyMetronome v1.0

  Created by Peter Kyle (PK Cubed)

  A dual bpm metronome built for polyrhythmic capability.

  This metronome works by using the millis() function for the most accurate click timing.

  To achieve this,

  minutes = seconds/60

  BPM = 1 minute / beat duration
  Beat duration (ms) = 60000ms / BPM

  old_ms = 10000000;

  if (millis() % duration < old_seconds) {
  old_ms = millis() % duration;
  tone(speaker_pin, 440, 100); // Tone on A4 for 100ms
  }
*/

// Encoder
#include <Encoder.h>

// Character LCD
#include <LiquidCrystal.h>



// Rotary Encoder Initialization
Encoder myEnc(2, 3);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 9, en = A0, d4 = A5, d5 = 10, d6 = A1, d7 = 4; // Character LCD Pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Init character LCD



int encoder_btn_pin = A2; // Encoder button pin, normally up, closed down
int encoder_btn_old = HIGH; // Old state of encoder button to detect changes

const int speaker_pin = 5; // Speaker pin to transistor

const int reset_btn_pin = 8; // Reset beats button pin

const int dec_btn_pin = A3; // Decimal Button Pin

const int led0_pin = 13; // LED 0 Red Left
const int led1_pin = 12; // LED 1 Green Middle
const int led2_pin = 11; // LED 2 LCD Backlight

float bpm_init = 120; // Beats per minute init
float bpm0 = bpm_init; // Beats per minute 0
float bpm1 = bpm_init; // Beats per minute 1

float mspb0 = 60000 / bpm0; // Miliseconds per beat 0
float mspb1 = 60000 / bpm1; // Miliseconds per beat 1

float last_beat0 = 0; // Last beat number
float last_beat1 = 0; // Last beat number

float last_led0 = 0; // Last LED beat
float last_led1 = 0; // Last LED beat

float bank_start = bpm_init; // BPM bank save
int encoder_bank = 0; // BPM Bank

int ts_init = 4; // Beats per bar init
int ts0 = ts_init; // Beats per bar 0
int ts1 = ts_init; // Beats per bar 1

int mspm; // OLD - NOT USED
int ms; // OLD - NOT USED

int beat0_played; // If beat 0 is being played.

int last_enc = 0; // Last encoder position (to detect changes)

int dec_btn_old = HIGH; // Decimal Button old state (to detect changes)

long led_on_ms = -5000; // ms since last activity

int backlight_on_time = 5000; // Number of ms to leave backlight on after activity

void setup() {
  Serial.begin(9600); // Initialize serial comunication
  Serial.println("Polymetronome v1.0 Starting...");

  pinMode(encoder_btn_pin, INPUT_PULLUP); // Encoder btn init

  pinMode(reset_btn_pin, INPUT_PULLUP); // Reset btn init

  pinMode(dec_btn_pin, INPUT_PULLUP); // Reset btn init

  pinMode(speaker_pin, OUTPUT); // Speaker pin init
  pinMode(led0_pin, OUTPUT); // LED 0 init
  pinMode(led1_pin, OUTPUT); // LED 1 init
  pinMode(led2_pin, OUTPUT); // LED 2 init

  digitalWrite(led0_pin, HIGH); // Set led to high
  digitalWrite(led1_pin, HIGH); // Set led to high
  digitalWrite(led2_pin, HIGH); // Set led to high

  lcd.begin(16, 2); // Character LCD init (16 char width, 2 char height)

  // Print a message to the LCD.
  lcd.print("Polymetronome");
  lcd.setCursor(0, 1); // Set cursor to second line
  lcd.print("v1.0");

  delay(1000); // Wait one second

  lcd.clear(); // Clear the lcd screen

  //Serial.println("Default BPM: " + bpm_init);

  digitalWrite(led0_pin, LOW); // Set leds to low
  digitalWrite(led1_pin, LOW); // Set leds to low
  led_on_ms = millis();
}

void loop() {

  if (led_on_ms + backlight_on_time < millis()) { // If backlight should be turned off
    digitalWrite(led2_pin, LOW);
  } else {
    digitalWrite(led2_pin, HIGH);
  }

  /*

    // Find number of ms per longest measure
    if (mspb0 >= mspb1) {
    mspm = mspb0*ts0;
    } else {
    mspm = mspb1*ts1;
    }

    // Serial.println(mspm);

    ms = millis() % mspm;

    Serial.println(ms);

  */


  if (!digitalRead(reset_btn_pin)) { // If the beat reset button is pressed
    Serial.println("Reset button pressed...");
    last_beat0 = 0;
    last_beat1 = 0;
    last_led0 = 0;
    last_led1 = 0;
    led_on_ms = millis(); // Turn on lcd backlight
  }

  if (digitalRead(encoder_btn_pin) != encoder_btn_old) { // If encoder button has just changed state

    led_on_ms = millis(); // Turn on lcd backlight

    if (digitalRead(encoder_btn_pin)) { // If encoder button is high, or released (inverted falling edge)

      Serial.println("Encoder button pressed falling edge.");

      encoder_btn_old = HIGH; // Set old button state variable to low
      encoder_bank += 1; // Invert the encoder bank
      if (encoder_bank >= 4) {
        encoder_bank = 0;
      }

      Serial.println(encoder_bank);

      if (encoder_bank == 0) { // If encoder bank is now 0

        bank_start = bpm0; // Set encoder start pos to bpm0

      } else if (encoder_bank == 1) { // If encoder bank is now 1

        bank_start = bpm1; // Set encoder start pos to bpm1

      } else if (encoder_bank == 2) { // If encoder bank is now 2

        bank_start = ts0; // Set encoder start pos to ts0

      } else if (encoder_bank == 3) { // If encoder bank is now 3

        bank_start = ts1; // Set encoder start pos to ts1

      }

      Serial.print("Changing to bank ");
      Serial.print(encoder_bank);

      myEnc.write(0); // Clear the encoder

    } else { // If the encoder button is low or pressed

      encoder_btn_old = LOW; // ! Encoder button old = LOW

      Serial.print("Encoder button pressed.");

    }
  }



  if (!digitalRead(dec_btn_pin)) { // If encoder button is pressed, change bpm by only 0.1

    led_on_ms = millis(); // Turn on lcd backlight

    if (dec_btn_old) {

      dec_btn_old = LOW;

      if (encoder_bank == 0) { // If encoder bank is now 0

        bank_start = bpm0; // Set encoder start pos to bpm0

      } else if (encoder_bank == 1) { // If encoder bank is now 1

        bank_start = bpm1; // Set encoder start pos to bpm1

      } else if (encoder_bank == 2) { // If encoder bank is now 2

        bank_start = ts0; // Set encoder start pos to ts0

      } else if (encoder_bank == 3) { // If encoder bank is now 3

        bank_start = ts1; // Set encoder start pos to ts1

      }

      myEnc.write(0); // Clear the encoder

    }

    if (encoder_bank == 0) { // If encoder bank is 0

      bpm0 = float(myEnc.read()) / 40 + bank_start; // set bpm0 from encoder

      Serial.println(bpm0);

    } else if (encoder_bank == 1) { // If encoder bank is 1

      bpm1 = float(myEnc.read()) / 40 + bank_start; // set bpm1 from encoder

      Serial.println(bpm1);

    } else if (encoder_bank == 2) { // If encoder bank is 2

      ts0 = myEnc.read() / 4 + bank_start; // set ts0 from encoder

    } else { // If encoder bank is 3

      ts1 = myEnc.read() / 4 + bank_start; // set ts1 from encoder

    }

  } else {

    if (!dec_btn_old) {

      dec_btn_old = HIGH;

      if (encoder_bank == 0) { // If encoder bank is now 0

        bank_start = bpm0; // Set encoder start pos to bpm0

      } else if (encoder_bank == 1) { // If encoder bank is now 1

        bank_start = bpm1; // Set encoder start pos to bpm1

      } else if (encoder_bank == 2) { // If encoder bank is now 2

        bank_start = ts0; // Set encoder start pos to ts0

      } else if (encoder_bank == 3) { // If encoder bank is now 3

        bank_start = ts1; // Set encoder start pos to ts1

      }

      myEnc.write(0); // Clear the encoder

    }

    if (encoder_bank == 0) { // If encoder bank is 0

      bpm0 = float(myEnc.read()) / 4 + bank_start; // set bpm0 from encoder

    } else if (encoder_bank == 1) { // If encoder bank is 1

      bpm1 = float(myEnc.read()) / 4 + bank_start; // set bpm1 from encoder

    } else if (encoder_bank == 2) { // If encoder bank is 2

      ts0 = myEnc.read() / 4 + bank_start; // set ts0 from encoder

    } else { // If encoder bank is 3

      ts1 = myEnc.read() / 4 + bank_start; // set ts1 from encoder

    }

  }



  if (myEnc.read() / 4 != last_enc) { // If encoder has moved

    // Reset all beats
    last_beat0 = 0;
    last_beat1 = 0;
    last_led0 = 0;
    last_led1 = 0;

    last_enc = myEnc.read() / 4; // Set last_enc to Current encoder value

    led_on_ms = millis(); // Turn on lcd backlight

  }




  if (ts0 < 0) { // If ts0 is less than 0
    ts0 = 0; // Limit ts0 to 0
  }
  if (ts1 < 0) { // If ts1 is less than 0
    ts1 = 0; // Limit ts1 to 0
  }

  lcd.setCursor(0, 0); // Cursor to beginning
  lcd.print("A: ");

  if (encoder_bank == 0) { // If encoder bank = 0
    if (millis() % 1000 > 500) { // Divide second in half
      lcd.print("      ");
    } else {
      lcd.print(bpm0);
    }
  } else {
    lcd.print(bpm0);
  }

  lcd.print(" ");

  lcd.setCursor(0, 1); // Cursor to beginning second line
  lcd.print("B: ");

  if (encoder_bank == 1) { // If encoder bank = 1
    if (millis() % 1000 > 500) { // Divide second in half
      lcd.print("      ");
    } else {
      lcd.print(bpm1);
    }
  } else {
    lcd.print(bpm1);
  }

  lcd.print(" ");

  lcd.setCursor(10, 0);
  if (encoder_bank == 2) { // If encoder bank = 1
    if (millis() % 1000 > 500) { // Divide second in half
      lcd.print("      ");
    } else {
      lcd.print(ts0); // Print time signature 0
    }
  } else {
    lcd.print(ts0); // Print time signature 0
  }

  lcd.setCursor(10, 1);
  if (encoder_bank == 3) { // If encoder bank = 1
    if (millis() % 1000 > 500) { // Divide second in half
      lcd.print("      ");
    } else {
      lcd.print(ts1); // Print time signature 0
    }
  } else {
    lcd.print(ts1); // Print time signature 0
  }



  mspb0 = 60000 / bpm0; // Redifine mspb0
  mspb1 = 60000 / bpm1; // Redifine mspb1

  //Serial.println(millis()/mspb0);
  //Serial.println(last_beat0);

  if (floor(millis() / mspb0) > last_beat0) { // If a beat0 needs to play

    Serial.println("beat 0");

    beat0_played = true; // A beat0 is being played

    last_beat0 = millis() / mspb0; // Set last beat to the last beat
    digitalWrite(led0_pin, HIGH); // Turn on led0

    if (floor(fmod(last_beat0, ts0)) == 0) { // If this is the first beat in the measure

      tone(speaker_pin, 880, 100); // Speaker pin tone at 880 hz (A5) for 100 ms (0.1 s)

    } else { // If this is not the first beat in the measure

      tone(speaker_pin, 440, 100); // Speaker pin tone at 440 hz (A4) for 100 ms (0.1 s)

    }

  }

  if (floor(millis() / mspb1) > last_beat1) { // If a beat1 needs to play

    Serial.println("beat 1");

    last_beat1 = millis() / mspb1; // Set last beat to the last beat
    digitalWrite(led1_pin, HIGH); // Turn on led1

    if (not(beat0_played)) { // If beat 0 is not being played

      if (floor(fmod(last_beat1, ts1)) == 0) { // If this is the first beat in the measure

        tone(speaker_pin, 1318.51, 100); // Speaker pin tone at 440 hz for 100 ms (0.1 s)

      } else { // If this is not the first beat in the measure

        tone(speaker_pin, 659.25, 100); // Speaker pin tone at 440 hz for 100 ms (0.1 s)

      }

    } else { // If beat 0 is being played.
      Serial.println("Beat 0 was played");
    }
  }



  // Control LEDs

  if (floor((millis() - 50) / mspb0) > last_led0) { // If it's been 100 ms since beat0
    Serial.println("led0 off");
    last_led0 = (millis()) / mspb0; // Set last beat to the last beat
    digitalWrite(led0_pin, LOW); // Turn led0 off
  }
  if (floor((millis() - 50) / mspb1) > last_led1) { // If it's been 100 s since beat1
    Serial.println("led1 off");
    last_led1 = (millis()) / mspb1; // Set last beat to the last beat
    digitalWrite(led1_pin, LOW); // Turn led1 off
  }

  beat0_played = false; // Beat0 was played already or was not played at all
  delay(10);
}
