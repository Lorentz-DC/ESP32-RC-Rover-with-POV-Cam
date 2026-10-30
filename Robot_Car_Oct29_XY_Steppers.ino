#include <Bluepad32.h>
#include <IRremote.hpp>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <AccelStepper.h>
//#include <ESP32Servo.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS]; // This callback gets called any time a new gamepad is connected. // Up to 4 gamepads can be connected at the same time.
Adafruit_INA219 ina219;
//Servo myservo;  // create servo object to control a servo

//define motor output pins
#define FL_dir_pin 32 //Front Left M1 direction 
#define FL_PWM_pin 33 //Front Left M1 speed 
#define RL_PWM_pin 25 //Rear Left M2 Speed
#define RL_dir_pin 26 //Rear Left M2 Direction 
#define RR_dir_pin 5 //Rear right M3 Direction 
#define RR_PWM_pin 17 //Rear right M3 speed
#define FR_PWM_pin 12 //Front right M4 speed
#define FR_dir_pin 13 //Front right M4 direction 
#define Blue_LED_pin 27//blue LED 
#define IR_signal_pin 35 //IR remote signal pin

// Define Input pins of the Motor
#define X_In1   16               
#define X_In2   4               
#define X_In3   2                
#define X_In4   15      

#define Y_In1   14               
#define Y_In2   18              
#define Y_In3   19                
#define Y_In4   23  

// 28BYJ-48 safe physical limits: ~14 RPM max = ~500 steps per second max.
const float maxSpeedLimit = 512.0;  

#define servo_pin 14

// Initialize the stepper motor with corrected internal sequence (1, 3, 2, 4)
AccelStepper myStepperX(AccelStepper::FULL4WIRE, X_In1, X_In3, X_In2, X_In4);
AccelStepper myStepperY(AccelStepper::FULL4WIRE, Y_In1, Y_In3, Y_In2, Y_In4);

unsigned long lastCarUpdateTime = 0;
const unsigned long carUpdateInterval = 150; 

int IR_code = 0; //IR remote signal code received
bool controller_connected_state = false; //added the state flag for bluetooth connection check in onConnectedController(ControllerPtr ctl)
int test_index = 0; //test index in Lab motor test
bool start_motor_test = false;
int test_sequence = 0; 


//********************processIR control menu, when PS4 bluetooth is not connected****************
void processIR(){
    INA219();
    //**************LAB TEST CODE SELECTION***********
    //Lab2_Motor_Test(); //run lab test code
    Lab3_Motor_Bench_Test();
    Lab3_Motor_Floor_Test();
    Lab3_Encoder_Test();

    if (IrReceiver.decode())
    {
        IrReceiver.resume();
        IR_code = IrReceiver.decodedIRData.command;
        Serial.println(IR_code);

    if (IR_code == 12){
        start_motor_test = true;
        test_sequence = 1;//Bench test
        }
    if (IR_code == 24){
        start_motor_test = true;
        test_sequence = 2;//Floor test
        }
    if (IR_code == 94){
        start_motor_test = true;
        test_sequence = 3;//Encoder test
        }

    if(IR_code == 67){
        test_index = 0; //reset test index 
        Motor_stop();
        start_motor_test = false;
        }


    }

}//function bracket


//*******INA219 Current and Voltage monitor******
void INA219(){
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

    unsigned long current_time = millis(); 
  // 2. Make previous_time 'static' so it retains its value between loops
  static unsigned long previous_time = 0; 
  unsigned long measure_interval = 1000;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
if (current_time - previous_time >= measure_interval) {
    previous_time = current_time; 

  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");
  
  }
}//function bracket

//-----PS4 Controller Comms BluePad32 Code---------
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            controller_connected_state = true; //set state flag to true
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}
//-----PS4 Controller Comms BluePad32 Code---------
void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;
    
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            controller_connected_state = false;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
        controller_connected_state = false;
    }
}
//-----PS4 Controller Comms BluePad32 Code---------
void dumpGamepad(ControllerPtr ctl) {
    static unsigned long previous_time = 0; 
    const int interval = 1000;
    unsigned long current_time = millis();
  
  // ONLY read and log once per second to completely clear the CPU for smooth stepping
  if(current_time - previous_time >= interval){
    previous_time = current_time;

    Serial.printf(
        "idx=%d, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d\n",
        ctl->index(),        // Controller Index
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle()     // (0 - 1023): throttle (AKA gas) button
    );
}
}//function bracket

