#include <Servo.h>  
#include <ArduinoSort.h>

const int irSensorPin = A0;
const float voltsPerMeasurement = 5.0/1024.0;

const int echoPin = 8;
const int trigPin = 11;

Servo UsServo;
int UsPosition = 0;
Servo IrServo;
int IrPosition = 0;

int filterWindowUs = 8;
int distsUs[8] = {0} ;
int currentFilterIndex = 0;

void setup()
{
  UsServo.attach(10);
  IrServo.attach(9);
  updatePositions();
  Serial.begin(115200);

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
}

void loop() {
  float irDistance = readDistIR();
  irDistance = constrain(irDistance, 20, 120);
  IrPosition = convertToDegrees(irDistance);
  
  float usDistance = readFilteredUs();
  usDistance = constrain(usDistance, 20, 120);
  UsPosition = convertToDegrees(usDistance);
  
  Serial.print(UsPosition);Serial.print(" ");Serial.println(IrPosition);

  UsServo.write(UsPosition);
  IrServo.write(IrPosition);
  
  delay(150);
}

int convertToDegrees(float dist)
{
  float constrained = constrain(dist, 20, 120);
  return (int)(constrained - 20) * 1.8;
}

float readLenUs()
{
  const float speedOfSoundMPerSec = 340.0;
  const float speedOfSoundCmPerUs = speedOfSoundMPerSec / 10000.0;
  return readPulseUs() * speedOfSoundCmPerUs / 2.0;    
}

float readPulseUs()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 1000*1000*15);

  return duration;
}

float readDistIR(){
  float volts = readAnalogIR() * voltsPerMeasurement;
  return 65 * pow(volts, -1.10); // for big IR sensor (SHARP 2Y0A02)
}

float readAnalogIR()
{
  int sum = 0;
  int maxV = -5000;
  int minV = 5000;
  int n = 15;
  for (int i = 0; i < n; i++)
  {
    int cur = analogRead(irSensorPin);
    if (cur > maxV)
    {
      maxV = cur;
    }
    if (cur < minV)
    {
      minV = cur;
    }
    sum += cur;
    delay(6);
  }
  return (sum - maxV - minV) / (float)(n - 2);
}

float readFilteredUs()
{
  int cur = readLenUs();
  distsUs[currentFilterIndex]=cur;
  currentFilterIndex = (currentFilterIndex + 1) % filterWindowUs;
  delay(6);
  int curDist[filterWindowUs] = {0};
  for (int i = 0; i < filterWindowUs; i++){
    curDist[i] = distsUs[i];
  }
  sortArray(curDist,filterWindowUs);
  return curDist[int(filterWindowUs/2)];
}

void updatePositions(){
  UsServo.write(UsPosition);
  IrServo.write(IrPosition);
  Serial.print(UsPosition);Serial.print(" ");Serial.print(IrPosition);
}
