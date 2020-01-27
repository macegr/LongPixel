#include "panel_wreath_mapping.h"
#include <FastLED.h>
#include <memory>

CRGBPalette16 xmas = CRGBPalette16(
  CRGB(255,0,0), CRGB(255,0,0),
  CRGB(255,0,0), CRGB(255,0,0),
  CRGB(255,0,0), CRGB(255,0,0),
  CRGB(0,0,0), CRGB(0,0,0),
  CRGB(0,255,0), CRGB(0,255,0),
  CRGB(0,255,0), CRGB(0,255,0),
  CRGB(0,255,0), CRGB(0,255,0),
  CRGB(0,0,0), CRGB(0,0,0));

  CRGBPalette16 wreath_p = CRGBPalette16(
  CRGB(255,0,0), CRGB(255,0,0),
  CRGB(0,0,0), CRGB(0,120,0),
  CRGB(0,190,0), CRGB(0,92,0),
  CRGB(0,56,0), CRGB(0,48,0),
  CRGB(0,128,0), CRGB(0,192,0),
  CRGB(0,64,0), CRGB(0,72,0),
  CRGB(0,16,0), CRGB(0,32,0),
  CRGB(0,130,0), CRGB(0,0,0));

CRGBPalette16 xmas2 = CRGBPalette16(
  CRGB(0,0,0), CRGB(255,0,0),
  CRGB(255,0,0), CRGB(255,0,0),
  CRGB(0,0,0), CRGB(255,255,255),
  CRGB(255,255,255), CRGB(255,255,255),
  CRGB(0,0,0), CRGB(0,255,0),
  CRGB(0,255,0), CRGB(0,255,0),
  CRGB(0,0,0), CRGB(255,255,255),
  CRGB(255,255,255), CRGB(255,255,255));

CRGBPalette16 PlasmaColors_p = CRGBPalette16(
  CRGB(200,100,0),
  CRGB(200,0,255),
  CRGB(100,0,50),
  CRGB(0,0,0)
);

CRGBPalette16 SnowColors_p = CRGBPalette16(
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0)
);

CRGBPalette16 bwstripeColors_p = CRGBPalette16(
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255),
  CRGB(0,0,0),
  CRGB(255,255,255)
);

CRGBPalette16 thematrix_p = CRGBPalette16(

  CRGB(0,200,0),
  CRGB(0,210,0),
  CRGB(20,216,0),
  //CRGB(140,255,120),
  CRGB(255,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,0,0),
  CRGB(0,64,0),
  CRGB(0,64,0),
  CRGB(0,96,0),
  CRGB(0,96,0),
  CRGB(0,128,0),
  CRGB(0,160,0),
  CRGB(0,192,0)
);

CRGBPalette16 * palette_list[] = {
  &xmas,
  &xmas2,
  &PlasmaColors_p,
  &bwstripeColors_p,
  &SnowColors_p,
  (new CRGBPalette16(RainbowStripeColors_p)),
  (new CRGBPalette16(CloudColors_p)),
  (new CRGBPalette16(PartyColors_p))
};


typedef struct {
  int32_t x;
  int32_t y;
  int32_t phys_x;
  int32_t phys_y;
  float r;
  float angle;
  CRGB color;
} pixel_info;

class Mask {
  public:
    int32_t incr_ratio = 65536;
    virtual void init() {};
    virtual bool get();
};

class Driver {
  public:
    virtual ~Driver() {};
    virtual void init(void);
    virtual int32_t get(void);
    bool done = false;
};

#define Sine8_p std::make_unique<Sine8>
class Sine8 final : public Driver {
  public:
    uint8_t cnt;
    int8_t incr;

    Sine8(int8_t incr = 1) : incr(incr) {
      init();
    }

    void init(void) {
      cnt = 0;
    }

    int32_t get(void) {
      int32_t retval = sin8(cnt);
      cnt += incr;
      return retval;
    }
};

#define Ramp8_p std::make_unique<Ramp8>
class Ramp8 : public Driver {
  public:
    uint8_t cnt;
    uint8_t incr;

    Ramp8(uint8_t incr = 1) : incr(incr) {
      init();
    }

    void init(void) {
      cnt = 0;
    }

    int32_t get(void) {
      int32_t retval = cnt;
      cnt += incr;
      return retval;
    }
};

#define SlowRamp8_p std::make_unique<SlowRamp8>
class SlowRamp8 : public Driver {
  public:
    uint16_t cnt;
    uint16_t incr;
    bool repeat;

