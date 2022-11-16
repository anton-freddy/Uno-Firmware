
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 4);

//  PIN ASSIGNEMENTS    ////////////////////////////////////////////////////////////////////////////////////////////////////

//  SENSORS:

/*  EXTRA SENSOR 1  */  #define H1_PIN 2
/*  EXTRA SENSOR 2  */  #define H2_PIN 3
/*  EXTRA SENSOR 3  */  #define H3_PIN 4
/*  EXTRA SENSOR 4  */  #define H4_PIN 5
/*  X-AXIS MAX      */  #define X_MAX_PIN 6
/*  X-AXIS HOME     */  #define X_HOME_PIN 7
/*  R-AXIS RPM      */  #define RPM_PIN 8


//  MOTORS:

/*  SERVO           */  #define SERVO_PIN 9
/*  Servo Object    */  Servo C_SERVO;
/*  R-AXIS STEPPER  */  #define R_DIR_PIN 10
/*  R-AXIS STEPPER  */  #define R_STEP_PIN 11  
/*  X-AXIS STEPPER  */  #define X_DIR_PIN 12
/*  X-AXIS STEPPER  */  #define X_STEP_PIN 13


//  SCREEN:

/*  BUTTON 1        */  #define BUTTON_BACK_PIN A0
/*  BUTTON 2        */  #define BUTTON_DEC_PIN A1
/*  BUTTON 3        */  #define BUTTON_INC_PIN A2
/*  BUTTON 4        */  #define BUTTON_OK_PIN A3
/*  SCREEN SDA      */  #define SDA_PIN A4
/*  SCREEN SCL      */  #define SCL_PIN A5



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  DIRECTIONS & SPEED ////////////////////////////////////////////////////////////////////////////////////////////////////

/*  R-AXIS FORWARD  */  #define RF HIGH
/*  R-AXIS BACKWARD */  #define RB LOW
/*  X-AXIS POSITIVE */  #define XP LOW
/*  X-AXIS NEGATIVE */  #define XN HIGH

/*  X-AXIS HOME SPD */  #define XHS 500
/*  R-AXIS HOME SPD */  #define RHS 4000

/*  LIMIT ACTIVE    */  #define LA LOW
/*  LIMIT INACTIVE  */  #define LN HIGH
/*  RPM ACTIVE      */  #define RA LOW
/*  RPM INACTIVE    */  #define RN HIGH

/*  RAMP inital speed*/ #define ramp_int_speed 1000
/*  RAMP ACCELERATION*/ #define ramp_acc 0.2


/*  BOBBIN TOLERANCE */ #define XTmin 0.80
/*  BOBBIN TOLERANCE */ #define XTmax 11.20         
/*  MAX BOBBIN NUM   */ #define max_bobbin_num 10   // Sets maximum number of bobbins machine can handle



//  GLOBAL VARIABLES    ///////////////////////////////////////////////////////////////////////////////////////////////////
volatile int rotNum = 0; 
volatile int speed = 0;
volatile int bobbinNum = 0;
volatile int cur_bobbinNum = 0;
volatile int x_dir = 0;
volatile double x_pos;          // Stores Position of X axis in mm
volatile int r_pos = 0;         // Stores Position of rotational axis in number of rotaions
volatile int c_pos = 0;         // Stores Position of cutting servo in degrees (0-180)
volatile int ramp_delay = 0;
volatile int target_del = 0;    // Traget Delay to dictate running speed of rotation axis
bool break_menu;


unsigned long start_time;
unsigned long elapsed_time;
unsigned long end_time;

#define X_SPEED_DEFAULT 1000    //  delay for stepping X
#define R_SPEED_DEFAULT 1000    //  delay for stepping R





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//  Function Definitions    ///////////////////////////////////////////////////////////////////////////////////////////////

