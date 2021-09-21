//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int demo(NcCam camera);
int binningDemo(NcCam camera);
int roiDemo(NcCam camera);

int main() {

	NcCam myCam = NULL;

	int	error = NC_SUCCESS;	//We initialize an error flag variable

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
	
	NcImage	*			myNcImage = 0;
	enum ImageFormat	saveFormat = FITS;

	int	error = NC_SUCCESS;	//We initialize an error flag variable
	
	//Acquiring reference image
	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}

	//Saves the image acquired
	//"Reference Image" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "Image-Full", saveFormat, "Reference Image", 1);
	if (error) {
		return error;
	}

	error = binningDemo(camera);
	if (error) {
		return error;
	}

	error = roiDemo(camera);
	if (error) {
		return error;
	}

	return error;
}


int binningDemo(NcCam camera) {
	
	NcImage* myNcImage;
	int i;

	int error = NC_SUCCESS;		//We initialize an error flag variable
	int	binSupported;

	char imageName[128];

	printf("Checking if binning is supported and if so what are the supported modes.\n\n");

	binSupported = ncCamParamAvailable(camera, BINNING_Y, 2);

	if (binSupported == NC_SUCCESS) 
	{
		printf("Binning in Y is supported\n");

		//We set the binning mode in X to 1 (no binning), and the binning in Y to 2
		error = ncCamSetBinningMode(camera, 1, 2);
		if (error) {
			return error;
		}

		printf("Acquiring Y binning test image...\n");
		//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
		error = acquireOneImage(&camera, &myNcImage);
		if (error) {
			return error;
		}

		//Saves the image acquired
		//"Binning Test" parameter is used to add a header to our images
		//Overwrite flag set to '1' to overwrite an existing file if it has the same name
		error = ncCamSaveImage(camera, myNcImage, "X-1 Y-2 binning", FITS, "Binning Test", 1);
		if (error) {
			return error;
		}
		printf("Done.\n");
	}

	for (i = 2; i <= 16; i*=2) {
		binSupported = ncCamParamAvailable(camera, BINNING_X, i);

		if (binSupported == NC_SUCCESS) {
			printf("Binning %i in X is supported\n", i);

			//Update the image name
			sprintf(imageName, "X-%i Y-1 binning", i);

			//We set the binning mode in X to i, and the binning in Y to 1 (no binning)
			error = ncCamSetBinningMode(camera, i, 1);
			if (error) {
				return error;
			}

			printf("Acquiring X binning test image (%i binning)...\n", i);
			//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
			error = acquireOneImage(&camera, &myNcImage);
			if (error) {
				return error;
			}

			//Saves the image acquired
			//"Binning Test" parameter is used to add a header to our images
			//Overwrite flag set to '1' to overwrite an existing file if it has the same name
			error = ncCamSaveImage(camera, myNcImage, imageName, FITS, "Binning Test", 1);
			if (error) {
				return error;
			}

			printf("Done.\n");
		}
	}

	//Remove binning for the rest of the example
	error = ncCamSetBinningMode(camera, 1, 1);
	if (error) {
		return error;
	}
	
	return error;
}


int roiDemo(NcCam camera) {

	NcImage	*			myNcImage = 0;
	enum ImageFormat	saveFormat = FITS;

	int error = NC_SUCCESS;	//We initialize an error flag variable
	int fullWidth, fullHeight, roiWidth, roiHeight;

	//Recover the range available for ROIs
	error = ncCamGetMaxSize(camera, &fullWidth, &fullHeight);
	if (error) {
		return error;
	}

	//We will use ROIs a small fraction of the available image area
	roiWidth = fullWidth / 8;
	roiHeight = fullHeight / 8;

	//There must always be at least one ROI. 
	//The first ROI is initially the entire available image area,
	//with zero horizontal and vertical offset.
	//We will set its size to a small fraction of that available;
	//the offset will be unchanged
	error = ncCamSetMRoiSize(camera, 0, roiWidth, roiHeight);
	if (error) {
		return error;
	}

	//Synchronise the camera with the required ROI configuration
	error = ncCamMRoiApply(camera);
	if (error) {
		return error;
	}

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &myNcImage);
	if (error) {
		return error;
	}

	printf("Acquired an image with a %d by %d region of interest\n", roiWidth, roiHeight);

	//Saves the image acquired
	//"ROI Test" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, myNcImage, "Image-ROI", saveFormat, "ROI Test", 1);
	if (error) {
		return error;
	}

	//Set ROI back to full size for the rest of the example
	error = ncCamSetMRoiSize(camera, 0, fullWidth, fullHeight);
	if (error) {
		return error;
	}

	//Synchronise the camera with the required ROI configuration
	error = ncCamMRoiApply(camera);
	if (error) {
		return error;
	}

	return error;
}
