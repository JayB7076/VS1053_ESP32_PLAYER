# VS1053_ESP32_PLAYER
A feature-rich ESP32-based music player using the Adafruit VS1053 codec and SSD1306 OLED display. The player supports FLAC and MP3 file playback with OLED display and Button Controls like volume control, bass/treble adjustments, and spatial audio.

Hardware Requirements:
ESP32
VS1053 Codec Module
SSD1306 OLED Display
SD Card Module (Mine has Inbuild in VS1053)
7 Push Buttons

Features:
SD card browsing and playback support with OLED.
Adjustable bass, treble, and spatial audio.
FLAC Support (Crashes and sometimes unable to load song.)

Example code which is used :-
https://www.youtube.com/watch?v=GQP0tIAjIE0&t=1s&ab_channel=Gr%C3%A9goryPaul
 
libraries Required / Used :-
official Adafruit_VS1053 / Adafruit_SSD1306 / Adafruit_GFX libraries
SD for SD card content read
SPI for SPI communication between VS1053 and ESP32
Wire for 128/64 OLED Display communication

Pin Connection :- (For Custom Pin Configuration Please refer ESP32's Datasheet)

----- VS1053 CONNECTION TO ESP32 ----

      ESP32 -- TO -- VS1053
       D18  --------  CLK
       D19  --------  MISO
       D23  --------  MOSI
       EN   --------  VS_RESET
       D5   --------  VS_CS
       D32  --------  VS_DCS
       D15  --------  VS_DREQ
       D4   --------  SD_CARD_CS


---- OLED CONNECTION TO ESP32 ----

      ESP32 -- TO -- OLED
       D21  --------  OLED_SDA
       D22  --------  OLED_SCL

---- BUTTONS CONNECTION TO ESP32 ---- 
Refer (https://www.instructables.com/Accessing-5-buttons-through-1-Arduino-pin-Revisi/)
(in my case an 5 Tactile Switch with Additional Button)

      ESP32 -- TO -- SWITCH
           |--R82K--  UP
           |--R68K-- DOWN
      D34 -|--R47K-- LEFT
           |--R33Ω-- RIGHT
           |--R10K--  OK
           |--R100K- BT_1


---- Remaining Functions and Errors ----
Fast Forward / Back
Status Bar
FLAC Files Do Not Load Intermittently on ESP32. (May be Due to FLAC ID Jumper not used?)
ESP32 Crashes When Going Back After FLAC Song Fails to Load. (May be Due to FIle names are Cleared?)
