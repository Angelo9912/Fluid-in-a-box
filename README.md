# Fluid-in-a-box
Project realized from Mattia Tinfena and Angelo Massara.

In this project, we simulated the behavior of a fluid in a box, through the cellular automata model. Particularly we focused the attenction on two specific fluids: sand and water. Our aim is to show how the particles which composethe fluid interact with each other when we rotate the box (and hence the frame).  

The project consists of two parts:
• software
• hardware  

The software, is realized as a real-time multi-thread application in C language on the Linux operating system. We used the FIFO scheduling policy, implemenenting precedence costraints. Were also imported Pthread library(to allow multi-threading programmation) and the Allegro library (version 4.2.2) to make the Graphical User Interface.  

For the hardware has been used a Raspberry Pi 4 model B where we installed the Raspberry Pi Operative System (also known as Respbian), based on Debian and optimized for the Raspberry Pi hardware, equipped with adxl345 accelerometer and a 24x32 LED dot matrix driven by the MAX7219 chip.
