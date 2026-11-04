//I will approach Lab4 in three methods, vTaskdelay, gate latching through state flags, and gated millis(). 

#include <IRremote.hpp>
#include <Wire.h>
#include <Adafruit_INA219.h>


Adafruit_INA219 ina219;


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



static unsigned long lastSensorUpdateTime;
const unsigned long SensorUpdateInterval = 2000; 

int IR_code = 0; //IR remote signal code received
int test_index = 0; //test index in Lab motor test
bool start_motor_test = false;
bool motor_start_toggle = false;
int test_sequence = 0; 

int last_speed[4] = {0, 0, 0, 0};
bool last_dir[4]   = {0, 0, 0, 0};


//********************processIR control menu, when PS4 bluetooth is not connected****************


//*******INA219 Current and Voltage monitor******
void INA219(){
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

    //unsigned long current_time = millis(); 
  // 2. Make previous_time 'static' so it retains its value between loops
  static unsigned long previous_time = 0; 
  unsigned long measure_interval = 1000;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
//if (current_time - previous_time >= measure_interval) {
   // previous_time = current_time; 

  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");
  
  //}
}//function bracket


//*******Motor stop*******
void Motor_stop(){

        write_Motor(0, LOW, 0, 0);
        write_Motor(1, LOW, 0, 0);
        write_Motor(2, LOW, 0, 0);
        write_Motor(3, LOW, 0, 0);
    return;
    
}//function bracket

// 1. Global Arrays to remember states for each motor (0 to 3)


// 2. The Parameterized Gated Function Blueprint
void write_Motor(int motor_num, bool dir, int pwm_val, int veer_val) {
int write_speed = constrain(pwm_val + veer_val, 0, 255); 
bool write_dir = dir;

    // THE GATE LATCH
    // Compare current parameters against global history arrays using motor_num
        if(write_speed == last_speed[motor_num] && write_dir == last_dir[motor_num]) {
        return; // BLOCK! speed and dir haven't changed. Exit immediately.
    }

     if (write_dir != last_dir[motor_num]) { //best to pause motor briefly and reverse dir
       
        last_dir[motor_num] = write_dir;
       switch(motor_num) {
        case 0: // Front Left
            digitalWrite(FL_dir_pin, write_dir);
            break;
        case 1: // Rear Left
            digitalWrite(RL_dir_pin, write_dir);
            break;
        case 2: // Rear right
            digitalWrite(RR_dir_pin, write_dir);
            break;
        case 3: // front right
            digitalWrite(FR_dir_pin, write_dir);
            break;
            }//switch case
     }//if

        if (write_speed != last_speed[motor_num]){
            last_speed[motor_num] = write_speed;

        switch(motor_num) {
        case 0: // Front Left
            analogWrite(FL_PWM_pin, write_speed);
            break;
        case 1: // Rear Left
            analogWrite(RL_PWM_pin, write_speed);
            break;
        case 2: // Rear right
            analogWrite(RR_PWM_pin, write_speed);
            break;
        case 3: // front right
            analogWrite(FR_PWM_pin, write_speed);
            break;
            }//switch case
        }//if
    
    // Step E: UPDATE HISTORY
    // Save current states to memory arrays for the next loop execution
    

}//function bracket


void Lab4_Motor_Forward_Reverse(){
    unsigned long current_time= millis();
    static unsigned long previous_time = 0; 
    static int step = 0; 

   if (start_motor_test == false) {
        step = 0;
        previous_time = 0;
        test_sequence = 0;
        Motor_stop();
        return;//important!
        //return; //exit function
    }

    // End sequence reset parameters
    if (step >= 4) {
        //step = 0;   this caused problems in sequence execution due to delayed loop processing         
        previous_time = 0;   // Reset timer variable initialization
        //unsigned long current_time = millis(); //???
        start_motor_test = false; // Turn off flag automatically
        Serial.println("--- Lab 2 Motor Test Sequence Complete ---"); //print once
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 1){
    // Capture the absolute start time when the test begins
    if (previous_time == 0) {
        previous_time = current_time; //snapshot previous time
        Serial.println("--- Starting Lab 4 Forward Reverse Test ---");
    }
    // STEP 0: Run Clockwise (CW) for 5 Seconds
    if (step == 0) {
        write_Motor(0, LOW, 128, 0);
        write_Motor(1, LOW, 128, 0);
        write_Motor(2, LOW, 128, 0);
        write_Motor(3, LOW, 128, 0);
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; // Reset timer for next step
            step = 1;                     // Move to next step
            Serial.println("Step 0 Done");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 1) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 2; 
            Serial.println("Step 1 Done");
        }
    }

    else if (step == 2) {
        write_Motor(0, HIGH, 178, 0);
        write_Motor(1, HIGH, 178, 0);
        write_Motor(2, HIGH, 178, 0);
        write_Motor(3, HIGH, 178, 0);
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
            Serial.println("Step 3 Done, Test Complete");
        }
     }
     

    }//if start_motor_test == true

}//function bracket