void home_x(void);
void run_task(int r, int s, int b);     // r -> num of rotations, s -> speed, b -> num of bobbins
void move_x(double d, int s, int dir);  // move X axis by specfied direction, speed & distance
void go_to_x(double x, int s);          // go to a specified X pos
void home_R(void);                      // Homes the rotational axis
void home_R_no_disp(void);              // Homes the rotation axis (no LCD output)
void set_task();                        // Runs menus for setting up a task
void set_bobbin_num();                  // Sub menu for setting number of bobbins
void set_speed();                       // Sub-menu for setting machine speed
void set_rotation_num();                // Sub-menu for setting number of coil turns
void wait_confirmation();               // Waits for user confirmation before continuing
void wait_for_setup();                  // Waits for user to confirm that wire is connected before starting task
void task_end_display();                // LCD displays that task is complete and shows time taken
void make_cut();                        // Activates the cutting mechanisim to cut the wire






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//  Machine Launch initialization function  ///////////////////////////////////////////////////////////////////////////////
void setup(){

    Serial.begin(115200);
    lcd.init();
    lcd.backlight();


//  PIN MODES:
    pinMode(X_DIR_PIN, OUTPUT);
    pinMode(X_STEP_PIN, OUTPUT);
    pinMode(R_DIR_PIN, OUTPUT);
    pinMode(R_STEP_PIN, OUTPUT);
    pinMode(X_HOME_PIN, INPUT);
    pinMode(X_MAX_PIN, INPUT);
    pinMode(RPM_PIN, INPUT);
    pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DEC_PIN, INPUT_PULLUP);
    pinMode(BUTTON_INC_PIN, INPUT_PULLUP);
    pinMode(BUTTON_OK_PIN, INPUT_PULLUP);
    C_SERVO.attach(9);
    C_SERVO.write(200);
    digitalWrite(R_DIR_PIN, 0);
    digitalWrite(R_STEP_PIN, 0);
    digitalWrite(X_DIR_PIN, 0);
    digitalWrite(X_STEP_PIN, 0);

    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Coils");
    lcd.setCursor(6, 1);
    lcd.print("PRO");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("by Will, Farhana");
    lcd.setCursor(2,1);
    lcd.print("Tiaan & Anton");
    delay(3000);
    lcd.clear();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//  Main Program Loop  ////////////////////////////////////////////////////////////////////////////////////////////////////


