//----------------------------------------------------------------------
//-----------------------AUSILIARY LIBRARY------------------------------
//-------------------PROJECT: FLUID IN A BOX----------------------------
//---------------TINFENA MATTIA & MASSARA ANGELO------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// This library contains the functions to 
// handle the GUI and the movement of the fluid 
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--------------------------LIBRARIES-----------------------------------
//----------------------------------------------------------------------

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <allegro.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>
#include "auxlib.h"
#include "ptask.h"
#include "hwlib.h"

//----------------------------------------------------------------------
//---------------------ALLEGRO COSTANTS---------------------------------
//----------------------------------------------------------------------

// Box coordinates

#define XBOXL     224
#define YBOXL     812
#define XBOXH     866
#define YBOXH     330

#define R         10                           // Ball's radius

//----------------------------------------------------------------------
//---------------------GLOBAL VARIABLES---------------------------------
//----------------------------------------------------------------------

int               start = 0;                   // Play/pause flag
int               end = 0;                     // End program flag

struct fluid      elem[MAXELEM];               // Array of elements 

int               width = 1680, height = 960;  // Dimensions of GUI windows

int               color = 64912;               // Fluid color
int               fluid_type = sand;           // Type of the fluid

//------------------------------------------------------
// Bitmaps for the GUI and the arrow that indicate the
// gravity direction
//-----------------------------------------------------

BITMAP            *buffer;
BITMAP            *arrowup;
BITMAP            *arrowdown;
BITMAP            *arrowleft;
BITMAP            *arrowright; 

float             g = 9.81;                   // Gravity
int               NELEM = 0;                  // Number of elements
float             cnext, rnext;               // Next position
int               dir_prec = down;            // Precedent direction of the gravity
int               dir;                        // Actual direction of the gravity
float             scale = 6;                  // Coefficient to change the speed of the simulation
float             TSCALE = 1.5;               // Coefficient to change the integration interval

//----------------------------------------------------------------------
//--------------------GENERIC FUNCTIONS---------------------------------
//----------------------------------------------------------------------

char get_scancode(void)
{
  // This function retourns the key pressed

  if(keypressed())
    return readkey() >> 8;
  else return 0;
}

static int irand(int max)
{
  // This function retourns a random integer number between 0 and max

  int res;
  res = rand() % (max);
  return res;
}

//----------------------------------------------------------------------
//-------------------------ALLEGRO FUNCTIONS----------------------------
//----------------------------------------------------------------------

void start_allegro(void)
{

  // Initialize allegro library and set the GUI

  allegro_init();
  install_keyboard();
  install_mouse();

  set_color_depth(16);                      //number of bits used for color 
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1680, 960, 0, 0);
  
  Mask_reset();
  
  buffer = load_bitmap("GUI.bmp", NULL);
  if (buffer == NULL) {
    printf("GUI not found\n");
    exit(1);
  }
      
  arrowup = create_bitmap(180,180);
  clear_bitmap(arrowup);
  arrowup = load_bitmap("arrowup.bmp", NULL);
  
  if (arrowup == NULL) 
  {
  printf("arrowup not found\n");
  exit(1);
  }
  
  arrowdown = create_bitmap(180,180);
  clear_bitmap(arrowdown);
  arrowdown = load_bitmap("arrowdown.bmp", NULL);
  
  if (arrowdown == NULL) 
  {
  printf("arrowdown not found\n");
  exit(1);
  }
  
  arrowright = create_bitmap(180,180);
  clear_bitmap(arrowright);
  arrowright = load_bitmap("arrowright.bmp", NULL);
  
  if (arrowright == NULL) 
  {
  printf("arrowright not found\n");
  exit(1);
  }
  
  arrowleft = create_bitmap(180,180);
  clear_bitmap(arrowleft);
  arrowleft = load_bitmap("arrowleft.bmp", NULL);
  
  if (arrowleft == NULL) 
  {
  printf("arrowleft not found\n");
  exit(1);
  }
  
  rect(buffer, XBOXL, YBOXL, XBOXH, YBOXH, makecol(255,255,255));
  blit(buffer, screen, 0, 0, XBOXL, YBOXL, buffer->w, buffer->h);

}