void Lab4_Motor_Veer_test(){
    unsigned long current_time= millis();
    static unsigned long previous_time = 0; 
    static int step = 0; 

   if (start_motor_test == false) {
        step = 0;
        previous_time = 0;
        test_sequence = 0;
        Motor_stop();
        return;//important!
        //return; //exit function
    }

    // End sequence reset parameters
    if (step >= 4) {
        //step = 0;   this caused problems in sequence execution due to delayed loop processing         
        previous_time = 0;   // Reset timer variable initialization
        //unsigned long current_time = millis(); //???
        start_motor_test = false; // Turn off flag automatically
        Serial.println("--- Veer Test complete---"); //print once
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 2){
    // Capture the absolute start time when the test begins
    if (previous_time == 0) {
        previous_time = current_time; //snapshot previous time
        Serial.println("--- Starting Veer Test ---");
    }
    // STEP 0: Run Clockwise (CW) for 5 Seconds
    if (step == 0) { //Veer right
        write_Motor(0, LOW, 128, 100);
        write_Motor(1, LOW, 128, 100);
        write_Motor(2, LOW, 128, 0);
        write_Motor(3, LOW, 128, 0);
        // Every 1 second or so, you could print a status, but let's check the step transition:
        if (current_time - previous_time >= 3000) {
            previous_time = current_time; // Reset timer for next step
            step = 1;                     // Move to next step
            Serial.println("Step 0 Done");
        }
    }

    // STEP 1: Stop for 2 Seconds
    else if (step == 1) {
        Motor_stop();
        if (current_time - previous_time >= 1000) {
            previous_time = current_time; 
            step = 2; 
            Serial.println("Step 1 Done");
        }
    }

    else if (step == 2) {
        write_Motor(0, LOW, 128, 0);
        write_Motor(1, LOW, 128, 0);
        write_Motor(2, LOW, 128, 100);
        write_Motor(3, LOW, 128, 100);
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
            Serial.println("Step 3 Done, Test Complete");
        }
     }
     

    }//if start_motor_test == true

}//function bracket

/*void Lab3_Encoder_Test(){

    int Output_percentage_val = 70;

   if (start_motor_test == false) {
        test_sequence = 0;
        motor_stop_toggle = true;();
    }

    // Initialize the baseline time stamp upon your very first loop step execution

    if (start_motor_test == true && test_sequence == 3){

    // STEP 0: Run Clockwise (CW) for 5 Seconds
        digitalWrite(FL_dir_pin, LOW); 
        analogWrite(FL_PWM_pin, 255 * Output_percentage_val / 100);
    }
}//function bracket
*/
/*
Void Lab4_Test(){

• Forward and reverse: 
@PWM 178 70%
1.Forward 3s,
2. stop 1s, 
3. Backwards 3s.
4. stop. 

• Left and right 90 degree turn test

• veer 

• Coast to a stop.
• Fast Stop.

}
*/

void processIR(){

    //**************LAB TEST CODE SELECTION***********
    //Lab2_Motor_Test(); //run lab test code
    //Lab3_Motor_Bench_Test();
    //Lab3_Motor_Floor_Test();
    //Lab3_Encoder_Test();

    if (IrReceiver.decode()){
        IrReceiver.resume();
        IR_code = IrReceiver.decodedIRData.command;
        Serial.println(IR_code);

    if (IR_code == 12){
        start_motor_test = true;
        test_sequence = 1;//Forward Reverse test
        }
    if (IR_code == 24){
        start_motor_test = true;
        test_sequence = 2;//Veer test
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
     //this else block seems like a good idea for safety but it will trigger TB6612 unexpectly in the mainloop 


}//function bracket

void setup() {
    Serial.begin(115200);
    IrReceiver.begin(IR_signal_pin);

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

  }  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

}//void setup bracket
// Arduino loop function. Runs in CPU 1.
// Global tracking variable for the non-blocking car control timer

void loop() {

      unsigned long currentTime = millis();
            if (currentTime - lastSensorUpdateTime >= SensorUpdateInterval) {
            lastSensorUpdateTime = currentTime;
            INA219();
            }
            
        processIR(); // Switch to IR if no bluetooth connection
        Lab4_Motor_Forward_Reverse();
        Lab4_Motor_Veer_test();

    // Yield to background tasks
    //vTaskDelay(2); 

}
