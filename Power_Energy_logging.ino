////////////////////////////////////////////////////////////
/* Code written by Gaurav Duggal 06-09-2018               */
//Reference: Atmega 328 datasheet and Arduino Reference page
////////////////////////////////////////////////////////////

#define led1 2
#define led2 3
#define led3 4
#define sig A0
#define resistor 330
#define time_step 1/62.5
volatile float v_buffer[64] = {0};
volatile float i_buffer[64] = {0};
volatile unsigned int buf_fill_ctr=0;
volatile unsigned int buf_read_ctr=0;
volatile byte flag = 0;
volatile float power[64] = {0};
volatile float energy = 0;

void setup() {

Serial.begin(115200);
pinMode(led1,OUTPUT);
pinMode(led2,OUTPUT);
pinMode(led3,OUTPUT);
//checking all leds
led_on(1);
delay(1000);
led_on(2);
delay(1000);
led_on(3);
delay(1000);
//temporarily disable all interrupts
cli();
//initialise timer1
timer_init();
//enable all configured interrupts
sei();
}

void loop() 
{
  if (flag==1)
  {
    while (buf_read_ctr!=buf_fill_ctr)
    {
      //increment buffer
      buf_read_ctr++;
      //ensure buffer counter doesn't cross 63
      buf_read_ctr&=63;
      //convert to voltage 
      v_buffer[buf_read_ctr] = 5*(v_buffer[buf_read_ctr]/1023);
      //measure current through resistor
      //i_buffer[buf_read_ctr] = (5 - v_buffer[buf_read_ctr])/resistor;
      //power = v*i
      power[buf_read_ctr] = v_buffer[buf_read_ctr]*v_buffer[buf_read_ctr]/resistor;
      //energy
      energy = energy + power[buf_read_ctr]*time_step;
      Serial.print("$");
      Serial.print(power[buf_read_ctr],3);
      Serial.print(',');
      Serial.println(energy,2);
      //psedo random turning led on
      //led_on(buf_read_ctr%10+1);
      flag = 0;
      
    }
  }
}
//shitty code to switch on appropriate led
void led_on(int i)
{
  //switch off all leds
  digitalWrite(led1,HIGH);
  digitalWrite(led2,HIGH);
  digitalWrite(led3,HIGH);
  switch(i)
  {
    case 1:
    digitalWrite(led1,LOW);
    break;
    case 2:
    digitalWrite(led2,LOW);
    break;
    case 3:
    digitalWrite(led3,LOW);
    break;
  }
}

void timer_init()
{
  //please refer to datasheet to understand why these registers were set to these values
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 62.5hz increments
  // = (16*10^6) / (OCR2A*1024) - 1 is the frequency the ISR is called
  //set for 62.5 Hz
  OCR2A = 249;
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 1024 prescaler
  TCCR2B |= (1 << CS22) | (1 << CS21)| (1 << CS20)  ;   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}


ISR(TIMER2_COMPA_vect){
//flag to check if there is a fresh value in the buffer  
flag = 1;
//read value into buffer
v_buffer[buf_fill_ctr] = analogRead(A0);
//increment fill buffer counter
buf_fill_ctr++;
//ensure buffer counter doesn't cross 63
buf_fill_ctr=buf_fill_ctr & 63;

}