//-----Main process, Motion Control is nested here---------
void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a()) {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
            case 0:
                // Red
                ctl->setColorLED(255, 0, 0);
                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 0);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                break;
        }
        colorIdx++;
    }

    if (ctl->b()) {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x()) {
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
        // It is possible to set it by calling:
        // Some controllers have two motors: "strong motor", "weak motor".
        // It is possible to control them independently.
        ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                            0x40 /* strongMagnitude */);
    }

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.

    dumpGamepad(ctl);

}

//-----PS4 Controller Comms BluePad32 Code---------
void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {

                processGamepad(myController);
            } 
          
             else {
                Serial.println("Unsupported controller");
            }
        }
    }
}


//*******Motor stop*******
void Motor_stop(){
    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);
    digitalWrite(FR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, 0);
    analogWrite(RL_PWM_pin, 0);
    analogWrite(FR_PWM_pin, 0);
    analogWrite(RR_PWM_pin, 0);
}//function bracket

//***********************PS4-Motor Control Functions, nested in processGamepad(ControllerPtr ctl)*******************************************
void Motor_Control(ControllerPtr ctl) {
int basespeed = 70;
int PWM_output_left;
int PWM_output_right;
int x_val = ctl->axisX();
int y_val = ctl->axisY();
int throttle_value;
int steer_value;

if ((abs(x_val) < 40)&&(abs(y_val) < 40)){ //deadzone x axis
    steer_value = 0;
    throttle_value = 0;
    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(FR_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, 0);
    analogWrite(RL_PWM_pin, 0);
    analogWrite(FR_PWM_pin, 0);
    analogWrite(RR_PWM_pin, 0);

    }


if ((abs(x_val) <= 40)&&(y_val >= -508 && y_val <= -40)){ //Forward
    steer_value = 0;
    throttle_value = basespeed + map(ctl->throttle(), 0, 1020, 0, 255-basespeed); //255-basespeed = 195

    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(FR_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, throttle_value);
    analogWrite(RL_PWM_pin, throttle_value);
    analogWrite(FR_PWM_pin, throttle_value);
    analogWrite(RR_PWM_pin, throttle_value);

}


if ((abs(y_val) <= 40)&&(x_val >= -508 && x_val <= -40)){ //Piviot Left
    steer_value = map(abs(x_val), 0, 508, 0, 150);

    digitalWrite(FL_dir_pin, HIGH);
    digitalWrite(RL_dir_pin, HIGH);
    digitalWrite(FR_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, steer_value);
    analogWrite(RL_PWM_pin, steer_value);
    analogWrite(FR_PWM_pin, steer_value);
    analogWrite(RR_PWM_pin, steer_value);
    }

if ((abs(y_val) <= 40)&&(x_val >= 40 && x_val <= 512)){ //Piviot right
     steer_value = map(abs(x_val), 0, 508, 0, 150);

    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(FR_dir_pin, HIGH);
    digitalWrite(RR_dir_pin, HIGH);

    analogWrite(FL_PWM_pin, steer_value);
    analogWrite(RL_PWM_pin, steer_value);
    analogWrite(FR_PWM_pin, steer_value);
    analogWrite(RR_PWM_pin, steer_value);
    }

if ((x_val >= -508 && x_val <= -40)&&(y_val >= -508 && y_val <= -40)){ //Veer left
     steer_value = map(abs(x_val), 0, 508, 0, 180);
     throttle_value = basespeed + map(ctl->throttle(), 0, 1020, 0, 255-basespeed); 

    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(FR_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, constrain(throttle_value - steer_value, 0, 255)); //constrain needed
    analogWrite(RL_PWM_pin, constrain(throttle_value - steer_value, 0, 255));
    analogWrite(FR_PWM_pin, constrain(throttle_value + steer_value, 0, 255));
    analogWrite(RR_PWM_pin, constrain(throttle_value + steer_value, 0, 255));
}

if ((x_val >= 40 && x_val <= 512)&&(y_val >= -508 && y_val <= -40)){ //Veer right
    steer_value = map(abs(x_val), 0, 508, 0, 180);
    throttle_value = basespeed + map(ctl->throttle(), 0, 1020, 0, 255-basespeed); 

    digitalWrite(FL_dir_pin, LOW);
    digitalWrite(RL_dir_pin, LOW);
    digitalWrite(FR_dir_pin, LOW);
    digitalWrite(RR_dir_pin, LOW);

    analogWrite(FL_PWM_pin, constrain(throttle_value + steer_value, 0, 255)); //constrain needed
    analogWrite(RL_PWM_pin, constrain(throttle_value + steer_value, 0, 255));
    analogWrite(FR_PWM_pin, constrain(throttle_value - steer_value, 0, 255));
    analogWrite(RR_PWM_pin, constrain(throttle_value - steer_value, 0, 255));
}

}//function bracket

