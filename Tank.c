

#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"

// include CC65 NES Header (PPU)
#include <nes.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// BCD arithmetic support
#include "bcd.h"
//#link "bcd.c"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

extern const byte climbr_titles_pal[16];
extern const byte tankgames_rle[];


#define TILE 0xd8
#define ATTR 0x0




const unsigned char metasprite[]={
        0,      0,      TILE+0,   ATTR, 
        0,      8,      TILE+1,   ATTR, 
        8,      0,      TILE+2,   ATTR, 
        8,      8,      TILE+3,   ATTR, 
        128};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x02,0x02,0x02,0x02,	// background palette 0
  0x02,0x02,0x02,0x02,	// background palette 1
  0x02,0x02,0x02,0x02,	// background palette 2
  0x02,0x02,0x02,0x02,   // background palette 3

  0x16,0x35,0x02,0x00,	// sprite palette 0
  0x0B,0x30,0x0B,0x00,	// sprite palette 1
  0x0D,0x2D,0x3A,0x00,	// sprite palette 2
  0x0D,0x27,0x2A	// sprite palette 3
};

void fade_in() {
  byte vb;
  for (vb=0; vb<=4; vb++) {
    // set virtual bright value
    pal_bright(vb);
    // wait for 4/60 sec
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
    ppu_wait_frame();
  }
}

void show_title_screen(const byte* pal, const byte* rle) {
  // disable rendering
  ppu_off();
  // set palette, virtual bright to 0 (total black)
  pal_bg(pal);
  pal_bright(0);
  // unpack nametable into the VRAM
  vram_adr(0x2000);
  vram_unrle(rle);
  // enable rendering
  ppu_on_all();
  // fade in from black
  fade_in();
}

// setup PPU and tables
void setup_graphics() {
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
}
#define NUM_ACTORS 1

// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];
void main(void)
{
  byte i;
  setup_graphics();
  for (i=0; i<NUM_ACTORS; i++) {
    actor_x[i] = i*8;
    actor_y[i] = i*8;
    actor_dx[i] =  15;
    actor_dy[i] = 15;
  }
  // draw message  
  // enable rendering
   show_title_screen(climbr_titles_pal, tankgames_rle);
  // infinite loop
  while(1) {
   
    oam_off = 0;
    // draw and move all actors
    // (note we don't reset i each loop iteration)
    while (oam_off < 256-4*4) {
       
      // advance and wrap around actor array
      if (++i >= NUM_ACTORS)
        i -= NUM_ACTORS;
      // draw and move actor
      oam_meta_spr_pal(
        actor_x[3],	// add x+dx and pass param
        actor_y[1],	// add y+dy and pass param
        i&7,				// palette color
        metasprite);
  }
     oam_hide_rest(oam_off);
    // wait for next NMI
    // we don't want to skip frames b/c it makes flicker worse
    ppu_wait_nmi();
  }
}


//#link "tankgame.s"