    SlowRamp8(uint16_t incr = 1, bool repeat = true) :
        incr(incr), repeat(repeat) {
        init();
    }

    void init(void) {
      cnt = 0;
      done = false;
    }

    int32_t get(void) {
      int32_t retval = cnt;
      cnt += incr;
      if (!repeat && incr > 0 && cnt < retval ) { // overflow
        cnt = UINT16_MAX; // saturate
        done = true;
      } else if (!repeat && incr < 0 && cnt > retval) { // underflow
        cnt = 0; // saturate
        done = true;
      }
      return retval/256;
    }

};

class Renderer {
  public:
    virtual void init(void);
    virtual CRGB render(pixel_info);
    virtual void update(void);
    virtual ~Renderer(){};
    bool done = false;
};

class Inoise : public Renderer {
  public:
    int32_t xoffset, yoffset, zoffset;
    int32_t xoffset_incr, yoffset_incr, scale;

    uint8_t hue = 0;
    int8_t hue_incr;
    CRGBPalette16 palette;

    bool halfrender = false;
  
    Inoise(CRGBPalette16 palette) : palette(palette) {
        xoffset_incr = 0;
        yoffset_incr = 0;
        scale = 2048;
        hue_incr = 1;
        init();
    }
    
    Inoise(int32_t xoffset_incr = 0, int32_t yoffset_incr = 0, int32_t scale = 2048, int8_t hue_incr = 1,
      CRGBPalette16 palette = CRGBPalette16(RainbowStripeColors_p)) :
      xoffset_incr(xoffset_incr), yoffset_incr(yoffset_incr), scale(scale), hue_incr(hue_incr), 
      palette(palette) {
        init();
    }

    ~Inoise() {
    }

    void init(void) {
        //xoffset = 0;
        //yoffset = 0;
        //zoffset = 0;
    }

    void update(void) {
      hue += hue_incr;
      xoffset += xoffset_incr;
      yoffset += yoffset_incr;
    }

    CRGB render(pixel_info pix) {
      // inoise is CPU heavy, only render every other frame


      uint8_t color = inoise16(pix.phys_x/scale+xoffset, pix.phys_y/scale+yoffset)/256;
      CRGB ret_color;
      ret_color = ColorFromPalette(palette, color+hue+pix.r*5, 255);
      return ret_color;
    }

    
};

class Spiral : public Renderer {
  public:
    uint16_t angle;
    uint8_t hue;
    int8_t hue_incr;
    int16_t angle_incr;
    CRGBPalette16 palette;
  
    Spiral(CRGBPalette16 palette) : palette(palette) {
        hue_incr = -2;
        angle_incr = 32;
        init();
    }
    
    Spiral(int8_t hue_incr = -2, int16_t angle_incr = 32, CRGBPalette16 palette = CRGBPalette16(RainbowStripeColors_p)) :
      hue_incr(hue_incr), angle_incr(angle_incr), palette(palette) {
        init();
    }

    ~Spiral() {
    }

    void init(void) {
        angle = 0;
        hue = 0;
    }

    void update(void) {
      hue += hue_incr;
      angle += angle_incr;
    }

    CRGB render(pixel_info pix) {
      int8_t color = (int32_t)(pix.r/12*(int32_t)(sin16(angle))/256) + pix.angle*256/PI*1;
      CRGB ret_color;
      ret_color = ColorFromPalette(palette, color+hue, 255);
      return ret_color;
    }

    
};

class Plasma : public Renderer {
    public:
        uint16_t angle;
        uint8_t hue;
        uint16_t lobe;
        int8_t hue_incr;
        int16_t angle_incr;
        int16_t lobe_incr;
        CRGBPalette16 palette;

        Plasma(int8_t hue_incr = -2, int16_t angle_incr = 1024, int16_t lobe_incr = 32,
            CRGBPalette16 palette = CRGBPalette16(RainbowColors_p)) :
            hue_incr(hue_incr), angle_incr(angle_incr), lobe_incr(lobe_incr), palette(palette) {
                init();
        }

        ~Plasma() {
        }

        Plasma(CRGBPalette16 palette) : palette(palette) {
            hue_incr = -2;
            angle_incr = 1024;
            lobe_incr = 32;
            init();
        }

        void init(void) {
            angle = 0;
            hue = 0;
            lobe = 0;
        }

        void update(void) {
            hue += hue_incr;
            angle += angle_incr;
            lobe += lobe_incr;
        }

