#include <MsTimer2.h>            // タイマー割り込みを利用する為に必要なヘッダファイル
#include <Wire.h>
#include "Adafruit_Trellis.h"
 
#define NUMTRELLIS 4        // **** SET # OF TRELLISES HERE
 
#define PIN_ENCODER_A_RIGHT A1
#define PIN_ENCODER_A_LEFT A0
#define PIN_ENCODER_B_RIGHT A3
#define PIN_ENCODER_B_LEFT A2
#define PIN_ENCODER_C_RIGHT 9
#define PIN_ENCODER_C_LEFT 8
#define PIN_ENCODER_D_RIGHT 7
#define PIN_ENCODER_D_LEFT 6

#define PIN_ENCODER_A_PUSH 13
#define PIN_ENCODER_B_PUSH 12
#define PIN_ENCODER_C_PUSH 11
#define PIN_ENCODER_D_PUSH 10

Adafruit_Trellis matrix[NUMTRELLIS] = {
  Adafruit_Trellis(), Adafruit_Trellis(),
  Adafruit_Trellis(), Adafruit_Trellis()
#if NUMTRELLIS > 4
 ,Adafruit_Trellis(), Adafruit_Trellis(),
  Adafruit_Trellis(), Adafruit_Trellis()
#endif
};
 
Adafruit_TrellisSet trellis = Adafruit_TrellisSet(
  &matrix[0], &matrix[1], &matrix[2], &matrix[3]
#if NUMTRELLIS > 4
 ,&matrix[4], &matrix[5], &matrix[6], &matrix[7]
#endif
);
 
#define numKeys (NUMTRELLIS * 16)

#define ROT_RIGHT_UNIT 0.40;
#define ROT_LEFT_UNIT 0.51;

const unsigned long OPE_TIME = 1500L;

char rot_state_1; // 状態 0~3
char rot_state_2; // 状態 0~3
char rot_state_3; // 状態 0~3
char rot_state_4; // 状態 0~3

boolean rot_button_state_1 = false;
boolean rot_button_state_2 = false;
boolean rot_button_state_3 = false;
boolean rot_button_state_4 = false;

float rot_value_1 = 0;
float rot_value_2 = 0;
float rot_value_3 = 0;
float rot_value_4 = 0;

float rot_tmp_1 = 0;
float rot_tmp_2 = 0;
float rot_tmp_3 = 0;
float rot_tmp_4 = 0;

int frameMode = 0;
int nextFrame[numKeys];
int rot_Frame_1[numKeys];
int rot_Frame_2[numKeys];
int rot_Frame_3[numKeys];
int rot_Frame_4[numKeys];

unsigned long last_ope_time; 

