#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//Ekran uzun kenar x yani satırlar kısa kenar y yani sütunlardır ve roket en üst satırda kalır yani ekranı tamamiyle tersten düşün
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define RESET 4 
#define LATCH 2 //STCP
#define DATA A2 //DS
#define CLK 3 //SHCP
#define MR A0 //MR
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 16
Servo myservo;
#define BLOCK_SIZE 8
int potansiyometre = A3;
int pot_deger;
int ldr_deger = A1;
uint16_t i_1 = SSD1306_WHITE;
uint16_t i_2 = SSD1306_BLACK;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RESET);
byte cc_rakamlar[10] = {
        63, //0  (0b00111111)
        6, //1   (0b00000110)
        91, //2  (0b01011011)
        79, //3  (0b01001111)
        102, //4 (0b01100110)
        109, //5 (0b01101101)
        125, //6 (0b01111101)
        7, //7   (0b00000111)
        127, //8 (0b01111111)
        111 //9  (0b01101111)
      }; 
const int BUZZ = 0;
const int LED_CAN_1 = 5;
const int LED_CAN_2 = 6;
const int LED_CAN_3 = 7;
const int LED_SILAH_1 = 8;
const int LED_SILAH_2 = 9;
const int LED_SILAH_3 = 10;
const int FIRE_PIN = 11;
int rocket_pos = MATRIX_WIDTH / 2 - 1; // roketin başlangıç pozisyonu
int score = 0;
int score_1 = 0;
int lives = 3; // oyuncunun can sayısı
int ammo = 3;// silah hakkı
bool game_over = false; // oyunun bitip bitmediğini tutacak değişken
bool in_menu = true; // menüde miyiz?
int menu_selection = 1; // Başlangıçta 1. seçeneği seçili göster
const int MENU_OPTIONS = 2; // Menüdeki seçenek sayısı
int hiz = 1000;
const int beklenen_sure = 10000;
unsigned long zaman = 0;
void setup() {
  myservo.attach(1);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(MR, OUTPUT);//BUNLARI YAZDIRMADA KULLANICAZ
  digitalWrite(MR, LOW);
  digitalWrite(MR, HIGH);
  pinMode(BUZZ, OUTPUT);
  pinMode(FIRE_PIN, INPUT_PULLUP);
  pinMode(LED_CAN_1, OUTPUT);
  pinMode(LED_CAN_2, OUTPUT);
  pinMode(LED_CAN_3, OUTPUT);
  pinMode(LED_SILAH_1,OUTPUT);
  pinMode(LED_SILAH_2,OUTPUT);
  pinMode(LED_SILAH_3,OUTPUT);
  Wire.begin();
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.setRotation(1);
  display.display();
  delay(10);
  display.clearDisplay();
  display.setTextColor(i_1);
  display.setTextSize(1);
}
void yaz(int deger) {
  digitalWrite(LATCH, LOW);
  byte yuzler = deger / 100;  // deger = 354 ise 54 / 10 = 5 (yani YÜZLÜK değer 3)
  byte onlar = (deger / 10) % 10;  // deger = 354 ise 54 / 10 = 5 (yani onluk değer 5)
  byte birler = deger % 10; // deger = 354 ise 54 % 10 = 4 (yani birler değer 4)
  shiftOut(DATA, CLK, MSBFIRST, cc_rakamlar[birler]);
  shiftOut(DATA, CLK, MSBFIRST, cc_rakamlar[onlar]);
  shiftOut(DATA, CLK, MSBFIRST, cc_rakamlar[yuzler]);
  digitalWrite(LATCH, HIGH);
}

void drawMenu() {
  display.setCursor(0,10);
  display.print("ZORLUK");
  display.setCursor(0,20);
  display.print("1-EASY");
  display.setCursor(0,30);
  display.print("2-HARD");
  display.display();
  pot_deger = analogRead(potansiyometre);
  int servo_deger = map(pot_deger,0,1023,0,180);
  myservo.write(servo_deger); 
  if (servo_deger > 90 && menu_selection == 1) { // Sağ tuşa basıldıysa
    menu_selection = 2;
  if (menu_selection > MENU_OPTIONS) {
    menu_selection = 1;
  }
  } else if (servo_deger < 90 && menu_selection == 2) { // Sol tuşa basıldıysa
    menu_selection = 1;
  if (menu_selection < 1) {
    menu_selection = MENU_OPTIONS;
  }
  }
  // Seçilen seçenek işaretlenir
  if (menu_selection == 1) {
    display.setCursor(0,40);
    display.print("S1");
    display.display();
  } else if (menu_selection == 2) {
    display.setCursor(0,40);
    display.print("S2");
    display.display();
  }

  if (digitalRead(FIRE_PIN) == LOW) { // Başlat düğmesine basıldıysa
    if (menu_selection == 1) {
      in_menu = false;
      game_over=false;
      delay(1000);
      display.clearDisplay();
  }
  if (menu_selection == 2) {
      in_menu = false;
      game_over=false;
      delay(1000);
      display.clearDisplay();
  }
  }
  display.clearDisplay();
}

