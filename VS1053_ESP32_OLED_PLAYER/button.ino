uint8_t readButton(uint16_t action) {
      if (action > 2000 && action < 2100) {
        return BT_UP = true;
      } else if (action > 2200 && action < 2300) {
        return BT_DOWN = true;
      } else if (action > 2550 && action < 2650) {
        return BT_LEFT = true;
      } else if (action > 4000 && action < 4100) {
        return BT_RIGHT = true;
      } else if (action > 3750 && action < 3850) {
        return BT_OK = true;
      } else  if (action > 1850 && action < 1950) {
        return BT_FUNC = true;
      } else if (action > 950 && action < 1050) {
        return BT_Res = true; 
      } else {
        return buttonState = false;
      }
    }
