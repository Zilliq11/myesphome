#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include <math.h>


namespace esphome {
namespace zilloscope {

  class DisplayEffect {
    public:
      explicit DisplayEffect( const std::string &name) : name_(name) {}
      const std::string &get_name() { return this->name_; }

      /// Initialize this LightEffect. Will be called once after creation.
      virtual void start() {}
      virtual void start_internal() { this->start(); }
      /// Called when this effect is about to be removed
      virtual void stop() {}
      /// Apply this effect. Use the provided state for starting transitions, ...
      virtual void apply(display::DisplayBuffer &it) = 0;
      /// Internal method called by the LightState when this light effect is registered in it.
      virtual void init() {}
      void init_internal(display::DisplayBuffer *display) {
        this->display_ = display;
        this->init();
      }
    protected:
      display::DisplayBuffer *get_display_() const { return (display::DisplayBuffer *) this->display_; }
      display::DisplayBuffer *display_{nullptr};
      std::string name_;
  };

  class DisplayLambdaEffect : public DisplayEffect {
    public:
      DisplayLambdaEffect(const std::string &name,
                          const std::function<void(display::DisplayBuffer &, bool initial_run)> &f,
                          uint32_t update_interval)
          : DisplayEffect(name), f_(f), update_interval_(update_interval) {}

      void start() override { this->initial_run_ = true; }
      void stop() override {}
      void apply(display::DisplayBuffer &it) override {
        const uint32_t now = millis();
        if (now - this->last_run_ >= this->update_interval_) {
          this->last_run_ = now;
          this->f_(it, this->initial_run_);
          this->initial_run_ = false;
        }
      }
    protected:
      std::function<void(display::DisplayBuffer &, bool initial_run)> f_;
      uint32_t update_interval_;
      uint32_t last_run_{0};
      bool initial_run_;
  };

// Thanks to Liemmerle for the fire effect algorythm <3
  class DisplayFireEffect : public DisplayEffect {
    public:
      const char *TAG = "zilloscope.displayeffect";

      long unsigned int max_flame_height = 1000;
      long unsigned int max_heat_spots = 1000;
      long unsigned int min_x_attenuation = 500;
      long init_flame_height = 1;
      long init_heat_spots = 1000;
      long init_x_attenuation = 5000;
      //long speed = 15000;
      long starting_speed = 1000000;

      explicit DisplayFireEffect(const std::string &name) : DisplayEffect(name) {

      }

      int apow(int a, int b) {
        return 1000 + (a - 1000) * b / 1000;
      }

      int rnd(int x, int y) {
        int X = x ^ 64228;
        int Y = y ^ 61356;
        return ((
          X * 71521
          + Y * 13547)
          ^ 35135) % 1000;
      }

      int noise(int X, int Y, int T, int flame_height, int heat_spots, int x_attenuation) {
        int x = X;
        int n = 0;

        int attenuation = (height_ - Y) * 1000 / height_ * 1000 / flame_height
          + (x_attenuation == 0 ? 0
          : max(0, apow(1000 - (X + 1) * (width_- X) * 4000 / ((width_ + 2) * (width_+ 2)), 1000000 / x_attenuation)));

        int sum_coeff = 0;

        for(int i = 8 ; i > 0 ; i >>= 1) {
          int y = Y + T * 8 / i;

          int rnd_00 = rnd(x / i, y / i);
          int rnd_01 = rnd(x / i, y / i + 1);
          int rnd_10 = rnd(x / i + 1, y / i);
          int rnd_11 = rnd(x / i + 1, y / i + 1);

          int coeff = i;

          int dx = x % i;
          int dy = y % i;

          n += ((rnd_00 * (i - dx) + rnd_10 * dx) * (i - dy)
              + (rnd_01 * (i - dx) + rnd_11 * dx) * dy)
            * coeff / (i * i);
          sum_coeff += coeff;
        }
        return max(0, apow(n / sum_coeff, 1000000 / heat_spots * 1000 / (attenuation + 1000)) - attenuation);
      }


      uint32_t heat_color(int heat) {
        int r = min(255, (int) (heat * 255 / 333));
        int g = min(255, max(0, (int) ((heat - 333) * 255 / 333)));
        int b = min(255, max(0, (int) ((heat - 667) * 255 / 333)));
        return (r << 16) | (g << 8) | b;
      }

