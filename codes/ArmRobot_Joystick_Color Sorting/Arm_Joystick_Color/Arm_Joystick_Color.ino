/* This code for MeArm robot using joystick shield with arduino uno
   Please use orginal arduino uno
   I use power adaptor 12v
   Don't forget to adjust the servo position in 0 degree or 180 degree when attaching the servo horn
   Make sure the linkages are smooth, you can enlarge diameter
   Use LCD for show information
   GY-31 for color detection
*/

#include <Servo.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); //20 characters, 4 rows

//for color detection--------
const int s0 = A8;
const int s1 = A9;
const int s2 = A10;
const int s3 = A11;
const int out = A12;
// values
int red = 0;
int green = 0;
int blue = 0;

String color_detection;
int target_x;//for target x axis position each color
int target_y;
int target_z;

int check_times = 0;
//for color detection--------

//for IR obstacle sensor----------------------------
const int sensor_pin = 22; //For IR obstacle sensor
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin
bool status_object = false;
//for IR obstacle sensor----------------------------

Servo servo_x;  // create servo object to control a servo
Servo servo_y;  // create servo object to control a servo
Servo servo_z;  // create servo object to control a servo
Servo servo_grip;  // create servo object to control a servo

// Declare the pins for the Button
int x_pin = A0;
int y_pin = A1;
int z_pin = A2;
int grip_pin = A3;

int button1 = 2; //for starting pickup 1
int button2 = 4; //for starting pickup 2

bool status_button1 = false;
bool status_button2 = false;

int x_pos;    // variable to store the servo position
int y_pos;    // variable to store the servo position
int z_pos;    // variable to store the servo position
int grip_pos;    // variable to store the servo position
bool object_ready = false;
//millis================================
//Set every  sec detection
unsigned long previousMillis = 0; // variable to store the last time the task was run
const long interval = 1000; // time interval in milliseconds (eg 1000ms = 1 second)
//======================================

void setup() {
  Serial.begin(9600);
  // initialize the Arduino's pin as input sensor IR
  pinMode(sensor_pin, INPUT);

  //button center of joystick
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  //LCD
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Information:");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // For color detection
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

  // Define pin as input and activate the internal pull-up resistor
  //setup servo
  servo_x.attach(11);  // attaches the servo x
  servo_y.attach(10);  // attaches the servo y
  servo_z.attach(9);  // attaches the servo z
  servo_grip.attach(5);  // attaches the servo grip

  //set to home  position
  x_pos = 99;
  y_pos = 0;
  z_pos = 180;
  grip_pos = 4;

  servo_x.write(x_pos);
  servo_y.write(y_pos);
  servo_z.write(z_pos);
  servo_grip.write(grip_pos);
  print_position();
}

void loop() {
  // Every interval 1 sec, check object and check color
  //-------------------------------------------------
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time the task was run
    previousMillis = currentMillis;
    check_object();
    if (status_object == true) {
      //check color
      check_color();
      lcd.setCursor(0, 3);
      lcd.print(color_detection + " COLOR   ");
      check_times = check_times + 1;
      if (check_times == 4) {
        object_ready = true;
        check_times = 0;
      }
    }
    else {
      lcd.setCursor(0, 3);
      lcd.print("NO OBJECT!!");
    }
  }
  //-------------------------------------------------

  //Run Auto when detect object
  if (object_ready == true) {
    object_ready = false;
    //Go to pickup
    delay(1500);
    pickup();
    delay(2000);
    move_target();
    delay(1500);
    home_position();
  }

  //center button check--------------------------------------------
  int buttonValue1 = digitalRead(button1);
  int buttonValue2 = digitalRead(button2);
  //button 1========================================
  if (buttonValue1 == LOW) {
    if (status_button1 == false) {
      Serial.println("Button 1 Pressed");
      status_button1 = true;
      target_x = 158;
      target_y = 122;
      target_z = 126;
      //Go to pickup
      pickup();
      delay(3000);
      move_target();
      delay(1500);
      home_position();
    }
  }
  if (buttonValue1 == HIGH) {
    if (status_button1 == true) {
      status_button1 = false;
    }
  }
  //button 1========================================

  //button 2========================================
  if (buttonValue2 == LOW) {
    if (status_button2 == false) {
      Serial.println("Button 2 Pressed");
      status_button2 = true;
      target_x = 40;
      target_y = 122;
      target_z = 126;
      //Go to pickup
      pickup();
      delay(3000);
      move_target();
      delay(1500);
      home_position();
    }
  }
  if (buttonValue2 == HIGH) {
    if (status_button2 == true) {
      status_button2 = false;
    }
  }
  //button 2========================================
  //center button check--------------------------------------------
  delay(15);
  manual_move();
}