//**********************Stepper and Servo Motor controls for the POV cam********************
void Stepper_Motor_Xaxis_Control(ControllerPtr ctl){
  // Read joystick value continuously for instant responsiveness
 
    float x_val = ctl->axisRX();    // (-511 - 512) right Y axis

  if (x_val < -15) { // Joystick pushed UP (CW)
    // Map joystick range (480 down to 0) to speed range (0 up to maxSpeedLimit)
    float speed_val = map(abs(x_val), 0, 511, 10, maxSpeedLimit);
    myStepperX.setSpeed(-speed_val);
  }
  else if (x_val > 15) { // Joystick pushed DOWN (CCW)
    // Map joystick range (540 up to 1023) to negative speed range (0 down to -maxSpeedLimit)
    float speed_val = map(x_val, 0, 512, 10, maxSpeedLimit);
    myStepperX.setSpeed(speed_val); 
  }
  else {
    // DEADZONE: Joystick is centered. Stop the motor completely.
    myStepperX.setSpeed(0);
  }
}

void Stepper_Motor_Yaxis_Control(ControllerPtr ctl){
  // Read joystick value continuously for instant responsiveness
 
    float y_val = ctl->axisRY();    // (-511 - 512) right Y axis

  if (y_val < -15) { // Joystick pushed UP (CW)
    // Map joystick range (480 down to 0) to speed range (0 up to maxSpeedLimit)
    float speed_val = map(abs(y_val), 0, 511, 10, maxSpeedLimit);
    myStepperY.setSpeed(-speed_val);
  }
  else if (y_val > 15) { // Joystick pushed DOWN (CCW)
    // Map joystick range (540 up to 1023) to negative speed range (0 down to -maxSpeedLimit)
    float speed_val = map(y_val, 0, 512, 10, maxSpeedLimit);
    myStepperY.setSpeed(speed_val); 
  }
  else {
    // DEADZONE: Joystick is centered. Stop the motor completely.
    myStepperY.setSpeed(0);
  }
}

/*void Servo_Motor_Xaxis_Control(ControllerPtr ctl){
  // 1. Constantly read the joystick for real-time responsiveness
  float x_val = ctl->axisRX();    // Range: -511 to 512
  
  unsigned long current_time = millis();
  static unsigned long previous_servo_time = 0;
  const int servo_interval = 30;  // Pacing interval window
  
  static int current_pos = 90;    // Tracks the persistent position state cleanly

  // 2. Only calculate and move the position on a strict pacing window
  if (current_time - previous_servo_time >= servo_interval) {
    previous_servo_time = current_time;

    // FIX: Map starting from the edge of the deadzone (-25) down to full lock (-508)
    if (x_val < -25) { // Joystick pushed LEFT
      int step = map(x_val, -25, -508, 1, 4); 
      current_pos = current_pos + step; // Clean subtraction to pan left
    }
    // FIX: Map starting from the edge of the deadzone (25) up to full lock (512)
    else if (x_val > 25) { // Joystick pushed RIGHT
      int step = map(x_val, 25, 512, 1, 4); 
      current_pos = current_pos - step; // Clean addition to pan right
    }
    // Else: Joystick is in deadzone (-25 to 25). current_pos is unchanged.

    // 3. Keep the angle locked within safe structural limits AFTER math operations
    current_pos = constrain(current_pos, 0, 180);

    // 4. Update the servo hardware once per pacing frame
    myservo.write(current_pos);
  }
}*/





