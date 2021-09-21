//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>

int acquireReadoutModes(NcCam camera,  int* nbrReadoutModes);
int acquireReadoutModesByAmpliTypes(NcCam camera);
int readoutModesAvailability(NcCam camera, int nbrReadoutModes);

int main() {

	NcCam myCam = NULL;

	int	error = NC_SUCCESS;		//We initialize an error flag variable
	int ncNbrReadoutModes;		//The number of readout modes available

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		error = acquireReadoutModes(myCam, &ncNbrReadoutModes);
	}

	if (error == NC_SUCCESS) {
		error = acquireReadoutModesByAmpliTypes(myCam);
	}

	if (error == NC_SUCCESS) {
		error = readoutModesAvailability(myCam, ncNbrReadoutModes);
	}

	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	cleanUp(myCam);

	return error;
}


int acquireReadoutModes(NcCam camera, int* nbrReadoutModes) {
	
	int			i, error = NC_SUCCESS;		//We initialize an error flag variable
	enum Ampli	ncAmpliNo;
	int			vertFreq, horizFreq;
	char		ncAmpliString[32];
	
	//Investigates about the readout modes available on the camera
	error = ncCamGetNbrReadoutModes(camera, nbrReadoutModes);
	if (error) {
		return error;
	}

	printf("READOUT MODES AVAILABLE\n\n");

	for (i = 1; i <= *nbrReadoutModes; ++i) {
		error = ncCamGetReadoutMode(camera, i, &ncAmpliNo, ncAmpliString, &vertFreq, &horizFreq);
		//If there is an error, we exit the function to prevent unncessary looping
		if (error) {
			return error;
		}

		//Print out the readout mode
		printf("Readout mode %i :\n", i);
		printf("Ampli No : %i\n", ncAmpliNo);
		printf("Vertical frequency : %i Hz\n", vertFreq);
		printf("Horizontal frequency : %i Hz\n\n", horizFreq);
	}

	return error;
}


int acquireReadoutModesByAmpliTypes(NcCam camera) {
	
	int		i, error = NC_SUCCESS;		//We initialize an error flag variable
	int		ncAmpliAvail, vertFreq, horizFreq, readoutModeNo;
	
	//Second method to inquire the available readout modes, doing so per Ampli type
	error = ncCamGetAmpliTypeAvail(camera, CONV, &ncAmpliAvail);
	if (error) {
		return error;
	}

	printf("READOUT MODES AVAILABLE BY AMPLI TYPE\n\n");

	if (ncAmpliAvail) {
		for (i = 1; i <= ncAmpliAvail; ++i) 
		{
			error = ncCamGetFreqAvail(camera, CONV, i, &vertFreq, &horizFreq, &readoutModeNo);
			//If there is an error, we exit the function to prevent unncessary looping
			if (error) {
				return error;
			}

			//Print out the readout mode
			printf("Ampli No : %i\t(CONV)\n", i);
			printf("Readout mode %i :\n", readoutModeNo);
			printf("Vertical frequency : %i Hz\n", vertFreq);
			printf("Horizontal frequency : %i Hz\n\n", horizFreq);
		}
	}

	error = ncCamGetAmpliTypeAvail(camera, EM, &ncAmpliAvail);
	if (error) {
		return error;
	}

	if (ncAmpliAvail) {
		for (i = 1; i <= ncAmpliAvail; ++i) {
			error = ncCamGetFreqAvail(camera, EM, i, &vertFreq, &horizFreq, &readoutModeNo);
			//If there is an error, we exit the function to prevent unncessary looping
			if (error) {
				return error;
			}

			//Print out the readout mode
			printf("Ampli No : %i\t(EM)\n", i);
			printf("Readout mode %i :\n", readoutModeNo);
			printf("Vertical frequency : %i Hz\n", vertFreq);
			printf("Horizontal frequency : %i Hz\n\n", horizFreq);
		}
	}

	return error;
}


int readoutModesAvailability(NcCam camera, int nbrReadoutModes) {
	
	NcImage		*myNcImage;
	int			i, error = NC_SUCCESS;		//We initialize an error flag variable
	double		readoutTime;
	char		filename[64];
	
	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	// Set the exposure and waiting time to zero to acquire as quickly as possible
	error = ncCamSetExposureTime(camera, 0.0);
	if (error) {
		return error;
	}
	error = ncCamSetWaitingTime(camera, 0.0);
	if (error) {
		return error;
	}

	if (nbrReadoutModes > 0) {
		//Testing all the readout modes available
		for (i = 1; i <= nbrReadoutModes; ++i) {
			printf("Setting readout mode %d...", i);
			error = ncCamSetReadoutMode(camera, i);
			if (error) {
				return error;
			}

			// Update the timeout to account for the new 
			error = ncCamGetReadoutTime(camera, &readoutTime);
			if (error) {
				return error;
			}
			error = ncCamSetTimeout(camera, (int)readoutTime + 1000);


			//Launches an acquisition by the framegrabber and requests an image from the camera (this function
			//does not wait for the acquisition to be complete before returning)
			printf(" Acquiring (%.3lf s)...", (readoutTime / 1000.0));
			error = ncCamStart(camera, 1);
			if (error) {
				return error;
			}

			//Reads the image received, if a timeout occurs an error code will be returned
			error = ncCamRead(camera, &myNcImage);
			if (error) {
				return error;
			}

			//Saves the image acquired
			//"This is an image grab with the readout mode selected" parameter is used to add a header to our image
			//Overwrite flag set to '1' to overwrite an existing file if it has the same name
			sprintf(filename, "readout_mode_%d", i);
			error = ncCamSaveImage(camera, myNcImage, filename, FITS, "This is an image grab with the readout mode selected", 1);
			if (error) {
				return error;
			}
			printf(" Saved.\n");
		}

		//Close the shutter, now that the acquistion is complete
		error = ncCamSetShutterMode(camera, CLOSE);
		if (error) {
			return error;
		}
	}

	return error;
}
