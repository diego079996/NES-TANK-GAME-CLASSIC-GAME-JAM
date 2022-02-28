
#include "Tanktitles.h"

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

const unsigned char palTitle[16]={ 0x0f,0x08,0x1A,0x2A,0x0f,0x08,0x1a,0x2a,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };


extern const byte climbr_titles_pal[16];
extern const byte tankgames_rle[];

#define BLANK 0

//game uses 12:4 fixed point calculations for enemy movements

#define FP_BITS  4

//max size of the game map

#define MAP_WDT      16
#define MAP_WDT_BIT    4
#define MAP_HGT      13

//macro for calculating map offset from screen space, as
//the map size is smaller than screen to save some memory

#define MAP_ADR(x,y)  ((((y)-2)<<MAP_WDT_BIT)|(x))

//size of a map tile

#define TILE_SIZE    16
#define TILE_SIZE_BIT  4


#define DIR_NONE    0
#define DIR_LEFT    PAD_LEFT
#define DIR_RIGHT    PAD_RIGHT
#define DIR_UP      PAD_UP
#define DIR_DOWN    PAD_DOWN



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

void clrscr() {
  vrambuf_clear();
  ppu_off();
  vram_adr(NAMETABLE_A);
  vram_fill(BLANK, 32*28);
  vram_adr(0x0);
  ppu_on_all();
}




DEF_METASPRITE_TANK_H(tankSpriteR1,0xd8,0xd9,0);
DEF_METASPRITE_TANK_H(tankSpriteR2,0xda,0xdb,0);
DEF_METASPRITE_TANK_H(tankSpriteL1,0xd9,0xd8,1);
DEF_METASPRITE_TANK_H(tankSpriteL2,0xdb,0xda,1);

DEF_METASPRITE_TANK_V(tankSpriteT1,0xdc,0xdd,0);
DEF_METASPRITE_TANK_V(tankSpriteT2,0xde,0xdf,0);
DEF_METASPRITE_TANK_V(tankSpriteB1,0xdd,0xdc,1);
DEF_METASPRITE_TANK_V(tankSpriteB2,0xdf,0xde,1);

const unsigned char* const playerTankSeq[12] = {
  tankSpriteB1,tankSpriteB2,
  tankSpriteL1,tankSpriteL2,
  tankSpriteT1,tankSpriteT2,
  tankSpriteR1,tankSpriteR2
};

const unsigned char sprPlayer[]={
  0,-1,0x49,0,
  8,-1,0x4a,0,
  0, 7,0x4b,0,
  8, 7,0x4c,0,
  128
};

const unsigned char sprEnemy1[]={
  0,-1,0x4d,1,
  8,-1,0x4e,1,
  0, 7,0x4f,1,
  8, 7,0x50,1,
  128
};
static int i;
static unsigned char wait;
static int iy,dy;

static unsigned char frame_cnt;
static unsigned char bright;



const unsigned char* const sprListPlayer[]={ sprPlayer,sprEnemy1};


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


void pal_fade_to(unsigned to)
{
  if(!to) {}

  while(bright!=to)
  {
    delay(4);
    if(bright<to) ++bright; else --bright;
    pal_bright(bright);
  }

  if(!bright)
  {
    ppu_off();
    set_vram_update(NULL);
    scroll(0,0);
  }
}

