//----------------------------------------------------------------------
//----------------HARDWARE LIBRARY HEADER FILE--------------------------
//-------------------PROJECT: SAND IN A BOX-----------------------------
//--------------TINFENA MATTIA & MASSARA ANGELO-------------------------
//----------------------------------------------------------------------

#ifndef HWLIB_H
#define HWLIB_H

//----------------------------------------------------------------------
//---------------------GLOBAL COSTANTS----------------------------------
//----------------------------------------------------------------------

#define mask_col        32        // Number of columns of the mask
#define mask_rig        24        // Number of rows of the mask
#define MAT_NUM         11        // Number of 8x8 led matrix (starting from 0)

#define CHANNEL         0         // Channel of the SPI port
#define FREQ            1000000   // Frequence of the SPI port

#define DELTA           3         // Maximum offset to consider a lecture from the accelerometer changed
#define N_CAMPIONE      10        // Number of lectures of the accelerometer
#define MINSENSOR       -240      // Minimum values retourned from the accelerometer
#define MAXSENSOR       240       // Maximum values retourned from the accelerometer

//----------------------------------------------------------------------
//---------------------GLOBAL VARIABLES---------------------------------
//----------------------------------------------------------------------

extern int          matrix_mask[][32][2];
extern int          fd;
extern int          Min, Max;
extern int          x, y, z;
extern int          p_x, p_y, p_z;
extern float        theta, phi; 
extern const float  PiGreco;
extern int          acc;
extern int          rig,col;

//----------------------------------------------------------------------
//-----------------GENERIC FUNCTIONS DECLARATION------------------------
//----------------------------------------------------------------------

extern int vect_to_int(int vect[]);
extern float mapfloat(long v, long in_min, long in_max, long in_med, long out_min, long out_max);

//----------------------------------------------------------------------
//------------------MASK FUNCTIONS DECLARATION--------------------------
//----------------------------------------------------------------------

extern void Mask_reset();
extern void get_coordinate(int mat);

//----------------------------------------------------------------------
//-----------------LED MATRIX FUNCTIONS DECLARATION---------------------
//----------------------------------------------------------------------

extern void Matrix_init();

//----------------------------------------------------------------------
//----------------ACCELEROMETER FUNCTIONS DECLARATION-------------------
//----------------------------------------------------------------------

extern void Accelerometer_setData();
extern int Accelerometer_init();

#endif