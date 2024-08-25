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

#include <Arduino.h>
#include <lvgl.h>
#include "main.h"
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <ChronosESP32.h>
#include <Timber.h>

#ifdef USE_UI
#include "ui/ui.h"
#endif

#define SCR 8

#define GFX_BL LCD_BL
const int freq = 1000;
const int ledChannel = 7;
const int resolution = 8;

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    LCD_DE, LCD_VSYNC, LCD_HSYNC, LCD_PCLK,
    LCD_R0, LCD_R1, LCD_R2, LCD_R3, LCD_R4,
    LCD_G0, LCD_G1, LCD_G2, LCD_G3, LCD_G4, LCD_G5,
    LCD_B0, LCD_B1, LCD_B2, LCD_B3, LCD_B4,
    LCD_HSP, LCD_HFP, LCD_HPW, LCD_HBP,
    LCD_VSP, LCD_VFP, LCD_VPW, LCD_VBP,
    LCD_PCK_AN, LCD_FREQ);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    LCD_WIDTH, LCD_HEIGHT , rgbpanel);


TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);
ChronosESP32 watch("Jarvis Clock");

static const uint32_t screenWidth = 800;
static const uint32_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

static lv_color_t disp_draw_buf[screenWidth * SCR];
static lv_color_t disp_draw_buf2[screenWidth * SCR];

static bool reloadNotifications = false;
int notificationIndex = 0;
ChronosTimer alertTimer;
ChronosTimer findTimer;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  #if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  #else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  #endif

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  tp.read();
  if (!tp.isTouched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = tp.points[0].x;
    data->point.y = tp.points[0].y;
  }
}

