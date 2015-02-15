// Arduino implementation for the original Novation Launchpad
// requires: USBH_MIDI library https://github.com/YuuichiAkagawa/USBH_MIDI
// requires: USBH_MIDI library https://github.com/felis/USB_Host_Shield_2.0
// USB Host Shield 2.0 for Arduino http://www.ebay.co.uk/itm/Hot-USB-Host-Shield-2-0-Arduino-UNO-MEGA-ADK-Compatible-Google-Android-ADK-/191197207021?ssPageName=ADME:L:OU:GB:1120
// Original Launchpad
// Arduino Uno or Duemilanova or compatible

// This is awful code, just a mockup. Since I'm not going to develop this, I'm releasing it as it is.

#include <Usb.h>
#include <usbh_midi.h>

USB  Usb;
USBH_MIDI  Midi(&Usb);
void MIDI_poll();
uint16_t pid, vid;
uint8_t scene[8][8];

void setup()
{
  vid = pid = 0;
  Serial.begin(115200);

  delay(5000);
  //Workaround for non UHS2.0 Shield 
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  delay(200);
  digitalWrite(7,LOW);
  delay(200);
  digitalWrite(7,HIGH);

  Serial.println("init");
  boolean ok = false;
  while (!ok)
  {
    ok=true;
    if (Usb.Init() == -1) 
    {
      Serial.println("USB init failed");
      ok=false;
      delay(2000);
    }
  }
  delay( 200 );
  Serial.println("start");



}

void loop()
{

  static long lastPrinted = 0;
  
  if (millis()-lastPrinted>1000)
  {
    Serial.print(millis(), DEC);
    Serial.println(" run ");
    lastPrinted = millis();
  }
  
  
  Usb.Task();

  if( Usb.getUsbTaskState() == USB_STATE_RUNNING )
  {
    static bool startFlag = true;
    if (startFlag)
    {
      startFlag = false;

      Serial.println("Startup");

      for (int i=0; i<8; i++)
      {
        setButton(8,i,3-i,i);
        delay(50);
      }
      setButton(0,0,3,3);
      delay(500);
      setButton(7,0,3,0);
      delay(500);
      setButton(0,7,0,3);
      delay(500);
      setButton(7,7,3,3);
      delay(500);

      uint8_t sf[3] = {
        0xB0, 0,0                  };
      Midi.SendData(sf, 3);
      delay(5);
    }
    MIDI_poll();
  }

}

void setButton(uint8_t x, uint8_t y, uint8_t green, uint8_t red)
{
  uint8_t vel = (red & 0x03) | ((green & 0x03) << 5);

  uint8_t sf[3] = {
    0x90, (y*16)+x, vel          };
  Midi.SendData(sf, 3);
  delay(5);
}

// Poll USB MIDI Controler and send to serial MIDI
void MIDI_poll()
{
  char buf[20];
  uint8_t bufMidi[64];
  uint16_t  rcvd;

  static long lastDropped=0;
  if (millis()-lastDropped>200)
  {
    lastDropped=millis();
    for (int x=0; x<8; x++)
    {
      for (int y=0; y<7; y++)
      {

        if (scene[x][y]>0) 
        {
          if (scene[x][y+1]==0) 
          {
            setButton(x,y,0,0);

            setButton(x,y+1,1,0);

            scene[x][y]=0;
            scene[x][y+1]=1;
            y = 7;
          }
        }
      }
    }  
  }
  //setButton(random(0,8), random(0,8), random(0,4), random(0,4));

  if(Midi.vid != vid || Midi.pid != pid){
    sprintf(buf, "VID:%04X, PID:%04X", Midi.vid, Midi.pid);
    Serial.println(buf);
    vid = Midi.vid;
    pid = Midi.pid;
  }
  if(Midi.RecvData( &rcvd,  bufMidi) == 0 ){
    sprintf(buf, "%08X:", millis());
    Serial.print(buf);
    for(int i=0; i<sizeof(bufMidi); i++){
      sprintf(buf, " %02X", bufMidi[i]);
      Serial.print(buf);
    }
    Serial.println("");

    static bool p;

    int bx = bufMidi[0] & 7;
    int by = bufMidi[0] / 16;

    if (bufMidi[1] == 0x7F)
    {
      /*
        uint8_t sf[3] = {0x90, bufMidi[0], 3};
       Midi.SendData(sf, 3);
       */
      //setButton(x,y,3,0);
      scene[bx][by]=1;

    }


    if (bufMidi[1] == 0x00)
    {

      //      setButton(bx,by,0,0);

    }

  }
}