void check_object() {
  // read the state of the the input pin:
  currentState = digitalRead(sensor_pin);

  if (lastState == HIGH && currentState == LOW) {
    Serial.println("Object is detected");
    status_object = true;
  }
  else if (lastState == LOW && currentState == HIGH) {
    Serial.println("Object is cleared");
    status_object = false;
    object_ready = false;
  }
  // save the the last state
  lastState = currentState;
}

void check_color() {
  color();
  Serial.println("RED");
  Serial.println(red, DEC);
  Serial.println(" GREEN ");
  Serial.println(green, DEC);
  Serial.println(" BLUE ");
  Serial.println(blue, DEC);
  if (red < blue && green < blue && green < 10 && blue >= 9)
  {
    color_detection = "YELLOW";
    target_x = 18;
    target_y = 135;
    target_z = 170;
    Serial.println(" - (Yellow Color)");
  }
  else if (red < blue && red < green && red < 20 && green >= 10)
  {
    color_detection = "RED";
    target_x = 180;
    target_y = 135;
    target_z = 170;
    Serial.println(" - (Red Color)");
  }
  else if (blue < red && blue < green)
  {
    color_detection = "BLUE";
    target_x = 140;
    target_y = 135;
    target_z = 170;
    Serial.println(" - (Blue Color)");
  }
  else if (green < red && green < blue)
  {
    color_detection = "GREEN";
    target_x = 59;
    target_y = 135;
    target_z = 170;
    Serial.println(" - (Green Color)");
  }
  else {
    color_detection = "UNKNOWN";
    target_x = 158;
    target_y = 122;
    target_z = 126;
  }
}

void color()
{
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
}

void manual_move() {
  lcd.setCursor(0, 1);
  lcd.print("Manual Operation");
  // Read the value of the input
  int val_x = analogRead(x_pin);
  int val_y = analogRead(y_pin);
  int val_z = analogRead(z_pin);
  int val_grip = analogRead(grip_pin);
  // joystick X========================================
  if (val_x < 400) {
    if (x_pos < 180) {
      x_pos = x_pos + 1;
      servo_x.write(x_pos);
      print_position();
    }
  }
  else if (val_x > 750) {
    if (x_pos > 0) {
      x_pos = x_pos - 1;
      servo_x.write(x_pos);
      print_position();
    }
  }
  // joystick X========================================

  // joystick Y========================================
  if (val_y < 400) {
    if (y_pos < 145) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
      print_position();
    }
  }
  else if (val_y > 750) {
    if (y_pos > 0) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);

      //check z pos
      if (z_pos < 150)
      {
        if (z_pos < 175) {
          z_pos = z_pos + 1;
          servo_z.write(z_pos);
        }
      }
      print_position();
    }
  }
  // joystick Y========================================

  // joystick Z========================================
  if (val_z < 400) {
    if (z_pos < 180) {
      z_pos = z_pos + 1;
      servo_z.write(z_pos);
      print_position();
    }
  }
  else if (val_z > 750 && y_pos > 40) {
    if (z_pos > 90) {
      z_pos = z_pos - 1;
      servo_z.write(z_pos);
      print_position();
    }
  }
  // joystick Z========================================

  // joystick grip========================================
  if (val_grip < 400) {
    if (grip_pos < 30) {
      grip_pos = grip_pos + 1;
      servo_grip.write(grip_pos);
      print_position();
    }
  }
  else if (val_grip > 750) {
    if (grip_pos > 0) {
      grip_pos = grip_pos - 1;
      servo_grip.write(grip_pos);
      print_position();
    }
  }
  // joystick grip========================================
}
void print_position()
{
  Serial.print("x_axis: ");
  Serial.print(x_pos);
  Serial.print(", y_axis: ");
  Serial.print(y_pos);
  Serial.print(", z_axis: ");
  Serial.print(z_pos);
  Serial.print(", grip_axis: ");
  Serial.println(grip_pos);
  String str = String(x_pos) + ", " + String(y_pos) + ", " + String(z_pos) + ", " + String(grip_pos);
  int len_str = str.length();
  if (len_str < 20) {
    int add_space = 20 - len_str;
    //add space
    for (int i = 1; i <= add_space; i++) {
      str = str + " ";
    }
  }
  lcd.setCursor(0, 2);
  lcd.print(str);
}

