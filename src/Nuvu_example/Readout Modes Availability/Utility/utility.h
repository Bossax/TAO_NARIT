#ifndef NC_UTILITY_H
#define NC_UTILITY_H

//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This function initializes the camera by setting the readout mode, readout time, exposure time and
//waiting time. The camera is then ready for the acquisition
int initialize(NcCam* camera);

//This function closes the shutter and calls the destructor
void cleanUp(NcCam camera);

//This function acquires one single image
int acquireOneImage(NcCam* camera, NcImage** image);

#endif
