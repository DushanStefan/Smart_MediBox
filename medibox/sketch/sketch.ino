//include libraries
#include <Wire.h>
#include<WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>


//define OLED parameters
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C


#define LED_1 15
#define BUZZER 5
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12

#define NTP_SERVER      "pool.ntp.org"
#define UTC_OFFSET      0
#define UTC_OFFSET_DST  0

//declare objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//global variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

int put_hours = 0;
int put_minutes = 0;
int tot_seconds = 0;
String sign[] = {"Positive", "Negative"};
int present_sign = 0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 2};
int alarm_minutes[] = {1, 10, 10};
int alarm_triggered[] = {false, false, false};


int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

int current_mode = 0;
int max_modes = 5;
String modes[] = {"1-Set time zone", "2-Set alarm 1", "3-Set alarm 2", "4-Set alarm 3", "5-Disable alarms"};

void setup() {

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);
  // put your setup code here, to run once:
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  Serial.begin(9600);

  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);

  }
  //show the display buffer on the screen. you should call display() after
  //drawing commands to make them visible on screen!
  display.display();
  delay(500);



  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
   
    String ss = "";
    for(int i=0;i<7;i++){
        ss+=".";
        print_line(" Connecting to WIFI"+ss, 0 , 0, 1);
        delay(500);
    }


  }

  display.clearDisplay();
  delay(500);
  print_line("Connecting to WIFI", 0, 0, 2);
  delay(500);
  //display.clearDisplay();
 

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  display.clearDisplay();


  print_line("Welcome to medibox!", 15, 20, 2);
  delay(500);
  display.clearDisplay();

  //clear the buffer

}

void loop() {
  update_time_with_check_alarm();
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
  // put your main code here, to run repeatedly:
  check_temp();
  display.display();
}
//in here this function use to print the text which we enter to this
void print_line(String text, int column, int row, int text_size) {

  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row); //(colomn,row)
  display.println(text);

  display.display();

}

void print_line_1(String text, int column, int row, int text_size) {

  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row); //(colomn,row)
  display.println(text);


}

//this function is used to print time
void print_time_now() {
  //Serial.println("this is print_time_now");
  display.clearDisplay();
  String test = String(hours) + ":" + String(minutes) + ":" + String(seconds);
  print_line(test, 0, 0, 2);


}
//this function is used to update time
void update_time() {

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("failed to get time");
    return;
  }

  char timeHour [3];
  strftime (timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);

  char timeMinute [3];
  strftime (timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);

  char timeSecond [3];
  strftime (timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);

  char timeDay [3];
  strftime (timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);

}

//this function is used to ring the alarm
void ring_alarm() {
  display.clearDisplay();
  print_line("Medicine time", 0, 0, 2);

  digitalWrite(LED_1, HIGH);

  bool break_happened = false;
  //ring the buzzer
  while (break_happened == false && digitalRead(PB_CANCEL) == HIGH) {
    for (int i = 0; i < n_notes; i++) {
      if (digitalRead(PB_CANCEL) == LOW) {
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(20);
    }
  }
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}


//this function is used to update time with checking alarm
void update_time_with_check_alarm(void) {
  display.clearDisplay();
  update_time();
  print_time_now();

  if (alarm_enabled == true) {
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}

//from this function we return an output as a number according we press a button
int wait_for_button_press() {
  while (true) {


    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    }
    update_time();
  }
}


//this is use to run the menu
void go_to_menu() {
  while (digitalRead(PB_CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(modes[current_mode], 0, 0, 2);

    int pressed = wait_for_button_press();
    //int pressed=PB_DOWN;

    if (pressed == PB_UP) {
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;

    }
    else if (pressed == PB_DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK) {
      delay(200);
      run_mode(current_mode);
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }

  }
}

//from this function we can set the time zone as negative or positive value
void set_time_zone() {
  int temp_sign = present_sign;
  int temp_hour = put_hours;
  
  while (true) {
    display.clearDisplay();
    print_line("Enter Sign: " + sign[temp_sign], 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_sign += 1;
      temp_sign = temp_sign % 2;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_sign += 1;
      temp_sign = temp_sign % 2;
    }
    else if (pressed = PB_OK) {
      delay(200);
      present_sign = temp_sign;
      break;
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
  


  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }
    else if (pressed = PB_OK) {
      delay(200);
      put_hours = temp_hour;
      break;
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }


  int temp_minute = put_minutes;

  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }
    else if (pressed = PB_OK) {
      delay(200);
      put_minutes = temp_minute;
      tot_seconds = put_hours * 3600 + put_minutes * 60;
      if (sign[present_sign]=="Negative") {
        tot_seconds = -1 * tot_seconds;
      }
      configTime(tot_seconds, UTC_OFFSET_DST, NTP_SERVER);
      break;
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Time is set ", 0, 0, 2);
  delay(1000);
}

// setting the alarm
void set_alarm(int alarm) {
  int temp_hour = alarm_hours[alarm];

  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed =  wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    }
    else if (pressed = PB_OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }


  int temp_minute = alarm_minutes[alarm];

  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }
    else if (pressed = PB_OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Alarm is set ", 0, 0, 2);
  delay(1000);
}


void run_mode(int mode) {
  if (mode == 0) {
    set_time_zone();
  }

  else if (mode == 1 || mode == 2 || mode == 3) {
    set_alarm(mode - 1);
  }

  else if (mode == 4) {
    alarm_enabled = false;
  }
}

//by this funtion check the temperature and the humidity
void check_temp() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (data.temperature > 32) {
    //display.clearDisplay();
    print_line("TEMP HIGH", 0, 40, 1);
    delay(1000);
  }
  else if (data.temperature < 26) {
    //display.clearDisplay();
    print_line("TEMP LOW", 0, 40, 1);
    delay(1000);
  }
  if (data.humidity > 80) {
    //display.clearDisplay();
    print_line("HUMIDITY HIGH", 0, 50, 1);
    delay(1000);
  }
  else if (data.humidity < 60) {
    //display.clearDisplay();
    print_line("HUMIDITY LOW", 0, 50, 1);
    delay(1000);
  }
}
