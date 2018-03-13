/////////////////////////////////////////////////////////////////////////////////////////////
// CC-by-www.Electrosmash.com based on OpenMusicLabs previous works
// 
// pedalshield1284delay.ino reads the ADC, 
// saves it into the DelayBuffer array and plays it into the PWM output
//  
// Pressing the pushbutton_1 or 2 makes the delay longer or shorter
// Pressing the footswitch disables the effect
// 
// Compared to the Uno R3 with the same support circuit the ATMega1284 sounds very quiet
// This is also true for the Tremelo breathing noise that was very noticable on te Uno
// Much longer delays now possible with the 16k RAM
//
// Code and accompying schematics adapted by Tobias van Dyk January 2018
// 
/////////////////////////////////////////////////////////////////////////////////////////////

// defining harware resources.
#define LED PB0
#define FOOTSWITCH PB1
#define PUSHBUTTON_1 A5
#define PUSHBUTTON_2 A4

// defining the output PWM parameters
#define PWM_FREQ 0x00FF             // pwm frequency - 31.3KHz
#define PWM_MODE 0                  // Fast (1) or Phase Correct (0)
#define PWM_QTY 2                   // 2 PWMs in parallel

// other variables
int input, vol_variable=512;
int counter=0;
unsigned int ADC_low, ADC_high;

#define MAX_DELAY 12000        // With this only 72% of ATMega1284 RAM used
byte DelayBuffer[MAX_DELAY];
byte EffectEnable = 1;
unsigned int DelayCounter = 0;
unsigned int Delay_Depth = MAX_DELAY;

void setup() {
  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(PUSHBUTTON_1, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  // setup ADC
  ADMUX = 0x60;   // left adjust, adc0, internal vcc
  ADCSRA = 0xe5;  // turn on adc, ck/32, auto trigger
  ADCSRB = 0x07;  // t1 capture for trigger
  DIDR0 = 0x01;   // turn off digital inputs for adc0

  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //
  TCCR1B = ((PWM_MODE << 3) | 0x11);                        // ck/1
  TIMSK1 = 0x20;                                            // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRD |= 0x30;                                             // turn on PWM outputs PD4 and PD5
  sei();                                                    // turn on interrupts  
  }

/*

  // setup ADC to sample on ADC2
  ADMUX = 0x62; // left adjust, adc2, internal vcc as reference
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB =0x06; // timer1 overflow as trigger
  DIDR0 = 0x04; // turn off digital inputs for adc2

  // setup PWM to output at 31.25ksps
  TCCR1A = 0xa0; // set to compare mode to positive output
  TCCR1B = 0x11; // ck/1, phase and frequency correct mode, ICR1 as top
  TIMSK1 = 0x01; // turn on overflow interrupt
  ICR1H = 0x00; // clear temporary high byte register
  ICR1L = 0xff; // set top of counter
  DDRB |= 0x06; // set pwm outputs (pins 9,10) to output

}


*/



void loop() 
{
  // Turn on the LED if the effect is ON.
  if (!digitalRead(FOOTSWITCH)) 
     { digitalWrite(LED, HIGH); 
       EffectEnable = 1;
     } else { digitalWrite(LED, LOW);
              EffectEnable = 0; 
            }
                                
  // Everthing happens in the Timer 1 interruption
  
}

ISR(TIMER1_CAPT_vect) 
{
  // get ADC data
  ADC_low = 0; // ADC_low always 0 to save space
  ADC_high = ADCH;
  
  //store the high bit only for 
  DelayBuffer[DelayCounter] = ADC_high;
 
counter++; //to save resources, the pushbuttons are checked every 100 times.
if(counter==100)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_1)) 
   { if (Delay_Depth<MAX_DELAY)Delay_Depth=Delay_Depth+1; //increase the depth
   }

if (!digitalRead(PUSHBUTTON_2)) 
   { if (Delay_Depth>0)Delay_Depth=Delay_Depth-1; //decrease depth
   }
}

//Increase/reset delay counter.   
  DelayCounter++;
  if(DelayCounter >= Delay_Depth) DelayCounter = 0; 

  input = (((DelayBuffer[DelayCounter] << 8) | ADC_low) + 0x8000)+(((ADC_high << 8) | ADC_low) + 0x8000); // make a signed 16b value
  
  //write the PWM signal
  if (EffectEnable==1)
  {
  OCR1AL = ((input + 0x8000) >> 8);       // convert to unsigned, send out high byte
  OCR1BL = input; // send out low byte    // PortD for the ATMega1284
  }

}
