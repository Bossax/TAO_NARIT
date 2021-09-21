//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>


int demo(NcCam camera);

int changeCcdTargetTemperature(NcCam camera);
int changeEmGain(NcCam camera, int* emCalGainAvailable, int* emRawGainAvailable);
int changeAnalogGain(NcCam camera);
int changeAnalogOffset(NcCam camera);

//The following function interrogates the camera to know what values were in effect for the CCD Temperature and the gain during the last acquisition
int displayNewSettings(NcCam camera, int emCalGainAvailable, int emRawGainAvailable);

int main() {

	NcCam	myCam = NULL;
	int		error = NC_SUCCESS;		//We initialize an error flag variable
	
	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = demo(myCam);
	}

	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);

	return error;
}


int demo(NcCam camera) {

	NcImage	*myNcImage = NULL;

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	
	int	emCalGainAvailable = 0;
	int	emRawGainAvailable = 0;

	//We acquire a reference image to compare it with the one with the settings changed
	printf("Acquiring a reference image with default settings...\n");

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}

	printf("Done.\n");

	//Saves the image acquired
	//"Default settings reference" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "ReferenceImage", FITS, "Default settings reference", 1);
	if (error) {
		return error;
	}
	
	//We change the CCD target temperature
	error = changeCcdTargetTemperature(camera);
	if (error) {
		return error;
	}
	printf("The CCD temperature setpoint has been modified.\n");

	//We change the EM gain
	error = changeEmGain(camera, &emCalGainAvailable, &emRawGainAvailable);
	if (error) {
		return error;
	}
	printf("The EM gain has been modified.\n");

	//We change the analog gain
	error = changeAnalogGain(camera);
	if (error) {
		return error;
	}
	printf("The analog gain has been modified.\n");

	//We change the analog offset
	error = changeAnalogOffset(camera);
	if (error) {
		return error;
	}
	printf("The analog offset has been modified.\n");
	if (error) {
		return error;
	}

	printf("\nAcquiring a new image with the new settings...\n");

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}

	printf("Done.\n");

	//Saves the image acquired
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "NewSettingsImage", FITS, "This is an image grab with the EM gain, analog gain, analog offset and target temperature modified", 1);
	if (error) {
		return error;
	}

	error = displayNewSettings(camera, emCalGainAvailable, emRawGainAvailable);
	if (error) {
		return error;
	}

	return error;
}


int changeCcdTargetTemperature(NcCam camera) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	double	ccdTargetTemp, ccdTargetTempMin, ccdTargetTempMax;

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

	//For the purpose of this example we will use the median value
	ccdTargetTemp = (ccdTargetTempMin + ccdTargetTempMax) / 2.0;

	//Sets the ccd target temperature on the camera
	error = ncCamSetTargetDetectorTemp(camera, ccdTargetTemp);
	if (error) {
		return error;
	}

	return error;
}


int changeEmGain(NcCam camera, int* emCalGainAvailable, int* emRawGainAvailable) {

	int			error = NC_SUCCESS;		//We initialize an error flag variable
	enum Ampli	ampliType;

	int		emGain, emGainMin, emGainMax;

	//"Initialize" has set readout mode to 1.  We call the following function to get the ampli type associated with that mode
	//The last 3 parameters are set to 0 since we don't need to save their value
	error = ncCamGetReadoutMode(camera, 1, &ampliType, 0, 0, 0);
	if (error) {
		return error;
	}

	if (ampliType == EM) {
		//Use of the calibrated gain, if available, is highly recommended
		error = ncCamParamAvailable(camera, CALIBRATED_EM_GAIN, 0);
		if (error == NC_SUCCESS) {
			*emCalGainAvailable = 1;
		}
		else if (error == NC_ERROR_CAM_NO_FEATURE) { //NC_ERROR_CAM_NO_FEATURE is expected if the calibrated EM gain is simply not supported
			error = ncCamParamAvailable(camera, RAW_EM_GAIN, 0);
			if (error == NC_SUCCESS) {
				*emRawGainAvailable = 1;
			}
			else if (error != NC_ERROR_CAM_NO_FEATURE) { //NC_ERROR_CAM_NO_FEATURE is expected if the raw EM gain is simply not supported
				// Some other error happened
				return error;
			}
		}
		else {
			return error;
		}
	}

	if (*emCalGainAvailable == 1) {
		error = ncCamGetCalibratedEmGainRange(camera, &emGainMin, &emGainMax);
		if (error) {
			return error;
		}

		//For the purpose of this example we will use the median value
		emGain = (int)(emGainMin + emGainMax) / 2;

		//Sets the calibrated EM gain on the camera
		error = ncCamSetCalibratedEmGain(camera, emGain);
		if (error) {
			return error;
		}
	}
	else if (*emRawGainAvailable == 1) {
		error = ncCamGetRawEmGainRange(camera, &emGainMin, &emGainMax);
		if (error) {
			return error;
		}

		//For the purpose of this example we will use the median value
		emGain = (emGainMin + emGainMax) / 2;

		//Sets the EM gain on the camera
		error = ncCamSetRawEmGain(camera, emGain);
		if (error) {
			return error;
		}
	}

	if (*emCalGainAvailable == 1 || *emRawGainAvailable == 1) {
		if (error != NC_SUCCESS) {
			printf("*** The camera cannot modify the EM gain at this point, please make sure there's no other application that is already communicating with the camera.\n");
		}
	}

	return error;
}