      void start() override {

      }
      // code exécuté a chaque frame
      void apply(display::DisplayBuffer &it) override {
        unsigned long timer = millis();
        /*
        int flame_height = (int) min(init_flame_height + starting_speed * timer / 1000L, max_flame_height);
        int heat_spots = (int) min(init_heat_spots + starting_speed * timer / 1000L, max_heat_spots);
        int x_attenuation = (int) max(init_x_attenuation - starting_speed * timer / 1000L, min_x_attenuation);
        */
        int flame_height = 1000;
        int heat_spots = 1000;
        int x_attenuation = 500;

        for(int x = 0 ; x < width_; x ++) {
          for(int y = 0 ; y < height_; y ++) {
            int heat = heat_color(noise(x, y, (int) (timer * speed_ / 1000), flame_height, heat_spots, x_attenuation));
            it.draw_pixel_at(x,y,Color(heat));
          }
        }
         //unsigned long timer2 = millis();
         //ESP_LOGD(TAG, "draw time: %lu" , (timer2-timer));
      }
      void set_speed(uint32_t speed) { this->speed_ = speed; }
      void set_width(uint16_t width) { this->width_ = width; }
      void set_height(uint16_t height) { this->height_ = height; }
    protected:
      uint32_t speed_{10};
      uint16_t width_{35};
      uint16_t height_{25};
  };

  class DisplayRainbowEffect : public DisplayEffect {
    public:
      explicit DisplayRainbowEffect(const std::string &name) : DisplayEffect(name) {}

      void apply(display::DisplayBuffer &it) override {
        //TODO
      }
      void set_speed(uint32_t speed) { this->speed_ = speed; }
      void set_width(uint16_t width) { this->width_ = width; }

    protected:
      uint32_t speed_{10};
      uint16_t width_{50};
  };


















































  

  // Thanks to Liemmerle for the bubble effect algorythm <3
  class DisplayBubblesEffect : public DisplayEffect {
    public:
      const char *TAG = "zilloscope.displayeffect";
      explicit DisplayBubblesEffect(const std::string &name) : DisplayEffect(name) {}

      int rnd(int x, int y, int z) {
          int X = x ^ 1273419445;
          int Y = y ^ 897982756;
          int Z = z ^ 453454122;
          int tmp = ((
            X * 315132157
            + Y * 1325782542
            + Z * 351213151)
            ^ 351356521);
          return tmp ^ (tmp >> 8) ^ (tmp << 8);
        }

      int apow(int a, int b) {
          return 1000 + (a - 1000) * b / 1000;
        }

      int repeat(int X, int r) {
          int tmp = X % (2 * r);
          if(tmp > r) {
            return 2 * r - tmp;
          }
          return tmp;
        }

      int sqr_dist(int x0, int y0, int x1, int y1) {
          return (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1);
        }

      uint32_t color_bubble (int x, int y, int nx, int ny, int radius) {
          int dist = 0;
          for(int x0 = x-1 ; x0 <= x+1 ; x0++){
            for(int y0 = y-1 ; y0 <= y+1 ; y0++){
              dist += sqr_dist(x0, y0, nx, ny);
            }
          }
          dist /= 9;
          int coeff = dist * 1024 / (radius == 0 ? 1 : radius * radius);
          return (uint32_t) (min(255, max(0, apow(coeff, 1200) / 3))) * 0x010101;
        }

      int noise(int X, int Y, int T) {
          uint32_t n = background_;
          for(int i = biggest_bubble_ ; i >= smallest_bubble_ ; i -= 4) {
            if(i == 0)
              i = 1;

            boolean bkg = false;

            int nx = 0;
            int ny = 0;
            int bdist = 1 << 30;
            int y = Y + T * i / 1000;
            int x = X + repeat(T / 1000 + i, 3) - 1;
            int xi = x - (x % i);
            int yi = y - (y % i);
            int crad = i / 2;
            if(crad == 0)
              crad = 1;

            int rad = i / 2;

            for(int x0 = xi - i ; x0 <= xi + i ; x0 += i)
              for(int y0 = yi - i ; y0 <= yi + i ; y0 += i)
                if(x0 / i % 2 == 0 && y0 / i % 2 == 0){
                int xp = (rnd(x0, y0, 0) % crad) + x0;
                int yp = (rnd(x0, y0, 1) % crad) + y0;
                int b = rnd(x0, y0, 3512);
                int trad = i / 2 + ((b & 4) >> 2);
                int dist = sqr_dist(xp, yp, x, y);
                if((b & (128 | 64 | 16 | 512 |2048)) == 0 && dist < bdist && dist <= trad * trad) {
                  bdist = dist;
                  nx = xp;
                  ny = yp;
                  rad = trad;
                  bkg = true;
                }
              }
            n |= !bkg ? 0 : color_bubble(x, y, nx, ny, rad);
          }
          return n | (n >> 8) | (n >> 16);
        }

