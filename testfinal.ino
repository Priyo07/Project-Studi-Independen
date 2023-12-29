#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>

LiquidCrystal_I2C lcd (0x27, 20, 4);
//--------------------------------------------------------------------------------------keypad program
const int row       = 4;
const int column    = 3;

byte rowpin[4]      = {14, 15, 16, 17};
byte columnpin[3]   = {8, 9, 10};

char mapping[row][column] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

Keypad keypad4x3 = Keypad(makeKeymap(mapping), rowpin, columnpin, row, column);
//---------------------------------------------------------------------------------------keypad program

//--------------------------------------------------- variabel declaration
enum stage {
  mainmenu,
  modechoice,
  injectionflowrate,
  calibrationvol,
  calibrationstepper1,
  calibrationstepper2,
  volumeinput, 
  pumpaction};
  
stage injectionstage;

char sign;
String modeaction;
double volumevalueinput;
double injectionvolstep;
double injectiontime; 
double injectedvolume;                                     
double flowrate;                //flowrate is related to step/s of stepper
double flowratestep_sec;
double flowraterpm;
double pumpcapablestep;
double revpumpcapablestep;
double fluidvolume;  
double calibrationvolstep;
volatile bool emergencycondition = false;

/*  IMPORTANT SPECIFICATION

    1. stepper using 200 step per revolution
    2. 1 step = 0.0025 ml of fluid thus 1 step per second = 9 ml/h
    3. maximum speed of stepper in PROTEUS is 8 rpm
       8 rpm is 240 ml/h

    MATH Equation for this code

    ml    -> step = (input volume in ml / 0.0025 ml)
    ml/h  -> rpm  = (input volume in ml / 9 ml) * 60 / 200
 */

Stepper stepper(200, 7, 6, 5, 4);

//-------------------------------------------------- variabel declaration


//-------------------------------------------------- function declaration
void mainmenu_func(){
  lcd.setCursor(0,0); lcd.print("Main menu :");
  lcd.setCursor(0,1); lcd.print("1.injection");
  lcd.setCursor(0,2); lcd.print("2.inj flowrate");
  lcd.setCursor(0,3); lcd.print("3.vol calibration");

  sign = keypad4x3.getKey();

  if(sign == '1'){
    lcd.clear();
    injectionstage = modechoice;
    }

  if(sign == '2'){
    lcd.clear();
    injectionstage = injectionflowrate;
    }
    
  if(sign == '3'){
    lcd.clear();
    injectionstage = calibrationvol;
  }
}

void modechoice_func(){
  lcd.setCursor(0,0); lcd.print("choose action mode");
  lcd.setCursor(0,2); lcd.print("* = pumping");
  lcd.setCursor(0,3); lcd.print("# = reverse");

  sign = keypad4x3.getKey();

  if(sign == '*'){
    lcd.clear();
    modeaction = "pump";
    //kurang program kalibrasi stepper terhadap last postition
    injectionstage = volumeinput;
  }
  if(sign == '#'){
    lcd.clear();
    modeaction = "reverse";
    //kurang program kalibrasi stepper terhadap last position
    injectionstage = volumeinput;
  }

  if(digitalRead(12) == LOW){
    lcd.clear();
    injectionstage = mainmenu;
  }
}