void title_screen(void)
{
  scroll(-8,240);//title is aligned to the color attributes, so shift it a bit to the right

  vram_adr(NAMETABLE_A);
  vram_unrle(Tanktitles);

  vram_adr(NAMETABLE_C);//clear second nametable, as it is visible in the jumping effect
  vram_fill(0,1024);

  pal_bg(palTitle);
  pal_bright(4);
  ppu_on_bg();
  delay(20);//delay just to make it look better

  iy=240<<FP_BITS;
  dy=-8<<FP_BITS;
  frame_cnt=0;
  wait=160;
  bright= 4;

  while(1)
  {
    ppu_wait_frame();

    scroll(0,iy>>FP_BITS);

    if(pad_trigger(0)&PAD_START) break;

    iy+=dy;

    if(iy<0)
    {
      iy=0;
      dy=-dy>>1;
    }

    if(dy>(-8<<FP_BITS)) dy-=2;

    if(wait)
    {
      --wait;
    }
    else
    {
      pal_col(2,(frame_cnt&32)?0x09:0x1B);//blinking press start text
      ++frame_cnt;
    }
  }

  scroll(-8,0);//if start is pressed, show the title at whole
  //sfx_play(SFX_START,0);

  for(i=0;i<16;++i)//and blink the text faster
  {
    pal_col(2,i&1?0x0f:0x20);
    delay(4);
  }

  pal_fade_to(0);
 
}




void show_game_screen(const byte* pal, const byte* rle) {
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
  char oam_id;// sprite ID
  char pad;	// controller flags
  
  // print instructions

 
   for (i=0; i<NUM_ACTORS; i++) {
    actor_x[i] = i*98;
    actor_y[i] = i*64;
    actor_dx[i] = 0;
    actor_dy[i] = 0;
  }
  title_screen();
 
  setup_graphics();
  // draw message  
  // enable rendering
   show_game_screen(climbr_titles_pal, tankgames_rle);
  // infinite loop
  while(1) {
   // start with OAMid/sprite 0
    oam_id = 0;
    // set player 0/1 velocity based on controller
    for (i=0; i<2; i++) {
      // poll controller i (0-1)
      pad = pad_poll(i);
      // move actor[i] left/right
      if(pad&PAD_LEFT && pad&PAD_DOWN){
        actor_dx[i]=0;
        actor_dy[i]=0;}
      if(pad&PAD_RIGHT && pad&PAD_DOWN){
        actor_dx[i]=0;
        actor_dy[i]=0;}
      if (pad&PAD_LEFT && actor_x[i]>32){
        
        if(pad&PAD_LEFT && actor_y[i]<61 && actor_y[i]>32) actor_dx[i]=0;
        else if(pad&PAD_LEFT && actor_y[i]<93 && actor_y[i]>65) actor_dx[i]=0;
        else if(pad&PAD_LEFT && actor_y[i]<125 && actor_y[i]>98) actor_dx[i]=0;
        else if(pad&PAD_LEFT && actor_y[i]<157 && actor_y[i]>128) actor_dx[i]=0;
        else if(pad&PAD_LEFT && actor_y[i]<189 && actor_y[i]>158) actor_dx[i]=0;
        else actor_dx[i]=-1;
      }
      else if (pad&PAD_RIGHT && actor_x[i]<207){
        if(pad&PAD_RIGHT && actor_y[i]<61 && actor_y[i]>32)actor_dx[i]=0;
        else if(pad&PAD_RIGHT && actor_y[i]<93 && actor_y[i]>63) actor_dx[i]=0;
        else if(pad&PAD_RIGHT && actor_y[i]<125 && actor_y[i]>98) actor_dx[i]=0;
        else if(pad&PAD_RIGHT && actor_y[i]<157 && actor_y[i]>125) actor_dx[i]=0;
        else if(pad&PAD_RIGHT && actor_y[i]<190 && actor_y[i]>158) actor_dx[i]=0;
        else actor_dx[i]=1;
      }
      else actor_dx[i]=0;
      // move actor[i] up/down
      if (pad&PAD_UP && actor_y[i]>30){
        if(pad&PAD_UP && actor_x[i]<206 && actor_x[i]>34)actor_dy[i]=0;
        else actor_dy[i]=-1;
      }
      else if (pad&PAD_DOWN && actor_y[i]<190){
        if(pad&PAD_DOWN && actor_x[i]<206 && actor_x[i]>34)actor_dy[i]=0;
        else actor_dy[i]=1;
      }
      else actor_dy[i]=0;
    }
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 4;
      if (actor_dx[i] >= 0)
        runseq += 2;
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


