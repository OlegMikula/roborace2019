#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);// Гіроскоп
#define pin_servo 5   // Задали пін на серво поворотів коліс
Servo myServo;
#define enA 6         // Ззадали пін на задній мотор (швидкість)
#define in1 3         // Ззадали пін на задній мотор
#define in2 4         // Ззадали пін на задній мотор
#define sens_left A2 // Задали пін на лівий дальномір
#define sens_center A1 // Задали пін на центральний дальномір
#define sens_right A0 // Задали пін на правий дальномір
#define rear_leds 10  // Задали пін на задні світлодіоди
#define leds_qnt 4    // Задали кількість світлодіодів
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(leds_qnt, rear_leds, NEO_GRB + NEO_KHZ800);
int temp = 0;
int basicSpeed = 0;   // Задали базову швидкість їзди
int motorSpeedA = 0;
int gyro_speed = 0;   // Додаткова швидкість від гіроскопа
int PotInfo = 0;
int Obstacle = 0;
int Left_distance;
int Right_distance;
int Center_distance;

long timer = 0;
float setupgyro = 0;
int Turning = 0; // Поворот коліс на основі дальномірів


// Користувацькі функції

float distance(int n, int pin) {
  long sum = 0;
  for(int i = 0; i < n; i++) {
    sum += analogRead(pin);
  } return(17569.7 * pow(sum / n, -1.2062));
}

int calcTurning(int Right_distance, int Left_distance, int flazhok){
  if(flazhok == 1){
    Turning = Right_distance - Left_distance; // Визначаємо кут повороту коліс
    if (Turning > 50) Turning = 50;   // Обмежуємо кут в 50 градусів ЛІВО
    if (Turning < -50) Turning = -50; // Обмежуємо кут в 50 градусів ПРАВО
  }
  if(flazhok == 0){
    if(Right_distance > Left_distance){
      Turning = -50;}
    if(Left_distance > Right_distance){
      Turning = 50;}
  }
  Serial.println(Turning);
  return Turning;
}

void showLight(String signal){
  if(signal == "left"){
    pixels.setPixelColor(0, pixels.Color(255,255,0));
    pixels.setPixelColor(1, pixels.Color(255,255,0));
  }
  if(signal == "right"){
    pixels.setPixelColor(2, pixels.Color(255,255,0));
    pixels.setPixelColor(3, pixels.Color(255,255,0));
  }
  if(signal == "stop"){
    pixels.setPixelColor(0, pixels.Color(255,0,0));
    pixels.setPixelColor(1, pixels.Color(255,0,0)); 
    pixels.setPixelColor(2, pixels.Color(255,0,0)); 
    pixels.setPixelColor(3, pixels.Color(255,0,0));
  }
  if(signal == "show"){
    pixels.show();
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));  
  }
}

/////////////////////////////

void setup() {
  Serial.begin(9600);         // Відкриваємо серійний порт
  Wire.begin();
  mpu6050.begin();            // Активували гіроскоп
  pixels.begin();             // Активували задні світлодіоди
  myServo.attach(pin_servo);  // Активували серво
  myServo.write(100);         // Повернули колеса по центру
  
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  analogWrite(enA, basicSpeed);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW); 

}

void loop() {
//Зчитуємо гіроскоп
  mpu6050.update();
  if (temp == 0) {
    if (millis() - timer > 1000) {
      setupgyro = mpu6050.getAngleY();
      timer = millis();  
      temp=1;
    }
  }
  if (millis() - timer > 1000) {
    gyro_speed = (setupgyro - mpu6050.getAngleY()) * 2;
    timer = millis();
    if (gyro_speed > 100) gyro_speed = 100;
    if (gyro_speed < -10) gyro_speed = -10;
    Serial.println(gyro_speed);
  }

  // Зчитуємо дані з усіх сенсорів
  PotInfo = analogRead(A6); // Потенціометер 
  // Визначаємо та обмежуємо відстані до перешкод по сторонах
  Center_distance = distance(5, sens_center) + 4;
  if(Center_distance > 80) Center_distance = 80;
  Left_distance = distance(5, 2);
  if (Left_distance > 80) Left_distance = 80;
  Right_distance = distance(5, sens_right); 
  if (Right_distance > 80) Right_distance = 80;

  // Serial.print("Center: ");
  // Serial.println(Center_distance);
  // Serial.print("Left: ");
  // Serial.println(Left_distance);
  // Serial.print("Right: ");
  // Serial.println(Right_distance);
  // delay(500);

  // Вираховуємо швидкість руху
  basicSpeed = map(PotInfo, 0, 1023, 60, 155);
  motorSpeedA = basicSpeed + gyro_speed;
  
  if (motorSpeedA > 255) motorSpeedA = 255;
  analogWrite(enA, motorSpeedA);
  
  if (Right_distance <= 25) {
    showLight("left");
  }
  if (Left_distance <= 25) {
    showLight("right");
  }
  myServo.write(100 + calcTurning(Right_distance, Left_distance, 1)); // Повертаємо колеса на вирахуваний кут

  
  // Якщо попереду перешкода
  if (Center_distance <= 20){
    
    showLight("stop");

    myServo.write(100 + calcTurning(Right_distance, Left_distance, 0));
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    delay(200);

    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
   }

  showLight("show");  
}