void volumeinput_func(){
  lcd.setCursor(0,0); lcd.print("volume to be");
  lcd.setCursor(0,1); lcd.print("infused : ");
  lcd.setCursor(0,3); lcd.print(volumevalueinput);
  lcd.setCursor(16,3); lcd.print("ml");

  sign = keypad4x3.getKey();
  
  if(sign == '1'){
    if(volumevalueinput == 0){
      volumevalueinput = 1;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 1;
      }
    }

  if(sign == '2'){
    if(volumevalueinput == 0){
      volumevalueinput = 2;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 2;
      }
    }
    
  if(sign == '3'){
    if(volumevalueinput == 0){
      volumevalueinput = 3;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 3;
      }
    }

  if(sign == '4'){
    if(volumevalueinput == 0){
      volumevalueinput = 4;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 4;
      }
    }

  if(sign == '5'){
    if(volumevalueinput == 0){
      volumevalueinput = 5;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 5;
      }
    }

  if(sign == '6'){
    if(volumevalueinput == 0){
      volumevalueinput = 6;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 6;
      }
    }

  if(sign == '7'){
    if(volumevalueinput == 0){
      volumevalueinput = 7;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 7;
      }
    }

  if(sign == '8'){
    if(volumevalueinput == 0){
      volumevalueinput = 8;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 8;
      }
    }

  if(sign == '9'){
    if(volumevalueinput == 0){
      volumevalueinput = 9;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 9;
      }
    }

  if(sign == '0'){
    if(volumevalueinput == 0){
      volumevalueinput = 0;
      }
    else{
      volumevalueinput = (volumevalueinput * 10) + 0;
      }
    }

  if(sign == '*'){
    lcd.clear();
    if(volumevalueinput == 0){
      volumevalueinput = 0;
      }
    else{
      volumevalueinput = floor((volumevalueinput / 10));
      }
    }

  if(sign == '#'){
    if(volumevalueinput == 0){
      volumevalueinput = 0;
      }
    else{
      volumevalueinput = volumevalueinput;
      }
      
    injectionvolstep = volumevalueinput / 0.0025;
    injectiontime = injectionvolstep / flowratestep_sec;
    
    injectionstage = pumpaction;
    lcd.clear();

    lcd.setCursor(0,0); lcd.print("Estimated time :"); 
    lcd.setCursor(0,1); lcd.print(injectiontime);
    lcd.setCursor(10,1); lcd.print("second");
    lcd.setCursor(0,2); lcd.print("transfered volume :");
    lcd.setCursor(10,3); lcd.print("ml");
    }

  if(digitalRead(12) == LOW){
    lcd.clear();
    injectionstage = modechoice;
    delay(250);
  }
  }

void pump_func(){
  if(emergencycondition == false){
    if(pumpcapablestep > injectionvolstep){
      stepper.step(1);
      --pumpcapablestep;
      ++revpumpcapablestep;
      --injectionvolstep;
      injectedvolume = injectedvolume + 0.0025;
      lcd.setCursor(0,3); lcd.print(injectedvolume); 
  
      if(injectionvolstep == 0){
        injectionvolstep = 0;
        injectedvolume = 0;

        lcd.clear();
        lcd.setCursor(4,1); lcd.print("injection is");
        lcd.setCursor(7,2); lcd.print("done !");
        for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
        lcd.clear();

        injectionstage = mainmenu;
        }
      } 
    
    else{
      lcd.clear();
      lcd.setCursor(5,0); lcd.print("cant reach");
      lcd.setCursor(4,1); lcd.print("minimum step");
      lcd.setCursor(7,2); lcd.print("please");
      lcd.setCursor(5,3); lcd.print("calibrate!");
      delay(2000);
      lcd.clear();

      injectionstage = mainmenu; 
      }
  }

  else{
    lcd.clear();
    lcd.setCursor(6,0); lcd.print("Warning!");
    lcd.setCursor(4,2); lcd.print("injection is");
    lcd.setCursor(5,3); lcd.print("canceled !");
    for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
    lcd.clear();

    injectionstage = mainmenu;
    injectedvolume = 0;
    emergencycondition = !emergencycondition;
  }
}

void reversepump_func(){
  if(emergencycondition == false){
    if(revpumpcapablestep > injectionvolstep){
      stepper.step(-1);
      ++pumpcapablestep;
      --revpumpcapablestep;
      --injectionvolstep;
      injectedvolume = injectedvolume + 0.0025;
      lcd.setCursor(0,3); lcd.print(injectedvolume); 
  
      if(injectionvolstep == 0){
        injectionvolstep = 0;
        injectedvolume = 0;

        lcd.clear();
        lcd.setCursor(4,1); lcd.print("injection is");
        lcd.setCursor(7,2); lcd.print("done !");
        for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
        lcd.clear();

        injectionstage = mainmenu;
        }
    }
    else{
      lcd.clear();
      lcd.setCursor(5,0); lcd.print("cant reach");
      lcd.setCursor(4,1); lcd.print("minimum step");
      lcd.setCursor(7,2); lcd.print("please");
      lcd.setCursor(5,3); lcd.print("calibrate!");
      delay(2000);
      lcd.clear();

      injectionstage = mainmenu; 
      }
  }

  else{
    lcd.clear();
    lcd.setCursor(6,0); lcd.print("Warning!");
    lcd.setCursor(4,2); lcd.print("injection is");
    lcd.setCursor(5,3); lcd.print("canceled !");
    for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
    lcd.clear();

    injectionstage = mainmenu;
    injectedvolume = 0;
    emergencycondition = !emergencycondition;
  }
}

