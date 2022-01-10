//This is a header file that needs to be added to all your projects as all the Nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>


int timedAcquisition(NcCam camera, const int nbrImages);

int main() {

	NcCam		myCam = NULL;

	int			error = NC_SUCCESS;		//We initialize an error flag variable
	const int	nbrImages = 15;			//The number of images we wish to acquire, we suppose that nbrImages is greater than 0	
	
	error = initialize(&myCam);
		
	if (error == NC_SUCCESS) {
		error = timedAcquisition(myCam, nbrImages);
	}

	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}
	
	cleanUp(myCam);
	
	return error;
}


int timedAcquisition(NcCam camera, const int nbrImages) {

	NcImage	*myNcImage;

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		i;
	double	ncImageTime;
	char	imageName[31];


	//Start the timer
	error = ncCamResetTimer(camera, 0);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	//Launches an acquisition on the frame grabber and requests an image from the camera (this function
	//does not wait for the acquisition to be complete before returning)
	error = ncCamStart(camera, nbrImages);
	if (error) {
		return error;
	}

	//Loop in which the acquired images are read
	for (i = 0; i < nbrImages; ++i)
	{
		//Reads the image received, if a timeout occurs an error code will be returned
		error = ncCamReadTimed(camera, &myNcImage, &ncImageTime);
		if (error) {
			return error;
		}
			
		//Saves each image acquired, at the end of each name the loop index will be added
		sprintf(imageName, "Image %d", i);

		//Saves the image acquired
		//"Timed acquisition test" parameter is used to add a header to our images
		//Overwrite flag set to '1' to overwrite an existing file if it has the same name
		error = ncCamSaveImage(camera, myNcImage, imageName, FITS, "Timed acquisition test", 1);
		if (error) {
			return error;
		}
			
		printf("Saved %s with timestamp %g ms\n", imageName, ncImageTime);
	}
	printf("\n");

	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	return error;
}
