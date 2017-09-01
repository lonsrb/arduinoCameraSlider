const int xJoystickPin     = A3;
const int yJoystickPin     = A0;
const int joystickClickPin = 13;

const int redLedPin         = A5;//12;
const int encoderClickPin   = A4;//12;
const int encoderDtPinB     = A1;  //pin B in example
const int encoderSwPinA     = A2;  //pin A in example
int encoderPosCount = 0;
int encoderPinALast;
int encoderAVal;
bool encoderBCW;

const int xStepPin         =  2;
const int yStepPin         =  3;
const int zStepPin         =  4;
const int xDirPin          =  5;
const int yDirPin          =  6;
const int zDirPin          =  7;
const int enablePin        =  8;

int xSpeedDelay = 100; //300-4000 is valid
int ySpeedDelay = 100; //300-4000 is valid


int lastEncoderButtonState = LOW;
int encoderButtonState = 0;
unsigned long lastEncoderDebounceTime = 0;

int lastJoystickButtonState = LOW;
int joystickButtonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long lastButtonClickCheckTime = 0;

unsigned long debounceDelay = 50;


unsigned long lastXStepTime = 0;
unsigned long lastXStepLevel = LOW;
unsigned long lastYStepTime = 0;
unsigned long lastYStepLevel = LOW;

bool initialized = false;
bool isPlayingBack = false;
int playbackXSpeedDelay = 100;
int playbackYSpeedDelay = 100;
bool playbackXLastStepHigh = false;
bool playbackYLastStepHigh = false;
bool startPositionSet = false;
bool endPositionSet = false;
int recordedXSteps = 0;
int recordedYSteps = 0;
int playedBackXSteps = 0;
int playedBackYSteps = 0;

bool xPositiveDirection = true;
bool yPositiveDirection = true;
bool moveX = false;
bool moveY = false;

int maxXSpeed = 115;
int maxYSpeed = 0;

void setup() {
//  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(redLedPin, OUTPUT);
    pinMode(joystickClickPin, INPUT);
    pinMode(encoderClickPin, INPUT);
    
    pinMode(zStepPin,OUTPUT);
    pinMode(zDirPin,OUTPUT);
    pinMode(yStepPin,OUTPUT);
    pinMode(yDirPin,OUTPUT);
    
    pinMode (encoderSwPinA, INPUT);
    pinMode (encoderDtPinB, INPUT);
    encoderPinALast = digitalRead(encoderSwPinA);
    
    pinMode(enablePin,OUTPUT);
    digitalWrite(enablePin, LOW);
    Serial.begin(9600);
    digitalWrite(zDirPin,HIGH); // Enables the motor to move in a particular direction
}