void draw_balls(void)
{

  //--------------------------------------------------
  // Read the matrix mask and draw the balls on the 
  // correspondent position of the screen
  //--------------------------------------------------

  int x, y;

  BITMAP *bmp_balls;
  bmp_balls = create_bitmap(640, 480);
  clear_bitmap(bmp_balls);
  clear_to_color(bmp_balls, makecol(21,23,36));

  for (int i = 0; i < mask_rig; i++)
  {
    for(int j = 0; j < mask_col; j++)
      {
        if (matrix_mask[i][j][0] == 1)
        {
          x = (20 * (0.5 + j));
          y = 480 - 1 - (20 * (0.5 + i));
          circlefill(bmp_balls, x, y, R, color);
        }
      }
  }
  blit(bmp_balls, buffer, 0, 0, XBOXL + 1, YBOXH + 1, bmp_balls->w, bmp_balls->h);
  destroy_bitmap(bmp_balls);
}


void draw_gravity(void)
{

  //---------------------------------------------
  // Draw an arrow to show the gravity direction.
  // The arrow dimension varies according to 
  // theta & phi values
  //---------------------------------------------

  float ascale;
  int ycentro= 928;   
  int xcentro= 1504;  
  int larghezza;
  int altezza;
  float incertezza = 10 * PiGreco / 180;

  if (start && (theta>incertezza || phi>incertezza || theta<-incertezza || phi<-incertezza))
  {
      circlefill(buffer,1414,833,89,makecol(21, 23, 36));
      switch (dir)
    {
      case down:
          ascale = (PiGreco / 2) / (-theta);
          larghezza = 180 / ascale;
          altezza = 180 / ascale;
          stretch_sprite(buffer, arrowdown,xcentro-(180/2)-(larghezza/2), ycentro-(180/2)-(altezza/2), 180/ascale, 180/ascale);
            break;
          
          
      case up:
            ascale = (PiGreco / 2) / theta;
            larghezza = 180 / ascale;
            altezza = 180 / ascale;
            stretch_sprite(buffer, arrowup,xcentro-(180/2)-(larghezza/2), ycentro-(180/2)-(altezza/2), 180/ascale, 180/ascale);
            break;
      
      case left:
          ascale = (PiGreco / 2) / (-phi);
          larghezza = 180 / ascale;
           altezza = 180 / ascale;
            stretch_sprite(buffer, arrowleft,xcentro-(180/2)-(larghezza/2), ycentro-(180/2)-(altezza/2), 180/ascale, 180/ascale);
            break;
          
      case right:
            ascale = (PiGreco / 2) / phi;
            larghezza = 180 / ascale;
           altezza = 180 / ascale;
            stretch_sprite(buffer, arrowright,xcentro-(180/2)-(larghezza/2), ycentro-(180/2)-(altezza/2), 180/ascale, 180/ascale);
            break;
      }      
  }

}

//----------------------------------------------------------------------
//---------------------PHYSICS FUNCTIONS--------------------------------
//----------------------------------------------------------------------