void loop(){

    set_task();
    run_task(rotNum, speed, bobbinNum);
    delay(200);
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//  Program Functions  ////////////////////////////////////////////////////////////////////////////////////////////////////

void run_task(int r, int s, int b){
    home_R();
    delay(500);
    home_x();
    delay(500);
    cur_bobbinNum = 0;
    go_to_x(0.50, X_SPEED_DEFAULT);
    x_pos = 0;
    wait_for_setup();
    start_time = millis();
    while(cur_bobbinNum < b){
        
        cur_bobbinNum = cur_bobbinNum + 1;
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Current Bobbin");
        lcd.setCursor(1,1);
        lcd.print("in progress: ");
        lcd.print(cur_bobbinNum);


        switch(s){
            default:
                target_del = 1000;
                break;
            case 50:
                target_del = 800;
                break;
            case 75:
                target_del = 600;
                break;

            case 100:
                target_del = 400;
                break;

            case 125:
                target_del = 300;
                break;

        }

        
        int carriage_act;   // Determines whether to move carriage
        float del = ramp_int_speed;          // Delay to dtermine stepper speed during ramp up
        int x_dir;
        while(r_pos < r){
            //status_display();
            
            if(del > target_del){del = del - ramp_acc;}
            if(digitalRead(RPM_PIN) == RA){
                r_pos = r_pos + 1;
                carriage_act = 5;
            }
            if(x_pos >= XTmax){digitalWrite(X_DIR_PIN, XN); x_dir = 0;}
            if(x_pos <= XTmin){digitalWrite(X_DIR_PIN, XP); x_dir = 1;}
            digitalWrite(R_STEP_PIN, HIGH);
            if(carriage_act > 0){digitalWrite(X_STEP_PIN, HIGH);
                if(x_dir == 0){x_pos = x_pos - 0.05;}
                if(x_dir == 1){x_pos = x_pos + 0.05;}
                carriage_act = carriage_act - 1;
            } 
            delayMicroseconds(del);
            digitalWrite(X_STEP_PIN, LOW);
            digitalWrite(R_STEP_PIN, LOW);
            delayMicroseconds(del);
        }
        
        home_R_no_disp();

        delay(2000);
        if(cur_bobbinNum != b){
        go_to_x(42.00, X_SPEED_DEFAULT);
        for(int i = 0; i <16; i++){
            digitalWrite(R_DIR_PIN, RF);
            delayMicroseconds(8000);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(8000);
            digitalWrite(R_STEP_PIN, LOW);
            digitalWrite(R_DIR_PIN, RF);
        }
        delay(500);
        go_to_x((28.10), X_SPEED_DEFAULT);
        x_pos = 0;
        delay(1000);
        }

    }
    go_to_x(-28.00, X_SPEED_DEFAULT);
    delay(100);
    while (digitalRead(RPM_PIN) != RA)
    {
        digitalWrite(R_DIR_PIN, RF);
        delayMicroseconds(8000);
        digitalWrite(R_STEP_PIN, HIGH);
        delayMicroseconds(8000);
        digitalWrite(R_STEP_PIN, LOW);
        digitalWrite(R_DIR_PIN, RF);
    }
    for(int i = 0; i <2; i++){
            digitalWrite(R_DIR_PIN, RB);
            delayMicroseconds(8000);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(8000);
            digitalWrite(R_STEP_PIN, LOW);
            digitalWrite(R_DIR_PIN, RB);
        }
    go_to_x(18.00,X_SPEED_DEFAULT);
    make_cut();
    x_pos = 0;

    for(int C = bobbinNum - 2; C > 0; C--){
        go_to_x(-28.00,X_SPEED_DEFAULT);
        make_cut();
        x_pos=0;
    }

    end_time = millis();
    home_x();
    digitalWrite(R_STEP_PIN, LOW);
    digitalWrite(X_STEP_PIN, LOW);
    task_end_display();

    return;

}

void home_x(){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Please Wait...");
    lcd.setCursor(0,1);
    lcd.print("X-Axis Homing...");
    digitalWrite(X_DIR_PIN, XN);
    while(digitalRead(X_HOME_PIN) == LN){
        delayMicroseconds(XHS);
        digitalWrite(X_STEP_PIN, HIGH);
        delayMicroseconds(XHS);
        digitalWrite(X_STEP_PIN, LOW);
    }
    digitalWrite(X_DIR_PIN, XP);
    for(int i = 0; i < 50; i++){
        delayMicroseconds(XHS*2);
        digitalWrite(X_STEP_PIN, HIGH);
        delayMicroseconds(XHS*2);
        digitalWrite(X_STEP_PIN, LOW);
    }
    digitalWrite(X_DIR_PIN, XN);
    while(digitalRead(X_HOME_PIN) == LN){
        delayMicroseconds(XHS*2);
        digitalWrite(X_STEP_PIN, HIGH);
        delayMicroseconds(XHS*2);
        digitalWrite(X_STEP_PIN, LOW);
    }
    x_pos = 0.00;
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("X-Axis Homed");
    lcd.setCursor(2,1);
    lcd.print("X-POS: ");
    lcd.print(x_pos);
    delay(500);
}

void move_x(double d, int s){
    int STEPS;
    if(d > 0.00){
        digitalWrite(X_DIR_PIN, XP);
        STEPS = d/0.05;
        }
    else{
        digitalWrite(X_DIR_PIN, XN);
        STEPS = d/-0.05;
    }

    for(int i = 0; i < STEPS; i++){
        digitalWrite(X_STEP_PIN, HIGH);
        delayMicroseconds(s);
        digitalWrite(X_STEP_PIN, LOW);
        delayMicroseconds(s);
        if(d > 0.00){ x_pos = x_pos+ 0.05;}
        else if(d < 0.00){ x_pos = x_pos - 0.05;}

    }

}


void go_to_x(double x, int s){
    double distance = x - x_pos; // Distance to move
    move_x(distance, s);
}


void home_R(void){
    digitalWrite(R_STEP_PIN, LOW);
    if(digitalRead(RPM_PIN) == RA){
        digitalWrite(R_DIR_PIN, RB);
        for(int i = 0; i < 40; i++){
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, LOW);
        }
    }

    digitalWrite(R_DIR_PIN, RF);
    while(digitalRead(RPM_PIN) == RN){
        delayMicroseconds(RHS);
        digitalWrite(R_STEP_PIN, HIGH);
        delayMicroseconds(RHS);
        digitalWrite(R_STEP_PIN, LOW);
        
    }
    digitalWrite(R_DIR_PIN, RB);
        for(int i = 0; i < 20; i++){
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, LOW);
        }
    digitalWrite(R_DIR_PIN, RF);
    while(digitalRead(RPM_PIN) == RN){
        delayMicroseconds(RHS*2);
        digitalWrite(R_STEP_PIN, HIGH);
        delayMicroseconds(RHS*2);
        digitalWrite(R_STEP_PIN, LOW);
    }
     digitalWrite(R_STEP_PIN, LOW);

    r_pos = 0;
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("R-Axis Homed");
    lcd.setCursor(3,1);
    lcd.print("R-POS: ");
    lcd.print(r_pos);
    delay(500);
}

