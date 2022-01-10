//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int callbackSaveImage(NcCam camera);

//In this example, the callback function simply saves the acquired images
void callbackFunction(NcCam camera, NcImageSaved* imageSaved, void* dummy);

int main() {

	NcCam myCam = NULL;
	int 	error = NC_SUCCESS;		//We initialize an error flag global variable 
	
	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = callbackSaveImage(myCam);
	}
	
	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}
	
	cleanUp(myCam);

	return error;
}


int callbackSaveImage(NcCam camera) {

	NcImage*	myNcImage = NULL;
	int			error = NC_SUCCESS;		//We initialize an error flag global variable 

	//Sets a callback which will be called when an image is being saved.  Allows to include additional headers to the image being saved
	error = ncCamSaveImageSetHeaderCallback(camera, callbackFunction, &camera);
	if (error) {
		return error;
	}

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}
	
	printf("Saving an image to the file \"newImage\"\n");

	//Saves the image acquired.  The callback is called while this function is being executed.  This function will not return until the callback is completed
	//"This is an image on which an extra header will be added" parameter is used to add a header to our image
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "newImage", FITS, "This is an image on which an extra header will be added", 1);
	if (error) {
		return error;
	}

	return error;
}


void callbackFunction(NcCam camera, NcImageSaved* imageSaved, void* dummy) {
	
	int		error = NC_SUCCESS;		//We initialize an error flag global variable 
	double	currentTemp;

	//Inquires for the current temperature of the camera as it is not included in the default header
	error = ncCamGetDetectorTemp(camera, &currentTemp);
	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
		return;
	}
	
	printf("Appending the detector temperature (%.2lfC) to the image headers\n", currentTemp);

	//Adds the current temperature to the header of the image being saved
	error = ncWriteFileHeader(imageSaved, NC_DOUBLE, "ccd_temp", &currentTemp, "Current temperature on the detector");
	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
		return;
	}
}
