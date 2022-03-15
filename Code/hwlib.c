//----------------------------------------------------------------------
//----------------------HARDWARE LIBRARY--------------------------------
//-------------------PROJECT: FLUID IN A BOX----------------------------
//---------------TINFENA MATTIA & MASSARA ANGELO------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// This library contains the functions to read the accelerometer adxl345
// and to switch on the led matrix driven by max7219
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
#include "ptask.h"
#include "hwlib.h"

//----------------------------------------------------------------------
//---------------------GLOBAL COSTANTS----------------------------------
//----------------------------------------------------------------------

// The Max7219 Registers :

#define DECODE_MODE     0x09
#define INTENSITY       0x0a
#define SCAN_LIMIT      0x0b
#define SHUTDOWN        0x0c
#define DISPLAY_TEST    0x0f
#define NO_OP           0x00

// The adxl345 Registers:

#define DEVICE_ID       0x53
#define REG_POWER_CTL   0x2D
#define REG_DATA_X_LOW  0x32
#define REG_DATA_X_HIGH 0x33
#define REG_DATA_Y_LOW  0x34
#define REG_DATA_Y_HIGH 0x35
#define REG_DATA_Z_LOW  0x36
#define REG_DATA_Z_HIGH 0x37

//----------------------------------------------------------------------
//---------------------GLOBAL VARIABLES---------------------------------
//----------------------------------------------------------------------

int           matrix_mask[mask_rig][mask_col][2];   // Mask of the matrix
int           fd;                                   // Accelerometer ID
int           Min = MINSENSOR;
int           Max = MAXSENSOR;
int           x, y, z;                              // Values read from the accelerometer
int           p_x = -1, p_y = -1, p_z = -1;         // Precedent values read from the accelerometer 
float         theta, phi;                           // Accelerometer tilt
const float   PiGreco = 3.14159265358979323846;
int           acc = 1;                              // Accelerometer active flag
int           rig,col;                              // Coordinates of a matrix

//----------------------------------------------------------------------
//--------------------GENERIC FUNCTIONS---------------------------------
//----------------------------------------------------------------------

static int power(int n) 
{
  // Function that returns 2^n

  int res = 1;
  for (int i=0; i<n; i++)
    res *= 2;
  return res;
}

int vect_to_int(int vect[8]) 
{

  //----------------------------------------------
  // function that convert a binary 8-elem array                              
  // in a decimal integer
  //---------------------------------------------

  int res = 0;
  for (int i=0; i<8; i++)
  {
    int a = power(i);
    res += vect[i] * a;
  }
  return res;
}

float mapfloat(long v, long in_min, long in_max, long in_med, long out_min, long out_max)
{

  // Function that map a value from a certain range to another range

  float out_med = (out_max - out_min) / 2 + out_min;
  if (v < in_min)
    v = in_min; 
  if (v > in_max)   // Delete out-range values Min, Max
    v = in_max;
  if (v < in_med)
    return ((float)(v - in_min) * (float)(out_med - out_min) / (float)(in_med - in_min) + out_min);
  else
    return (float)(v - in_med) * (float)(out_max - out_med) / (float)(in_max - in_med) + out_med;
}

//----------------------------------------------------------------------
//---------------------------MASK FUNCTIONS-----------------------------
//----------------------------------------------------------------------

void Mask_reset() 
{
  // Function that set all data to 0

  for (int i=0; i<mask_rig; i++)
  {
    for (int j=0; j<mask_col; j++)
    {
      pthread_mutex_lock (&sem[0]);             // Lock shared resources with binary mutex
      matrix_mask[i][j][0] = 0;
      matrix_mask[i][j][1] = -1;
      pthread_mutex_unlock (&sem[0]);           // Unlock shared resources
    }
  }
}