      void apply(display::DisplayBuffer &it) override {
        unsigned long timer = millis();
        for(int x = 0 ; x < width_ ; x ++) {
          for(int y = 0 ; y < height_ ; y ++) {
            uint32_t color = (noise(x, y, (int) (timer * speed_ / 10)));
            it.draw_pixel_at(x,y,Color(color));
          }
        }
          //unsigned long timer2 = millis();
          //ESP_LOGD(TAG, "draw time: %lu" , (timer2-timer));
      }

      void set_speed(uint32_t speed) { this->speed_ = speed; }
      void set_width(uint16_t width) { this->width_ = width; }
      void set_height(uint16_t height) { this->height_ = height; }
      void set_background(uint32_t background) { this->background_ = background; }
      void set_smallest_bubble(uint8_t smallest_bubble) { this->smallest_bubble_ = smallest_bubble; }
      void set_background(uint8_t biggest_bubble) { this->biggest_bubble_ = biggest_bubble; }

    protected:
      uint32_t background_{0x193291};
      uint32_t speed_{15};
      uint16_t width_{32};
      uint16_t height_{32};
      uint8_t smallest_bubble_{0};
      uint8_t biggest_bubble_{16};


  };


































  
  // Thanks to Liemmerle for the matrix animation code
  class DisplayMatrixEffect : public DisplayEffect
  {
  public:
    const char *TAG = "zilloscope.displayeffect";
    explicit DisplayMatrixEffect(const std::string &name) : DisplayEffect(name) {}

    int rnd(int x, int y, int z)
    {
      int X = x ^ 1273419445;
      int Y = y ^ 897982756;
      int Z = z ^ 165135135;
      int tmp = ((
                     X * 315132157 + Y * 1325782542 + Z * 315132189) ^
                 351356521);
      if (tmp < 0)
        tmp = -tmp;
      return tmp ^ (tmp >> 8) ^ (tmp << 8);
    }

    int apow(int a, int b)
    {
      return 1000 + (a - 1000) * b / 1000;
    }

    int noise(int X, int Y, int T)
    {

      int n = background_;

      Y *= -1;

      for (int i = smallest_line_; i <= biggest_line_; i += line_iterator_)
      {
        if (i == 0)
          i = 1;

        int ny = 0;
        boolean bkg = true;

        int y = Y + T * i / 1000;
        int x = X;

        int yi = y - (y % i);

        for (int y0 = yi; y0 <= yi + i * 2; y0 += i)
        {
          int yp = (rnd(x, y0, 0) % i) + y0;

          int random = rnd(x, y0, 1) & (64 | 2048 | 4096);

          if (random == 0 && yp >= y && yp - y < i)
          {
            ny = yp;
            bkg = false;
            break;
          }
        }

        if (!bkg)
          n = color_line(y, ny, i);
      }

      return n;
    }

  void apply(display::DisplayBuffer &it) override {
    unsigned long timer = millis();
    for(int x = 0 ; x < width_ ; x ++) {
      for(int y = 0 ; y < height_ ; y ++) {

        uint32_t color = (noise(x, y, (int) (timer * speed_ / 10)));
        it.draw_pixel_at(x,y,Color(color));
      }
    }
    //unsigned long timer2 = millis();
    //ESP_LOGD(TAG, "draw time: %lu" , (timer2-timer));
  }

  uint32_t color_line(int y, int ny, int size)
  {
    int dist = ny - y;

    int r = max(0, min(255, 200 * apow(2716, (dist - size) * 1000 / size) / 1000));
    int g = max(0, min(255, 255 * apow(2716, (dist - size) * 500 / size) / 1000));
    int b = max(0, min(255, 200 * apow(2716, (dist - size) * 1000 / size) / 1000));

    return (r << 16) + (g << 8) + b;
    }

    void set_speed(uint32_t speed) { this->speed_ = speed; }
    void set_width(uint16_t width) { this->width_ = width; }
    void set_height(uint16_t height) { this->height_ = height; }
    void set_background(uint32_t background) { this->background_ = background; }
    void set_smallest_line(uint8_t smallest_line) { this->smallest_line_ = smallest_line; }
    void set_biggest_line(uint8_t biggest_line) { this->biggest_line_ = biggest_line; }

  protected:
    uint32_t background_{0x011902};
    uint32_t speed_{15};
    uint16_t width_{32};
    uint16_t height_{32};
    uint8_t smallest_line_{8};
    uint8_t biggest_line_{32};
    uint8_t line_iterator_{8};
  };
}
}