void home_R_no_disp(void){
    digitalWrite(R_STEP_PIN, LOW);
    if(digitalRead(RPM_PIN) == RA){
        digitalWrite(R_DIR_PIN, RB);
        for(int i = 0; i < 40; i++){
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, LOW);
        }
    }

    digitalWrite(R_DIR_PIN, RF);
    while(digitalRead(RPM_PIN) == RN){
        delayMicroseconds(RHS);
        digitalWrite(R_STEP_PIN, HIGH);
        delayMicroseconds(RHS);
        digitalWrite(R_STEP_PIN, LOW);
        
    }
    digitalWrite(R_DIR_PIN, RB);
        for(int i = 0; i < 20; i++){
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, HIGH);
            delayMicroseconds(RHS);
            digitalWrite(R_STEP_PIN, LOW);
        }
    digitalWrite(R_DIR_PIN, RF);
    while(digitalRead(RPM_PIN) == RN){
        delayMicroseconds(RHS*2);
        digitalWrite(R_STEP_PIN, HIGH);
        delayMicroseconds(RHS*2);
        digitalWrite(R_STEP_PIN, LOW);
    }
     digitalWrite(R_STEP_PIN, LOW);
    r_pos = 0;
}

void set_task(){
    break_menu = false;
    bobbin_menu:
    set_bobbin_num();
    if(break_menu == true){
        break_menu = false;
        goto bobbin_menu;
    }

    speed_menu:
    set_speed();
    if(break_menu == true){
        break_menu = false;
        goto bobbin_menu;
    }

    turns_menu:
    set_rotation_num();
    if(break_menu == true){
        break_menu = false;
        goto speed_menu;
    }

    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Task Set");
    lcd.setCursor(2,1);
    lcd.print("Successfully");
    delay(1000);
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Please Wait...");
    delay(500);
}

void set_bobbin_num(){
    int temp_bobbinNum = 1;
    int ok_state = HIGH;
    break_menu = false;
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Set Bobbins: ");
    lcd.print(temp_bobbinNum);
    delay(10);
    while(ok_state == HIGH){
        ok_state = digitalRead(BUTTON_OK_PIN);
        break_menu = false;
        if(digitalRead(BUTTON_DEC_PIN) == LOW){
            if(temp_bobbinNum > 1){
                temp_bobbinNum = temp_bobbinNum - 1;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Bobbins: ");
                lcd.print(temp_bobbinNum);
                delay(150);
            }
        }
        if(digitalRead(BUTTON_INC_PIN) == LOW){
            if(temp_bobbinNum < max_bobbin_num){
                temp_bobbinNum = temp_bobbinNum + 1;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Bobbins: ");
                lcd.print(temp_bobbinNum);
                delay(150);
            }
        }
    
        if(digitalRead(BUTTON_BACK_PIN) == LOW){
            break_menu = true;
            ok_state = LOW;
        }
    }
    if(break_menu == false){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Set Bobbins: ");
        lcd.print(temp_bobbinNum);
        lcd.setCursor(2,1);
        lcd.print("Value Stored!");
        bobbinNum = temp_bobbinNum;
        delay(1000);
        lcd.clear();
    }

}
void set_speed(){
    int temp_speed = 100;
    int ok_state = HIGH;
    break_menu = false;
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Set Speed: ");
    lcd.print(temp_speed);
    lcd.print("%");
    delay(10);
    while(ok_state == HIGH){
        ok_state = digitalRead(BUTTON_OK_PIN);
        break_menu = false;
        if(digitalRead(BUTTON_DEC_PIN) == LOW){
            if(temp_speed > 50){
                temp_speed = temp_speed - 25;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Speed: ");
                lcd.print(temp_speed);
                lcd.print("%");
                delay(150);
            }
        }

        if(digitalRead(BUTTON_INC_PIN) == LOW){
            if(temp_speed < 125){
                temp_speed = temp_speed + 25;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Speed: ");
                lcd.print(temp_speed);
                lcd.print("%");
                delay(150);
            }
        }
        if(digitalRead(BUTTON_BACK_PIN) == LOW){
            break_menu = true;
            ok_state = LOW;
        }
    }

    if(break_menu == false){
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("Speed: ");
        lcd.print(temp_speed);
        lcd.print("%");
        lcd.setCursor(2,1);
        lcd.print("Value Stored!");
        speed = temp_speed;
        delay(1000);
        lcd.clear();
    }
}
void set_rotation_num(){
    int temp_rotNum = 200;
    int ok_state = HIGH;
    break_menu = false;
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Set Turns: ");
    lcd.print(temp_rotNum);
    delay(10);
    while(ok_state == HIGH){
        ok_state = digitalRead(BUTTON_OK_PIN);
        break_menu = false;
        if(digitalRead(BUTTON_DEC_PIN) == LOW){
            if(temp_rotNum > 200){
                temp_rotNum = temp_rotNum - 10;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Turns: ");
                lcd.print(temp_rotNum);
                delay(150);
            }
        }

        if(digitalRead(BUTTON_INC_PIN) == LOW){
            if(temp_rotNum < 700){
                temp_rotNum = temp_rotNum + 10;
                lcd.clear();
                lcd.setCursor(1,0);
                lcd.print("Set Turns: ");
                lcd.print(temp_rotNum);
                delay(150);
            }
        }

        if(digitalRead(BUTTON_BACK_PIN) == LOW){
            break_menu = true;
            ok_state = LOW;

        }
    }

    if(break_menu == false){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Coil Turns: ");
        lcd.print(temp_rotNum);
        lcd.setCursor(2,1);
        lcd.print("Value Stored!");
        rotNum = temp_rotNum;
        delay(1000);
    }


}

