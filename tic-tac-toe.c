#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

byte empty_sprite = 0x2d; // -
byte cursor_sprite = 0x15; // heart, $7E is ~

byte oam_id = 0;
byte pad = 0;

// in which row and column is the cursorf
byte row = 0;
byte column = 0;

// x and y values for moving the cursor
byte x_values[3] = {24, 56, 88};
byte y_values[3] = {46, 70, 94};

// game field data
byte data[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
byte data_idx = 0;
byte data_at_idx = 0;
byte data_to_write = 0;

// formular for row, column to index would be:
// 3 * row + column, but we don't use multiplication
// so hard code the values
// so row 1, column 1 has index 4
byte row_offset[3] = {0, 3, 6};

// nametable rows and columns for drawing the field
byte ntadr_rows[3] = {6, 9, 12};
byte ntadr_columns[3] = {3, 7, 11};

// symbold for X and O
byte x = 0x58;
byte o = 0x4f;

// -, x, o
byte values[3] = {0x2d, 0x58, 0x4f};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x11,0x30,0x27,0x0,	// background palette 0
  0x1c,0x20,0x2c,0x0,	// background palette 1
  0x00,0x10,0x20,0x0,	// background palette 2
  0x06,0x16,0x26,0x0,   // background palette 3

  0x16,0x35,0x24,0x0,	// sprite palette 0
  0x00,0x37,0x25,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x2a	// sprite palette 3
};

// setup PPU and tables
void setup_graphics() {
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
}

void inital_board() {
  byte _row = 0;
  byte _column = 0;
  
  for (_row = 0; _row < 3; ++_row) {
    for (_column = 0; _column < 3; ++_column) {
	vram_adr(NTADR_A(ntadr_columns[_column], ntadr_rows[_row]));
      	vram_put(empty_sprite);
    }	
  }
}

void update_data_idx() {
    data_idx = row_offset[row] + column;
}

void control_cursor() {
    pad = pad_trigger(0);
    
    if (pad & PAD_LEFT) {
    	if (column > 0) {
            --column;
        }
    }
    
    if (pad & PAD_RIGHT) {
    	if (column < 2) {
            ++column;
        }
    }
    
    if (pad & PAD_UP) {
    	if (row > 0) {
            --row;
        }
    }
    
    if (pad & PAD_DOWN) {
    	if (row < 2) {
            ++row;
        }
    }
  
    update_data_idx();
    data_at_idx = data[data_idx];
  
    // if field is occupied display a different cursor
    if (data_at_idx == 0) {
    	cursor_sprite = 0x15;
    } else {
    	cursor_sprite = 0x7e;
    }
  
    if (pad & PAD_A) {
      // turning ppu off and on works, but flickers
      // writing during ppu is busy with write looks funny
      // without ppu_wait_frame nothing was drawn?!
      
      //ppu_off();
      //vram_adr(NTADR_A(2,4));
      //vram_write("TIC TAC TOE", 12);

      // get data
      if (data_at_idx == 0) {
      	data_to_write = values[1]; // write x
        data[data_idx] = 1;
      } else {
      	data_to_write = values[0]; // write -
        data[data_idx] = 0;
      }
      
      vrambuf_put(NTADR_A(ntadr_columns[column], ntadr_rows[row]), &data_to_write, 1);
      //scroll(0, 0);
      // and this was also important
      ppu_wait_frame();
      
      // THIS WAS IMPORTANT?!
      vrambuf_clear();
      
      // maybe scrolling too, read about that in the book?!
      scroll(0, 0);
      //ppu_on_all();
    }
}

void main(void)
{
  setup_graphics();
  // clear vram buffer
  vrambuf_clear();
  
  // draw message - TIC TAC TOE
  // column, row
  vram_adr(NTADR_A(2,2));
  vram_write("TIC TAC TOE", 12);
  
  inital_board();
  
  set_vram_update(updbuf);
  
  // enable rendering
  ppu_on_all();
  
  // infinite loop
  while(1) {

    control_cursor();
    
    oam_id = oam_spr(x_values[column], y_values[row], cursor_sprite, 1, oam_id);
    
    if (oam_id!=0) oam_hide_rest(oam_id);
  }
}
