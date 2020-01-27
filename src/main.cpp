#include <Arduino.h>
#include <SmartMatrix3.h>
#include <FastLED.h>
#include "panel_wreath_mapping.h"
#include "effects.h"
#include <memory>

CRGB rectbuf[58][58] = {0};

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const int kMatrixWidth = 288;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 16;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_16ROW_MOD8SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels, or use SMARTMATRIX_HUB75_64ROW_MOD32SCAN for common 64x64 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
//SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

void project_rect_to_circ(void) {
  rgb24 *buffer = backgroundLayer.backBuffer();
  int32_t x_phys, y_phys;
  
  for (int i = 0; i < kMatrixWidth; i++) {
    for (int j = 0; j < kMatrixHeight; j++) {

      map_xy_to_physical_coord(i, j, &x_phys, &y_phys);

      // shift to first quadrant
      x_phys += 29<<16;
      y_phys += 29<<16;

      CRGB outcol;

      int32_t x_frac = x_phys & 0xFFFF;
      int32_t y_frac = y_phys & 0xFFFF;

      if ((x_frac > 0xFFFF / 2) && (y_frac > 0xFFFF / 2)) {
        outcol = rectbuf[(x_phys>>16) + 1][(y_phys>>16) + 1];
      } else if (x_frac > 0xFFFF / 2) {
        outcol = rectbuf[(x_phys>>16) + 1][y_phys>>16];
      } else if (y_frac > 0xFFFF / 2) {
        outcol = rectbuf[(x_phys>>16)][(y_phys>>16) + 1];
      } else {
        outcol = rectbuf[x_phys>>16][y_phys>>16];
      }

      //CRGB tempCRGB1 = blend(nearest[0],nearest[1], (x_phys & 0xFFFF) >> 8);
     // CRGB tempCRGB2 = blend(nearest[2],nearest[3], (x_phys & 0xFFFF) >> 8);
      //CRGB outcol = blend(tempCRGB1, tempCRGB2, (y_phys & 0xFFFF) >> 8);

      /*
      uint32_t r_accum = 0;
      uint32_t g_accum = 0;
      uint32_t b_accum = 0;
      
      for (int b = 0; b < 4; b++) {
        r_accum += nearest[b].r;
        g_accum += nearest[b].g;
        b_accum += nearest[b].b;
      }

      CRGB outcol;
      outcol.r = r_accum / 4;
      outcol.g = g_accum / 4;
      outcol.b = b_accum / 4;
      
*/
      buffer[kMatrixWidth*j + i] = outcol;

    }
  }
}

#define PHYS_W 58
#define PHYS_H 58

bool mask_xwipe_phys(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.phys_x < (int)(ratio-128)*PHYS_W*65536/256) != flip;
}

bool mask_ywipe_phys(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.phys_y < (int)(ratio-128)*PHYS_H*65536/256) != flip;
}

bool mask_xwipe_panels(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return ((pix.x % PANEL_W) < ratio*(PANEL_W+1)/256) != flip;
}

bool mask_ywipe_panels(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.y < (int)ratio*(PANEL_H+1)/256) != flip;
}

bool mask_glitter(uint8_t ratio, pixel_info pix, bool flip) {
  return (ratio >= random8(254));
}

bool mask_dissolve(uint8_t ratio, pixel_info pix, bool flip) {
  return (ratio > inoise8(pix.x*120,pix.y*120));
}

bool mask_circle(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (ratio > pix.r * 8 ) != flip;
}

bool mask_burn(uint8_t ratio, pixel_info pix, bool flip) {
  static uint32_t z = 0;
  z++; // Don't burn the same way each time
  return (ratio > scale8(inoise8(pix.x*16,pix.y*16+z/2048), 240) + 4);
}

bool mask_singlepanel(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.x / PANEL_W == (int)ratio*9/256);
}

bool mask_seqpanels(uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.x / PANEL_W <= ratio*9/256) != flip;
}

bool mask_chevron (uint8_t ratio, pixel_info pix, bool flip) {
  if (flip) ratio = 255 - ratio;
  return (pix.x % PANEL_W - pix.y + PANEL_W/2 <= ratio*(PANEL_W+16)/256) != flip;
}