void get_coordinate(int m)
{

  //Returns the initial coordinate of each phisical 8x8 matrix

  switch (m)
  {
    case 11:
      rig = 16;
      col = 24;
      break;
    case 10:
      rig = 16;
      col = 16;
      break;
    case 9:
      rig = 16;
      col = 8;
      break;
    case 8:
      rig = 16;
      col = 0;
      break;
    case 7:
      rig = 15;
      col = 7;
      break;
    case 6:
      rig = 15;
      col = 15;
      break;
    case 5:
      rig = 15;
      col = 23;
      break;
    case 4:
      rig = 15;
      col = 31;
      break;
    case 3:
      rig = 0;
      col = 24;
      break;
    case 2:
      rig = 0;
      col = 16;
      break;
    case 1:
      rig = 0;
      col = 8;
      break;
    case 0:
      rig = 0;
      col = 0;
      break;
  }
}


//----------------------------------------------------------------------
//----------------MATRIX INITIALIZATION FUNCTIONS-----------------------
//----------------------------------------------------------------------

static void send_reg(int reg, int val)
{

  // Send the settings to the max7219 trough the SPI port

  unsigned char data [24];

  data[0] = reg;

  for (int i=1; i<24; i++)
  {
    if (i%2 == 0)
      data[i] = reg;
    else
      data[i] = val;
  }

  wiringPiSPIDataRW(0, data, 24);
}

void Matrix_init()
{

  // Set the matrix registers

  if (wiringPiSetup() == -1)
    printf("\nError occured during WiringPi setup\n");

  if (wiringPiSPISetup(CHANNEL, FREQ) == -1)
    printf("\nError occured during SPI setup\n");

  Mask_reset();

  send_reg(SCAN_LIMIT,7);		// To display all digits
  send_reg(DECODE_MODE,0);		// No decode mode for any digit
  send_reg(DISPLAY_TEST,0);		// Normal operations, no display test
  send_reg(INTENSITY,0);		// Lowest intensity of light to reduce the consumption
  send_reg(SHUTDOWN,1);			// Normal operations, no shutdown mode
}


//----------------------------------------------------------------------
//----------------------ACCELEROMETER FUNCTIONS-------------------------
//----------------------------------------------------------------------

void Accelerometer_setData()
{
  //------------------------------------------------------
  // Makes multiple lectures of the accelerometer data
  // and returnes the average values. This function
  //  discard the first lecture to reduce noise
  //------------------------------------------------------

  wiringPiI2CWriteReg8(fd, REG_POWER_CTL, 0b00001000);

  long letturax = 0, letturay = 0, letturaz = 0;
  int xa, ya, za;

  xa = wiringPiI2CReadReg16(fd, REG_DATA_X_LOW);
  xa = -(~(int16_t)xa + 1);

  ya = wiringPiI2CReadReg16(fd, REG_DATA_Y_LOW);
  ya = -(~(int16_t)ya + 1);

  za = wiringPiI2CReadReg16(fd, REG_DATA_Z_LOW);
  za = -(~(int16_t)za + 1);

  xa = 0;
  ya = 0;
  za = 0;

  for (int i = 0; i < N_CAMPIONE; i++)
  {
    xa = wiringPiI2CReadReg16(fd, REG_DATA_X_LOW);
    xa = -(~(int16_t)xa + 1);
    letturax += xa;

    ya = wiringPiI2CReadReg16(fd, REG_DATA_Y_LOW);
    ya = -(~(int16_t)ya + 1);
    letturay += ya;

    za = wiringPiI2CReadReg16(fd, REG_DATA_Z_LOW);
    za = -(~(int16_t)za + 1);
    letturaz += za;
  }

  x = letturax / N_CAMPIONE;
  y = letturay / N_CAMPIONE;
  z = letturaz / N_CAMPIONE;
}


int Accelerometer_init()
{
  // Setup communication whith the accelerometer trough I2C port

  fd = wiringPiI2CSetup(DEVICE_ID);
  if (fd == -1)
  {
    printf("Failed to init Accelerometer.\n");
    return -1;
  }
  return 0;
}
