//BUTTON BOX 
//USE w ProMicro
//Tested in WIN10 + Assetto Corsa
//AMSTUDIO
//20.8.17

#include <Keypad.h>
#include <Joystick.h>
#include <Mouse.h>
#include <Keyboard.h>

#define ENABLE_PULLUPS
#define NUMROTARIES 1
#define NUMBUTTONS 10
#define NUMROWS 3
#define NUMCOLS 4


byte buttons[NUMROWS][NUMCOLS] = {
//  {0,1,2,6}, //first row 1,2,3
//  {3,4,5,6}, //second row 4,5,6
//  {6,6,6,6} //rotary button

//   {0x7C,0x7D,0x7E,0x82}, //first row |,},~
//  {0x7F,0x80,0x81,0x82}, //second row blank,blank,blank
//  {0x82,0x82,0x82,0x82} //rotary button blank
//What is the code for F13-F18 then????
//https://www.arduino.cc/en/Reference/KeyboardModifiers
  {'q','w','e','r'}, //first row q,w,e
  {'a','s','d','r'}, //second row a,s,d
  {'r','r','r','r'} //rotary button r
};

struct rotariesdef {
  byte pin1;
  byte pin2;
  int ccwchar;
  int cwchar;
  volatile unsigned char state;
};

rotariesdef rotaries[NUMROTARIES] {
  //{15,10,8,9,0}, //pin1,pin2,output1,output2,ground
  {15,10,'n','m',0}, //pin1,pin2,output1,output2,ground
};

#define DIR_CCW 0x10
#define DIR_CW 0x20
#define R_START 0x0

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

byte rowPins[NUMROWS] = {7,9,19}; //first row, second row, rotary button
byte colPins[NUMCOLS] = {4,6,8}; //first column, second, third

Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS); //assign the buttons to a Keypad map

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 32, 0,
  false, false, false, false, false, false,
  false, false, false, false, false);

void setup() {
  Keyboard.begin();
  Joystick.begin();
  rotary_init();
  }

void loop() { 

  CheckAllEncoders();

  CheckAllButtons();

}

void CheckAllButtons(void) {
      if (buttbx.getKeys())
    {
       for (int i=0; i<LIST_MAX; i++)   
        {
           if ( buttbx.key[i].stateChanged )   
            {
            switch (buttbx.key[i].kstate) {  
                    case PRESSED:
                    case HOLD:
                              //Joystick.setButton(buttbx.key[i].kchar, 1); //assigns the button from they Keypad to the Joystick
                              //I dont know how to assign F13 keys
                              //use key combinations instead
                              //https://www.arduino.cc/en/Reference/KeyboardModifiers
                              Keyboard.press(KEY_RIGHT_ALT);
                              Keyboard.press(buttbx.key[i].kchar);
                              break;
                    case RELEASED:
                    case IDLE:
                              //Joystick.setButton(buttbx.key[i].kchar, 0); //deactivates the button
                              Keyboard.release(KEY_RIGHT_ALT);
                              Keyboard.release(buttbx.key[i].kchar);
                              break;
            }
           }   
         }
     }
}


void rotary_init() {
  for (int i=0;i<NUMROTARIES;i++) {
    pinMode(rotaries[i].pin1, INPUT);
    pinMode(rotaries[i].pin2, INPUT);
    #ifdef ENABLE_PULLUPS
      digitalWrite(rotaries[i].pin1, HIGH);
      digitalWrite(rotaries[i].pin2, HIGH);
    #endif
  }
}


unsigned char rotary_process(int _i) {
   unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
  rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
  return (rotaries[_i].state & 0x30);
}

void CheckAllEncoders(void) {
  for (int i=0;i<NUMROTARIES;i++) {
    unsigned char result = rotary_process(i);
    if (result == DIR_CCW) {
      Keyboard.press(KEY_RIGHT_ALT);
      Keyboard.press(rotaries[i].ccwchar);
      Keyboard.release(KEY_RIGHT_ALT);
      Keyboard.release(rotaries[i].ccwchar);
      //Joystick.setButton(rotaries[i].ccwchar, 1); delay(50); Joystick.setButton(rotaries[i].ccwchar, 0); //presses button with each tick
    };
    if (result == DIR_CW) {
      Keyboard.press(KEY_RIGHT_ALT);
      Keyboard.press(rotaries[i].cwchar);
      Keyboard.release(KEY_RIGHT_ALT);
      Keyboard.release(rotaries[i].cwchar);
      //Joystick.setButton(rotaries[i].cwchar, 1); delay(50); Joystick.setButton(rotaries[i].cwchar, 0); //presses button with each tick
    };
  }
}