        CRGB render(pixel_info pix) {
            int8_t color = (pix.r)*12 + (int32_t)(((sin16(angle+(pix.angle+PI)*65536/(2*PI)*9)-16383)/16) * (sin16(lobe)-32767)) / (2<<20);
            CRGB ret_color;
            ret_color = ColorFromPalette(palette, color+hue, 255);
            return ret_color;
        }
};

class TheMatrix : public Renderer {
    public:
        uint16_t inc;

        TheMatrix() {
            init();
        }

        CRGB render(pixel_info pix) {
            CRGB ret_color;
            ret_color = ColorFromPalette(thematrix_p,inoise8(pix.x*1800,pix.y*30+inc*(sin8(pix.x*60)/24+2)), 255);
            return ret_color;
        }

        void init(void) {
            inc = 0;
        }

        void update(void) {
            inc += 1;
        }

};

class Wreath : public Renderer {
    public:
        uint16_t angle;
        uint8_t hue;
        float pre_cos;
        float pre_sin;

        Wreath() {
            init();
        }

        CRGB render(pixel_info pix) {
            CRGB ret_color;
            int32_t new_x = (pix.phys_x * pre_cos) - (pix.phys_y * pre_sin);
            int32_t new_y = (pix.phys_y * pre_cos) + (pix.phys_x * pre_sin);
            ret_color = ColorFromPalette(wreath_p,inoise8((new_x & ~0x3FFFF)/3048, (new_y & ~0x3FFFF)/3048)*2+hue, 255);
            return ret_color;
        }

        void init(void) {
            angle = 0;
            hue = 0;
            pre_cos = cos(angle*2*PI/65536.0);
            pre_sin = sin(angle*2*PI/65536.0);
        }

        void update(void) {
            angle += 200;
            hue +=1;
            pre_cos = cos(angle*2*PI/65536.0);
            pre_sin = sin(angle*2*PI/65536.0);
        }

};

class Mix : public Renderer {

  public:
    std::shared_ptr<Renderer> render_1;
    std::shared_ptr<Renderer> render_2;
    std::function<bool(uint8_t, pixel_info, bool)> mask_func;
    std::unique_ptr<Driver> driver;
    uint8_t mix;
    bool flip = false;

    Mix(std::shared_ptr<Renderer> render_1,
        std::shared_ptr<Renderer> render_2,
        std::function<bool(uint8_t, pixel_info, bool)> mask_func,
        std::unique_ptr<Driver> driver) :
      render_1(render_1), render_2(render_2), mask_func(mask_func), driver(std::move(driver)) {
      }

    ~Mix() {
      Serial.println("Deleted Mix");
    }

    CRGB render(pixel_info curr_pix) {    
        CRGB mix_color;
                
        // Default to first render function if mask function blank for some reason
        bool mask = true;
        if (mask_func) mask = mask_func(mix, curr_pix, flip);
        
        // Choose which pattern to render at the current pixel
        if (!mask) {
          mix_color = render_1->render(curr_pix);
        } else {
          mix_color = render_2->render(curr_pix);
        }
        
        return mix_color;
    }

    void update(void) {
      render_1->update();
      render_2->update();
      mix = driver->get();
      done = driver->done;
    }

    void init(void) {
        done = false;
        driver->init();
        mix = 0;
        flip = (random8(2) == 0);
    }

};

class Blend : public Renderer {

  public:
    std::shared_ptr<Renderer> render_1;
    std::shared_ptr<Renderer> render_2;
    std::unique_ptr<Driver> driver;
    uint8_t blend_r;
    bool halfrender = false;

    Blend(std::shared_ptr<Renderer> render_1,
        std::shared_ptr<Renderer> render_2,
        std::unique_ptr<Driver> driver) :
      render_1(render_1), render_2(render_2), driver(std::move(driver)) {
      }

    ~Blend() {
            Serial.println("Deleted Blend");

    }

    CRGB render(pixel_info curr_pix) {
        CRGB blend_color;
        CRGB temp_blend_1 = render_1->render(curr_pix);
        CRGB temp_blend_2 = render_2->render(curr_pix);
        blend_color = blend(temp_blend_1, temp_blend_2, blend_r);
        return blend_color;
    }

    void update(void) {
      render_1->update();
      render_2->update();
      blend_r = driver->get();
      done = driver->done;
    }

    void init(void) {
        done = false;
        blend_r = 0;
        driver->init();
    }

};