void pickup() {
  //position: x=90, y=140, z=160
  Serial.println("Pick Up Position");
  lcd.setCursor(0, 1);
  lcd.print("Pick Up Position");

  //avoiding hit the obstacle, move x,y first
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis x-----------------------------
    if (x_pos < 99) {
      x_pos = x_pos + 1;
      servo_x.write(x_pos);
    }
    if (x_pos > 99) {
      x_pos = x_pos - 1;
      servo_x.write(x_pos);
    }
    //Move axis x-----------------------------
    //Move axis y-----------------------------
    if (y_pos < 50) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
    }
    if (y_pos > 50) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------
    if (x_pos == 99 && y_pos == 50) {
      break;
    }
  }

  delay(500);
  open_grip();
  delay(500);

  //go to position
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis y-----------------------------
    if (y_pos < 135) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
    }
    if (y_pos > 135) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------

    //Move axis z-----------------------------
    if (z_pos < 170) {
      z_pos = z_pos + 1;
      servo_z.write(z_pos);
    }
    if (z_pos > 170) {
      z_pos = z_pos - 1;
      servo_z.write(z_pos);
    }
    //Move axis z-----------------------------

    if (y_pos == 135 && z_pos == 170) {
      break;
    }
  }
  print_position();
}

void open_grip () {
  //Open Grip--------------------------------
  for (int i = 1; i <= 30; i++) {
    delay(15);  // Delay for smooth movement
    if (grip_pos <= 30) {
      grip_pos = grip_pos + 1;
      servo_grip.write(grip_pos);
    }
  }
  //Open Grip--------------------------------
}
void close_grip() {
  //Close Grip--------------------------------
  for (int i = 1; i <= 30; i++) {
    delay(15);  // Delay for smooth movement
    if (grip_pos > 3) {
      grip_pos = grip_pos - 1;
      servo_grip.write(grip_pos);
    }
  }
  //Close Grip--------------------------------
}

void move_target() {
  Serial.println("Target Position");
  lcd.setCursor(0, 1);
  lcd.print("Target Position ");

  close_grip();
  delay(1000);
  //Move y------------------------------------
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis y-----------------------------
    if (y_pos < 50) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
    }
    if (y_pos > 50) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------
    if (y_pos == 50) {
      break;
    }
  }
  //Move y------------------------------------
  delay(500);
  //Go to position
  //avoiding hit the obstacle, move x first
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis x-----------------------------
    if (x_pos < target_x) {
      x_pos = x_pos + 1;
      servo_x.write(x_pos);
    }
    if (x_pos > target_x) {
      x_pos = x_pos - 1;
      servo_x.write(x_pos);
    }
    //Move axis x-----------------------------
    if (x_pos == target_x) {
      break;
    }
  }
  delay(500);
  //to position
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis y-----------------------------
    if (y_pos < target_y) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
    }
    if (y_pos > target_y) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------
    //Move axis z-----------------------------
    if (z_pos < target_z) {
      z_pos = z_pos + 1;
      servo_z.write(z_pos);
    }
    if (z_pos > target_z) {
      z_pos = z_pos - 1;
      servo_z.write(z_pos);
    }
    //Move axis z-----------------------------
    if (y_pos == target_y && z_pos == target_z) {
      break;
    }
  }
  delay(500);
  open_grip();
  print_position();
}

void home_position() {
  Serial.println("Go to Home Position");
  lcd.setCursor(0, 1);
  lcd.print("Home Position  ");

  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis y-----------------------------
    if (y_pos < 50) {
      y_pos = y_pos + 1;
      servo_y.write(y_pos);
    }
    if (y_pos > 50) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------
    if (y_pos == 50) {
      break;
    }
  }
  delay(300);
  close_grip();
  delay(300);
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis x-----------------------------
    if (x_pos <= 99) {
      x_pos = x_pos + 1;
      servo_x.write(x_pos);
    }
    if (x_pos >= 99) {
      x_pos = x_pos - 1;
      servo_x.write(x_pos);
    }
    //Move axis x-----------------------------
    if (x_pos == 99) {
      break;
    }
  }
  delay(300);
  for (int i = 1; i <= 180; i++) {
    delay(15);  // Delay for smooth movement
    //Move axis y-----------------------------
    if (y_pos > 0) {
      y_pos = y_pos - 1;
      servo_y.write(y_pos);
    }
    //Move axis y-----------------------------
    //Move axis z-----------------------------
    if (z_pos < 180) {
      z_pos = z_pos + 1;
      servo_z.write(z_pos);
    }
    //Move axis z-----------------------------
    if (y_pos == 0 && z_pos == 180) {
      break;
    }
  }
  print_position();
}