std::function<bool(uint8_t, pixel_info, bool)> mask_funcs[] = {
  mask_xwipe_phys,
  mask_ywipe_phys,
  mask_xwipe_panels,
  mask_ywipe_panels,
  mask_glitter,
  mask_dissolve,
  mask_circle,
  mask_burn,
  mask_seqpanels,
  mask_chevron
};

std::function<bool(uint8_t, pixel_info, bool)> get_random_mask(void) {
  return mask_funcs[random8(10)];
}


static uint32_t z;
uint16_t speed = 5; // a nice starting speed, mixes well with a scale of 100
uint16_t scale = 3200;
uint16_t noise[kMatrixWidth][kMatrixHeight];

void setup() {
  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  //matrix.addLayer(&indexedLayer); 
  
  //Serial.begin(1000000);

  matrix.begin();
  matrix.setBrightness(192);
}

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  static uint8_t angle = 0;

  static int32_t xoff = 0;
  static int32_t yoff = 0;

  //xoff += (sin8(angle)-127) * 3;
  //yoff += (cos8(angle)-127) * 3;

  float precos = cos(angle*2*PI/256.0);
  float presin = sin(angle*2*PI/256.0);

  for(uint16_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {
      int32_t x_phys;
      int32_t y_phys;
      map_xy_to_physical_coord(i, j, &x_phys, &y_phys);


      int32_t new_x = (x_phys * precos) - (y_phys * presin);
      int32_t new_y = (y_phys * precos) + (x_phys * presin);
      int32_t dist = hypotf(new_x, new_y) / 65536;
      

      //noise[i][j] = inoise16((x_phys/15+xoff) & 0xFFFF8000,  (y_phys/15+yoff) & 0xFFFF8000, z);
      noise[i][j] = inoise16((new_x/dist+xoff),  (new_y/dist+yoff), z);
      if (dist > 20) noise[i][j] = 0;

    }
  }
  
  angle += 1;
  
  z += 40;
}

CRGB render_static(int32_t x, int32_t y, uint8_t panel, uint8_t density, CRGB color) {

  CRGB ret_color;

  if (x / PANEL_W == panel || panel == 255) {
    ret_color = (density > random8(255)) ? color : CRGB(0,0,0);
  } else {
    ret_color = 0;
  }

  return ret_color;

}


CRGB render_staticAll(int32_t x, int32_t y, bool nextframe) {
  if (nextframe) return 0;
  return render_static(x, y, 255, 50, CRGB::Blue);
}

/*
void staticChase(void) {
  static float angle = 0;
  static uint8_t density = 50;
  for (int i = 0; i < 9; i++) {
    CRGB color = ColorFromPalette(PlasmaColors_p, i*255/9+angle, 255);
    fillStatic(i, density, color);
  }

  angle += 1;

}
*/



void fillRect(void) {
  static float xcoord = 0;
  static bool flip = true;

    for (int j = 0; j < 58; j++) {
      for (int i = 0; i < 58; i++) {
        //rectbuf[j][i] = (i == (int)xcoord) ? CRGB::Blue : CRGB::Black;
        rectbuf[j][i] = (random8(20) == 0) ? CRGB::Blue : CRGB::Black;

      }
    }

  xcoord += .01;
  if (xcoord > 57) {
    xcoord = 0;
    flip = !flip;
  }

  project_rect_to_circ();

}

#define Sine8_p std::make_unique<Sine8>


//template<typename...Args>
//std::unique_ptr<Sine8> Sine8_p(Args&&... args)
//{
//  return std::make_unique<Sine8>(std::forward<Args>(args)...);
//}
//Mix mix_1(spiral1, spiral2, mask_burn, Sine8_p());

//auto mix_1 = std::make_shared<Mix>(plasma1, spiral2, mask_chevron, Sine8_p(2));
//auto blend_1 = std::make_shared<Blend>(plasma1, spiral2, Sine8_p(1));



//Mix mix_2(spiral3, mix_1, mask_dissolve, Sine8_p(-4));