void loadNotification(int index)
{
  int c = watch.getNotificationCount();
  Notification n = watch.getNotificationAt(index);
  lv_label_set_text(ui_alertTitle, n.app.c_str());
  lv_label_set_text(ui_alertText, n.message.c_str());
  lv_label_set_text(ui_alertTime, n.time.c_str());

  if (index == 0)
  {
    lv_obj_add_flag(ui_alertPrev, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_clear_flag(ui_alertPrev, LV_OBJ_FLAG_HIDDEN);
  }
  if (index == c - 1)
  {
    lv_obj_add_flag(ui_alertNext, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_clear_flag(ui_alertNext, LV_OBJ_FLAG_HIDDEN);
  }

  lv_obj_clear_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
}

void messageClick(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  uint32_t index = (uint32_t)lv_event_get_user_data(e);
  if (event_code == LV_EVENT_CLICKED)
  {
    // set the notification to the panel
    alertTimer.time = millis();
    alertTimer.active = true;
    alertTimer.duration = 10000;
    notificationIndex = index;
    loadNotification(notificationIndex);
  }
}

void alertPrev(lv_event_t *e)
{
  notificationIndex--;
  alertTimer.time = millis();
  alertTimer.active = true;
  alertTimer.duration = 10000;
  loadNotification(notificationIndex);
}

void alertNext(lv_event_t *e)
{
  notificationIndex++;
  alertTimer.time = millis();
  alertTimer.active = true;
  alertTimer.duration = 10000;
  loadNotification(notificationIndex);
}

void brightnessChange(lv_event_t *e)
{
  int32_t v = lv_slider_get_value(ui_brightnessSlider);
  ledcWrite(ledChannel, int(2.55 * v));
}

void cameraCapture(lv_event_t *e)
{
  watch.capturePhoto();
}

void findPhone(lv_event_t *e)
{
  if (findTimer.active)
  {
    // stop timer
    findTimer.active = false;
    lv_arc_set_value(ui_findTimer, 0);
    watch.findPhone(false);
  }
  else
  {
    // start timer
    findTimer.active = true;
    findTimer.time = millis();
    watch.findPhone(true);
  }
}

void musicPrevious(lv_event_t *e)
{
  watch.musicControl(MUSIC_PREVIOUS);
}

void musicNext(lv_event_t *e)
{
  watch.musicControl(MUSIC_NEXT);
}

void musicToggle(lv_event_t *e)
{
  watch.musicControl(MUSIC_TOGGLE);
}

void addNotification(Notification n, uint32_t index)
{
  lv_obj_t *ui_messagePanel = lv_obj_create(ui_notificationPanel);
  lv_obj_set_width(ui_messagePanel, 90);
  lv_obj_set_height(ui_messagePanel, 40);
  lv_obj_set_align(ui_messagePanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_messagePanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(ui_messagePanel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_messagePanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_messagePanel, lv_color_hex(0x01D3D1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_messagePanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_messagePanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_top(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(ui_messagePanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_messageTitle = lv_label_create(ui_messagePanel);
  lv_obj_set_width(ui_messageTitle, 75);
  lv_obj_set_height(ui_messageTitle, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_messageTitle, -2);
  lv_obj_set_y(ui_messageTitle, -7);
  lv_obj_set_align(ui_messageTitle, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_messageTitle, LV_LABEL_LONG_CLIP);
  lv_label_set_text(ui_messageTitle, n.app.c_str());
  lv_obj_set_style_text_color(ui_messageTitle, lv_color_hex(0x01D3D1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_messageTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_messageTitle, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_messageText = lv_label_create(ui_messagePanel);
  lv_obj_set_width(ui_messageText, 75);
  lv_obj_set_height(ui_messageText, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_messageText, -1);
  lv_obj_set_y(ui_messageText, 9);
  lv_obj_set_align(ui_messageText, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_messageText, LV_LABEL_LONG_CLIP);
  lv_label_set_text(ui_messageText, n.message.c_str());
  lv_obj_set_style_text_color(ui_messageText, lv_color_hex(0x01D3D1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_messageText, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_messageText, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_messagePanel, messageClick, LV_EVENT_ALL, (void *)index);
}

void connectionCallback(bool state)
{
  Timber.i(state ? "Connected" : "Disconnected");
}

void notificationCallback(Notification notification)
{
  Timber.i("Notification received");

  reloadNotifications = true;
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_WEATHER:
    // weather is saved
    Timber.i("Weather received");
    if (a)
    {
      // if a == 1, high & low temperature values might not yet be updated
      if (a == 2)
      {
        int n = watch.getWeatherCount();

        if (n > 1)
        {
          Weather w = watch.getWeatherAt(0);

          lv_label_set_text_fmt(ui_tempValue, "%d°", w.temp);
          lv_label_set_text_fmt(ui_tempHigh, "%d", w.high);
          lv_label_set_text_fmt(ui_tempLow, "%d", w.low);

          int range = w.high - w.low;
          int dif = w.temp - w.low;
          if (range > 0 && (w.temp <= (range + w.low)))
          {
            int16_t perc = int16_t(float(dif) / float(range) * 100.0);
            lv_arc_set_value(ui_tempArc, perc);
          }
        }
      }
    }
    if (b)
    {
      String city = watch.getWeatherCity();
      lv_label_set_text(ui_cityLabel, city.c_str());
    }
    break;
  }
}

void logCallback(Level level, unsigned long time, String message)
{
  Serial.print(message);

  switch (level)
  {
  case ERROR:
    // maybe save only errors to local storage
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  // Serial0.begin(115200);
  Timber.setLogCallback(logCallback);

  gfx->begin();
  gfx->setRotation(2);
  gfx->fillScreen(BLACK);


  // #ifdef GFX_BL
  //   pinMode(GFX_BL, OUTPUT);
  //   digitalWrite(GFX_BL, HIGH);
  // #endif

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(GFX_BL, ledChannel);
  ledcWrite(ledChannel, int(2.55 * 95));
  tp.begin();
  tp.setRotation(ROTATION_NORMAL);

  lv_init();

  Timber.i("Width: %d\tHeight: %d", screenWidth, screenHeight);

  if (!disp_draw_buf)
  {
    Timber.e("LVGL disp_draw_buf allocate failed!");
  }
  else
  {

    Timber.i("Display buffer size: %d", screenWidth * SCR);

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, disp_draw_buf2, screenWidth * SCR);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

#ifdef USE_UI
    ui_init();

#elif
    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 100);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label1, screenWidth - 30);
    lv_label_set_text(label1, "Hello there! You have not included UI files, add you UI files and "
                              "uncomment this line\n'//#define USE_UI' in include/main.h\n"
                              "You should be able to move the slider below");

    /*Create a slider below the label*/
    lv_obj_t *slider1 = lv_slider_create(lv_scr_act());
    lv_obj_set_width(slider1, screenWidth - 40);
    lv_obj_align_to(slider1, label1, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
#endif
    Timber.i("Setup done");
  }

  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);

  watch.begin();
  watch.setBattery(80);
  watch.set24Hour(true);
  Timber.i("Watch started");

  reloadNotifications = true;
  findTimer.duration = 10000;
  lv_arc_set_value(ui_findTimer, 0);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  watch.loop();
  delay(5);

  String time = String(watch.getHourC()) + ":" + watch.getTime("%M") + watch.getAmPmC(true);
  String weekday = watch.getTime("%A");
  String month = watch.getTime("%B");

  lv_label_set_text(ui_timeLabel, time.c_str());
  lv_label_set_text(ui_weekdayLabel, weekday.c_str());
  lv_label_set_text(ui_monthLabel, month.c_str());
  lv_label_set_text_fmt(ui_dayLabel, "%d", watch.getDay());

  if (watch.isConnected())
  {
    lv_obj_clear_flag(ui_findTimer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_musicPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(ui_connectPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_connectPanel1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  else
  {
    lv_obj_add_flag(ui_findTimer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_musicPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(ui_connectPanel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_connectPanel1, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  if (watch.isCameraReady())
  {
    lv_obj_clear_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_add_flag(ui_cameraPanel, LV_OBJ_FLAG_HIDDEN);
  }

  if (reloadNotifications)
  {
    // show current received notification, auto dismiss
    alertTimer.time = millis();
    alertTimer.active = true;
    alertTimer.duration = 5000;
    loadNotification(0);
    reloadNotifications = false;
    lv_obj_clean(ui_notificationPanel);
    int nCount = watch.getNotificationCount();

    for (int i = 0; i < nCount; i++)
    {
      Notification notif = watch.getNotificationAt(i);
      addNotification(notif, i);
    }
  }

  if (alertTimer.active)
  {
    if (alertTimer.time + alertTimer.duration < millis())
    {
      alertTimer.active = false;
      lv_obj_add_flag(ui_alertPanel, LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (findTimer.active)
  {
    if (findTimer.time + findTimer.duration < millis())
    {
      findTimer.active = false;
      lv_arc_set_value(ui_findTimer, 0);
      watch.findPhone(false);
    }
    else
    {
      long t = findTimer.duration - (millis() - findTimer.time);
      int percent = int((float(t) / float(findTimer.duration)) * 100.0);
      lv_arc_set_value(ui_findTimer, percent);
    }
  }
}