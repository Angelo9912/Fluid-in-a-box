//----------------------------------------------------------------------
//---------------------------MAIN---------------------------------------
//-------------------PROJECT: FLUID IN A BOX----------------------------
//---------------TINFENA MATTIA & MASSARA ANGELO------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// This file contains the main function of the project and the tasks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--------------------------LIBRARIES-----------------------------------
//----------------------------------------------------------------------

#include <stdio.h>
#include <allegro.h>
#include <math.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "ptask.h"
#include "auxlib.h"
#include "hwlib.h"

//----------------------------------------------------------------------
//--------------------T0: ACCELEROMETER TASK----------------------------
//----------------------------------------------------------------------

void *Accelerometer_read(void *arg)
{
	// This task reads the accelerometer and set theta and phi 

	float xdeg, ydeg, zdeg;
	float argPhi, argTheta;

	int ti;
	ti = get_task_index(arg);
	set_activation(ti);

	while (end == 0)
	{
		if(acc && start)
		{
			Accelerometer_setData();

			if (x > Max)
				x = Max;
			if (x < Min)
				x = Min;
			if (y > Max)
				y = Max;
			if (y < Min)
				y = Min;
			if (z > Max)
				z = Max;
			if (z < Min)
				z = Min;

			// I consider the values changed if the difference exceeds a certain value DELTA

			if ((abs(p_x - x) > DELTA) || (abs(p_y - y) > DELTA) || (abs(p_z - z) > DELTA))
			{
				xdeg = mapfloat(x, Min, Max, 0, -90, 90);
				ydeg = mapfloat(y, Min, Max, 0, -90, 90);
				zdeg = mapfloat(z, Min, Max, 0, -90, 90);

				// Phi = angle between y axis and gravity(ROLL)
				// Theta = angle between x axis and gravity (PITCH)

				argTheta = sqrt((ydeg * ydeg) + (zdeg * zdeg));
				argPhi = sqrt((xdeg * xdeg) + (zdeg * zdeg));

				pthread_mutex_lock (&sem[1]);			// Lock r1 - theta & phi to update the values
				
				if (argTheta == 0)
					theta = 0;
				else
					theta = atan(xdeg / argTheta);
				if (argPhi == 0)
					phi = 0;
				else
					phi = atan(ydeg / argPhi);
				
				pthread_mutex_unlock (&sem[1]);		// Unlock r1 - theta & phi
			}
			else;

			p_x = x;
			p_y = y;
			p_z = z;
		}		
		
		if (deadline_miss(ti))
		{
			//to do in case of deadline miss

			printf("\n%d", tp[ti].dmiss);
			printf(" deadline misses of the task ");
			printf("%d", ti);
			printf("\n");
		}
		wait_for_activation(ti);
	}
}

//----------------------------------------------------------------------
//----------------------T1: PHYSICS TASK--------------------------------
//----------------------------------------------------------------------

void *physics(void *arg)
{
	// This task implement our phisical model, the cellular automata

	int ti;
	ti = get_task_index(arg);
	set_activation(ti);

	while (end == 0)
	{
		if (start)
		{
			compute_position();
		}

		if (deadline_miss(ti))
		{
			//to do in case of deadline miss

			printf("\n%d", tp[ti].dmiss);
			printf(" deadline miss of the task ");
			printf("%d", ti);
			printf("\n");
		}
		wait_for_activation(ti);
	}
}

//----------------------------------------------------------------------
//--------------------T2: LED MATRIX TASK-------------------------------
//----------------------------------------------------------------------