void elem_init(void)
{
  // Initializate the balls

  Mask_reset();
  
  matrix_mask[1][5][0] = 1;
  matrix_mask[1][6][0] = 1;
  matrix_mask[1][7][0] = 1;
  matrix_mask[1][8][0] = 1;
  matrix_mask[1][9][0] = 1;
  matrix_mask[1][10][0] = 1;
  matrix_mask[1][15][0] = 1;
  matrix_mask[1][16][0] = 1;
  matrix_mask[1][17][0] = 1;
  matrix_mask[1][21][0] = 1;
  matrix_mask[1][26][0] = 1;

  matrix_mask[2][5][0] = 1;
  matrix_mask[2][11][0] = 1;
  matrix_mask[2][14][0] = 1;
  matrix_mask[2][18][0] = 1;
  matrix_mask[2][22][0] = 1;
  matrix_mask[2][25][0] = 1;

  matrix_mask[3][5][0] = 1;
  matrix_mask[3][11][0] = 1;
  matrix_mask[3][13][0] = 1;
  matrix_mask[3][19][0] = 1;
  matrix_mask[3][23][0] = 1;
  matrix_mask[3][24][0] = 1;

  matrix_mask[4][5][0] = 1;
  matrix_mask[4][6][0] = 1;
  matrix_mask[4][7][0] = 1;
  matrix_mask[4][8][0] = 1;
  matrix_mask[4][9][0] = 1;
  matrix_mask[4][10][0] = 1;
  matrix_mask[4][11][0] = 1;
  matrix_mask[4][13][0] = 1;
  matrix_mask[4][19][0] = 1;
  matrix_mask[4][23][0] = 1;
  matrix_mask[4][24][0] = 1;

  matrix_mask[5][5][0] = 1;
  matrix_mask[5][11][0] = 1;
  matrix_mask[5][14][0] = 1;
  matrix_mask[5][18][0] = 1;
  matrix_mask[5][22][0] = 1;
  matrix_mask[5][25][0] = 1;

  matrix_mask[6][5][0] = 1;
  matrix_mask[6][6][0] = 1;
  matrix_mask[6][7][0] = 1;
  matrix_mask[6][8][0] = 1;
  matrix_mask[6][9][0] = 1;
  matrix_mask[6][10][0] = 1;
  matrix_mask[6][15][0] = 1;
  matrix_mask[6][16][0] = 1;
  matrix_mask[6][17][0] = 1;
  matrix_mask[6][21][0] = 1;
  matrix_mask[6][26][0] = 1;

  matrix_mask[9][6][0] = 1;
  matrix_mask[9][8][0] = 1;
  matrix_mask[9][13][0] = 1;
  matrix_mask[9][19][0] = 1;
  matrix_mask[9][25][0] = 1;

  matrix_mask[10][6][0] = 1;
  matrix_mask[10][8][0] = 1;
  matrix_mask[10][12][0] = 1;
  matrix_mask[10][13][0] = 1;
  matrix_mask[10][19][0] = 1;
  matrix_mask[10][25][0] = 1;

  matrix_mask[11][6][0] = 1;
  matrix_mask[11][8][0] = 1;
  matrix_mask[11][11][0] = 1;
  matrix_mask[11][13][0] = 1;
  matrix_mask[11][20][0] = 1;
  matrix_mask[11][21][0] = 1;
  matrix_mask[11][22][0] = 1;
  matrix_mask[11][23][0] = 1;
  matrix_mask[11][24][0] = 1;

  matrix_mask[12][6][0] = 1;
  matrix_mask[12][8][0] = 1;
  matrix_mask[12][10][0] = 1;
  matrix_mask[12][13][0] = 1;
  matrix_mask[12][20][0] = 1;
  matrix_mask[12][24][0] = 1;

  matrix_mask[13][6][0] = 1;
  matrix_mask[13][8][0] = 1;
  matrix_mask[13][9][0] = 1;
  matrix_mask[13][13][0] = 1;
  matrix_mask[13][21][0] = 1;
  matrix_mask[13][23][0] = 1;

  matrix_mask[14][6][0] = 1;
  matrix_mask[14][8][0] = 1;
  matrix_mask[14][13][0] = 1;
  matrix_mask[14][22][0] = 1;

  matrix_mask[17][3][0] = 1;
  matrix_mask[17][9][0] = 1;
  matrix_mask[17][10][0] = 1;
  matrix_mask[17][11][0] = 1;
  matrix_mask[17][12][0] = 1;
  matrix_mask[17][13][0] = 1;
  matrix_mask[17][16][0] = 1;
  matrix_mask[17][17][0] = 1;
  matrix_mask[17][18][0] = 1;
  matrix_mask[17][19][0] = 1;
  matrix_mask[17][22][0] = 1;
  matrix_mask[17][24][0] = 1;
  matrix_mask[17][25][0] = 1;
  matrix_mask[17][26][0] = 1;

  matrix_mask[18][3][0] = 1;
  matrix_mask[18][9][0] = 1;
  matrix_mask[18][15][0] = 1;
  matrix_mask[18][20][0] = 1;
  matrix_mask[18][22][0] = 1;
  matrix_mask[18][24][0] = 1;
  matrix_mask[18][27][0] = 1;

  matrix_mask[19][3][0] = 1;
  matrix_mask[19][9][0] = 1;
  matrix_mask[19][15][0] = 1;
  matrix_mask[19][20][0] = 1;
  matrix_mask[19][22][0] = 1;
  matrix_mask[19][24][0] = 1;
  matrix_mask[19][28][0] = 1;

  matrix_mask[20][3][0] = 1;
  matrix_mask[20][4][0] = 1;
  matrix_mask[20][5][0] = 1;
  matrix_mask[20][6][0] = 1;
  matrix_mask[20][9][0] = 1;
  matrix_mask[20][15][0] = 1;
  matrix_mask[20][20][0] = 1;
  matrix_mask[20][22][0] = 1;
  matrix_mask[20][24][0] = 1;
  matrix_mask[20][28][0] = 1;

  matrix_mask[21][3][0] = 1;
  matrix_mask[21][9][0] = 1;
  matrix_mask[21][15][0] = 1;
  matrix_mask[21][20][0] = 1;
  matrix_mask[21][22][0] = 1;
  matrix_mask[21][24][0] = 1;
  matrix_mask[21][27][0] = 1;

  matrix_mask[22][3][0] = 1;
  matrix_mask[22][4][0] = 1;
  matrix_mask[22][5][0] = 1;
  matrix_mask[22][6][0] = 1;
  matrix_mask[22][7][0] = 1;
  matrix_mask[22][9][0] = 1;
  matrix_mask[22][15][0] = 1;
  matrix_mask[22][20][0] = 1;
  matrix_mask[22][22][0] = 1;
  matrix_mask[22][24][0] = 1;
  matrix_mask[22][25][0] = 1;
  matrix_mask[22][26][0] = 1;

  int k = 0;

  for (int i=0; i<mask_rig; i++)
  {
    for (int j=0; j<mask_col; j++)
    {
      if (matrix_mask[i][j][0] == 1)
      {
        elem[k].c = j;
        elem[k].r = i;
        elem[k].vc = 0;
        elem[k].vr = 0;
        matrix_mask[i][j][1] = k;
        k++;
      }
    }
  }

  NELEM = k;
}