void drawRocket() {
  display.fillRect(rocket_pos * BLOCK_SIZE, SCREEN_WIDTH - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, i_1);
  display.display();
}

void eraseRocket() {
  display.fillRect(rocket_pos * BLOCK_SIZE, SCREEN_WIDTH - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, i_2);
  display.display();
}

void drawObstacle(int x, int y) {
  display.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, i_1);
  display.display();
}

void eraseObstacle(int x, int y) {
  display.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, i_2);
  display.display();
}

void drawFire(int x, int y) {
  display.drawLine(x, y, x, y+2, x);
  display.drawPixel(x-1, y+1, x);
  display.drawPixel(x+1, y+1, x);
  delay(100);
}

// Ateş etme grafiklerini siler
void eraseFire(int x, int y) {
  display.drawLine(x, y, x, y+2, y);
  display.drawPixel(x-1, y+1, y);
  display.drawPixel(x+1, y+1, y);
}

void fireRocket() {
  int x = rocket_pos * BLOCK_SIZE + BLOCK_SIZE / 2;
  int y = SCREEN_WIDTH - BLOCK_SIZE - 3;
  for (int i = 0; i < 3; i++) {
    drawFire(x, y);
    delay(50);
    eraseFire(x, y);
    y -= 2;
  }
}

void moveRocketLeft() {
  if (rocket_pos > 0 && !game_over) {
    eraseRocket();
    rocket_pos -= 1;
    drawRocket();
  }
}

void moveRocketRight() {
  if (rocket_pos < MATRIX_WIDTH - 1 && !game_over) {
    eraseRocket();
    rocket_pos += 1;
    drawRocket();
  }
}

void endGame() {
  game_over = true;
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("GAME OVER");
  display.setCursor(0,10);
  display.print("Score = ");
  display.print(score_1);
  display.display();
  display.setCursor(0,20);
  display.print("PRESS FIRE BUTTON");
}

