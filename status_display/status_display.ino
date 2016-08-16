#include <EtherCard.h>
#include <IPAddress.h>
#include <Adafruit_NeoPixel.h>

#include "settings_wazoku.h"

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define TYPE_COLORS 1
#define TYPE_SOUND 2


Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, 6, NEO_GRB + NEO_KHZ800);

static byte mac_addr[] = MAC_ADDR;


byte Ethernet::buffer[ETHERNET_BUFFER_SIZE];
static BufferFiller bfill;  // used as cursor while filling the buffer


void set_colors(uint8_t* data) {
  for (int i=0; i<8; i++) {
      strip.setPixelColor(
          i, 
          data[i*3],
          data[i*3+1],
          data[i*3+2]
      );  
  }
  
  strip.show();
}


static word sendResponse(uint8_t* data, uint16_t len) {
  char* status_msg = "OK";
  
  switch (data[0]) {
    case TYPE_COLORS:
        if (len < (1+3*8)) {
          status_msg = "Invalid msg length";
          break;
        }
        set_colors(data+1);
        break;
    default:
        status_msg = "Unknown command";
  }
  
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "Status: $S\r\n"
     ),
     status_msg);
  return bfill.position();
}


void setup_LEDs() {
  strip.begin();
  
  for (int i=0; i<8; i++) {
      strip.setPixelColor(i, 100, 100, 100);  
  }
  strip.show(); 
  
  play_melody__start();
  
  for (int i=0; i<8; i++) {
      strip.setPixelColor(i, 0, 0, 0);  
  }
  
  strip.show(); 
}


void setup_ethernet() {
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mac_addr[i], HEX);
  }
  
  Serial.println();

  if (ether.begin(sizeof Ethernet::buffer, mac_addr) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }

  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup()) {
    Serial.println(F("DHCP failed"));
  }

  ether.printIp("IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
}


void _play_melody(int* notes, int* durations, int len) {
  for (int i = 0; i < len; i++) {
    int duration = 1000 / durations[i];
    tone(BUZZER_POWER_PIN, notes[i], duration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_POWER_PIN);
  }
}

// Having separate functions for playing melodies is
// better than declaring data globally, it saves memory.
void play_melody__start() {
  int notes[] = {
    NOTE_E3, NOTE_E4, NOTE_E7,
  };
  int durations[] = {
    8, 8, 4,
  };
  _play_melody(notes, durations, 3);
}


void setup () {
  Serial.begin(57600);
  
  delay(500);
  pinMode(BUZZER_POWER_PIN, OUTPUT);

  setup_ethernet();
  
  setup_LEDs();
}


void loop () {
   //ether.packetLoop(ether.packetReceive());
   
    word len = ether.packetReceive();
    word pos = ether.packetLoop(len);   // receive data from ENC28J60
 
    if (pos) {  // pos !=0 of there was a valid HTTP GET received
      Serial.println("New message");
      uint8_t* data = (uint8_t *) Ethernet::buffer + pos;
      ether.httpServerReply(sendResponse(data, len));     
    }
       
}