std::shared_ptr<Renderer> effect_list[] = {
  std::make_shared<Spiral>(-2, 64, RainbowStripeColors_p),
  std::make_shared<Spiral>(xmas),
  std::make_shared<TheMatrix>(),
  std::make_shared<Spiral>(2, 16, xmas2),
  std::make_shared<Plasma>(xmas2),
  std::make_shared<Spiral>(-2, 32, bwstripeColors_p),
  std::make_shared<Plasma>(thematrix_p),
  std::make_shared<Inoise>(xmas2),
};

std::shared_ptr<Renderer> current_effect = effect_list[0];
std::shared_ptr<Renderer> next_effect = effect_list[1];
std::shared_ptr<Renderer> transition;

CRGBPalette16 * get_random_palette(void) {
  return palette_list[random8(8)];
}


std::shared_ptr<Renderer> generate_random_effect(void) {

  CRGBPalette16 * rand_palette = get_random_palette();

  switch(random(8)) {
  //switch(6) {
    case 0:
    case 1:

      return std::make_shared<Spiral>(random8(8)-4, random16(128)-64, *rand_palette);
    break;

    case 2:
    case 3:
      return std::make_shared<Plasma>(random8(8)-4, random16(2048), random16(64)+1, *rand_palette);
    break;

    case 4:
    case 5:
      return std::make_shared<Inoise>(random16(1024)-512,random16(1024)-512,random16(16)+2, random8(3)+1, *rand_palette);
    break;

    case 6:
      return std::make_shared<Wreath>();
    break;

    case 7:
      return std::make_shared<TheMatrix>();
    break;

    default:
      return std::make_shared<Spiral>(*rand_palette);
    break;
  }

}




bool inTransition = false;
void get_next_effect(void) {
   //while (next_effect == current_effect) next_effect = effect_list[random8(8)];
   next_effect = generate_random_effect();
}


void render_pixels(void) {
  CRGB *buffer = (CRGB *)backgroundLayer.backBuffer();
  
  CRGB pixel_color;
  pixel_info curr_pixel;
  
  // Iterate through pixel space
  for(int32_t x = 0; x < kMatrixWidth; x++) {
    for(int32_t y = 0; y < kMatrixHeight; y++) {

      // Calculate pixel info for use by rendering functions
      curr_pixel.x = x;
      curr_pixel.y = y;
      map_xy_to_physical_coord(x, y, &curr_pixel.phys_x, &curr_pixel.phys_y);
      curr_pixel.r = hypotf(((float)curr_pixel.phys_x)/65536, ((float)curr_pixel.phys_y)/65536);
      curr_pixel.angle = (atan2f(curr_pixel.phys_x, curr_pixel.phys_y) + PI);
      curr_pixel.color = buffer[kMatrixWidth*y + x];

      pixel_color = current_effect->render(curr_pixel);

      // Write final color to pixel space
      buffer[kMatrixWidth*y + x] = pixel_color;
    }
  }
  current_effect->update();
}

void checkEffect(uint32_t curr_ms) {
  static uint32_t change_ms = millis();
  if (inTransition) {
    if (current_effect->done) {
      current_effect.swap(next_effect);
      inTransition = false;
    }
    change_ms = curr_ms; // reset effect cycle timer
  } else if (curr_ms - change_ms > 45*1000) {
    get_next_effect();
    inTransition = true;

    // create new temporary renderer for transition
    std::shared_ptr<Renderer> temp_effect;
    current_effect.swap(temp_effect);
    switch(random(4)) {
      case 0:
      case 1:
      case 2: 
        current_effect = std::make_shared<Mix>(temp_effect, next_effect,
          get_random_mask(), SlowRamp8_p(1024, false));

        break;
      
      default:
        current_effect = std::make_shared<Blend>(temp_effect, next_effect,
          SlowRamp8_p(256, false));
        break;

    }

    current_effect->init();
    change_ms = curr_ms;
  }
}


void loop() {
  
  while(backgroundLayer.isSwapPending());

  uint32_t curr_ms = millis();
  static uint32_t draw_ms = millis();
  checkEffect(curr_ms);

  if (curr_ms - draw_ms > 30) {
    render_pixels();
    draw_ms = curr_ms;
  }


  backgroundLayer.swapBuffers(true);
}