void setup() {
  pinMode(PIN_ENCODER_A_PUSH, INPUT);
  digitalWrite(PIN_ENCODER_A_PUSH, HIGH);
  pinMode(PIN_ENCODER_B_PUSH, INPUT);
  digitalWrite(PIN_ENCODER_B_PUSH, HIGH);
  pinMode(PIN_ENCODER_C_PUSH, INPUT);
  digitalWrite(PIN_ENCODER_C_PUSH, HIGH);
  pinMode(PIN_ENCODER_D_PUSH, INPUT);
  digitalWrite(PIN_ENCODER_D_PUSH, HIGH);

  pinMode(PIN_ENCODER_A_RIGHT, INPUT);
  digitalWrite(PIN_ENCODER_A_RIGHT, HIGH);
  pinMode(PIN_ENCODER_A_LEFT, INPUT);
  digitalWrite(PIN_ENCODER_A_LEFT, HIGH);

  pinMode(PIN_ENCODER_B_RIGHT, INPUT);
  digitalWrite(PIN_ENCODER_B_RIGHT, HIGH);
  pinMode(PIN_ENCODER_B_LEFT, INPUT);
  digitalWrite(PIN_ENCODER_B_LEFT, HIGH);

  pinMode(PIN_ENCODER_C_RIGHT, INPUT);
  digitalWrite(PIN_ENCODER_C_RIGHT, HIGH);
  pinMode(PIN_ENCODER_C_LEFT, INPUT);
  digitalWrite(PIN_ENCODER_C_LEFT, HIGH);

  pinMode(PIN_ENCODER_D_RIGHT, INPUT);
  digitalWrite(PIN_ENCODER_D_RIGHT, HIGH);
  pinMode(PIN_ENCODER_D_LEFT, INPUT);
  digitalWrite(PIN_ENCODER_D_LEFT, HIGH);
 
  // begin() with the addresses of each panel.
  // I find it easiest if the addresses are in order.
  trellis.begin(
    0x70, 0x71, 0x72, 0x73
#if NUMTRELLIS > 4
   ,0x74, 0x75, 0x76, 0x77
#endif
  );
 
  // light up all the LEDs in order
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();    
    delay(5);
  }
  // then turn them off
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();    
    delay(5);
  }
  
  rot_state_1 = rotary_getState(PIN_ENCODER_A_RIGHT, PIN_ENCODER_A_LEFT); // 現在の状態を保存しておく  
  rot_state_2 = rotary_getState(PIN_ENCODER_B_RIGHT, PIN_ENCODER_B_LEFT); // 現在の状態を保存しておく  
  rot_state_3 = rotary_getState(PIN_ENCODER_C_RIGHT, PIN_ENCODER_C_LEFT); // 現在の状態を保存しておく  
  rot_state_4 = rotary_getState(PIN_ENCODER_D_RIGHT, PIN_ENCODER_D_LEFT); // 現在の状態を保存しておく
  
  MsTimer2::set(2, rotary);     // 500ms毎にflash( )割込み関数を呼び出す様に設定
  MsTimer2::start();             // タイマー割り込み開始

  Keyboard.begin(); // キーボード制御スタート  
}
 
void loop() {
  delay(30); // 30ms delay is required, dont remove me!
  
  // If a button was just pressed or released...
  if (trellis.readSwitches()) {
    // go through every button
    for (uint8_t i=0; i<numKeys; i++) {        
      // if it was pressed, turn it on
      if (trellis.justPressed(i)) {
	Serial.print("v"); Serial.println(i);
        nextFrame[i] = 1;
      }
      // if it was released, turn it off
      if (trellis.justReleased(i)) {
	nextFrame[i] = 0;
      }
    }    
  }

  for (uint8_t i=0; i<numKeys; i++) {        
    // if it was pressed, turn it on
    if (rot_value_1 > i) {
      rot_Frame_1[i] = 1;
    } else { 
      rot_Frame_1[i] = 0;
    }
    if (rot_value_2 > i) {
      rot_Frame_2[i] = 1;
    } else { 
      rot_Frame_2[i] = 0;
    }
    if (rot_value_3 > i) {
      rot_Frame_3[i] = 1;
    } else { 
      rot_Frame_3[i] = 0;
    }
    if (rot_value_4 > i) {
      rot_Frame_4[i] = 1;
    } else { 
      rot_Frame_4[i] = 0;
    }
  }
  
  // ロータリーエンコーダーをおした時
  if (rot_button_state_1) {
//    nextFrame[44] = 1;
//    nextFrame[45] = 1;
    frameMode = 1;
    last_ope_time = millis();
  } else {
//    nextFrame[44] = 0;
//    nextFrame[45] = 0;
  }

  if (rot_button_state_2) {
//    nextFrame[46] = 1;
//    nextFrame[47] = 1;
    frameMode = 2;
    last_ope_time = millis();
  } else {
//    nextFrame[46] = 0;
//    nextFrame[47] = 0;
  }

  if (rot_button_state_3) {
//    nextFrame[60] = 1;
//    nextFrame[61] = 1;
//    frameMode = 3;
    last_ope_time = millis();
  } else {
//    nextFrame[60] = 0;
//    nextFrame[61] = 0;
  }

  if (rot_button_state_4) {
//    nextFrame[62] = 1;
//    nextFrame[63] = 1;
    frameMode = 4;
    last_ope_time = millis();
  } else {
//    nextFrame[62] = 0;
//    nextFrame[63] = 0;
  }
  
  unsigned long current_time = millis();
  if ((current_time - last_ope_time) >= OPE_TIME) {
    frameMode = 0;
    Keyboard.releaseAll(); // キーボード全開放
  }
  
  if(frameMode == 1) {
    updateMap(rot_Frame_1);
  } else if (frameMode == 2) {
    updateMap(rot_Frame_2);    
  } else if (frameMode == 3) {
    updateMap(rot_Frame_3);    
  } else if (frameMode == 4) {
    updateMap(rot_Frame_4);    
  } else {
    updateMapRow(nextFrame);
  }
  
  // tell the trellis to set the LEDs we requested
  trellis.writeDisplay();
}

