#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "plugin.h"

#define CLK 18
#define MISO 19
#define MOSI 23
#define VS1053_RESET -1
#define VS1053_CS 5
#define VS1053_DCS 32 
#define VS1053_DREQ 15 
#define VS1053_CARDCS 4

#define PAUSE_AFTER_ACTION 250
#define MAX_FILENAME_LENGTH 50
#define VOL_MAX 99
#define ROWS 3
#define MAX_FILES 250
#define MAX_DEPTH 25
#define MAX_NAME_WIDTH 21
#define ROW_HEIGHT 12



const int SDA_Pin = 21;
const int SCL_Pin = 22;

bool buttonState = false;
bool VS_RESET = false;
bool BT_UP = false;
bool BT_DOWN = false;
bool BT_LEFT = false;
bool BT_RIGHT = false;
bool BT_OK = false;
bool BT_FUNC = false;
bool BT_Res = false;
bool HIRES = false;
boolean playing = false;
boolean directoryList[MAX_FILES];

uint8_t offset = 0;
uint8_t selection = 0;
uint8_t fileNumber = 0;
uint8_t scrollOffset = 0;

char* fileList[MAX_FILES];
unsigned int scrollDelay = 200;
unsigned long lastScrollTime = 0;
unsigned long previousMillis = 0;
const long interval = 100;

int bassAmplitude = 0;
int bassFreqLimit = 2;
int trebleAmplitude = 0;
int trebleFreqLimit = 1;

int SA = 0; 
int Function = 0;
int DUI = 0;

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_CARDCS);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

struct StateStruct {
  uint8_t volume;
  char path[MAX_DEPTH];
} state = {30, "/"};

void show(char* m) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(m);
  display.display();
}