/*
void Lab2_Motor_Test(){

    int decrement_val = 25;
    unsigned long current_time = millis(); 
    static unsigned long previous_time = 0; 
    
    // Use 'step' to track exactly where we are in our timeline
    // 0 = CW Run, 1 = First Stop, 2 = CCW Run, 3 = Second Stop, 4 = Sequence Done
    static int step = 0; 
    int Output_percentage_val = 75;

   if (start_motor_test == false) {
        step = 0;
        previous_time = 0;
        return;
    }

    // End sequence reset parameters
    if (step >= 4) {
        Motor_stop();
        step = 0;            // Reset step tracking for next run
        previous_time = 0;   // Reset timer variable initialization
        start_motor_test = false; // Turn off flag automatically
        Serial.println("--- Lab 2 Motor Test Sequence Complete ---");
        return; 
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true){
    // Capture the absolute start time when the test begins
    if (previous_time == 0) {
        previous_time = current_time;
        Serial.println("--- Starting Lab 2 Motor Test Sequence ---");
    }
    // STEP 0: Run Clockwise (CW) for 5 Seconds
    if (step == 0) {
        digitalWrite(RR_dir_pin, LOW); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 5000) {
            previous_time = current_time; // Reset timer for next step
            step = 1;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 1) {
        analogWrite(RR_PWM_pin, 0);
        
        if (current_time - previous_time >= 2000) {
            previous_time = current_time; 
            step = 2; 
            Serial.println("Step 1 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 2) {
        digitalWrite(RR_dir_pin, HIGH); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        
        if (current_time - previous_time >= 5000) {
            previous_time = current_time; 
            step = 3; 
            Serial.println("Step 2 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 3) {
        analogWrite(RR_PWM_pin, 0);
        
        if (current_time - previous_time >= 2000) {
            step = 4; // Mark sequence as fully done
           test_index = 1; // Sync back to your global stop flag
            Serial.println("--- Lab 2 Motor Test Sequence Complete ---");
        } 
    }
    }//check motor test stage flag


}//function bracket
*/
//****************Lab test codes**************************
void Lab3_Motor_Bench_Test(){

    int decrement_val = 25;
    unsigned long current_time = millis(); 
    static unsigned long previous_time = 0; 
    
    // Use 'step' to track exactly where we are in our timeline
    // 0 = CW Run, 1 = First Stop, 2 = CCW Run, 3 = Second Stop, 4 = Sequence Done
    static int step = 0; 
    int Output_percentage_val = 70;

   if (start_motor_test == false) {
        step = 0;
        previous_time = 0;
        test_sequence = 0;
        return;
    }

    // End sequence reset parameters
    if (step > 27) {
        //Motor_stop();
        step = 0;            // Reset step tracking for next run
        previous_time = 0;   // Reset timer variable initialization
        unsigned long current_time = millis(); 
        //start_motor_test = false; // Turn off flag automatically
        Serial.println("--- Lab 2 Motor Test Sequence Complete ---");
        return; 
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 1){
    // Capture the absolute start time when the test begins
    if (previous_time == 0) {
        previous_time = current_time;
        Serial.println("--- Starting Lab 3 Motor Test Sequence ---");
    }
    // STEP 0: Run Clockwise (CW) for 5 Seconds
    if (step == 0) {
        digitalWrite(FL_dir_pin, LOW); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);

        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 2000) {
            previous_time = current_time; // Reset timer for next step
            step = 1;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 1) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 2; 
            Serial.println("Step 1 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 2) {
        digitalWrite(FL_dir_pin, HIGH); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
        
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; 
            step = 3; 
            Serial.println("Step 2 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 3) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            step = 4; // Mark sequence as fully done
        }
    }

    else if (step == 4) {
        digitalWrite(FR_dir_pin, LOW); 
        analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);

        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; // Reset timer for next step
            step = 5;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 5) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 6; 
            Serial.println("Step 5 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 6) {
        digitalWrite(FR_dir_pin, HIGH); 
        analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);
        
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; 
            step = 7; 
            Serial.println("Step 6 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 7) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            step = 8; // Mark sequence as fully done
        
        }
    }

     else if (step == 8) {
        digitalWrite(RL_dir_pin, LOW); 
        analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);

        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; // Reset timer for next step
            step = 9;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 9) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 10; 
            Serial.println("Step 9 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 10) {
        digitalWrite(RL_dir_pin, HIGH); 
        analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);
        
        if (current_time - previous_time >= 2000) {
            previous_time = current_time; 
            step = 11; 
            Serial.println("Step 10 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 11) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            step = 12; // Mark sequence as fully done
        }
    }

    else if (step == 12) {
        digitalWrite(RR_dir_pin, LOW); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);

        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; // Reset timer for next step
            step = 13;                     // Move to next step
            Serial.println("Step 12 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 13) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 14; 
        Serial.println("Step 13 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 14) {
        digitalWrite(RR_dir_pin, HIGH); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; 
            step = 15; 
            Serial.println("Step 14 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 15) {
        Motor_stop();
        
        if (current_time - previous_time >= 1000) {
            step = 16; // Mark sequence as fully done
        
        }
    }
     else if (step == 16) {
        digitalWrite(FL_dir_pin, LOW); 
        digitalWrite(FR_dir_pin, LOW); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
         analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 4000) {
            previous_time = current_time; // Reset timer for next step
            step = 17;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 17) {
       Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 18; 
            Serial.println("Step 17 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 18) {
        digitalWrite(FL_dir_pin, HIGH); 
        digitalWrite(FR_dir_pin, HIGH); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
        analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; 
            step = 19; 
            Serial.println("Step 18 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 19) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            step = 20; // Mark sequence as fully done
        }
    }

    else if (step == 20) {
        digitalWrite(RR_dir_pin, LOW); 
         digitalWrite(RL_dir_pin, LOW); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
    analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 4000) {
            previous_time = current_time; // Reset timer for next step
            step = 21;                     // Move to next step
            Serial.println("Step 20 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 21) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 22; 
        Serial.println("Step 13 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 22) {
        digitalWrite(RR_dir_pin, HIGH); 
        digitalWrite(RL_dir_pin, HIGH); 
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; 
            step = 23; 
            Serial.println("Step 22 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 23) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            step = 24; // Mark sequence as fully done
        
        }
    }

    else if (step == 24) {
        digitalWrite(FL_dir_pin, LOW); 
        digitalWrite(FR_dir_pin, LOW); 
        digitalWrite(RL_dir_pin, LOW); 
        digitalWrite(RR_dir_pin, LOW); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
         analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);
            analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);
         analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 4000) {
            previous_time = current_time; // Reset timer for next step
            step = 25;                     // Move to next step
            Serial.println("Step 0 Done: Stopping motor...");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 25) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 26; 
            Serial.println("Step 17 Done: Reversing to CCW...");
        }
    }

    // STEP 2: Run Counter-Clockwise (CCW) for 5 Seconds
    else if (step == 26) {
        digitalWrite(FL_dir_pin, HIGH); 
        digitalWrite(FR_dir_pin, HIGH); 
        digitalWrite(RL_dir_pin, HIGH); 
        digitalWrite(RR_dir_pin, HIGH); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
        analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100);
           analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100);
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100);
        if (current_time - previous_time >= 4000) {
            previous_time = current_time; 
            step = 27; 
            Serial.println("Step 18 Done: Stopping motor...");
        }
    }

    // STEP 3: Final Stop and End Sequence
    else if (step == 27) {
            Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 28; 
            Serial.println("--- Lab 3 Motor Test Sequence Complete ---"); 
    }  
    }
    }//check motor test stage flag

}//function bracket

