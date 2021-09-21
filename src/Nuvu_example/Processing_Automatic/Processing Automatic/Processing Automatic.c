//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int processingAutomatic(NcCam camera);

int main() {

	NcCam	myCam = NULL;
	int		error = NC_SUCCESS;		//We initialize an error flag variable

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = processingAutomatic(myCam);
	}
	
	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);
	
	return error;
}

void biasCallback(NcCam cam, int imageNo, void *data)
{
	(void)data;
	(void)cam;

	printf("\rAcquired bias image %d", imageNo);
}


int processingAutomatic(NcCam camera) {

	NcImage *myNcImage = NULL;
	int			error = NC_SUCCESS;		//We initialize an error flag variable
	const int	biasStackSize = 50;		//The number of images to acquire the bias


	error = ncCamCreateBiasNewImageCallback(camera, biasCallback, NULL);
	if (error) {
		return error;
	}

	printf("Acquiring bias ...\n");

	//Creates the bias with 200 acquisitions.  This function will take sometime prior to returning (200 times the acquisition rate)
	//BIAS_DEFAULT means the camera shutter will be closed during the acquisition.  It will return to its previous state afterwards
	error = ncCamCreateBias(camera, biasStackSize, BIAS_DEFAULT);
	if (error) {
		return error;
	}

	printf("Done\n");

	//Sets the processing type for the bias subtraction. (Last parameter ignored)
	error = ncCamSetProcType(camera, LM, 0);
	if (error) {
		return error;
	}

	printf("Acquiring bias-subtracted image ...\n");

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}

	//Saves the image acquired
	//"This is an image with bias subtraction" parameter is used to add a header to our image
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "imageInLM", FITS, "This is an image with bias subtraction", 1);
	if (error) {
		return error;
	}

	printf("Done\n");

	//Sets the processing type to photon counting with 10 acquisitions per image
	error = ncCamSetProcType(camera, PC, 10);
	if (error) {
		return error;
	}

	//If there was enough illumination for the LM image acquired above,  
	//even nominally zero exposure will likely give too much light for photon-counting
	error = ncCamSetExposureTime(camera, 0.0);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}
	
	printf("Acquiring photon-counting image stack ...\n");

	//Launches a continuous acquisition by the frame grabber (this function does not wait for the acquisition to be complete before returning)
	error = ncCamStart(camera, 0);
	if (error) {
		return error;
	}
	
	//Reads the image created once the stack has been acquired, if a timeout occurs an error code will be returned
	error = ncCamRead(camera, &myNcImage);
	if (error) {
		return error;
	}

	printf("Done\n");

	//We have acquired enough images so we tell the frame grabber to stop acquiring frames
	//and the camera to stop sending images to the frame grabber
	error = ncCamAbort(camera);
	if (error) {
		return error;
	}
	
	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	//Saves the single image created from the ten images acquired
	//"This is an image in photon counting" parameter is used to add a header to our image
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "imageInPC", FITS, "This is an image in photon counting", 1);
	if (error) {
		return error;
	}

	return error;
}