void flowrate_func(){
  lcd.setCursor(0,0); lcd.print("injection");
  lcd.setCursor(0,1); lcd.print("flowrate : ");
  lcd.setCursor(0,3); lcd.print(flowrate);
  lcd.setCursor(8,3); lcd.print("ml/h");

  sign = keypad4x3.getKey();
  
  if(sign == '1'){
    if(flowrate == 0){
      flowrate = 1;
      }
    else{
      flowrate = (flowrate * 10) + 1;
      }
    }

  if(sign == '2'){
    if(flowrate == 0){
      flowrate = 2;
      }
    else{
      flowrate = (flowrate * 10) + 2;
      }
    }
    
  if(sign == '3'){
    if(flowrate == 0){
      flowrate = 3;
      }
    else{
      flowrate = (flowrate * 10) + 3;
      }
    }

  if(sign == '4'){
    if(flowrate == 0){
      flowrate = 4;
      }
    else{
      flowrate = (flowrate * 10) + 4;
      }
    }

  if(sign == '5'){
    if(flowrate == 0){
      flowrate = 5;
      }
    else{
      flowrate = (flowrate * 10) + 5;
      }
    }

  if(sign == '6'){
    if(flowrate == 0){
      flowrate = 6;
      }
    else{
      flowrate = (flowrate * 10) + 6;
      }
    }

  if(sign == '7'){
    if(flowrate == 0){
      flowrate = 7;
      }
    else{
      flowrate = (flowrate * 10) + 7;
      }
    }

  if(sign == '8'){
    if(flowrate == 0){
      flowrate = 8;
      }
    else{
      flowrate = (flowrate * 10) + 8;
      }
    }

  if(sign == '9'){
    if(flowrate == 0){
      flowrate = 9;
      }
    else{
      flowrate = (flowrate * 10) + 9;
      }
    }

  if(sign == '0'){
    if(flowrate == 0){
      flowrate = 0;
      }
    else{
      flowrate = (flowrate * 10) + 0;
      }
    }

  if(sign == '*'){
    lcd.clear();
    if(flowrate == 0){
      flowrate = 0;
      }
    else{
      flowrate = floor((flowrate / 10));
      }
    }

  if(sign == '#'){
    if(flowrate == 0){
      flowrate = 0;
      }
    else{
      flowrate = flowrate;
      }

  flowratestep_sec = flowrate / 9;
  flowraterpm = (flowratestep_sec / 200) * 60;
  stepper.setSpeed(flowraterpm);
  
  injectionstage = mainmenu;
  lcd.clear();
  }

  if(digitalRead(12) == LOW){
    lcd.clear();
    injectionstage = mainmenu;
  }

}

void calibrationvol_func(){
  lcd.setCursor(0,0); lcd.print("input fluid");
  lcd.setCursor(0,1); lcd.print("volume : ");
  lcd.setCursor(0,3); lcd.print(fluidvolume);
  lcd.setCursor(16,3); lcd.print("ml");

  sign = keypad4x3.getKey();

  if(sign == '1'){
    if(fluidvolume == 0){
      fluidvolume = 1;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 1;
      }
    }

  if(sign == '2'){
    if(fluidvolume == 0){
      fluidvolume = 2;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 2;
      }
    }
    
  if(sign == '3'){
    if(fluidvolume == 0){
      fluidvolume = 3;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 3;
      }
    }

  if(sign == '4'){
    if(fluidvolume == 0){
      fluidvolume = 4;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 4;
      }
    }

  if(sign == '5'){
    if(fluidvolume == 0){
      fluidvolume = 5;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 5;
      }
    }

  if(sign == '6'){
    if(fluidvolume == 0){
      fluidvolume = 6;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 6;
      }
    }

  if(sign == '7'){
    if(fluidvolume == 0){
      fluidvolume = 7;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 7;
      }
    }

  if(sign == '8'){
    if(fluidvolume == 0){
      fluidvolume = 8;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 8;
      }
    }

  if(sign == '9'){
    if(fluidvolume == 0){
      fluidvolume = 9;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 9;
      }
    }

  if(sign == '0'){
    if(fluidvolume == 0){
      fluidvolume = 0;
      }
    else{
      fluidvolume = (fluidvolume * 10) + 0;
      }
    }

  if(sign == '*'){
    lcd.clear();
    if(fluidvolume == 0){
      fluidvolume = 0;
      }
    else{
      fluidvolume = floor((fluidvolume / 10));
      }
    }

  if(sign == '#'){
    if(fluidvolume == 0){
      fluidvolume = 0;
      }
    else{
      fluidvolume = fluidvolume;
      }

    flowrate = 240;
    flowratestep_sec = flowrate / 9;
    flowraterpm = (flowratestep_sec / 200) * 60;
    stepper.setSpeed(flowraterpm);

    calibrationvolstep = (10 - fluidvolume) / 0.0025;
    injectionstage = calibrationstepper1;
    lcd.clear();

    lcd.setCursor(3,0); lcd.print("calibration is");
    lcd.setCursor(5,1); lcd.print("on process");
    lcd.setCursor(4,3); lcd.print("please wait!");
  }

  if(digitalRead(12) == LOW){
    lcd.clear();
    injectionstage = mainmenu; 
  }
}