static void next_position(int in)
{
  // Compute the next position for a ball

  float alpha, spost_r, spost_c, gr, gc, tilt_r = 1, tilt_c = 1;
  
  float dt = TSCALE * (float)0.05;

  pthread_mutex_lock (&sem[1]);
  
  switch (dir)
  {
  case down:
      alpha = -PiGreco / 2;
      tilt_r = (-theta) / (PiGreco / 2);
      break;
      
  case up:
        alpha = PiGreco / 2;
        tilt_r = theta / (PiGreco / 2);
        break;
  
  case left:
      alpha = PiGreco;
      tilt_c = (-phi) / (PiGreco / 2);
      break;
      
  case right:
        alpha = 0;
        tilt_c = phi / (PiGreco / 2);
        break;
  }
  
  pthread_mutex_unlock (&sem[1]);

  // If the direction change, reset the speed 

  if (dir != dir_prec)
  {
    for (int i = 0; i < NELEM; i++)
    {
      elem[i].vc = 0;
      elem[i].vr = 0;
    }
  }

  gc = g * cos(alpha) * tilt_c;
  gr = g * sin(alpha) * tilt_r;

  spost_c = elem[in].vc;
  spost_r = elem[in].vr;

  rnext = elem[in].r + spost_r;
  cnext = elem[in].c + spost_c;
  
  elem[in].vc += (gc * dt / scale);
  elem[in].vr += (gr * dt / scale);
  
  if (cnext < 0)
    cnext = 0;
  if (cnext > 31)
    cnext = 31;
  if (rnext < 0)
    rnext = 0;
  if (rnext > 23)
    rnext = 23;

  dir_prec = dir;
}

static int is_empty_under(int in)
{

  // Verify if the position computed previously is free 

  if ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c))
  {
    if (matrix_mask[(int)roundf(rnext)][(int)roundf(cnext)][0] == 0)
      return 1;
    else
      return 0;
  }
  else
    return 0;
}

static int is_empty_lateral(int in)
{

  //---------------------------------------------------
  // According to cellular automata rules, if the
  // position computed previously is filled check if the 
  // positions at the right or at the left of the
  // previously position are free 
  //---------------------------------------------------

  if (dir == down && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31)
    {
      cnext += 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0)
    {
      cnext -= 1;
      return 1;
    }
    else
      return 0;
  }

  else if (dir == up && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0)
    {
      cnext -= 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31)
    {
      cnext += 1;
      return 1;
    }
    else
      return 0;
  }

  else if (dir == right && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext + 1)][(int)roundf(cnext)][0] == 0 && (int)roundf(rnext) < 23)
    {
      rnext += 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext - 1)][(int)roundf(cnext)][0] == 0 && (int)roundf(rnext)  > 0)
    {
      rnext -= 1;
      return 1;
    }
    else
      return 0;
  }

  else if (dir == left && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext - 1)][(int)roundf(cnext)][0] == 0 && (int)roundf(rnext)  > 0)
    {
      rnext -= 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext + 1)][(int)roundf(cnext)][0] == 0 && (int)roundf(rnext)  < 23)
    {
      rnext += 1;
      return 1;
    }
    else
      return 0;
  }
}

