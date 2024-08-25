/*
   MIT License

  Copyright (c) 2024 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#define USE_UI

#ifdef ELECROW_7

#define TOUCH_SDA 19
#define TOUCH_SCL 20
#define TOUCH_INT -1
#define TOUCH_RST -1
#define TOUCH_WIDTH 800
#define TOUCH_HEIGHT 480

#define LCD_BL 2

#define LCD_DE 41
#define LCD_VSYNC 40
#define LCD_HSYNC 39
#define LCD_PCLK 0
#define LCD_G0 9
#define LCD_G1 46
#define LCD_G2 3
#define LCD_G3 8
#define LCD_G4 16
#define LCD_G5 1
#define LCD_R0 14
#define LCD_R1 21
#define LCD_R2 47
#define LCD_R3 48
#define LCD_R4 45
#define LCD_B0 15
#define LCD_B1 7
#define LCD_B2 6
#define LCD_B3 5
#define LCD_B4 4
#define LCD_HSP 0
#define LCD_HFP 40
#define LCD_HPW 48
#define LCD_HBP 40
#define LCD_VSP 0
#define LCD_VFP 1
#define LCD_VPW 31
#define LCD_VBP 13
#define LCD_PCK_AN 1
#define LCD_FREQ 15000000

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

#elif PANLEE_7

#define TOUCH_SDA  48
#define TOUCH_SCL  47
#define TOUCH_INT -1
#define TOUCH_RST -1
#define TOUCH_WIDTH 800
#define TOUCH_HEIGHT 480

#define LCD_BL 45

#define LCD_DE 39
#define LCD_VSYNC 38
#define LCD_HSYNC 5
#define LCD_PCLK 9
#define LCD_G0 21
#define LCD_G1 0
#define LCD_G2 46
#define LCD_G3 3
#define LCD_G4 8
#define LCD_G5 18
#define LCD_R0 10
#define LCD_R1 11
#define LCD_R2 12
#define LCD_R3 13
#define LCD_R4 14
#define LCD_B0 17
#define LCD_B1 16
#define LCD_B2 15
#define LCD_B3 7
#define LCD_B4 6
#define LCD_HSP 0
#define LCD_HFP 0
#define LCD_HPW 210
#define LCD_HBP 30
#define LCD_VSP 0
#define LCD_VFP 0
#define LCD_VPW 22
#define LCD_VBP 13
#define LCD_PCK_AN 1
#define LCD_FREQ 16000000

#define LCD_WIDTH 800
#define LCD_HEIGHT 481

#else

#error "you need to define your display pin configuration here"

#endif