void loop() {
//   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    
    if (isPlayingBack) {
        
        if (micros() - lastXStepTime >= playbackXSpeedDelay) //do a step cycle
        {
            if(playbackXLastStepHigh) {
                digitalWrite(zStepPin,LOW);
            } else {
                digitalWrite(zStepPin,HIGH);
                playedBackXSteps++;
            }
            playbackXLastStepHigh = !playbackXLastStepHigh;
            lastXStepTime = micros();
        }
        
        if (micros() - lastYStepTime >= playbackYSpeedDelay) //do a step cycle
        {
            if(playbackYLastStepHigh) {
                digitalWrite(yStepPin,LOW);
            } else {
                digitalWrite(yStepPin,HIGH);
                playedBackYSteps++;
            }
            playbackYLastStepHigh = !playbackYLastStepHigh;
            lastYStepTime = micros();
        }
        
        if(playedBackXSteps >= recordedXSteps && playedBackYSteps >= recordedYSteps ){
            isPlayingBack = false;
            startPositionSet = false;
            endPositionSet = false;
            playedBackXSteps = 0;
            recordedXSteps = 0;
            playedBackYSteps = 0;
            recordedYSteps = 0;
            lastXStepTime = 0;
            lastYStepTime = 0;
            Serial.println("finishedPlayback!");
        }
        
        return;
    }
    
    if (millis() - lastButtonClickCheckTime > 100 ) {
        checkJoyStickButton();
        checkEncoderButton();
        lastButtonClickCheckTime =  millis();
    }
    
    encoderAVal = digitalRead(encoderSwPinA);
    if (encoderAVal != encoderPinALast && encoderAVal == HIGH){ // Means the knob is rotating
        // if the knob is rotating, we need to determine direction
        // We do that by reading pin B.
        if (digitalRead(encoderDtPinB) != encoderAVal) {  // Means pin A Changed first - We're Rotating Clockwise
            encoderPosCount ++;
            encoderBCW = true;
        } else {// Otherwise B changed first and we're moving CCW
            encoderBCW = false;
            encoderPosCount--;
        }
        Serial.print ("Rotated: ");
        if (encoderBCW){
            Serial.println ("clockwise");
        }else{
            Serial.println("counterclockwise");
        }
        Serial.print("Encoder Position: ");
        Serial.println(encoderPosCount);
    }
    encoderPinALast = encoderAVal;
    
    if(startPositionSet && endPositionSet && !isPlayingBack) {
      return;
    }
    if (lastXStepTime == 0) {
        int rawXValue = analogRead(xJoystickPin);
        int xValue = map(rawXValue, 0, 1023, 0, 1000);
        
        moveX = !(xValue < 550 && xValue > 450);
        if (!moveX) {
            //do nothing
        }
        else {
            if (xValue >= 550) { //move left
                xSpeedDelay = map(xValue-550, 0, 450, 4000, maxXSpeed);
                digitalWrite(zDirPin,HIGH);
                xPositiveDirection = true;
            }
            else if (xValue <= 450){ //move right
                xSpeedDelay = map(450-xValue, 0, 450, 4000, maxXSpeed);
                digitalWrite(zDirPin,LOW);
                xPositiveDirection = false;
            }
        }
    }
    
    if (lastYStepTime == 0) {
        int rawYValue = analogRead(yJoystickPin);
        int yValue = map(rawYValue, 0, 1023, 0, 1000);
        
        moveY = !(yValue < 550 && yValue > 450);
        if (!moveY) {
            //do nothing
        }
        else {
            if (yValue >= 550) { //move left
                ySpeedDelay = map(yValue-550, 0, 450, 4000, maxYSpeed);
                digitalWrite(yDirPin,HIGH);
                yPositiveDirection = true;
            }
            else if (yValue <= 450){ //move right
                ySpeedDelay = map(450-yValue, 0, 450, 4000, maxYSpeed);
                digitalWrite(yDirPin,LOW);
                yPositiveDirection = false;
            }
        }
    }
    
    //--------------- do step if its time ----------------------
    if (moveY) {
        if (lastYStepTime == 0 || (micros() - lastYStepTime) > ySpeedDelay) { //then its time to take step
            
            digitalWrite(yStepPin, !lastYStepLevel);
            lastYStepTime = micros();
            
            if (lastYStepLevel == HIGH) {
                if(yPositiveDirection) {
                    recordedYSteps++;
                }
                else {
                    recordedYSteps--;
                }
                lastYStepTime = 0;//this should trigger a resample for speed
            }
            
            lastYStepLevel = !lastYStepLevel;
        }
    }
    
    if (moveX) {
        if (lastXStepTime == 0 || (micros() - lastXStepTime) > xSpeedDelay) { //then its time to take step
            
            digitalWrite(zStepPin, !lastXStepLevel);
            lastXStepTime = micros();
            
            if (lastXStepLevel == HIGH) { //finished the step resample speed
                if(xPositiveDirection) {
                    recordedXSteps++;
                }
                else {
                    recordedXSteps--;
                }
                lastXStepTime = 0; //this should trigger a resample for speed
            }
            
            lastXStepLevel = !lastXStepLevel;
        }
    }
}