void Lab3_Motor_Floor_Test(){
     int Output_percentage_val = 70;
    int travel_time = 3000;
    static unsigned long previous_time = 0;
    int left_offset = 3; //positive value will adjust veer to left, negative value veer to right

   if (start_motor_test == false) {
        previous_time = 0;
        Motor_stop();
        test_sequence = 0;
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 2){

        unsigned long current_time = millis();

        if (previous_time == 0){//true for exactly one cycle
            previous_time = current_time;
        } //previous_time take a snapshot of millis() value

        digitalWrite(FL_dir_pin, LOW); 
        digitalWrite(FR_dir_pin, LOW); 
        digitalWrite(RL_dir_pin, LOW); 
        digitalWrite(RR_dir_pin, LOW); 

        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100 - left_offset);//front left
        analogWrite(RL_PWM_pin, 255 * Output_percentage_val / 100 - left_offset);//read left

        analogWrite(FR_PWM_pin, 255 * Output_percentage_val / 100 + left_offset);//front right
        analogWrite(RR_PWM_pin, 255 * Output_percentage_val / 100 + left_offset);//rear right
        
        if (current_time - previous_time >= travel_time){
            Motor_stop();
            start_motor_test = false;
        }
        
        }

}//function bracket

void Lab3_Encoder_Test(){

    int Output_percentage_val = 70;

   if (start_motor_test == false) {
        test_sequence = 0;
        Motor_stop();
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 3){

    // STEP 0: Run Clockwise (CW) for 5 Seconds
        digitalWrite(FL_dir_pin, LOW); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
    }
}//function bracket



