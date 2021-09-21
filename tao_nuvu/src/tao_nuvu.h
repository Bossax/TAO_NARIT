#ifndef TAO_NUVU_H
#define TAO_NUVU_H

#include <tao.h>
#include "nc_driver.h"

extern void tao_nuvu_error_push(const char* func, int err);

/*---------------------- Camera configuration -----------------------------*/
/*-------------------------------------------------------------------------*/
/* Times*/

// Get readout time
extern tao_status tao_nuvu_get_readout_time(NcCam cam, double* readoutTime);

// Get readout mode

// Set Readout mode
extern tao_status tao_nuvu_readout_mode(NcCam cam,int modeNum );

// Set exposure time
extern tao_status tao_nuvu_exposure_time(NcCam cam,double exposureTime) ;

// Set waiting time
extern tao_status tao_nuvu_set_waiting_time(NcCam cam,double waitingTime);


// Set time out
extern tao_status tao_nuvu_set_timeout(NcCam cam,int timeTimeout);
/*-------------------------------------------------------------------------*/
/* EM gain */

// change EM gain
/*
*   function pointer to opeartion on the emMax and emMin
*/
extern tao_status changeEmGain( NcCam camera,
                          int (*emGainOp)(int* num),
                          int emGainInput);



/*---------------------------------------------------------------------------*/
/* Processing */
/* Linear Mode */


/* Photon Counting*/


/*---------------------------------------------------------------------------*/
/* Analog gain and offset */

/*---------------------------------------------------------------------------*/
/* Image Format */
// Format

// Compression

// Data type

/*---------------------------------------------------------------------------*/
/* ROI */

// Binning
// mode

// x

// y

//Width

// Height


/*---------------------------------------------------------------------------*/
/* Temperature */
extern tao_status tao_nuvu_detector_temperature(NcCam cam, double* temp_ptr);
/*---------------------------------------------------------------------------*/
/* Status*/

/*-------------------------- Helper Function -------------------------------*/
// Param availability

//

/*--------------------------- Camera Utilities ------------------------------*/
/*-------------------------------------------------------------------------*/
// Open
extern tao_status tao_nuvu_cam_open(int unit, int channel, int nbrBuffer, NcCam* cam);

// Start
extern tao_status tao_nuvu_cam_start(NcCam cam,
                              int nbrImages);

// Shutter mode
extern tao_status tao_nuvu_set_shuttermode(NcCam cam,
                                    enum ShutterMode mode);

// Read
extern tao_status tao_nuvu_read_image(NcCam cam,
                              NcImage** image_ptrptr);

// SaveImage
extern tao_status tao_nuvu_save_image(NcCam cam,
                              NcImage* image_ptr,
                              const char* save_name,
                              enum ImageFormat saveFormat,
                              const char* addComments,
                              int overwriteFlag);

// Abort
extern tao_status tao_nuvu_cam_abort(NcCam cam);

// Close
extern tao_status tao_nuvu_cam_close(NcCam cam);


#endif