int changeAnalogGain(NcCam camera) {

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int	analogGain, analogGainMin, analogGainMax;

	error = ncCamGetAnalogGainRange(camera, &analogGainMin, &analogGainMax);
	if (error) {
		return error;
	}

	//For the purpose of this example we will use the median value
	analogGain = (analogGainMin + analogGainMax) / 2;

	//Sets the analog gain on the camera
	error = ncCamSetAnalogGain(camera, analogGain);
	if (error) {
		return error;
	}

	return error;
}


int changeAnalogOffset(NcCam camera) {

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int	analogOffset, analogOffsetMin, analogOffsetMax;

	error = ncCamGetAnalogOffsetRange(camera, &analogOffsetMin, &analogOffsetMax);
	if (error) {
		return error;
	}

	//For the purpose of this example we will use the median value
	analogOffset = (analogOffsetMin + analogOffsetMax) / 2;

	//Sets the analog offset on the camera
	error = ncCamSetAnalogOffset(camera, analogOffset);
	if (error) {
		return error;
	}

	return error;
}


//Interrogate the camera to know what values were in effect for the CCD Temperature and the gain during the last acquisition
int displayNewSettings(NcCam camera, int emCalGainAvailable, int emRawGainAvailable) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		emCalGain, emRawGain, analogGain, analogOffset;
	double	actCcdTemp, ccdTargetTemp;

	error = ncCamGetDetectorTemp(camera, &actCcdTemp);
	if (error) {
		return error;
	}

	//The previous ncCam function always polls the camera
	//The following functions will also poll the camera 
	//but may be used to verify the set values in software, 
	//without polling the camera, by passing 0 as the second argument
	error = ncCamGetTargetDetectorTemp(camera, 1, &ccdTargetTemp);
	if (error) {
		return error;
	}

	if (emCalGainAvailable == 1)
	{
		error = ncCamGetCalibratedEmGain(camera, 1, &emCalGain);
		if (error) {
			return error;
		}

	}
	else if (emRawGainAvailable == 1)
	{
		error = ncCamGetRawEmGain(camera, 1, &emRawGain);
		if (error) {
			return error;
		}
	}

	error = ncCamGetAnalogGain(camera, 1, &analogGain);
	if (error) {
		return error;
	}

	error = ncCamGetAnalogOffset(camera, 1, &analogOffset);
	if (error) {
		return error;
	}

	printf("The values we retrieved from the camera are :\n");
	printf("Actual detector temperature: %0.2lf\n", actCcdTemp);
	printf("Detector target temperature: %0.2lf\n", ccdTargetTemp);
	if (emCalGainAvailable == 1) {
		printf("Calibrated EM gain: %d\n", emCalGain);
	}
	else if (emRawGainAvailable == 1) {
		printf("Raw EM gain: %d\n", emRawGain);
	}
	printf("Analog gain: %d\n", analogGain);
	printf("Analog offset: %d\n", analogOffset);

	return error;
}