void *led_matrix(void *arg)
{
	//-----------------------------------------------------------------
	// This task reads the mask row by row and send the value of each 
	// row at the led matrix trough the SPI port
	//----------------------------------------------------------------

	int v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int val;
	unsigned char row[24];
	
	int ti;
	ti = get_task_index(arg);
	set_activation(ti);

	while (end == 0)
	{
		for (int r = 0; r < 8; r++)
		{
			for (int mat = 0; mat <= MAT_NUM; mat++)
			{
				get_coordinate(mat);					// Get the initial coordinates of each 8x8 led matrix
				
				for (int c = 0; c < 8; c++)				// Read the 8-bit row and convert it in a decimal  integer
				{
					if (mat >= 4 && mat <= 7)
					{
						pthread_mutex_lock (&sem[0]);	// Lock r0 - matrix_mask to read it
						v[c] = matrix_mask[rig - r][col - c][0];
						pthread_mutex_unlock (&sem[0]);	// Unlock ro- matrix_mask
					}
					else
					{
				
						pthread_mutex_lock (&sem[0]);
						v[c] = matrix_mask[rig + r][col + c][0];
						pthread_mutex_unlock (&sem[0]);
					}
				}
				val = vect_to_int(v);
				row[((MAT_NUM-mat)*2)] = r+1;		// Set the register for each row
				row[((MAT_NUM-mat)*2)+1] = val;		// Set the value for each row
			}
		wiringPiSPIDataRW(CHANNEL, row, 24);
		}
		
		if (deadline_miss(ti))
		{
			//to do in case of deadline miss

			printf("\n%d", tp[ti].dmiss);
			printf(" deadline miss of the task ");
			printf("%d", ti);
			printf("\n");
		}
		wait_for_activation(ti);
	}
}

//----------------------------------------------------------------------
//-------------------------T3: USER TASK--------------------------------
//----------------------------------------------------------------------

void *user(void *arg)
{

	//----------------------------------------------
	// This task read the key pressed from the user 
	// and compute the following action 
	//---------------------------------------------

	int x_mouse, y_mouse, x_matrix, y_matrix;
	char scan;

	int ti;
	ti = get_task_index(arg);
	set_activation(ti);

	while (end == 0)
	{		
		mouse_func();
		
		scan = get_scancode();
		switch (scan)
		{
		case KEY_S:
		
			color = makecol(255,178,128);
			fluid_type = sand;
			scale = 6;
			break;
			
		case KEY_W:
		
			color = makecol(51, 153, 255);
			fluid_type = water;
			scale = 1;
			break;
			
		case KEY_O:
			
			start = 0;
			elem_reset();
			start = 1;
			break;
			
		case KEY_T:
		
			TSCALE += 0.25;
			break;
			
		case KEY_Y:
		
			TSCALE -= 0.25;
			if(TSCALE < 0) TSCALE = 0;
			break;
			
		case KEY_P:
		
			if( !start)
				start = 1;
			else start = 0;
			break;
			
		case KEY_A:
		
			acc = 1;
			break;
			
		case KEY_R:
		
			elem_init();
			break;
			
		case KEY_RIGHT:
		
			acc = 0;
			phi += 10 * PiGreco / 180;
			if (phi > PiGreco / 2)
				phi = PiGreco / 2;
			break;
			
		case KEY_LEFT:
		
			acc = 0;
			phi -= 10 * PiGreco / 180;
			if (phi < -PiGreco / 2)
				phi = -PiGreco / 2;
			break;
			
		case KEY_DOWN:
		
			acc = 0;
			theta -= 10 * PiGreco / 180;
			if (theta < -PiGreco / 2)
				theta = -PiGreco / 2;
			break;
			
		case KEY_UP:
		
			acc = 0;
			theta += 10 * PiGreco / 180;
			if (theta > PiGreco / 2)
				theta = PiGreco / 2;
			break;
			
		case KEY_ESC:
		
			end = 1;
			break;
			
		case KEY_B:
		
			elem_create();
			break;
			
		case KEY_G:
		
			g += 0.5;
			break;
			
		case KEY_H:
		
			g -= 0.5;
			if (g < 0) g = 0;
			break;
	  }
	  
	 if (deadline_miss(ti))
	{
		//to do in case of deadline miss

		printf("\n%d", tp[ti].dmiss);
		printf(" deadline miss of the task ");
		printf("%d", ti);
		printf("\n");
	}
	wait_for_activation(ti);
	}
}

//----------------------------------------------------------------------
//----------------------T4: GRAPHIC TASK--------------------------------
//----------------------------------------------------------------------

