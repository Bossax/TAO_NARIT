//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int dataCubesDemo(NcCam camera, const int nbrImagesToSave);

int main() {

	NcCam		myCam = NULL;

	int			error = NC_SUCCESS;		//We initialize an error flag variable
	const int	nbrImagesToSave = 15;	//The number of images we wish to save

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = dataCubesDemo(myCam, nbrImagesToSave);
	}

	if (error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);

	return error;
}


int dataCubesDemo(NcCam camera, const int nbrImagesToSave) {

	NcImage				*myNcImage;
	enum ImageFormat	saveFormat = FITS;

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int	i;

	printf("Saving %d images, 5 images per file\n", nbrImagesToSave);

	//This function starts a separate image saving thread.
	//This thread saves all the acquired images into a data cube
	//We use cubes of 5 image and we set the number of cubes to 0 so cubes of images
	//will be created until the ncCamStopSaveAcquisition is called.
	//"Images grouped in data cubes of 5 images" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamStartSaveAcquisition(camera, "DataCube", saveFormat, 5, "Images grouped in data cubes of 5 images", 0, 1);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	//Launches an acquisition on the frame grabber and requests images from the camera (this function
	//does not wait for the acquisition to be complete before returning)
	error = ncCamStart(camera, 0);
	if (error) {
		return error;
	}

	//Loop in which acquired images are read
	for (i = 0; i < nbrImagesToSave; ++i)
	{
		printf("Reading image %d\n", i+1);
		//Reads the image received, if a timeout occurs an error code will be returned
		error = ncCamRead(camera, &myNcImage);
		if (error) {
			return error;
		}
	}

	//If the camera is in continuous acquisition, a call to ncCamAbort should be used to prevent all future acquisitions
	error = ncCamAbort(camera);
	if (error) {
		return error;
	}

	//Close the shutter, now that the acquistion is complete.
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	//Stops the separate thread that saves the images
	error = ncCamStopSaveAcquisition(camera);
	if (error) {
		return error;
	}

	return error;
}