void updateMapRow(int* frames) {
  // Update the map
  for (uint8_t i=0; i<numKeys; i++) {
    if(frames[i] == 1) {
      trellis.setLED(i);
    } else {
      trellis.clrLED(i);
    } 
  }
}

void updateMap(int* frames) {
  int localFrame[numKeys];

  // Update the map (convert to serial)
  for (uint8_t i=0; i<numKeys; i++) {
    if (i % 8 < 4 && i < 32) {
      if(frames[i] == 1) {
        localFrame[i - (4 * (i / 8))] = 1;
      } else {
        localFrame[i - (4 * (i / 8))] = 0;
      }
    } else if (i % 8 >= 4 && i < 32) {
      if(frames[i] == 1) {
        localFrame[16 - 4 * ((i + 8) / 8) + i] = 1;
      } else {
        localFrame[16 - 4 * ((i + 8) / 8) + i] = 0;
      }
    }  
    if (i % 8 < 4 && i >= 32) {
      if(frames[i] == 1) {
        localFrame[i - (4 * ((i - 32) / 8))] = 1;
      } else {
        localFrame[i - (4 * ((i - 32) / 8))] = 0;
      }
    } else if (i % 8 >= 4 && i >= 32) {
      if(frames[i] == 1) {
        localFrame[16 - 4 * ((i - 24) / 8) + i] = 1;
      } else {
        localFrame[16 - 4 * ((i - 24) / 8) + i] = 0;
      }
    }
  } 
  
  // Update the map
  for (uint8_t i=0; i<numKeys; i++) {
    if(localFrame[i] == 1) {
      trellis.setLED(i);
    } else {
      trellis.clrLED(i);
    } 
  }
}