void *graphic(void *arg)
{
	// This is the graphic task that handle the GUI

	char th[50];
	char ph[50];
	char gf[50];
	char ts[50];
	float phi_deg;
	float theta_deg;

	int ti;
	ti = get_task_index(arg);
	set_activation(ti);

	while (end == 0)
	{		
		phi_deg	= phi * 180 / PiGreco;
		theta_deg = theta * 180 / PiGreco;

		// Update the theta, phi, gravity and TSCALE values

		rectfill(buffer, 610, 245, 700, 233, makecol(21, 23, 36));
		sprintf(ph, "%5.2f\n", phi_deg);
		textout_ex(buffer, font, ph, 615, 238, makecol(255,255,255), -1);
		rectfill(buffer, 1045, 536, 1150, 520, makecol(21, 23, 36));
		sprintf(th, "%5.2f\n", theta_deg);
		textout_ex(buffer, font, th, 1050, 529, makecol(255,255,255), -1);
		rectfill(buffer, 1559, 730, 1680, 750, makecol(21, 23, 36));
		sprintf(gf, "%5.2f\n", g);
		textout_ex(buffer, font, gf, 1564, 737, makecol(255,255,255), -1);
		rectfill(buffer, 588, 910, 700, 960, makecol(21, 23, 36));
		sprintf(ts, "%5.2f\n", TSCALE);
		textout_ex(buffer, font, ts, 593, 917, makecol(255,255,255), -1);

		// Draw the ball and the gravity's arrow
		
		draw_balls();
		draw_gravity();

		blit(buffer, screen, 0, 0, 0, 0, width, height);

		if (deadline_miss(ti))
		{
			//to do in case of deadline miss

			printf("\n%d", tp[ti].dmiss);
			printf(" deadline miss of the task ");
			printf("%d", ti);
			printf("\n");
		}
		wait_for_activation(ti);
	}
}

//----------------------------------------------------------------------
//------------------------MAIN FUNCTION---------------------------------
//----------------------------------------------------------------------

int main()
{
	printf("\nSand in a box");
	printf("\nProject made by Tinfena Mattia e Massara Angelo\n");

	int ntask = 0, ret, ris;

	//Init functions
	
	start_allegro();

	ris = semaphore_init(0, STAND);					// semaphore for r0 - matrix_mask
	if (ris == 1)
		printf("\nProblem on semaphore initlization");
	ris = semaphore_init(1, STAND);					// semaphore for r1 - theta & phi
	if (ris == 1)
		printf("\nProblem on semaphore initlization");

	int init = Accelerometer_init();
	if (init == -1)
		printf("\nProblem on accelerometer initlization");
		
	Matrix_init();
	elem_init();
	enable_hardware_cursor();
	show_mouse(screen);

	//Task creation

	ret = task_create(Accelerometer_read, 0, (50), (50), 13);
	if (ret == -1)
	{
		printf("\nProblem on task ");
		printf("%d", 0);
		printf(" creation\n ");
	}
	ntask++;

	ret = task_create(physics, 1, (50), (50), 12);
	if (ret == -1)
	{
		printf("\nProblem on task ");
		printf("%d", 1);
		printf(" creation\n ");
	}
	ntask++;

	ret = task_create(led_matrix, 2, (50), (50), 11);
	if (ret == -1)
	{
		printf("\nProblem on task ");
		printf("%d", 2);
		printf(" creation\n ");
	}
	ntask++;

	ret = task_create(user, 3, (50), (50), 10);
	if (ret == -1)
	{
		printf("\nProblem on task ");
		printf("%d", 3);
		printf(" creation\n ");
	}
	ntask++;

	ret = task_create(graphic, 4, (50), (50), 10);
		if (ret == -1)
		{
			printf("\nProblem on task ");
			printf("%d", 4);
			printf(" creation\n ");
		}
	ntask++; 
		
	while (end == 0);

	//Exit functions

	start = 0;
	elem_reset();
	allegro_exit();
	printf("\n\nProgram termination \n");
	for (int i = 0; i < ntask; i++)
		wait_for_task_end(i);
	return 0;
}
