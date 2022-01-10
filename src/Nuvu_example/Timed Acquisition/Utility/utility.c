#include "utility.h"
#include <stdio.h>

int initialize(NcCam* camera) 
{

	int error = NC_SUCCESS;			//We initialize an error flag variable
	double readoutTime, waitingTime, exposureTime;

	if (*camera == NULL) {
		//Opens the acquisition channel using the automatic detection and 4 loop buffers (recommended)
		error = ncCamOpen(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, camera);
		if (error) {
			return error;
		}
	}

	//For this example we will not bother checking for the available readout modes on the camera and
	//simply select the first one as it is the prefered mode and always valid
	error = ncCamSetReadoutMode(*camera, 1);
	if (error) {
		return error;
	}

	//Recover the readout time for use later
	error = ncCamGetReadoutTime(*camera, &readoutTime);
	if (error) {
		return error;
	}

	//The exposure time is set to the same value as the readout time
	exposureTime = readoutTime;
	error = ncCamSetExposureTime(*camera, exposureTime);
	if (error) {
		return error;
	}

	//We set a reasonable waiting time
	waitingTime = 0.1 * exposureTime;
	error = ncCamSetWaitingTime(*camera, waitingTime);
	if (error) {
		return error;
	}

	//We set a long enough timeout
	error = ncCamSetTimeout(*camera, (int)(waitingTime + readoutTime + exposureTime) + 1000);
	if (error) {
		return error;
	}

	return error;
}


//This function closes the shutter and calls the destructor
void cleanUp(NcCam camera)
{
	int error = NC_SUCCESS;		//We initialize an error flag variable

	//Closes and free NcCam structure
	error = ncCamClose(camera);

	if (error) {
		printf("\nThe error %d happened while closing the Nuvu Camera driver. For more information about this error, the file nc_error.h can be used\n", error);
	}
}


int acquireOneImage(NcCam* camera, NcImage** image)
{
	int error = NC_SUCCESS;		//We initialize an error flag variable

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(*camera, OPEN);
	if (error) {
		return error;
	}

	//Launches an acquisition by the framegrabber and requests an image from the camera (this function 
	//does not wait for the acquisition to be complete before returning)
	error = ncCamStart(*camera, 1);
	if (error) {
		return error;
	}

	//Reads the image received, if a timeout occurs an error code will be returned
	error = ncCamRead(*camera, image);
	if (error) {
		return error;
	}

	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(*camera, CLOSE);
	if (error) {
		return error;
	}

	return error;
}
