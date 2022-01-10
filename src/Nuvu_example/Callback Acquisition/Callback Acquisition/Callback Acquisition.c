//Header file that need to be added to all your projects as all the the nc functions are define in there
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#ifdef WINDOWS
//Include windows header for the Sleep function
#include <Windows.h>
#endif

#include <stdio.h>

int callBackAcquisition(NcCam camera);
void imageCallback(void* camera);

//Global function used to advise the main thread once the callback is complete
//We set the global variable to 0 since it will be modified to 1 once the callback is done
int callbackEnded = 0;

int main() {

	NcCam	myCam = NULL;
	int		error = NC_SUCCESS;		//We initialize an error flag variable

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = callBackAcquisition(myCam);
	}
	
	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);

	return error;
}


int callBackAcquisition(NcCam camera) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	
	//We set a callback which will be called once an image is completely in memory and available to be read
	error = ncCamSetEvent(camera, imageCallback, camera);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	//Launching 1 acquisition on the framegrabber (this function doesn't wait for the acquisition to be 
	//complete before returning)
	error = ncCamStart(camera, 1);
	if (error) {
		return error;
	}
	
	//Since we want the callback function to be done prior to close this example, we wait for it with the callbackEnded variable
	while (callbackEnded != 1) {
	#ifdef WINDOWS
		Sleep(10);
	#else
		usleep(10000);
	#endif
	}

	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	return error;
}


void imageCallback(void* camera) {

	NcImage *myNcImage;
	NcCam	myCam = (NcCam)camera;

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	
	//The image requested by the "ncCamStart" function in the main thread is being read
	error = ncCamRead(myCam, &myNcImage);
	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
		callbackEnded = 1;
		return;
	}

	printf("Obtained an image\n");
	
	//We save the image acquired
	//"This is an image grabbed from a callback function" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(myCam, myNcImage, "callbackImage", FITS, "This is an image grabbed from a callback function", 1);
	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
		callbackEnded = 1;
		return;
	}

	//The global variable "callbackEnded" is set to 1, so the example can now be terminated
	callbackEnded = 1;
}
