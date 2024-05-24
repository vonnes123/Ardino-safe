#include "Display.h"

//declaring all the parts that will be needed for the safe
const int inputButton = 9;
const int confirmButton = 8;
const int LED_GREEN = 5;
const int LED_RED = 4;
const int LED_YELLOW = 7;
const int LDR_IN = 16;
const int LIGHT_THRESHOLD = 400;
const int BUZZER = 3;

//declaring all the variables needed for displaying the numbers
int displayNumber = 0;
int displayPos = 0;

//declaring all the variables needed to use the buttons
int lastButtonState;
int buttonState;
int phase = 1;
int lastButtonState2;
int buttonState2;
bool firstClick = true;

//declaring all the variables needed to check the password
int password = 2134;
int checkPassword;

int wrongPasCount = 0;
int alarmCounter;

bool properEntry;
bool entryBuzz = true;
bool entryBlink = true;

void setup() {
  //configuring the specific pin behaviour
  Serial.begin(9600);
  pinMode(inputButton, INPUT_PULLUP);
  pinMode(confirmButton, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LDR_IN, INPUT);
  pinMode(BUZZER, OUTPUT);

  Display.clear();
}
//The method we use to pick the numbers
int pickNumber () {
  //check if you need to display '-'
  if (displayNumber == 0 && firstClick == true) {
    Display.showCharAt(displayPos, '-');
  }
  else {
    if (displayNumber == 0) {
      displayNumber = 1;
    }
    Display.showCharAt(displayPos, '0' + displayNumber);
  }
  //checking if the input button is pressed
  buttonState = digitalRead(inputButton);
  if (buttonState == LOW && lastButtonState == HIGH) {
    tone(BUZZER, 440, 50);
    firstClick = false;
    displayNumber = (displayNumber + 1) % 6;
  }
  lastButtonState = digitalRead(inputButton);
  //checking if your trying to enter 0
  if (displayNumber == 0) {
    //I know this was not part of the assigment
    //however, it seem to me a good idea to add a indicator that you cannot enter a 0 as a password number
    //so I made the red LED blink and a short beep sound when you try to enter a 0
    buttonState2 = digitalRead(confirmButton);
    if (buttonState2 == LOW && lastButtonState2 == HIGH) {
      tone(BUZZER, 440, 100);
      digitalWrite(LED_RED, HIGH);
      delay(100);
      digitalWrite(LED_RED, LOW);
    }
    lastButtonState2 = digitalRead(confirmButton);
  } else {
    //checking if the confirm button is pressed
    buttonState2 = digitalRead(confirmButton);
    if (buttonState2 == LOW && lastButtonState2 == HIGH) {
      //collecting the entered password for later comparison
      if (phase == 1) {
        checkPassword = displayNumber * 1000 + checkPassword;
      }
      if (phase == 2) {
        checkPassword = displayNumber * 100 + checkPassword;
      }
      if (phase == 3) {
        checkPassword = displayNumber * 10 + checkPassword;
      }
      if (phase == 4) {
        checkPassword = displayNumber * 1 + checkPassword;
      }
      //moving to the next number
      tone(BUZZER, 440, 50);
      firstClick = true;
      phase++;
      displayPos++;
      displayNumber = 0;
    }
    lastButtonState2 = digitalRead(confirmButton);
  }
}
//method used hard reset everything
void clearAll() {
  Display.clear();
  phase = 1;
  displayNumber = 0;
  displayPos = 0;
  checkPassword = 0;
}
//method used to activate the alarm
void alarm() {
  tone(BUZZER, 440, 1000);
  while (alarmCounter <= 5)
  { digitalWrite(LED_RED, HIGH);
    delay(500);
    digitalWrite(LED_RED, LOW);
    delay(500);
    alarmCounter++;
  }
  alarmCounter = 0;
}
//method used when wrong password was entered 3 times
void WrongPas() {
  Serial.println("ALARM SAFE 1453");
  alarm();
  wrongPasCount = 0;
}
//method used to check if the door is opened or closed
bool openOrClosed() {
  int brightness = analogRead(LDR_IN);
  if (brightness > LIGHT_THRESHOLD) {
    //the safe door is open
    return false;
  } else {
    //the safe door is closed
    return true;
  }
}
void loop() {
  //declare a variable to be used to check if the door is opened or closed
  bool state = openOrClosed();
  //check if it is broken into the safe
  if (state == false && properEntry == false) {
    clearAll();
    alarm();
  }
  //check if a wrong password was entered 3 times
  if (wrongPasCount == 3) {
    WrongPas();
  }
  //pick the numbers on the screen
  if (phase == 1) {
    pickNumber();
    delay(50);
  } else if (phase == 2) {
    pickNumber();
    delay(50);
  }
  else if (phase == 3) {
    pickNumber();
    delay(50);
  }
  else if (phase == 4) {
    pickNumber();
    delay(50);
  } else if (phase == 5) {
    //check if the password is correct
    if (checkPassword == password) {
      //check if you entered the proper way
      if (entryBuzz == true) {
        //the safe is unlocked
        digitalWrite(LED_GREEN, HIGH);
        Display.clear();
        properEntry = true;
        if (entryBlink == true) {
          tone(BUZZER, 200, 300);
          digitalWrite(LED_YELLOW, HIGH);
          delay(300);
          digitalWrite(LED_YELLOW, LOW);
          entryBlink = false;
        }
        if (state == false) {
          //you enter the safe
          entryBuzz = false;
          digitalWrite(LED_GREEN, LOW);
        }
      } else if (state == true && properEntry == true) {
        //exit the safe if you have opened it the proper way
        delay(500); //we check for half a second if there was a cloud or if the safe is just being clossed. Where if its beeing closed there will be no light for more than half a second
        if (state == true && properEntry == true) {
          properEntry = false;
          clearAll();
          if (entryBuzz == false) {
            tone(BUZZER, 200, 300);
            entryBuzz = true;
            digitalWrite(LED_YELLOW, HIGH);
            delay(300);
            digitalWrite(LED_YELLOW, LOW);
            entryBlink = true;
          }
        }
      }  else if (state == false && properEntry == false) {
        //you have broken into the safe
        alarm();
        clearAll();
      }
    } else if (checkPassword != password) {
      //the password is not correct so reset everything
      if (state == true) {
        wrongPasCount++;
        clearAll();
      }
    }
  }
}