void setup() {
    Serial.begin(115200);
    IrReceiver.begin(IR_signal_pin);
    //ESP32PWM::allocateTimer(0);
	//ESP32PWM::allocateTimer(1);
	//ESP32PWM::allocateTimer(2);
	//ESP32PWM::allocateTimer(3);
    
    //myservo.setPeriodHertz(50);    // standard 50 hz servo
	//myservo.attach(servo_pin, 1000, 2000); 
    //myservo.write(90);

      myStepperX.setMaxSpeed(maxSpeedLimit);
      myStepperY.setMaxSpeed(maxSpeedLimit);
    myStepperX.setSpeed(0);
    myStepperY.setSpeed(0);
       
    
    //--------Bluepad32 Comms---------------------
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]); // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController); // "forgetBluetoothKeys()" should be called when the user performs // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example. // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect. // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys(); // Enables mouse / touchpad support for gamepads that support them. // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad // - Second one, which is a "virtual device", is a mouse. // By default, it is disabled.
    BP32.enableVirtualDevice(false);

    //--------- setup Motor output pins ------------
    pinMode(FL_dir_pin, OUTPUT); //
    pinMode(RL_dir_pin, OUTPUT);
    pinMode(RR_dir_pin, OUTPUT);
    pinMode(FR_dir_pin, OUTPUT);
    pinMode(Blue_LED_pin, OUTPUT);

    Motor_stop();
//*******INA219 COMMs******************
  if(! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();
}//void setup bracket
// Arduino loop function. Runs in CPU 1.
// Global tracking variable for the non-blocking car control timer

void loop() {
      // 4. STEPPER EXECUTION:
    // This MUST run absolutely every loop iteration without getting blocked!
    myStepperX.runSpeed(); 
    myStepperY.runSpeed(); 
    // 1. Constantly look for bluetooth updates (runs at maximum loop speed)
    bool dataUpdated = BP32.update();

    if (controller_connected_state == true) { 
        digitalWrite(Blue_LED_pin, HIGH); // Bluetooth connection indicator light

        // 2. NON-BLOCKING INTERVAL FOR CAR MOTORS:
        // Processes gamepads every 150ms instead of using a freezing delay()
    
            
            // Only drive the DC motors if we have valid controller updates
            if (dataUpdated) {
                processControllers(); 
            } else {
                Motor_stop();
            }

        // 3. CONTINUOUS STEPPER SPEED CALCULATION:
        // Must stay outside of the 150ms pacing timer and dataUpdated block!
        if (myControllers[0] != nullptr) {
            Motor_Control(myControllers[0]);
            Stepper_Motor_Yaxis_Control(myControllers[0]);
            Stepper_Motor_Xaxis_Control(myControllers[0]);
            
        }

    } else {
        digitalWrite(Blue_LED_pin, LOW);
        processIR(); // Switch to IR if no bluetooth connection
        myStepperX.setSpeed(0);
        myStepperY.setSpeed(0);
    }

    // Yield smoothly to the ESP32 background tasks
    vTaskDelay(2); 
}
