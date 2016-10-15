/* Melody
 * (cleft) 2005 D. Cuartielles for K3
 *
 * This example uses a piezo speaker to play melodies.  It sends
 * a square wave of the appropriate frequency to the piezo, generating
 * the corresponding tone.
 *
 * The calculation of the tones is made following the mathematical
 * operation:
 *
 *       timeHigh = period / 2 = 1 / (2 * toneFrequency)
 *
 * where the different tones are described as in the table:
 *
 * note  frequency  period  timeHigh
 * c          261 Hz          3830  1915
 * d          294 Hz          3400  1700
 * e          329 Hz          3038  1519
 * f          349 Hz          2864  1432
 * g          392 Hz          2550  1275
 * a          440 Hz          2272  1136
 * b          493 Hz          2028 1014
 * C         523 Hz         1912  956
 *
 * http://www.arduino.cc/en/Tutorial/Melody
 */

#include <RFduinoBLE.h>

int speakerPin = 5;                  // Grove Buzzer connect to GPIO2
// pin 2 on the RGB shield is the red led
int RedLedPin = 2;
// pin 3 on the RGB shield is the green led
int BlueLedPin = 4;
// pin 4 on the RGB shield is the blue led
int GreenLedPin = 3;

float distance;
boolean songPlayed;
float distThresh = 5;
//////////Moving average


float average = 1.0;  // must be outside value range (-0dBm to -127dBm)

// number of samples in the average
int samples = 20;

// sample #1 from environment (rssi -45dBm = distance 1.5ft)
float r1 = -45;
float d1 = 1.5;

// sample #2 from environment (rssi -75dBm = distance 20ft)
float r2 = -75;
float d2 = 50;

// constant to account for loss due to reflection, polzarization, etc
// n will be ~2 for an open space
// n will be ~3 for "typical" environment
// n will be ~4 for a cluttered industrial site
float n = (r2 - r1) / (10 * log10(d1 / d2));


//////////////








//Code for Twinkle Twinkle Little Star:
int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;
void playSongBlocking(){
  for (int i = 0; i < length; i++) 
    {
        if (notes[i] == ' ')
        {
            delay(beats[i] * tempo); // rest
        }
        else
        {
            playNote(notes[i], beats[i] * tempo);
        }

        // pause between notes
        delay(tempo / 2);
    }
}



void playTone(int tone, int duration) {
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(speakerPin, HIGH);
        delayMicroseconds(tone);
        digitalWrite(speakerPin, LOW);
        delayMicroseconds(tone);
    }
}

void playNote(char note, int duration) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

    // play the tone corresponding to the note name
    for (int i = 0; i < 8; i++) {
        if (names[i] == note) {
            playTone(tones[i], duration);
        }
    }
}

void setup()
{
  //Speaker setup
   pinMode(speakerPin, OUTPUT);
  // setup the leds for output
  pinMode(RedLedPin, OUTPUT);
  pinMode(BlueLedPin, OUTPUT);  
  pinMode(GreenLedPin, OUTPUT);

//show system is powered and ready
  analogWrite(RedLedPin, 0);
  analogWrite(BlueLedPin, 0);
  analogWrite(GreenLedPin, 255);

  //BLE setup
  // drop the transmission power to move the zones closer to
  // the RFduino (which also reduces the interference from
  // other wireless devices and reflection)
  // (all zones are within approxiately 10 feet of the RFduino)
  RFduinoBLE.txPowerLevel = -20;
  RFduinoBLE.begin();

  //Debugging serial port
  Serial.begin(9600);
  

//Setup moving average filter:  
  // dump the value of n in case your curious
  Serial.println(n);

    
}




void loop() 
{
    // switch to lower power mode
    RFduino_ULPDelay(INFINITE);

}

void RFduinoBLE_onConnect()
{
 analogWrite(RedLedPin, 0);
  analogWrite(BlueLedPin, 255);
  analogWrite(GreenLedPin, 0);
}


void RFduinoBLE_onDisconnect()
{
  // don't play song if disconnecting. 
  analogWrite(RedLedPin, 0);
  analogWrite(BlueLedPin, 0);
  analogWrite(GreenLedPin, 100);
 
}







// returns the dBm signal strength indicated by the receiver
// received signal strength indication (-0dBm to -127dBm)
void RFduinoBLE_onRSSI(int rssi)
{
  // initialize average with instaneous value to counteract the initial smooth rise
  if (average == 1.0)
    average = rssi;

  // single pole low pass recursive IIR filter
  average = rssi * (1.0 / samples) + average * (samples - 1.0) / samples;

  // approximate distance  
  distance = d1 * pow(10, (r1 - average) / (10 * n));

  // intensity (based on distance[linear] not rssi[logarithmic])
  float d = distance;
  if (d < d1)
    d = d1;
  else if (d > d2)
    d = d2;
  int intensity = (d2 - d) / (d2 - d1) * 255;

  doSongLogic();
  
  Serial.print((int)average);
  Serial.print("\t");
  Serial.print((int)distance);
  Serial.print("\t");
  Serial.print(intensity);
  Serial.println("");

}

void doSongLogic(void){
  if (distance >= distThresh && songPlayed == 0){
       Serial.println("PLAYING SONG!!!!");
      songPlayed = 1;
      playSongBlocking();
      analogWrite(RedLedPin, 128);
      analogWrite(BlueLedPin, 0);
      analogWrite(GreenLedPin, 128);
    }

    if (distance < distThresh){
       Serial.println("resetting SONG!!!!");
       songPlayed = 0; //Memory into switch
    }
}