void calibrationstepper1_func(){
  if(emergencycondition == false){
    if(digitalRead(13) == LOW){
      pumpcapablestep = 4000.01;
      revpumpcapablestep = 0.01;
      injectionstage = calibrationstepper2;
    }

    else{
      stepper.step(-1);
    }
  }

  else{
    lcd.clear();
    lcd.setCursor(6,0); lcd.print("Warning!");
    lcd.setCursor(3,2); lcd.print("calibration is");
    lcd.setCursor(5,3); lcd.print("canceled !");
    for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
    lcd.clear();
    
    injectionstage = mainmenu;
    emergencycondition = !emergencycondition;
  }
}

void calibrationstepper2_func(){
  if(emergencycondition == false){
    if(calibrationvolstep == 0){
      calibrationvolstep == 0;

      lcd.clear();
      lcd.setCursor(3,1); lcd.print("calibration is");
      lcd.setCursor(7,2); lcd.print("done !");
      for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
      lcd.clear();

      injectionstage = mainmenu;
    }

    else if(calibrationvolstep > 0){
      stepper.step(1);
      --calibrationvolstep;
      --pumpcapablestep;
      ++revpumpcapablestep;
    }
  }

  else{
    lcd.clear();
    lcd.setCursor(6,0); lcd.print("Warning!");
    lcd.setCursor(3,2); lcd.print("calibration is");
    lcd.setCursor(5,3); lcd.print("canceled !");
    for(int note = 3 ; note > 0 ; note--){
          digitalWrite(11, HIGH);
          delay(500);
          digitalWrite(11, LOW);
          delay(500);
        }
    lcd.clear();

    injectionstage = mainmenu;
    emergencycondition = !emergencycondition;
  }
}

void ISR_emergency(){
  emergencycondition = !emergencycondition;
}

//-------------------------------------------------- function declaration

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,1); lcd.print("injection pump");
  lcd.setCursor(6,2); lcd.print("machine!");
  delay(1000);
  lcd.clear();
  
  pinMode(13, INPUT_PULLUP); // limit switch max
  pinMode(12, INPUT_PULLUP); // back switch
  pinMode(11, OUTPUT); //buzzer output
  pinMode(3, INPUT_PULLUP); // emergency button with IST
  
  attachInterrupt(digitalPinToInterrupt(3),ISR_emergency,FALLING);
  
  flowrate = 30;
  flowratestep_sec = flowrate / 9;
  flowraterpm = 1;
  stepper.setSpeed(flowraterpm);

  injectionstage = mainmenu; 
}

void loop() {
  switch(injectionstage){
    case mainmenu :
      mainmenu_func();
    break; 
    case modechoice :
      modechoice_func();
    break;
    case injectionflowrate :
      flowrate_func();
    break;
    case pumpaction :
      if (modeaction == "pump"){
        pump_func();
        }
      if (modeaction == "reverse"){
        reversepump_func();
        }
    break;
    case volumeinput :
      volumeinput_func();      
    break;  
    case calibrationvol :
      calibrationvol_func();
    break;
    case calibrationstepper1 :
      calibrationstepper1_func();
    break;
    case calibrationstepper2 :
      calibrationstepper2_func();
    break;
  }
}