void fillFileList(char* path) {
  for (uint8_t i = 0; i < fileNumber; i++) {
    if (fileList[i] != nullptr) {
      free(fileList[i]);
      fileList[i] = nullptr;
    }
  }
  fileNumber = 0;

  File dir = SD.open(path);
  if (!dir) {
    display.setCursor(0, 0);
    display.println("Failed to open directory");
    display.display();
    return;
  }

  while (fileNumber < MAX_FILES) {
    File entry = dir.openNextFile();
    if (!entry) break;
    fileList[fileNumber] = strdup(entry.name());
    if (fileList[fileNumber] == nullptr) {
      display.setCursor(0, 0);
      display.println("Memory allocation failed for file name");
      display.display();
      break;
    }

    directoryList[fileNumber] = entry.isDirectory();
    fileNumber++;
    entry.close();
  }
  dir.close();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (!playing) {
    // Display file list when not playing
    for (uint8_t i = offset; i < (offset + ROWS) && i < fileNumber; i++) {
      display.print((selection == i) ? "> " : "");
      display.print(fileList[i]);
      display.println(directoryList[i] ? "/" : "");
      display.println();
    }
  } else {
    if (DUI == 0) { // UI - 1 for Song Name and Outher information
        // Display current song name (scroll if too long)
    String songName = fileList[selection];
    uint8_t nameLen = songName.length();

      if (nameLen > MAX_NAME_WIDTH) {
    
    // Maximum number of characters that can fit on the screen
    const uint8_t maxVisibleChars = 20; // Adjust based on your display width in characters

    if (nameLen > maxVisibleChars) {
      // Get the current time
      unsigned long currentTime = millis();

      // Check if it's time to update the scroll
      if (currentTime - lastScrollTime >= scrollDelay) {
        lastScrollTime = currentTime; // Update the last scroll time
        scrollOffset++; // Increment scroll offset

        // If the scroll offset exceeds the total length of the string plus padding, reset
        if (scrollOffset > nameLen + maxVisibleChars) {
          scrollOffset = 0;
        }
      }

      // Create the scrolling effect by wrapping the string
      String visiblePart = "";
      for (uint8_t i = 0; i < maxVisibleChars; i++) {
        // Compute the index in the original string with wrapping
        uint8_t charIndex = (scrollOffset + i) % (nameLen + maxVisibleChars);
        if (charIndex < nameLen) {
          visiblePart += songName[charIndex];
        } else {
          visiblePart += " "; // Add padding spaces after the text
        }
      }
      // Display the scrolling text
    display.setCursor(0, 0);
    display.print(visiblePart);
    } 
      } else {
    display.print(songName);
    }
    display.drawLine(0, 10, 128, 10, WHITE);
    
    // Display elapsed time
    uint16_t elapsedTime = musicPlayer.decodeTime();
    uint8_t hours = elapsedTime / 3600;
    uint8_t minutes = (elapsedTime % 3600) / 60;
    uint8_t seconds = elapsedTime % 60;
    display.setCursor(0, 14);
    display.print(hours);
    display.print(":");
    if (minutes < 10) display.print("0");
    display.print(minutes);
    display.print(":");
    if (seconds < 10) display.print("0");
    display.print(seconds);

    // Display volume
    char volBuffer[3];
    snprintf(volBuffer, 3, "%02i", VOL_MAX + 1 - state.volume);
    display.setCursor(80, 14);
    display.print("Vol: ");
    display.print(volBuffer);

    if (HIRES == true) {
    display.fillRect(0, 53, 37, 9, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(1, 54);
    display.print("HI-RES");
    display.setTextColor(WHITE);
    }
    
    } else if (DUI == 1) { // UI - 2 for Bass/Treble and SPITAL Audio
  
    // Display Bass and Treble settings
    display.setCursor(0, 0);
    display.print("B:");
    if (Function == 0) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print(bassAmplitude);
    display.print(" dB, ");
    display.print("Freq:");
    if (Function == 1) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print(bassFreqLimit * 10);
    display.print(" Hz");

    display.setCursor(0, 10);
    display.print("T:");
    if (Function == 2) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print(trebleAmplitude);
    display.print(" dB, ");
    display.print("Freq:");
    if (Function == 3) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.print(trebleFreqLimit);
    display.print(" kHz");

    // Display Spatial Audio status
    display.setCursor(0, 20);
    display.print("SPATIAL AUDIO:");
    if (Function == 4) {
      display.print(">");
    } else {
      display.print(" ");
    }
    if (SA == 0) {
      display.print("OFF");
    } else if (SA == 1) {
      display.print("LOW");
    } else if (SA == 2) {
      display.print("MID");
    } else if (SA == 3) {
      display.print("HIGH");
    }
  }
}
  display.display();
}

void setBassAndTreble(int bassAmplitude, int bassFreqLimit, int trebleAmplitude, int trebleFreqLimit) {
    uint8_t trebleAmp = (trebleAmplitude < 0) ? (trebleAmplitude + 16) : trebleAmplitude;
    uint16_t bassTrebleSetting = ((trebleAmp & 0xF) << 12) | ((trebleFreqLimit & 0xF) << 8) |
                                 ((bassAmplitude & 0xF) << 4) | (bassFreqLimit & 0xF);

    noInterrupts();
    musicPlayer.sciWrite(0x2, bassTrebleSetting);
    interrupts();
}

void loadPlugin(const unsigned short *plugin, size_t size) {
  for (size_t i = 0; i < size; ) {
    unsigned short addr = plugin[i++];
    unsigned short n = plugin[i++];
    if (n & 0x8000U) {
      n &= 0x7FFF;
      unsigned short val = plugin[i++];
      while (n--) {
        musicPlayer.sciWrite(addr, val);
      }
    } else {
      while (n--) {
        unsigned short val = plugin[i++];
        musicPlayer.sciWrite(addr, val);
      }
    }
  }
}

void controlSpatialAudio(int SA) { // Refer Datasheet SCI Registers (9.6.1)

  noInterrupts();
  uint16_t modeRegister = musicPlayer.sciRead(0x0); // Write's at SCI_MODE (0x0) Register
  interrupts();
  
  if (SA == 3) {
    modeRegister |= (1 << 4); // Set space bit
    modeRegister |= (1 << 7); // Set high
  } else if (SA == 2) {
    modeRegister &= ~(1 << 4); // Clear space bit
    modeRegister |= (1 << 7);  // Set mid
  } else if (SA == 1) {
    modeRegister |= (1 << 4);  // Set space bit
    modeRegister &= ~(1 << 7); // Clear high
  } else if (SA == 0) {
    modeRegister &= ~(1 << 4); // Clear space bit
    modeRegister &= ~(1 << 7); // Clear high
  }

  noInterrupts();
  musicPlayer.sciWrite(0x0, modeRegister); // Write's at SCI_MODE (0x0) Register 
  interrupts();
  display.display();
}

void buttonPress() {

 uint8_t action = readButton(analogRead(34));

  if (!playing) {
    if (VS_RESET == true) {

      buttonState = false;
      VS_RESET = false;

      musicPlayer.reset();

      // File navigation when not playing
    } else if (BT_UP == true) {
      BT_UP = false;
      if (selection > 0) {
        selection--;
        if (selection < offset) offset--;
      }
    } else if (BT_DOWN == true) {
      BT_DOWN = false;
      if (selection < fileNumber) {
        selection++;
        if (selection >= offset + ROWS) offset = selection - ROWS + 1;
      }
    } else if (BT_LEFT == true) {
      BT_LEFT = false;
      uint8_t l = strlen(state.path) - 2;
      while (l > 1 && state.path[l] != '/') l--;
      state.path[l] = '\0';
      fillFileList(state.path);
      offset = selection = 0;
    } else if (BT_RIGHT == true) {
      BT_RIGHT = false;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Reading Directory...");
      display.display();
      if (directoryList[selection]) {
        strcat(state.path, fileList[selection]);
        strcat(state.path, "/");
        fillFileList(state.path);
        offset = selection = 0;
      }
    } else if (BT_OK == true) {
      BT_OK = false;
      resetEsp();
      if (!directoryList[selection]) {
        char song[MAX_DEPTH + MAX_FILENAME_LENGTH] = "\0";
        snprintf(song, sizeof(song), "%s%s", state.path, fileList[selection]);
        String fileName = String(song);
        uint16_t FLAC = musicPlayer.sciRead(0x9);

        if (fileName.endsWith(".flac")) {
          musicPlayer.softReset();
          display.clearDisplay();
          HIRES = true;
          display.setCursor(0, 0);
          display.println("Loading FLAC PLUGIN");
          display.display();
          delay(50);
          loadPlugin(plugin, sizeof(plugin) / sizeof(plugin[0]));
          delay(50);
          display.setCursor(0 ,10);
          display.println("FLAC PLUGIN LOADED!!");
          display.display();
          delay(50);
          musicPlayer.startPlayingFile(song);
          playing = true;
        } else {
          delay(50);
          musicPlayer.startPlayingFile(song);
          playing = true;
        }
      }
    }
  } else {
    
    // Button controls when playing
    
     if (BT_RIGHT == true) {
      BT_RIGHT = false;
      
    } else if (BT_LEFT == true) {
      BT_LEFT = false;
      musicPlayer.stopPlaying();
      playing = false;
    } else if (BT_OK == true) {
      BT_OK = false;
      if (!musicPlayer.paused()) {
      musicPlayer.pausePlaying(true);
      } else { 
      musicPlayer.pausePlaying(false);
      }
    } else if (BT_Res == true) {
      BT_Res = false;
      if (DUI < 2) {
        DUI++;
        if (DUI == 2) {
          DUI = 0;
        }
      }  
    } else if (DUI == 0) { // Volume +/-
       if (BT_UP == true) {
         BT_UP = false;
          if (state.volume > 0) state.volume--;
          musicPlayer.setVolume(state.volume, state.volume);
         } else if (BT_DOWN == true) {
          BT_DOWN = false;
          if (state.volume < VOL_MAX) state.volume++;
          musicPlayer.setVolume(state.volume, state.volume);
         } 
         Function = 0;
     } else if (DUI == 1) {
      if (BT_FUNC == true) {
      BT_FUNC = false;
      if (Function < 5) {
        Function++;
        if (Function == 5) {
          Function = 0;
        }
      } 
    } else if (Function == 0) { // Bass Amplitude +/-
          if (BT_UP == true) {
           BT_UP = false;
          if (bassAmplitude < 15) {
           bassAmplitude++;
           setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
          }
       } else if (BT_DOWN == true) {
         BT_DOWN = false;
         if (bassAmplitude > 0) {
          bassAmplitude--;
          setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);         
          }
       }
      } else if (Function == 1) { // Bass Frequency +/-
       if (BT_UP == true) {
         BT_UP = false;
         if (bassFreqLimit < 15) {
          bassFreqLimit++;
          setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
         }
       } else if (BT_DOWN == true) {
         BT_DOWN = false;
         if (bassFreqLimit > 2) {
          bassFreqLimit--;
          setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
         }         
        }
      } else if (Function == 2) { // Treble Amplitude +/-
       if (BT_UP == true) {
         BT_UP = false;
         if (trebleAmplitude < 7) {
           trebleAmplitude++;
           setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
          }
       } else if (BT_DOWN == true) {
         BT_DOWN = false;
         if (trebleAmplitude > -8) {
           trebleAmplitude--;
           setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
          }
       }
       
      } else if (Function == 3) { // Treble Frequency +/-
       if (BT_UP == true) {
         BT_UP = false;
         if (trebleFreqLimit < 15) {
           trebleFreqLimit++;
           setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
          }
       } else if (BT_DOWN == true) {
         BT_DOWN = false;
         if (trebleFreqLimit > 1) {
           trebleFreqLimit--;
           setBassAndTreble(bassAmplitude, bassFreqLimit, trebleAmplitude, trebleFreqLimit);
          }
       }
      } else if (Function == 4) { // Treble Frequency +/-
       if (BT_UP == true) {
         BT_UP = false;
         if (SA < 3) {
          SA++;
         }
         controlSpatialAudio(SA);
       } else if (BT_DOWN == true) {
         BT_DOWN = false;
         if (SA > 0) {
          SA--;
         }
         controlSpatialAudio(SA);
       }
      }
    }
  }
  BT_Res = false;
  buttonState = false;
}

