#include<TinyGPS.h>
int x=0,y=0,z=0,xl=0,yl=0,zl=0,k=0;
int rate;
int s;
volatile int heart_rate;          

volatile int analog_data;              

volatile int time_between_beats = 600;            

volatile boolean pulse_signal = false;    

volatile int beat[10];         //heartbeat values will be sotred in this array    

volatile int peak_value = 512;          

volatile int trough_value = 512;        

volatile int thresh = 525;              

volatile int amplitude = 100;                 

volatile boolean first_heartpulse = true;      

volatile boolean second_heartpulse = false;    

volatile unsigned long samplecounter = 0;   //This counter will tell us the pulse timing

volatile unsigned long lastBeatTime = 0;


TinyGPS gps;
void LocMsg(int r);
void LocMsg(int r)
{
 float lat,lon;
 while(Serial1.available())
 {
  if(gps.encode(Serial1.read()))
  { 
   gps.f_get_position(&lat,&lon);
  }
 }
 String latitude = String(lat,6);
 String longitude = String(lon,6);
 Serial.println("AT+CMGF=1");
 delay(500);
 Serial.println("AT+CMGS=\"+919961166366\"\r");
 delay(500);
 Serial.println(latitude+";"+longitude);
 delay(500);
 Serial.println(heart_rate);
 delay(500);
 Serial.println((char)26);
 delay(500);
}
void setup()
{
 Serial.begin(9600);
 Serial1.begin(9600);
 pinMode(11,OUTPUT);
 pinMode(12,OUTPUT);
 pinMode(A0,INPUT);
 pinMode(A1,INPUT);
 pinMode(A2,INPUT);
  pinMode(A3,INPUT);//pulse sensor 
  pinMode(A4,INPUT);//piezo 
 pinMode(8,INPUT);
 digitalWrite(11,0);
 digitalWrite(12,0);
 interruptSetup();    
 delay(12000);
 Serial.println("ATE0");
 Serial.println("AT+CMGF=1");
 Serial.println("AT+CNMI=2,2,0,0,0");
 delay(1000);
}

void loop()
{
  s=analogRead(A4);
  Serial.println(s);
 digitalWrite(11,0);
 digitalWrite(12,0);
 x=analogRead(A0);
 y=analogRead(A1);
 z=analogRead(A2);
 Serial.print("X");
 Serial.print(x); //x-axis
 Serial.print("Y");
 Serial.print(y); //y-axis
 Serial.print("Z");
 Serial.print(z); //z-axis
 Serial.println();
 if(abs(x-xl)>=5 || abs(y-yl)>=5 || abs(z-zl)>=5)
 {
  if(k!=0 && s>100)
  {
   LocMsg(heart_rate);
   digitalWrite(12,1);
   digitalWrite(11,0);
   delay(2000);
  }
  xl=x;
  yl=y;
  zl=z;
  k=1;
 }
 delay(2000);  
}
void interruptSetup()

{    

  TCCR2A = 0x02;  // This will disable the PWM on pin 3 and 11

  OCR2A = 0X7C;   // This will set the top of count to 124 for the 500Hz sample rate

  TCCR2B = 0x06;  // DON'T FORCE COMPARE, 256 PRESCALER

  TIMSK2 = 0x02;  // This will enable interrupt on match between OCR2A and Timer

  sei();          // This will make sure that the global interrupts are enable

}


ISR(TIMER2_COMPA_vect)

{ 

  cli();                                     

  analog_data = analogRead(A3);            

  samplecounter += 2;                        

  int N = samplecounter - lastBeatTime;      


  if(analog_data < thresh && N > (time_between_beats/5)*3)

    {     

      if (analog_data < trough_value)

      {                       

        trough_value = analog_data;

      }

    }


  if(analog_data > thresh && analog_data > peak_value)

    {        

      peak_value = analog_data;

    }                          



   if (N > 250)

  {                            

    if ( (analog_data > thresh) && (pulse_signal == false) && (N > (time_between_beats/5)*3) )

      {       

        pulse_signal = true;          

//        digitalWrite(led_pin,HIGH);

        time_between_beats = samplecounter - lastBeatTime;

        lastBeatTime = samplecounter;     



       if(second_heartpulse)

        {                        

          second_heartpulse = false;   

          for(int i=0; i<=9; i++)    

          {            

            beat[i] = time_between_beats; //Filling the array with the heart beat values                    

          }

        }


        if(first_heartpulse)

        {                        

          first_heartpulse = false;

          second_heartpulse = true;

          sei();            

          return;           

        }  


      word runningTotal = 0;  


      for(int i=0; i<=8; i++)

        {               

          beat[i] = beat[i+1];

          runningTotal += beat[i];

        }


      beat[9] = time_between_beats;             

      runningTotal += beat[9];   

      runningTotal /= 10;        

      heart_rate = 60000/runningTotal;

    }                      

  }




  if (analog_data < thresh && pulse_signal == true)

    {  

      //digitalWrite(led_pin,LOW); 

      pulse_signal = false;             

      amplitude = peak_value - trough_value;

      thresh = amplitude/2 + trough_value; 

      peak_value = thresh;           

      trough_value = thresh;

    }


  if (N > 2500)

    {                          

      thresh = 512;                     

      peak_value = 512;                 

      trough_value = 512;               

      lastBeatTime = samplecounter;     

      first_heartpulse = true;                 

      second_heartpulse = false;               

    }


  sei();                                

}
