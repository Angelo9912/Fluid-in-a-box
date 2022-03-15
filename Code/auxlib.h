//----------------------------------------------------------------------
//----------------AUSILIARY LIBRARY HEADER FILE-------------------------
//-------------------PROJECT: SAND IN A BOX-----------------------------
//--------------TINFENA MATTIA & MASSARA ANGELO-------------------------
//----------------------------------------------------------------------

#ifndef AUXLIB_H
#define AUXLIB_H

//----------------------------------------------------------------------
//----------------------NEEDED LIBRARIES--------------------------------
//----------------------------------------------------------------------

#include <allegro.h>

//----------------------------------------------------------------------
//---------------------GLOBAL COSTANTS----------------------------------
//----------------------------------------------------------------------

#define sand      0
#define water     1

#define MAXELEM   768                         // Maximum number of the elements

#define up        0
#define down      1
#define left      2
#define right     3

//----------------------------------------------------------------------
//--------------------STRUCTURES DECLARATION----------------------------
//----------------------------------------------------------------------

struct fluid
{
  float r;
  float c;
  float vr;
  float vc;
};

//----------------------------------------------------------------------
//---------------------GLOBAL VARIABLES---------------------------------
//----------------------------------------------------------------------

extern struct fluid   elem[];
extern float          g;
extern int            width, height, color, fluid_type, start;
extern float          scale;
extern BITMAP         *buffer;
extern float           TSCALE;
extern int             end;

//----------------------------------------------------------------------
//------------------GENERIC FUNCTIONS DECLARATION-----------------------
//----------------------------------------------------------------------

extern char get_scancode();

//----------------------------------------------------------------------
//------------------ALLEGRO FUNCTIONS DECLARATION-----------------------
//----------------------------------------------------------------------

extern void start_allegro(void);
extern void draw_balls(void);
extern void draw_gravity(void);
extern void elem_init(void);
extern void elem_reset(void);
extern void elem_create(void);
extern void mouse_func(void);
extern void compute_position(void);

#endif