void loseLife() {
  lives--;
  digitalWrite(BUZZ, HIGH);
  tone(BUZZ, 523);
  delay(hiz);
  noTone(BUZZ);
  if (lives <= 0) {
    endGame();
  }
}
void start_game(){
  static unsigned long lastObstacleTime = 0;
  static int obstacle_x = 0;
  static int obstacle_y = 0;
  display.fillScreen(i_2);
  drawRocket();
  drawObstacle(obstacle_x, obstacle_y);
    if(lives==3){
      digitalWrite(LED_CAN_1, HIGH);
      digitalWrite(LED_CAN_2, HIGH);
      digitalWrite(LED_CAN_3, HIGH);
    }
    if(lives==2){
      digitalWrite(LED_CAN_1, HIGH);
      digitalWrite(LED_CAN_2, HIGH);
      digitalWrite(LED_CAN_3, LOW);
    }
    if(lives==1){
      digitalWrite(LED_CAN_1, HIGH);
      digitalWrite(LED_CAN_2, LOW);
      digitalWrite(LED_CAN_3, LOW);
    }
    if(lives==0){
      digitalWrite(LED_CAN_1, LOW);
      digitalWrite(LED_CAN_2, LOW);
      digitalWrite(LED_CAN_3, LOW);
    }
    if(ammo==3){
      digitalWrite(LED_SILAH_1, HIGH);
      digitalWrite(LED_SILAH_2, HIGH);
      digitalWrite(LED_SILAH_3, HIGH);
    }
    if(ammo==2){
      digitalWrite(LED_SILAH_1, HIGH);
      digitalWrite(LED_SILAH_2, HIGH);
      digitalWrite(LED_SILAH_3, LOW);
    }
    if(ammo==1){
      digitalWrite(LED_SILAH_1, HIGH);
      digitalWrite(LED_SILAH_2, LOW);
      digitalWrite(LED_SILAH_3, LOW);
    }
    if(ammo==0){
      digitalWrite(LED_SILAH_1, LOW);
      digitalWrite(LED_SILAH_2, LOW);
      digitalWrite(LED_SILAH_3, LOW);
    }
    if(menu_selection == 1){  
      if (millis() - lastObstacleTime > hiz && !game_over) {
        lastObstacleTime = millis();
        eraseObstacle(obstacle_x, obstacle_y);
        obstacle_y++;
        score_1++;
        if (obstacle_x == rocket_pos && obstacle_y == MATRIX_HEIGHT - 1) {
        score_1--;
      }
        yaz(score_1);  
        if (obstacle_y > MATRIX_HEIGHT - 1) {
          obstacle_x = random(MATRIX_WIDTH);
          obstacle_y = 0;
        }
        if (obstacle_x != rocket_pos && obstacle_y == MATRIX_HEIGHT -1) {
        score++;
        if(score % 14 == 0 && lives !=3){
            lives++;
            if(ammo!=3)
            {
              ammo++;
              }
          }  
        } 
      if (obstacle_x == rocket_pos && obstacle_y == MATRIX_HEIGHT - 1) {
        loseLife();
      }
      drawObstacle(obstacle_x, obstacle_y);
    }}
    if(menu_selection == 2){
      unsigned long suanki_zaman = millis();  
      if (millis() - lastObstacleTime > hiz && !game_over) {
        lastObstacleTime = millis();
        eraseObstacle(obstacle_x, obstacle_y);
        obstacle_y++;
        score_1++;
        if (obstacle_x == rocket_pos && obstacle_y == MATRIX_HEIGHT - 1) {
        score_1--;
      }
        yaz(score_1);
        if (obstacle_y > MATRIX_HEIGHT - 1) {
          obstacle_x = random(MATRIX_WIDTH);
          obstacle_y = 0;        
        }
        if (obstacle_x != rocket_pos && obstacle_y == MATRIX_HEIGHT -1) {
        score++;
        if(score % 14 == 0 && lives !=3){
            lives++;
            if(ammo!=3)
            {
              ammo++;
              }
          }  
        } 
      if (obstacle_x == rocket_pos && obstacle_y == MATRIX_HEIGHT - 1) {
        loseLife();
      }
      if(hiz>100 && suanki_zaman - zaman > beklenen_sure > 0){
        hiz = hiz * 0.8;
        zaman = suanki_zaman;
      }
      drawObstacle(obstacle_x, obstacle_y);
    }}

    if (digitalRead(FIRE_PIN) == LOW) {
      if(ammo!=0)
     {
       if(rocket_pos == obstacle_x){
         fireRocket();
         eraseObstacle(obstacle_x, obstacle_y);
         obstacle_y = MATRIX_HEIGHT;
       }
       
       ammo--;
      delay(50);}
    }
    pot_deger = analogRead(potansiyometre);
    int servo_deger = map(pot_deger,0,1023,0,180);
    myservo.write(servo_deger); 
    if (servo_deger<90) {
      moveRocketLeft();
      delay(500);
    }
    if (servo_deger>90) {
      moveRocketRight();
      delay(500);
    }

    if (game_over && digitalRead(FIRE_PIN) == LOW) {
      lives = 3;
      rocket_pos = MATRIX_WIDTH / 2 - 1;
      in_menu = true;
      score=0;
      score_1=0;
      hiz=1000;
      display.clearDisplay();
      drawMenu();
    }
}

void loop() {
  if (in_menu) {
    int isik = analogRead(ldr_deger);
    if(isik>50){
      i_1 = SSD1306_WHITE;
      i_2 = SSD1306_BLACK;
    }
    if(isik<50){
      i_2 = SSD1306_WHITE;
      i_1 = SSD1306_BLACK;
    }
    display.fillScreen(i_2);
    drawMenu();
  }
  
  else{
    int isik = analogRead(ldr_deger);
     if(isik>50){
      i_1 = SSD1306_WHITE;
      i_2 = SSD1306_BLACK;
    }
    if(isik<50){
      i_2 = SSD1306_WHITE;
      i_1 = SSD1306_BLACK;
    }
    //display.fillScreen(i_2);
    start_game();
    }
}