static int is_empty_upper_lateral(int in)
{

  //---------------------------------------------------
  // According to cellular automata rules, to simulate 
  // water, if the previous positions chek are all filled, 
  // check if the positions above and at the right or 
  // at the left of the previously position are free 
  //---------------------------------------------------

  if (fluid_type == water && dir == down && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext+1)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31 && (int)roundf(rnext) < 23)
    {
      rnext += 1;
      cnext += 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext + 1)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0 && (int)roundf(rnext) < 23)
    {
      rnext += 1;
      cnext -= 1;
      return 1;
    }
    else
      return 0;
  }
  else if (fluid_type == water && dir == up && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext-1)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0 && (int)roundf(rnext) > 0)
    {
      rnext -= 1;
      cnext -= 1;
      return 1;
    }
    else if (matrix_mask[(int)roundf(rnext - 1)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31 && (int)roundf(rnext) > 0)
    {
      rnext -= 1;
      cnext += 1;
      return 1;
    }
    else
      return 0;
  }
  
  else if (fluid_type == water && dir == right && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext + 1)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0 && (int)roundf(rnext) < 23)
    {
      rnext += 1;
      cnext -= 1;
      return 1;
    }
  else if (matrix_mask[(int)roundf(rnext - 1)][(int)roundf(cnext - 1)][0] == 0 && (int)roundf(cnext) > 0 && (int)roundf(rnext) > 0)
    {
      rnext -= 1;
      cnext -= 1;
      return 1;
    }
    else
      return 0;
  }
  
  else if (fluid_type == water && dir == left && ((int)roundf(rnext) != (int)roundf(elem[in].r) || (int)roundf(cnext) != (int)roundf(elem[in].c)))
  {
    if (matrix_mask[(int)roundf(rnext+1)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31 && (int)roundf(rnext) < 23)
    {
      rnext += 1;
      cnext += 1;
      return 1;
    }
  else if (matrix_mask[(int)roundf(rnext - 1)][(int)roundf(cnext + 1)][0] == 0 && (int)roundf(cnext) < 31 && (int)roundf(rnext) > 0)
    {
      rnext -= 1;
      cnext += 1;
      return 1;
    }
    else
      return 0;
  }

}

static void update_position(int in)
{

  //------------------------------------------------------------------
  // Once that the next position il correctly computed, this function 
  // update the ball parameters  to implement the changes
  //------------------------------------------------------------------

  matrix_mask[(int)roundf(elem[in].r)][(int)roundf(elem[in].c)][0] = 0;
  matrix_mask[(int)roundf(elem[in].r)][(int)roundf(elem[in].c)][1] = -1;

  elem[in].r = rnext;
  elem[in].c = cnext;

  matrix_mask[(int)roundf(elem[in].r)][(int)roundf(elem[in].c)][0] = 1;
  matrix_mask[(int)roundf(elem[in].r)][(int)roundf(elem[in].c)][1] = in;
}

