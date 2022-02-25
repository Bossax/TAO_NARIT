#include "tao_nuvu.h"
#include <stdlib.h>
#include <math.h>

/*-------------------------------------------------------------------------*/
/* ERROR */

void error_push(
                          const char* func,
                          int err)
{
    fprintf(stderr, "error code = %d in function %s\n", err, func);
}

/*---------------------- Camera configuration -----------------------------*/
/*-------------------------------------------------------------------------*/
/* Times*/
tao_status get_framerate(NcCam cam, double* fps)
{
	int err = NC_SUCCESS;
	err =  ncCamGetFramerate(cam, fps);
	if(err){
		error_push(__func__, err);
		return TAO_ERROR;
	}

	return TAO_OK;
}
// Get readout time
tao_status get_readout_time(NcCam cam, double* readoutTime)
{
    int err = NC_SUCCESS;
    err =  ncCamGetReadoutTime(cam, readoutTime);
    if(err){
      error_push(__func__, err);
      return TAO_ERROR;
    }

    return TAO_OK;
}

// Get readout mode

// Set Readout mode
tao_status set_readout_mode(NcCam cam,int modeNum )
{
  int err = NC_SUCCESS;
  err =  ncCamSetReadoutMode(cam, modeNum);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}

// Set exposure time
tao_status set_exposure_time(NcCam cam,double exposureTime)
{
  int err = NC_SUCCESS;
  err =  ncCamSetExposureTime(cam, exposureTime);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Set waiting time
tao_status set_waiting_time(NcCam cam,double waitingTime){
  int err = NC_SUCCESS;
  err =  ncCamSetWaitingTime(cam, waitingTime);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}


// Set time out
tao_status set_timeout(NcCam cam,
                                int timeTimeout)
{
  int err = NC_SUCCESS;
  err =  ncCamSetTimeout(cam, timeTimeout);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
/*-------------------------------------------------------------------------*/
/* EM gain */

// change EM gain
/*
*   function pointer to opeartion on the emMax and emMin
*/
tao_status set_em_gain(  NcCam camera,
                          int (*emGainOp)(int* num),
                          int emGainInput)
{

  int			error = NC_SUCCESS;

  int emCalGainAvailable = 0;
  int emRawGainAvailable = 0;
	enum Ampli	ampliType;

	int emGainMin, emGainMax;
  int* emGainArray = malloc(3*sizeof(int));  // array holds 3 integers
  *(emGainArray+2) =  emGainInput;
	error = ncCamGetReadoutMode(camera, 1, &ampliType, 0, 0, 0);
	if (error) {

    error_push("ncCamGetReadoutMode", error);
    return TAO_ERROR;
	}
  // Check Calibrated / Raw EM gain availability
	if (ampliType == EM) {
		//Use of the calibrated gain, if available
		error = ncCamParamAvailable(camera, CALIBRATED_EM_GAIN, 0);
		if (error == NC_SUCCESS) {
			emCalGainAvailable = 1;
		}
		else if (error == NC_ERROR_CAM_NO_FEATURE) {
      // NC_ERROR_CAM_NO_FEATURE is expected if the calibrated EM gain
      // is simply not supported
			error = ncCamParamAvailable(camera, RAW_EM_GAIN, 0);
			if (error == NC_SUCCESS) {
				emRawGainAvailable = 1;
			}
			else if (error != NC_ERROR_CAM_NO_FEATURE) {
        //NC_ERROR_CAM_NO_FEATURE is expected
        // if the raw EM gain is simply not supported
				// Some other error happened
        error_push("Raw EM Gain unavailable \n", error);
				return TAO_ERROR;
			}
		}
		else {
			error_push("Calibrated EM Gain unavailable \n \n", error);
		}
	}

	if (emCalGainAvailable == 1) {
		error = ncCamGetCalibratedEmGainRange(camera, &emGainMin, &emGainMax);
		if (error) {
			error_push("ncCamGetCalibratedEmGainRange", error);
      return TAO_ERROR;
		}
    *(emGainArray) = emGainMin;
    *(emGainArray+1) = emGainMax;

		// Calculate em gain
		int emGain = emGainOp(emGainArray);

		//Sets the calibrated EM gain on the camera
		error = ncCamSetCalibratedEmGain(camera, emGain);
		if (error) {
      error_push("ncCamSetCalibratedEmGain", error);
			return TAO_ERROR;
		}
	}
	else if (emRawGainAvailable == 1) {
		error = ncCamGetRawEmGainRange(camera, &emGainMin, &emGainMax);
		if (error) {
      error_push("ncCamGetRawEmGainRange", error);
			return TAO_ERROR;
		}

    *(emGainArray) = emGainMin;
    *(emGainArray+1) = emGainMax;
		//For the purpose of this example we will use the median value
		int emGain = emGainOp(emGainArray);

		//Sets the EM gain on the camera
		error = ncCamSetRawEmGain(camera, emGain);
		if (error) {
			return error;
		}
	}

	if (emCalGainAvailable == 1 || emRawGainAvailable == 1) {
		if (error != NC_SUCCESS) {
			printf("*** The camera cannot modify the EM gain at this point, please make sure there's no other application that is already communicating with the camera.\n");
      return TAO_ERROR;
    }
	}

  free(emGainArray);
	return TAO_OK;

}

tao_status set_analog_gain(NcCam camera, int analogGain)
{

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int analogGainMin, analogGainMax;

	error = ncCamGetAnalogGainRange(camera, &analogGainMin, &analogGainMax);
	if (error) {

		error_push("ncCamGetAnalogGainRange", error);
		return TAO_ERROR;
	}
	if (analogGain < analogGainMin)
		analogGain = analogGainMin;
	if (analogGain > analogGainMax)
		analogGain = analogGainMax;

	//For the purpose of this example we will use the median value

	//Sets the analog gain on the camera
	error = ncCamSetAnalogGain(camera, analogGain);
	if (error) {

		error_push("ncCamSetAnalogGain", error);
		return TAO_ERROR;
	}

	return error;
}

tao_status set_analog_offset(NcCam camera, int analogOffset)
{

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int	analogOffsetMin, analogOffsetMax;

	error = ncCamGetAnalogOffsetRange(camera, &analogOffsetMin, &analogOffsetMax);
	if (error) {
		return error;
	}

	if (analogOffset < analogOffsetMin)
		analogOffset = analogOffsetMin;
	if (analogOffset > analogOffsetMax)
		analogOffset = analogOffsetMax;

	printf("Analog gain offset = %d\n", analogOffset);
	//Sets the analog offset on the camera
	error = ncCamSetAnalogOffset(camera, analogOffset);
	if (error) {
		return error;
	}

	return error;
}

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
tao_status detector_temperature(NcCam cam, double* temp_ptr)
{
  int err = NC_SUCCESS;

  err = ncCamGetDetectorTemp(cam, temp_ptr);
  if(err)
  {
    error_push(__func__, err);
    return TAO_ERROR;
  }
  return TAO_OK;
}

tao_status set_temperature(NcCam camera, double	ccdTargetTemp) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	double	 ccdTargetTempMin, ccdTargetTempMax;

	// If the calibrated em gain is available, set a temperature that will enable it
	//Use of the calibrated gain, if available, is highly recommended
	error = ncCamParamAvailable(camera, CALIBRATED_EM_GAIN, 0);
	if (error == NC_SUCCESS)
	{
		error = ncCamGetCalibratedEmGainTempRange(camera, &ccdTargetTempMin, &ccdTargetTempMax);
		if (error) {
			return error;
		}
	}
	else
	{
		//Inquire range of temperatures available
		error = ncCamGetTargetDetectorTempRange(camera, &ccdTargetTempMin, &ccdTargetTempMax);
		if (error) {
			return error;
		}
	}

	if (ccdTargetTemp < ccdTargetTempMin)
		ccdTargetTemp = ccdTargetTempMin;
	if (ccdTargetTemp > ccdTargetTempMax)
		ccdTargetTemp = ccdTargetTempMax;

	//For the purpose of this example we will use the median value
	printf("Target Temperature = %f\n", ccdTargetTemp);
	//Sets the ccd target temperature on the camera
	error = ncCamSetTargetDetectorTemp(camera, ccdTargetTemp);
	if (error) {
		return error;
	}

	return error;
}

/*---------------------------------------------------------------------------*/
/* Status*/

/*-------------------------- Helper Function -------------------------------*/
// Param availability

//

/*--------------------------- Camera Utilities ------------------------------*/
/*-------------------------------------------------------------------------*/
// Open
tao_status cam_open(int unit, int channel, int nbrBuffer, NcCam* cam)
{
  int err = NC_SUCCESS;
  err =  ncCamOpen(unit, channel, nbrBuffer, cam);

  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Start
tao_status cam_take_image(NcCam cam){
	int err = NC_SUCCESS;
  err = ncCamPrepareAcquisition(cam,1);
	if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

	err = ncCamBeginAcquisition(cam);
	if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}

tao_status cam_start(NcCam cam, int nbrImages)
{
  int err = NC_SUCCESS;
  err =  ncCamStart(cam, nbrImages);

  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Shutter mode
tao_status set_shuttermode(NcCam cam,
                                    enum ShutterMode mode)
{
  int err = NC_SUCCESS;
  err =  ncCamSetShutterMode(cam, mode);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Read and return unsigned short
tao_status read_uint16_image(NcCam cam,
                              NcImage** image_ptrptr)
{
  int err = NC_SUCCESS;
  err =  ncCamRead(cam, image_ptrptr);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}

tao_status read_uint32_image(NcCam cam, uint32_t* image)
{
	int err = NC_SUCCESS;
  err =  ncCamReadUInt32(cam, image);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;

}

//apply ROI
tao_status set_ROI(NcCam camera,int width, int height)
{

		int error = NC_SUCCESS;
		int fullWidth, fullHeight,roiWidth, roiHeight;

		//Recover the range available for ROIs
		error = ncCamGetMaxSize(camera, &fullWidth, &fullHeight);
		if (error) {
			return error;
		}

		// check dimension inputs
		roiWidth = (width > fullWidth) ? fullWidth : width;
		roiHeight = (height > fullHeight) ? fullHeight : height;

		int offsetX = (int) floor((fullWidth-roiWidth)/2);
		int offsetY = (int) floor((fullHeight-roiHeight)/2);

		//There must always be at least one ROI.
		//The first ROI is initially the entire available image area,
		//with zero horizontal and vertical offset.
		//We will set its size to a small fraction of that available;
		//the offset will be unchanged
		error = ncCamSetMRoiSize(camera, 0, roiWidth, roiHeight);
		if (error) {
			return error;
		}

		error = ncCamSetMRoiPosition(camera, 0, offsetX, offsetY);
		if (error) {
			return error;
		}

		//Synchronise the camera with the required ROI configuration
		error = ncCamMRoiApply(camera);
		if (error) {
			return error;
		}


		int w,h = 0;
		ncCamGetMRoiSize(camera, 0, &w ,&h);
		printf("ROI is set to width = %d, height = %d\n",w, h);
		return TAO_OK;
}
// SaveImage
tao_status save_image(NcCam cam,
                              NcImage* image_ptr,
                              const char* save_name,
                              enum ImageFormat saveFormat,
                              const char* addComments,
                              int overwriteFlag)
{
  int err = NC_SUCCESS;
  err =  ncCamSaveImage(cam, image_ptr, save_name, saveFormat, addComments, overwriteFlag);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Abort
tao_status cam_abort(NcCam cam){
  int err = NC_SUCCESS;
  err = ncCamAbort(cam);
  if(err){
    error_push(__func__, err);
    return TAO_ERROR;
  }

  return TAO_OK;
}
// Close
tao_status cam_close(NcCam cam){
  int error =NC_SUCCESS;

  error = ncCamClose(cam);
  if(error){
    printf("\nThe error %d happened while closing the Nuvu Camera driver. For more information about this error, the file nc_error.h can be used\n", error);
    return TAO_ERROR;
  }
  return TAO_OK;

}
