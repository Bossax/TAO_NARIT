//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int continuousAcquisition(NcCam camera, const int nbrImagesToSave);

int main() {

	NcCam	myCam = NULL;

	int			error = NC_SUCCESS;		//We initialize an error flag variable
	const int	nbrImagesToSave = 10;	//The number of images we wish to read from the acquisition

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = continuousAcquisition(myCam, nbrImagesToSave);
	}

	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);

	return error;
}


int continuousAcquisition(NcCam camera, const int nbrImagesToSave) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		i;
	char	imageName[31];

	NcImage	*myNcImage;

	//Open the shutter for the acquisition
	enum ShutterMode mode =  OPEN;
	error = ncCamSetShutterMode(camera, mode);
	if (error) {
		return error;
	}

	//Launches continuous acquisitions by the framegrabber and the camera (this function does not wait
	//for the acquisition to be complete before returning)
	error = ncCamStart(camera, 0);
	if (error) {
		return error;
	}

	for (i = 0; i < nbrImagesToSave; ++i) {
		sprintf(imageName, "Image_%d", i);

		//Reads the image received, if a timeout occurs an error code will be returned
		error = ncCamRead(camera, &myNcImage);
		if (error) {
			return error;
		}

		printf("Saving image \"%s\"\n", imageName);

		//Saves each image acquired, at the end of each name the loop index will be added
		//"Image acquired in continuous acquisition" parameter is used to add a header to our images
		//Overwrite flag set to '1' to overwrite an existing file if it has the same name
		error = ncCamSaveImage(camera, myNcImage, imageName, FITS, "Image acquired in continuous acquisition", 1);
		if (error) {
			return error;
		}
	}

	//If the camera is in continuous acquisition, a call to ncCamAbort should be used to prevent all future acquisitions
	error = ncCamAbort(camera);
	if (error) {
		return error;
	}

	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	return error;
}