void compute_position(void)
{
  
  //----------------------------------------------------------
  // This function implement our phisical model, the cellular
  // automata, initially check the gravity direction and
  // detect the lower side of the box, then check the matrix
  // from the lower side to the upper side to find the elements,
  // when an elements is found this function compute and update 
  // its position. 
  //------------------------------------------------------------

  float incertezza = 10 * PiGreco / 180;

  if (theta>incertezza || phi>incertezza || theta<-incertezza || phi<-incertezza)
  {
    pthread_mutex_lock (&sem[1]);
    
    float alpha = atan2f(theta, phi);
    
    pthread_mutex_unlock (&sem[1]);

    if (alpha >= -3 * PiGreco / 4 && alpha <= -PiGreco / 4)
    {
      dir = down; 

      for (int i = 0; i < mask_rig; i++)
      {
        for (int j = 0; j < mask_col; j++)
        {
          if (matrix_mask[i][j][0] == 1)
          {
            int in = matrix_mask[i][j][1];
            
            next_position(in);
            
            while ((int)roundf(elem[in].r) != (int)roundf(rnext) && (!is_empty_under(in) && !is_empty_lateral(in) && !is_empty_upper_lateral(in)))
            {              
              rnext += 1;
            }
            update_position(in);
          }
          else;
        }
      }
    }

    else if (alpha >= PiGreco / 4 && alpha <= 3 * PiGreco / 4)
    {
      dir = up; 

      for (int a = mask_rig - 1; a >= 0; a--)
      {
        for (int b = mask_col - 1; b >= 0; b--)
        {
          if (matrix_mask[a][b][0] == 1)
          {
            int in = matrix_mask[a][b][1];

            next_position(in);

             while ((int)roundf(elem[in].r) != (int)roundf(rnext) && (!is_empty_under(in) && !is_empty_lateral(in) && !is_empty_upper_lateral(in)))
            {              
              rnext -= 1;
            }
            update_position(in);
          }
          else;
        }
      }
    }

    else if (alpha >= -PiGreco / 4 && alpha <= PiGreco / 4)
    {
      dir = right; 

      for (int i = mask_col - 1; i >= 0; i--)
      {
        for (int j = 0; j < mask_rig; j++)
        {
          if (matrix_mask[j][i][0] == 1)
          {
            int in = matrix_mask[j][i][1];

            next_position(in);
            
             while ((int)roundf(elem[in].c) != (int)roundf(cnext) && (!is_empty_under(in) && !is_empty_lateral(in) && !is_empty_upper_lateral(in)))
            {            
              cnext -= 1;
            }
            update_position(in);
          }
          else;
        }
      }
    }

    else if (alpha >= 3 * PiGreco / 4 && alpha >= -3 * PiGreco / 4)
    {
      dir = left; 
      for (int i = 0; i < mask_col; i++)
      {
        for (int j = 0; j < mask_rig; j++)
        {
          if (matrix_mask[j][i][0] == 1)
          {
            int in = matrix_mask[j][i][1];

            next_position(in);
            
            while ((int)roundf(elem[in].c) != (int)roundf(cnext) && (!is_empty_under(in) && !is_empty_lateral(in) && !is_empty_upper_lateral(in)))
            {      
                cnext += 1;
            }
            update_position(in);
          }
          else;
        }
      }
    }
  }
}

void elem_reset(void)
{

  // Reset all the elements

  for (int i = 0; i < MAXELEM; i++)
  {
    elem[i].c = -1;
    elem[i].r = -1;
    elem[i].vc = 0;
    elem[i].vr = 0;
  }

  NELEM = 0;
  Mask_reset();
}

void elem_create(void)
{

  // Create an element in a random empty position

  int x_matrix = irand(mask_rig);
  int y_matrix = irand(mask_col);

  if (start)
  {

    //-----------------------------------------------------
    // While the function doesn't found an empty position
    // in the matrix continue to generate random positions
    //-----------------------------------------------------

    while (matrix_mask[x_matrix][y_matrix][0] != 0)
    {
      x_matrix = irand(mask_rig);
      y_matrix = irand(mask_col);
    }

    elem[NELEM].c = y_matrix;
    elem[NELEM].r = x_matrix;
    elem[NELEM].vc = 0;
    elem[NELEM].vr = 0;
    matrix_mask[x_matrix][y_matrix][0] = 1;
    matrix_mask[x_matrix][y_matrix][1] = NELEM;
    NELEM++;
  }
}

void mouse_func(void)
{
  //--------------------------------------------
  // If the position of the mouse click
  // is empty this function generate an element
  // in that position
  //--------------------------------------------

  float x_mouse, y_mouse;
  int r_matrix, c_matrix;

  if (mouse_b == 1)
  {
    x_mouse = mouse_x;
    y_mouse = mouse_y;

    if(x_mouse > XBOXL && x_mouse < XBOXH && y_mouse < YBOXL && y_mouse > YBOXH)
    {
      x_mouse -= XBOXL;
      y_mouse = YBOXL - y_mouse;
      
      c_matrix = (int)roundf(x_mouse/20);
      r_matrix = (int)roundf(y_mouse/20);
      
      if (c_matrix < 0)
        c_matrix = 0;
      if (c_matrix > 31)
        c_matrix = 31;
      if (r_matrix < 0)
        r_matrix = 0;
      if (r_matrix > 23)
        r_matrix = 23;
    
      if (start && NELEM < MAXELEM-1 && matrix_mask[r_matrix][c_matrix][0] == 0)
      {
        elem[NELEM].c = c_matrix;
        elem[NELEM].r = r_matrix;
        elem[NELEM].vc = 0;
        elem[NELEM].vr = 0;
        matrix_mask[r_matrix][c_matrix][0] = 1;
        matrix_mask[r_matrix][c_matrix][1] = NELEM;
        NELEM++;
      }
    }
  }
}

