//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>
#include <time.h>

//This function does the same operations as "initialize" from "utility.h".  They only differ in
//the values they assign to the readout time, the exposure time and the waiting time
int initializeForTimestamp(NcCam* camera);
int internalTimestamp(NcCam camera, const int nbrImages);
void printTimestamp(struct tm * timestamp, const double fraction);

int main() {

	NcCam myCam = NULL;

	int			error = NC_SUCCESS;		//We initialize an error flag variable
	const int	nbrImages = 15;			//The number of images we wish to acquire, we suppose that nbrImages is greater than 0	

	error = initializeForTimestamp(&myCam);

	if (error == NC_SUCCESS) {
		error = internalTimestamp(myCam, nbrImages);
	}

	if(error != NC_SUCCESS) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}
	
	cleanUp(myCam);

	return error;
}


int initializeForTimestamp(NcCam* camera) {

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	double	readoutTime, exposure, exposureTime;

	//Opens the acquisition channel using automatic detection and 4 loop buffers (recommended)
	error = ncCamOpen(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, camera);
	if (error) {
		return error;
	}

	//For this example we will not bother checking for the available readout modes on the camera and
	//simply select the first one as it is the prefered mode and always valid
	error = ncCamSetReadoutMode(*camera, 1);
	if (error) {
		return error;
	}

	// Recover the readout time for comparison later
	error = ncCamGetReadoutTime(*camera, &readoutTime);
	if (error) {
		return error;
	}
	
	//The camera cannot return images faster than the readout time plus the waiting time
	//We set the waiting time to zero and make the exposure a little longer than the readout time
	//In this way the increment in time from one image to the next should be the exposure time
	error = ncCamSetWaitingTime(*camera, 0);
	if (error) {
		return error;
	}

	exposure = readoutTime + 10.0;
	error = ncCamSetExposureTime(*camera, exposure);
	if (error) {
		return error;
	}

	//Clock cycles on the camera controller may not allow the exact requested exposure 
	//but the true value will be very close
	//Verify the exposure set on the camera
	error = ncCamGetExposureTime(*camera, 1, &exposureTime);
	if (error) {
		return error;
	}

	//Set a reasonable timeout on reading an image
	error = ncCamSetTimeout(*camera, (int)exposureTime + 1000);
	if (error) {
		return error;
	}
	
	return error;
}


int internalTimestamp(NcCam camera, const int nbrImages) {

	NcImage 		*myNcImage;
	enum ImageFormat saveFormat = FITS;

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		i;
	char	imageName[31];

	struct tm 	dateTime;
	double 		secondFraction;
	double 		secsInDay = 0., secsInDay_diff = 0., secsInDay_previous = 0.;
	int 		flags;
	int			overruns = 0;


	//Fix the type of controller timestamp to INTERNAL_TIMESTAMP: the controller's clock will be synchronised with the host
	//If a GPS is available GPS_TIMESTAMP can be used
	//To disable the timestamp, use NO_TIMESTAMP but, in this example, it will cause an error to be returned below
	error = ncCamSetTimestampMode(camera, INTERNAL_TIMESTAMP);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	//Launch a series of acquisition on the frame grabber
	error = ncCamStart(camera, 0);
	if (error) {
		return error;
	}

	printf("Image timestamps :\n");

	for (i = 0; i<nbrImages; i++)
	{
		//Read the image received, if a timeout occurs an error code will be returned
		error = ncCamRead(camera, &myNcImage);
		if (error) {
			return error;
		}

		//Read the timestamp of the image acquired
		error = ncCamGetCtrlTimestamp(camera, myNcImage, &dateTime, &secondFraction, &flags);
		if (error) {
			return error;
		}

		// Display the timestamp
		printTimestamp(&dateTime, secondFraction);

		//Save each image acquired, at the end of each name the loop index is added
		//"This is part of a series of timestamped images." parameter is used to add a header to our images
		//Overwrite flag set to '1' to overwrite an existing file if it has the same name
		sprintf(imageName, "Image %d", i);
		error = ncCamSaveImage(camera, myNcImage, imageName, saveFormat, "This is part of a series of timestamped images.", 1);
		if (error) {
			return error;
		}
	}

	printf("\n");

	//Checks if an overrun occured on the last image (implying that the buffer we are reading has
	//been overwritten prior to this "read" call)
	error = ncCamGetNbrDroppedImages(camera, &overruns);
	if (error) {
		return error;
	}
	if (overruns == 1)
	{
		printf("Warning : 1 image was lost\n");
	}
	else if (overruns > 1)
	{
		printf("Warning : %d images were lost\n", overruns);
	}

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


void printTimestamp(struct tm * timestamp, const double fraction)
{
	printf("%.2d:%.2d:%.2d.%.3d\n", 
		timestamp->tm_hour,
		timestamp->tm_min,
		timestamp->tm_sec, 
		(int)(fraction * 1000));
}
