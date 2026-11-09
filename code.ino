#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

const int dataPin = 13;
const int clockPin = 9;
const int latchPin = 10;

volatile bool doUpdate = false;
volatile int phaseCounter = 1;
volatile int timerCounter = 0;

ISR(TIMER2_COMPA_vect){
  timerCounter++;

  if (timerCounter >= 61){
    doUpdate = true;
    phaseCounter = (phaseCounter + 1) % 3;
    timerCounter = 0;
  }
}

class segmentController{
  private:
  byte numbers[10] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,
  };

  public:
  segmentController(){
    //pinMode(dataPin, OUTPUT);
    //pinMode(clockPin, OUTPUT);
    //pinMode(latchPin, OUTPUT);
    DDRB |= (1 << DDB5) | (1 << DDB1) | (1 << DDB2);
  }

  void writeByte(byte value){
    for(int i = 7; i >= 0; --i){
      if(value & (1 << i)){
        PORTB |= (1 << PB5);
      }
      else{
        PORTB &= ~(1 << PB5);
      }

      PORTB |= (1 << PB1);
      PORTB &= ~(1 << PB1);
    }
  }

  void writeSingleNumber(int number){
    PORTB &= ~(1 << PB2);
    writeByte(~numbers[number]);
    PORTB |= (1 << PB2);
  }

  void clear(){
    PORTB &= ~(1 << PB2);
    writeByte(0b11111111);
    PORTB |= (1 << PB2);
  }
};

class LcdController{
  private:
  LiquidCrystal lcd;
  int animationCounter = 0;
  byte tilde[8] = {
    B00000,
    B00000,
    B00000,
    B01001,
    B10110,
    B00000,
    B00000,
    B00000
  };
  byte backslash[8] = {
    B00000,
    B10000,
    B01000,
    B00100,
    B00010,
    B00001,
    B00000,
    B00000
  };


  public:
  LcdController() : lcd(12, 11, 5, 4, 3, 2) {
    lcd.begin(16, 2);
    lcd.createChar(0, tilde);
    lcd.createChar(1, backslash);
  }

  void print(char *line, int row){
    lcd.setCursor(0, row);
    lcd.print(line);
  }

  void setByTemp(int temp){
    if(temp < 5){
      if(animationCounter == 0){
        print("  * Freezing *  ", 0);
        print("  > ( >_<) <    ", 1);
      }
      else{
        print("  * Freezing *  ", 0);
        print("    < ( >_<) >  ", 1);
      }
    }
    else if(temp >= 5 && temp < 18){
      if(animationCounter == 0){
        print("  * Chilly *  ", 0);
        print("    (-_- )    ", 1);
      }
      else{
        print("  * Chilly *  ", 0);
        print("    ( -_-)    ", 1);
      }
    }
    else if(temp >= 18 && temp < 22){
      if(animationCounter == 0){
        print("   * Fine *   ", 0);
        print("    ( ^_^)    ", 1);
      }
      else{
        print("   * Fine *   ", 0);
        print("    (^_^ )    ", 1);
      }
    }
    else if(temp >= 22 && temp < 28){
      if(animationCounter == 0){
        print("   * Warm *   ", 0);
        lcd.setCursor(0, 1);
        lcd.print("  ");
        lcd.write(byte(0)); 
        lcd.print("( ^.^)");
        lcd.write(byte(0));
        lcd.print("    ");
      }
      else{
        print("   * Warm *   ", 0);
        lcd.setCursor(0, 1);
        lcd.print("    ");
        lcd.write(byte(0)); 
        lcd.print("(^.^ )");
        lcd.write(byte(0));
        lcd.print("  ");
      }
    }
    else if(temp >= 28 && temp < 40){
      if(animationCounter == 0){
        print("    * Hot *   ", 0);
        lcd.setCursor(0, 1);
        lcd.print("  ");
        lcd.write(byte(1)); 
        lcd.print("( >o<)/    ");
      }
      else{
        print("   * Hot *    ", 0);
        lcd.setCursor(0, 1);
        lcd.print("    /(>.< )");
        lcd.write(byte(1));
        lcd.print("  ");
      }
    }
    else if(temp >= 40){
      if(animationCounter == 0){
        print("    * Dead *  ", 0);
        print("   ( x_x)     ", 1);
      }
      else{
        print("  * Dead *    ", 0);
        print("     ( x_x)   ", 1);
      }
    }
    animationCounter = (animationCounter + 1) % 2;
  }
};

class ServoController{
  private:
  Servo servo;
  int servoPhaseCounter = 0;
  int direction = 1;

  public:
  ServoController(){}

  void begin() {
    servo.attach(6);
    servo.write(90);
  }

  void turnByTemp(int temp){
    if(temp <= 20){
      servo.write(90);
      servoPhaseCounter = 0;
      direction = 1;
      return;
    }

    float turn = (temp - 20) * 10.0;
    if(turn > 60){
      turn = 60;
    }

    float interval = 6.0 - (temp - 20) * 0.5;
    if(interval > 6){
      interval = 6;
    }

    servoPhaseCounter++;

    if(servoPhaseCounter >= interval){
      servoPhaseCounter = 0;
      direction = -direction;
      servo.write(90 + direction * turn);
    }
  }
};

float readTemperature() {
  int total = 0;

  for(int i = 0; i < 10; ++i) {
    total += analogRead(A0);
  }

  float average = total / 10.0;
  float voltage = average * (5.0 / 1023.0);

  return voltage * 100.0;
}

segmentController sc;
LcdController lc;
ServoController src;
int temperature;
int digits[2];

class EepromController{
  private:
  struct SaveData{
    uint16_t magic;
    byte version;
    int temperature;
  };

  const uint16_t EEPROM_MAGIC = 123;
  const byte EEPROM_VERSION = 1;
  const int EEPROM_ADDRESS = 0;

  public:
  void saveData(){
    SaveData data;

    data.magic = EEPROM_MAGIC;
    data.version = EEPROM_VERSION;
    data.temperature = temperature;

    EEPROM.put(EEPROM_ADDRESS, data);
  }

  void loadData(){
    SaveData data;
    EEPROM.get(EEPROM_ADDRESS, data);

    if (data.magic != EEPROM_MAGIC || data.version != EEPROM_VERSION){
      temperature = 0;
      saveData();
      return;
    }

    temperature = data.temperature;
  }
};

EepromController ec;

void setup() {
  ec.loadData();
  src.begin();
  Serial.begin(9600);

  noInterrupts();

  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  OCR2A = 249;
  TCCR2A |= (1 << WGM21);
  TIMSK2 |= (1 << OCIE2A);
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);

  interrupts();

  //pinMode(7, OUTPUT);
  //DDRD |= (1 << DDD7);
}

void loop() {
  if(doUpdate){
    if(phaseCounter == 0){
      temperature = (int)readTemperature();
      Serial.println(readTemperature());
      int tempTemp = temperature;
      for(int i = 1; i >= 0; --i) {
        digits[i] = tempTemp % 10;
        tempTemp /= 10;
        sc.writeSingleNumber(digits[0]);
      }

      ec.saveData();

      if(temperature > 40){
        //digitalWrite(7, HIGH);
        //PORTD |= (1 << PD7);
        //delay(100);
        //digitalWrite(7, LOW);
        //PORTD &= ~(1 << PD7);
      }
    }
    else if(phaseCounter == 1){
      sc.clear();
      delay(10);
      sc.writeSingleNumber(digits[1]);
    }
    else if(phaseCounter == 2){
      sc.clear();
    }

    lc.setByTemp(temperature);
    src.turnByTemp(temperature);

    doUpdate = false;
  }
}
