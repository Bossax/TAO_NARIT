#ifndef H
#define H

#include <tao.h>
#include "nc_driver.h"

extern void error_push(const char* func, int err);

/*---------------------- Camera configuration -----------------------------*/
/*-------------------------------------------------------------------------*/
/* Times*/

// Get readout time
extern tao_status get_readout_time(NcCam cam, double* readoutTime);

// Get readout mode

// Set Readout mode
extern tao_status set_readout_mode(NcCam cam,int modeNum );

// Set exposure time
extern tao_status set_exposure_time(NcCam cam,double exposureTime) ;

// Set waiting time
extern tao_status set_waiting_time(NcCam cam,double waitingTime);


// Set time out
extern tao_status set_timeout(NcCam cam,int timeTimeout);

/*-------------------------------------------------------------------------*/
/* EM gain */

// change EM gain
/*
*   function pointer to opeartion on the emMax and emMin
*/
extern tao_status change_em_gain(NcCam camera,
                          int (*emGainOp)(int* num),
                          int emGainInput);



/*---------------------------------------------------------------------------*/
/* Processing */
/* Linear Mode */


/* Photon Counting*/


/*---------------------------------------------------------------------------*/
/* Analog gain and offset */
extern tao_status set_analog_gain (NcCam camera, int gain);
extern tao_status set_analog_offset(NcCam camera, int offset);
/*---------------------------------------------------------------------------*/
/* Image Format */
// Format

// Compression

// Data type

/*---------------------------------------------------------------------------*/
/* ROI */
extern tao_status set_ROI(NcCam camera,int width, int height);
// Binning
// mode

// x

// y

//Width

// Height


/*---------------------------------------------------------------------------*/
/* Temperature */
extern tao_status detector_temperature(NcCam cam, double* temp_ptr);
extern tao_status set_temperature(NcCam camera, double	ccdTargetTemp);
/*---------------------------------------------------------------------------*/
/* Status*/
extern tao_status get_framerate(NcCam cam, double* fps);

/*-------------------------- Helper Function -------------------------------*/
// Param availability

//

/*--------------------------- Camera Utilities ------------------------------*/
/*-------------------------------------------------------------------------*/
// Open
extern tao_status cam_open(int unit, int channel, int nbrBuffer, NcCam* cam);

// Start
extern tao_status cam_start(NcCam cam,
                                     int nbrImages);

// take one image
extern tao_status cam_take_image(NcCam cam);


// Shutter mode
extern tao_status set_shuttermode(NcCam cam,
                                          enum ShutterMode mode);

// Read
extern tao_status read_image(NcCam cam, NcImage** image_ptrptr);

extern tao_status read_uint16_image(NcCam cam, NcImage** image_ptrptr);

// Don't USE
extern tao_status read_uint32_image (NcCam cam, uint32_t* image);

// SaveImage
extern tao_status save_image(NcCam cam,
                              NcImage* image_ptr,
                              const char* save_name,
                              enum ImageFormat saveFormat,
                              const char* addComments,
                              int overwriteFlag);

// Abort
extern tao_status cam_abort(NcCam cam);

// Close
extern tao_status cam_close(NcCam cam);


#endif