void checkEncoderButton() {
    
    int encoderButtonReading = (analogRead(encoderClickPin) > 0) ? 1 : 0;
    if (encoderButtonReading != lastEncoderButtonState) {
        lastEncoderDebounceTime = millis();  // reset the debouncing timer
    }
    
    if ((millis() - lastEncoderDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:
        
        // if the button state has changed:
        if (encoderButtonReading != encoderButtonState) {
            encoderButtonState = encoderButtonReading;
            
            // only toggle the LED if the new button state is HIGH
            if (encoderButtonState == HIGH) {
                Serial.println("encoder button OFF");
            }
            else {
                Serial.println("encoder button ON");
            }
        }
    }
    lastEncoderButtonState = encoderButtonReading;
}

void checkJoyStickButton() {
    
    
    int joyStickButtonReading = digitalRead(joystickClickPin);
    
    if (joyStickButtonReading != lastJoystickButtonState) {
        lastDebounceTime = millis();  // reset the debouncing timer
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:
        
        // if the button state has changed:
        if (joyStickButtonReading != joystickButtonState) {
            joystickButtonState = joyStickButtonReading;
            
            // only toggle the LED if the new button state is HIGH
            if (joystickButtonState == HIGH) {
                Serial.println("button OFF");
                if (initialized == false) {
                    Serial.println("initialized");
                    initialized = true;
                }
                else if (startPositionSet == false) { //start recording
                    Serial.println("start recording");
                    digitalWrite(redLedPin, HIGH);
                    delay(250);
                    digitalWrite(redLedPin, LOW);    
                    recordedXSteps = 0;
                    recordedYSteps = 0;
                    startPositionSet = true;
                } else if (endPositionSet == false) { //set endpoint and playback to beginning
                    Serial.println("end recording");
                    endPositionSet = true;
                    
                    returnToStartPosition(recordedXSteps, recordedYSteps);
                } else { //if start and end are set start playblack
                    Serial.println("start playback");
                    endPositionSet = false;
                    startPositionSet = false;
                    playback(recordedXSteps, recordedYSteps);
                }
            }
            else {
                Serial.println("button ON");
            }
        }
    }
    lastJoystickButtonState = joyStickButtonReading;
}

void returnToStartPosition(int xStepsTaken, int yStepsTaken) {
    String recordedXStepsString = "recordedXSteps: ";
    String recordedYStepsString = "recordedYSteps: ";
    Serial.println(recordedXStepsString + xStepsTaken);
    Serial.println(recordedYStepsString + yStepsTaken);
    
    //do two for loops where we move x then y negative steps taken as fast as possible
    
    //    //---------------X---------------------
    //reverse for return
    if (xPositiveDirection) {
        digitalWrite(zDirPin, LOW);
    } else {
        digitalWrite(zDirPin, HIGH);
    }
    
    for (int i = 0; i < abs(xStepsTaken); i++) {
        takeStep(zStepPin, 150);
    }
    
    //set back to normal
    if (xPositiveDirection) {
        digitalWrite(zDirPin, HIGH);
    } else {
        digitalWrite(zDirPin, LOW);
    }
    
    //---------------Y---------------------
    //reverse for return
    if (yPositiveDirection) {
        digitalWrite(yDirPin, LOW);
    } else {
        digitalWrite(yDirPin, HIGH);
    }
    
    for (int i = 0; i < abs(yStepsTaken); i++) {
        takeStep(yStepPin, 50);
    }
    
    //set back to normal
    if (yPositiveDirection) {
        digitalWrite(yDirPin, HIGH);
    } else {
        digitalWrite(yDirPin, LOW);
    }
}

void playback(int xSteps, int ySteps) {
    Serial.println("playback");
    
    isPlayingBack = true;
    
    unsigned long playbackTotalMicros = getPlaybackMicrosecondsFromEncoder();
    Serial.println("playbackTotalMicros:");
    Serial.println(playbackTotalMicros);
    Serial.println("xSteps:");
    Serial.println(xSteps);
    Serial.println("ySteps:");
    Serial.println(ySteps);
    playbackXSpeedDelay = (playbackTotalMicros / xSteps) / 2;//divide by 2 again for high & low loop
    playbackYSpeedDelay = (playbackTotalMicros / ySteps) / 2;//divide by 2 again for high & low loop
    Serial.println("playbackXSpeedDelay:");
    Serial.println(playbackXSpeedDelay);
    Serial.println("playbackYSpeedDelay:");
    Serial.println(playbackYSpeedDelay);
}

void takeStep(int pin, int speedDelay) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(speedDelay);
    digitalWrite(pin, LOW);
    delayMicroseconds(speedDelay);
}


unsigned long getPlaybackMicrosecondsFromEncoder() {
    unsigned long returnValue = 7000;
    switch ( encoderPosCount ) {
        case 0: //7 seconds
            returnValue = 7 * 1000;
            break;
        case 1: //10 seconds
            returnValue = 10 * 1000;
            break;
        case 2: //15 seconds
            returnValue = 15 * 1000;
            break;
        case 3: //30 seconds
            returnValue = 30 * 1000;
            break;
        case 4: //60 seconds
            returnValue = 60 * 1000;
            break;
        case 5: //90 seconds
            returnValue = 90 * 1000;
            break;
        case 6: //2 mins
            returnValue = 2 * 60 * 1000;
            break;
        case 7: //2.5 mins
            returnValue = (90 + 60) * 1000;
            break;
        case 8: //3 mins
            returnValue = 3 * 60 * 1000;
            break;
        case 9: //4 mins
            returnValue = 4 * 60 * 1000;
            break;
        case 10: //5 mins
            returnValue = 5 * 60 * 1000;
            break;
        case 11: //7.5 mins
            returnValue = (30 + (7 * 60)) * 1000;
            break;
        case 12: //10 mins
            returnValue = 10 * 60 * 1000;
            break;
        case 13: //15 mins
            returnValue = 15 * 60 * 1000;
            break;
        case 14: //20 mins
            returnValue = 20 * 60 * 1000;
            break;
        case 15: //30 mins
            returnValue = 30 * 69 * 1000;
            break;
        case 16: //45 mins
            returnValue = 45 * 60 * 1000;
            break;
        case 17: //60 mins
            returnValue = 60 * 60 * 1000;
            break;
        case 18: //1hr 15 mins
            returnValue = (15 + 60) * 60 * 1000;
            break;
        case 19: //1hr 30 mins
            returnValue = (30 + 60) * 60 * 1000;
            break;
        default :  //7 seconds
            returnValue = 7 * 1000;
            break;
    }
    Serial.println("getPlaybackMicrosecondsFromEncoder in millis:");
    Serial.println(returnValue);
    return returnValue * 1000;
}

