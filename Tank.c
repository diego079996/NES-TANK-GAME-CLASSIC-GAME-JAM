
//#link "tankgame.s"
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



#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+37,   pal, \
        0,      8,      (code)+37,   pal, \
        8,      0,      (code)+37,   pal, \
        8,      8,      (code)+37,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+37,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+37,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+37,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+37,   (pal)|OAM_FLIP_H, \
        128};


#define DEF_METASPRITE_TANK_H(name,code1,code2,flip)\
const unsigned char name[]={\
	0,0,(code1),(flip<<6),\
        8,0,(code2),(flip<<6),\
        0,8,(code1),(flip<<6)|OAM_FLIP_V,\
        8,8,(code2),(flip<<6)|OAM_FLIP_V,\
        128};

#define DEF_METASPRITE_TANK_V(name,code1,code2,flip)\
const unsigned char name[]={\
	0,8,(code1),(flip<<7),\
        0,0,(code2),(flip<<7),\
        8,8,(code1),(flip<<7)|OAM_FLIP_H,\
        8,0,(code2),(flip<<7)|OAM_FLIP_H,\
        128};



DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);
DEF_METASPRITE_2x2(playerRJump, 0xe8, 0);
DEF_METASPRITE_2x2(playerRClimb, 0xec, 0);
DEF_METASPRITE_2x2(playerRSad, 0xf0, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);
DEF_METASPRITE_2x2_FLIP(playerLJump, 0xe8, 0);
DEF_METASPRITE_2x2_FLIP(playerLClimb, 0xec, 0);
DEF_METASPRITE_2x2_FLIP(playerLSad, 0xf0, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);

const unsigned char* const playerRunSeq[16] = {
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2,
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2,
};

DEF_METASPRITE_TANK_H(tankSpriteR1,0xd8,0xd9,0);
DEF_METASPRITE_TANK_H(tankSpriteR2,0xda,0xdb,0);
DEF_METASPRITE_TANK_H(tankSpriteL1,0xd9,0xd8,1);
DEF_METASPRITE_TANK_H(tankSpriteL2,0xdb,0xda,1);

DEF_METASPRITE_TANK_V(tankSpriteT1,0xdc,0xdd,0);
DEF_METASPRITE_TANK_V(tankSpriteT2,0xde,0xdf,0);
DEF_METASPRITE_TANK_V(tankSpriteB1,0xdd,0xdc,1);
DEF_METASPRITE_TANK_V(tankSpriteB2,0xdf,0xde,1);

const unsigned char* const playerTankSeq[8] = {
  tankSpriteR1,tankSpriteR2,
  tankSpriteL1,tankSpriteL2,
  tankSpriteT1,tankSpriteT2,
  tankSpriteB1,tankSpriteB2
};



/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x11,0x30,0x27,0x0,	// background palette 0
  0x1c,0x20,0x2c,0x0,	// background palette 1
  0x00,0x10,0x20,0x0,	// background palette 2
  0x06,0x16,0x26,0x0,	// background palette 3

  0x16,0x35,0x24,0x0,	// sprite palette 0
  0x00,0x37,0x25,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x2a	// sprite palette 3
};



void show_title_screen(const byte* pal, const byte* rle) {
  // disable rendering
  byte vb;
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

// setup PPU and tables
void setup_graphics() {
  // clear sprites
  oam_hide_rest(0);
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}
#define NUM_ACTORS 2

// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];
void main()
{
  char i;	// actor index
  //char j;     // actor 2 index
  char oam_id;// sprite ID
  //char oam_id2;
  char pad;	// controller flags
  
  // print instructions

  // setup graphics
  setup_graphics();
   for (i=0; i<NUM_ACTORS; i++) {
    actor_x[i] = i*32+128;
    actor_y[i] = i*8+64;
    actor_dx[i] = 0;
    actor_dy[i] = 0;
  }
  // draw message  
  // enable rendering
   show_title_screen(climbr_titles_pal, tankgames_rle);
  // infinite loop
  while(1) {
   // start with OAMid/sprite 0
    oam_id = 0;
    // set player 0/1 velocity based on controller
    for (i=0; i<2; i++) {
      // poll controller i (0-1)
      pad = pad_poll(i);
      // move actor[i] left/right
      if (pad&PAD_LEFT && actor_x[i]>0) actor_dx[i]=-2;
      else if (pad&PAD_RIGHT && actor_x[i]<240) actor_dx[i]=2;
      else actor_dx[i]=0;
      // move actor[i] up/down
      if (pad&PAD_UP && actor_y[i]>0) actor_dy[i]=-2;
      else if (pad&PAD_DOWN && actor_y[i]<212) actor_dy[i]=2;
      else actor_dy[i]=0;
    }
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 6;
      if (actor_dx[i] >= 0)
        runseq += 1;
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerTankSeq[runseq]);
      actor_x[i] += actor_dx[i];
      actor_y[i] += actor_dy[i];
    }
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    ppu_wait_frame();
  }
}