void rotary() {
  char rot_past_1 = rot_state_1;
  char rot_past_2 = rot_state_2;
  char rot_past_3 = rot_state_3;
  char rot_past_4 = rot_state_4;

  rot_state_1 = rotary_getState(PIN_ENCODER_A_RIGHT, PIN_ENCODER_A_LEFT); // 今の状態  
  rot_state_2 = rotary_getState(PIN_ENCODER_B_RIGHT, PIN_ENCODER_B_LEFT); // 今の状態  
  rot_state_3 = rotary_getState(PIN_ENCODER_C_RIGHT, PIN_ENCODER_C_LEFT); // 今の状態  
  rot_state_4 = rotary_getState(PIN_ENCODER_D_RIGHT, PIN_ENCODER_D_LEFT); // 今の状態  

  char rot_dir_1 = rotary_getDir(rot_state_1, rot_past_1); // 回転方向
  char rot_dir_2 = rotary_getDir(rot_state_2, rot_past_2); // 回転方向
  char rot_dir_3 = rotary_getDir(rot_state_3, rot_past_3); // 回転方向
  char rot_dir_4 = rotary_getDir(rot_state_4, rot_past_4); // 回転方向
  
  switch(rot_dir_1){
    case 0:
      // 変化なし
      break;
    case 1:
      rot_tmp_1 += ROT_RIGHT_UNIT;
      
      if(rot_tmp_1 > 1) {
        rot_value_1 += 1;
        rot_tmp_1 = 0;
        if(rot_value_1 > 64) {
          rot_value_1 = 64;
        }
        frameMode = 1;
        last_ope_time = millis();
        Keyboard.press( KEY_LEFT_GUI );
        Keyboard.press( KEY_TAB );
        Keyboard.release( KEY_TAB );
      }
      
      break;
    case 2:
      rot_tmp_1 -= ROT_LEFT_UNIT;

      if(rot_tmp_1 < -1) {
        rot_value_1 -= 1;
        rot_tmp_1 = 0;
        if(rot_value_1 < 0) {
          rot_value_1 = 0;
        }
        frameMode = 1;
        last_ope_time = millis();
        Keyboard.press( KEY_LEFT_GUI );
        Keyboard.press( KEY_LEFT_SHIFT );
        Keyboard.press( KEY_TAB );
        Keyboard.release( KEY_TAB );
        Keyboard.release( KEY_LEFT_SHIFT );
      }
      break;
  }

  switch(rot_dir_2){
    case 0:
      // 変化なし
      break;
    case 1:
      rot_value_2 += ROT_RIGHT_UNIT;
      if(rot_value_2 > 64) {
        rot_value_2 = 64;
      }
      frameMode = 2;
      last_ope_time = millis();
      break;
    case 2:
      rot_value_2 -= ROT_LEFT_UNIT;
      if(rot_value_2 < 0) {
        rot_value_2 = 0;
      }
      frameMode = 2;
      last_ope_time = millis();
      break;
  }

  switch(rot_dir_3){
    case 0:
      // 変化なし
      break;
    case 1:
      rot_value_3 += ROT_RIGHT_UNIT;
      if(rot_value_3 > 64) {
        rot_value_3 = 64;
      }
      frameMode = 3;
      last_ope_time = millis();
      break;
    case 2:
      rot_value_3 -= ROT_LEFT_UNIT;
      if(rot_value_3 < 0) {
        rot_value_3 = 0;
      }
      frameMode = 3;
      last_ope_time = millis();
      break;
  }

  switch(rot_dir_4){
    case 0:
      // 変化なし
      break;
    case 1:
      rot_value_4 += ROT_RIGHT_UNIT;
      if(rot_value_4 > 64) {
        rot_value_4 = 64;
      }
      frameMode = 4;
      last_ope_time = millis();
      break;
    case 2:
      rot_value_4 -= ROT_LEFT_UNIT;
      if(rot_value_4 < 0) {
        rot_value_4 = 0;
      }
      last_ope_time = millis();
      frameMode = 4;
      break;
  }
  
  if(!digitalRead(PIN_ENCODER_A_PUSH)) {
    if(!rot_button_state_1) {
      Serial.println("1P"); // push
      rot_button_state_1 = true;
    }
  } else {
    rot_button_state_1 = false;
  }
  
  if(!digitalRead(PIN_ENCODER_B_PUSH)) {
    if(!rot_button_state_2) {
      Serial.println("2P"); // push
      rot_button_state_2 = true;
    }
  } else {
    rot_button_state_2 = false;
  }
  
  if(!digitalRead(PIN_ENCODER_C_PUSH)) {
    if(!rot_button_state_3) {
      Serial.println("3P"); // push
      rot_button_state_3 = true;
    }
  } else {
    rot_button_state_3 = false;
  }
  
  if(!digitalRead(PIN_ENCODER_D_PUSH)) {
    if(!rot_button_state_4) {
      Serial.println("4P"); // push
      rot_button_state_4 = true;
    }
  } else {
    rot_button_state_4 = false;
  }
}

/* ロータリーエンコーダの状態番号0~3を取得する */
char rotary_getState(char pinA, char pinB){
  // 2つのピンからの入力状態をチェック
  if(digitalRead(pinA)){
    if(digitalRead(pinB)) return 2;
    else return 0;
  }
  else{
    if(digitalRead(pinB)) return 1;
    else return 3;
  }
}

/* 1つ前の状態と比較して、回転方向を取得する */
char rotary_getDir(int state, int past_state){
  if((state+3+1)%3 == past_state) return 1; // 左回り
  else if((state+3-1)%3 == past_state) return 2; // 右回り
  return 0; // 変化無し
}