void wait_for_setup(){
    int ok_state = HIGH;
    while(ok_state == HIGH){
        if(digitalRead(BUTTON_OK_PIN) == LOW){ok_state = LOW;}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Machine is Ready");
        lcd.setCursor(2,1);
        lcd.print("Connect Wire");
        for(int DELAY = 0; DELAY < 2000; DELAY = DELAY + 1){
            if(digitalRead(BUTTON_OK_PIN) == LOW){
                ok_state = LOW;
                DELAY = 2000;
                }
            delay(1);
        }
        if(digitalRead(BUTTON_OK_PIN) == LOW){ok_state = LOW;}
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("Press OK to");
        lcd.setCursor(4,1);
        lcd.print("continue");
        for(int DELAY = 0; DELAY < 2000; DELAY = DELAY + 1){
            if(digitalRead(BUTTON_OK_PIN) == LOW){
                ok_state = LOW;
                DELAY = 2000;
                }
            delay(1);
        }

    }
}

void wait_confirmation(){
    while(digitalRead(BUTTON_OK_PIN) == HIGH){
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("Press OK to");
        lcd.setCursor(4,1);
        lcd.print("continue");
        delay(100);
    }

}


void task_end_display(){
    unsigned long seconds = (end_time-start_time)/1000;
    unsigned long minutes = seconds/60;
    //end_time %= 1000;
    seconds %= 60;
    minutes %= 60;
    int ok_state = HIGH;
    while(ok_state == HIGH){
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Task Finished");
        lcd.setCursor(0,1);
        lcd.print("Time Taken ");
        lcd.setCursor(12,1);
        lcd.print(minutes);
        lcd.print(":");
        lcd.print(seconds);
        for(int DELAY = 0; DELAY < 2000; DELAY = DELAY + 1){
            if(digitalRead(BUTTON_OK_PIN) == LOW){
                ok_state = LOW;
                DELAY = 2000;
                }
            delay(1);
        }
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("Press OK to");
        lcd.setCursor(4,1);
        lcd.print("continue");
        for(int DELAY = 0; DELAY < 2000; DELAY = DELAY + 1){
            if(digitalRead(BUTTON_OK_PIN) == LOW){
                ok_state = LOW;
                DELAY = 2000;
                }
            delay(1);
        }
    }
}

void make_cut(){
    delay(300);
    C_SERVO.write(0);
    delay(1500);
    C_SERVO.write(200);
    delay(200);
}


// X Y Z