void debug() {

  // ----- Button Debug ----- //
  Serial.println("-------------------------------------------------");
  Serial.println("Button Status (Analog Value) :-"); 
  Serial.print(analogRead(34));
  Serial.println("-------------------------------------------------");

  // ----- Bass and Treble Debug ----- //
  Serial.print("Bass: Amplitude = ");
  Serial.print(bassAmplitude);
  Serial.print(" dB, Freq Limit = ");
  Serial.print(bassFreqLimit);
  Serial.println(" Hz");
  Serial.print("Treble: Amplitude = ");
  Serial.print(trebleAmplitude);
  Serial.print(" dB, Freq Limit = ");
  Serial.print(trebleFreqLimit);
  Serial.println(" Hz");
  
}

void resetEsp() {
  SA = 0;
  Function = 0;
  HIRES = false;
  DUI = 0;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_Pin, SCL_Pin);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  if (!musicPlayer.begin()) {
    show("VS1053 not found");
    while (1);
  }

  if (!SD.begin(VS1053_CARDCS)) {
    show("SD card initialization failed!");
    while (1);
  }

  if (!musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT)) {
    show("DREQ pin not irq");
    while (1);
  }

  pinMode(34, INPUT);

  musicPlayer.setVolume(state.volume, state.volume);

  fillFileList(state.path);
}

void loop() {

unsigned long currentMillis = millis(); 

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Update the previous time
        buttonPress();
    }
 
    if (playing && !musicPlayer.playingMusic) {
        playing = false;
        musicPlayer.reset();
        resetEsp();
      updateDisplay();
    }
  updateDisplay();
  delay(50);
//  debug();
}
