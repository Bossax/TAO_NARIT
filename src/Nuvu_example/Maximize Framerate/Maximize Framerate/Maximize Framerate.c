//This is a header file that needs to be added to all your projects as all the Nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Function containing the example
int demo(NcCam camera);

//Full image test
int testFullImage(NcCam camera, double* readoutTimeFull, double* acquisitionTimeFull);
//Binning test
int testBinImage(NcCam camera, double* readoutTimeBin, double* acquisitionTimeBin);
//Binning + ROI test
int testBinRoiImage(NcCam camera, double* readoutTimeRoi, double* acquisitionTimeRoi);
//Binning + ROI + crop mode test
int testBinRoiCropImage(NcCam camera, double* readoutTimeCrop, double* acquisitionTimeCrop);

int timedAcquisitionLoop(NcCam myCam, NcImage* myNcImg, enum ImageFormat saveFormat, char* imageNameRoot, double* acquisitionTime_msec, int* overrun);
int mk_secsInDay(struct tm * time);
int listBestCropModeSolutions(NcCropModeSolutions solutions, int maxNumberDisplayed);


int main() {
	
	NcCam myCam = NULL;
	int error = NC_SUCCESS;		//We initialize an error flag variable
	
	//Opens the acquisition channel using the automatic detection and 4 loop buffers (recommended)
	error = ncCamOpen(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, &myCam);
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

	int error = NC_SUCCESS;		//We initialize an error flag variable

	double	readoutTime_Full = 0.0, readoutTime_Bin = 0.0, readoutTime_ROI = 0.0, readoutTime_Crop = 0.0;
	double	acquisitionTime_Full = 0.0, acquisitionTime_Bin = 0.0, acquisitionTime_ROI = 0.0, acquisitionTime_Crop = 0.0;

	printf("=============================================================================\n");
	printf("This example will use various methods to accelerate the readout of the camera\n");
	printf("and compare the acquisition rates.\n");
	printf("=============================================================================\n");

	error = testFullImage(camera, &readoutTime_Full, &acquisitionTime_Full);
	if (error) {
		return error;
	}

	printf("\n");
	printf("==================================================\n\n");

	error = testBinImage(camera, &readoutTime_Bin, &acquisitionTime_Bin);
	if (error) {
		return error;
	}

	printf("\n");
	printf("==================================================\n\n");

	error = testBinRoiImage(camera, &readoutTime_ROI, &acquisitionTime_ROI);
	if (error) {
		return error;
	}

	printf("\n");
	printf("==================================================\n\n");

	error = testBinRoiCropImage(camera, &readoutTime_Crop, &acquisitionTime_Crop);
	if (error) {
		return error;
	}

	printf("\n\n\n");

	//Display the results
	printf("Mode\tReadout Time (ms/image)\tAcquisition Time (ms/image)\tReal Speed-up\n");
	printf("Full\t%14.3f\t\t%14.3f\t\t   %14.3f\n", readoutTime_Full, acquisitionTime_Full, 1.0);
	if (acquisitionTime_Bin > 0.)
		printf("Bin \t%14.3f\t\t%14.3f\t\t   %14.3f\n", readoutTime_Bin, acquisitionTime_Bin, acquisitionTime_Full / acquisitionTime_Bin);
	if (acquisitionTime_ROI > 0.)
		printf("ROI \t%14.3f\t\t%14.3f\t\t   %14.3f\n", readoutTime_ROI, acquisitionTime_ROI, acquisitionTime_Full / acquisitionTime_ROI);
	if (acquisitionTime_Crop > 0.)
		printf("Crop\t%14.3f\t\t%14.3f\t\t   %14.3f\n", readoutTime_Crop, acquisitionTime_Crop, acquisitionTime_Full / acquisitionTime_Crop);
	printf("\n\n");

	return error;
}


int testFullImage(NcCam camera, double* readoutTimeFull, double* acquisitionTimeFull) {

	NcImage*			myNcImage = NULL;
	enum ImageFormat	saveFormat = FITS;

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		overrunTotal = 0;
	char	imageName[32];

	//We set both exposure and waiting time to zero so that the achieved frame rate is the maximum possible
	error = ncCamSetExposureTime(camera, 0.0);
	if (error) {
		return error;
	}

	error = ncCamSetWaitingTime(camera, 0.0);
	if (error) {
		return error;
	}

	//For this example we will not bother checking for the available readout modes on the camera and
	//simply select the first one as it is the prefered mode and always valid
	error = ncCamSetReadoutMode(camera, 1);
	if (error) {
		return error;
	}

	//Recover the readout time for comparison later
	error = ncCamGetReadoutTime(camera, readoutTimeFull);
	if (error) {
		return error;
	}

	//Set a reasonable timeout on reading an image
	error = ncCamSetTimeout(camera, (int)(*readoutTimeFull) + 1000); 
	if (error) {
		return error;
	}

	printf("We'll now launch a full-frame continuous acquisition to validate it.\n\n");
	//The timedAcquisitionLoop will save, for reference, the last ten images acquired in the sequence
	sprintf(imageName, "Image-Full_");	// Number of image will be appended
	overrunTotal = 0;
	error = timedAcquisitionLoop(camera, myNcImage, saveFormat, imageName, acquisitionTimeFull, &overrunTotal);
	if (error) {
		return error;
	}

	if (overrunTotal > 0) {
		int droppedTotal;
		error = ncCamGetNbrDroppedImages(camera, &droppedTotal);
		if (error) {
			return error;
		}
		printf("WARNING: Over-run occurred (%d times with %d dropped!) during the acquisition.\n", overrunTotal, droppedTotal);
	}

	return error;
}


int testBinImage(NcCam camera, double* readoutTimeBin, double* acquisitionTimeBin) {

	NcImage*			myNcImage = 0;
	enum ImageFormat	saveFormat = FITS;
	
	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		overrunTotal = 0;
	int		i, binSupported;
	char	imageName[31];

	//We will retrieve these values to set an appropriate binning for the rest of this example
	//We are going to limit their value to 16
	int highestXbin = 1, highestYBin = 1;

	printf("Checking if binning is supported and if so what are the supported modes.\n");
	printf("Using maximum binning avaible up to 16.\n\n");

	binSupported = ncCamParamAvailable(camera, BINNING_Y, 2);

	if (binSupported == NC_SUCCESS) {
		printf("Binning in Y is supported\n");
		highestYBin = 16;
	}

	for (i = 2; i <= 16; i *= 2) {
		binSupported = ncCamParamAvailable(camera, BINNING_X, i);

		if (binSupported == NC_SUCCESS) {
			printf("Binning %i in X is supported\n", i);
			highestYBin = i;
		}
	}

	//Max binning to maximize the acquisition rate
	error = ncCamSetBinningMode(camera, highestXbin, highestYBin);
	if (error) {
		return error;
	}

	//Recover the readout time for comparison later
	error = ncCamGetReadoutTime(camera, readoutTimeBin);
	if (error) {
		return error;
	}

	printf("We'll now launch a full-frame continuous acquisition to validate it.\n\n");
	//The timedAcquisitionLoop will save, for reference, the last ten images acquired in the sequence
	sprintf(imageName, "Image-Bin_");	//Number of image will be appended

	overrunTotal = 0;
	error = timedAcquisitionLoop(camera, myNcImage, saveFormat, imageName, acquisitionTimeBin, &overrunTotal);
	if (error) {
		return error;
	}

	if (overrunTotal > 0) {
		int droppedTotal;
		error = ncCamGetNbrDroppedImages(camera, &droppedTotal);
		if (error) {
			return error;
		}
		printf("WARNING: Over-run occurred (%d times with %d dropped!) during the acquisition.\n", overrunTotal, droppedTotal);
	}

	return error;
}


int testBinRoiImage(NcCam camera, double* readoutTimeRoi, double* acquisitionTimeRoi) {
	
	NcImage*			myNcImage = 0;
	enum ImageFormat	saveFormat = FITS;

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		overrunTotal = 0;
	int		roiWidth, roiHeight, fullWidth, fullHeight;
	char	imageName[31];

	printf("Testing ROI...\n");

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
	//with zero horizintal and vertical offset.
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

	//Recover the readout time for comparison later
	error = ncCamGetReadoutTime(camera, readoutTimeRoi);
	if (error) {
		return error;
	}

	printf("We'll now launch a continuous acquisition to validate it.\n\n");
	//The timedAcquisitionLoop will save, for reference, the last ten images acquired in the sequence
	sprintf(imageName, "Image-ROI_");	//Number of image will be appended

	overrunTotal = 0;
	error = timedAcquisitionLoop(camera, myNcImage, saveFormat, imageName, acquisitionTimeRoi, &overrunTotal);
	if (error) {
		return error;
	}

	if (overrunTotal > 0) {
		int droppedTotal;
		error = ncCamGetNbrDroppedImages(camera, &droppedTotal);
		if (error) {
			return error;
		}
		printf("WARNING: Over-run occurred (%d times with %d dropped!) during the acquisition.\n", overrunTotal, droppedTotal);
	}

	return error;
}


int testBinRoiCropImage(NcCam camera, double* readoutTimeCrop, double* acquisitionTimeCrop) {
	
	NcImage	*				myNcImage = 0;
	enum ImageFormat		saveFormat = FITS;
	NcCropModeSolutions 	solutionSet;

	int		error = NC_SUCCESS;		//We initialize an error flag variable
	int		overrunTotal = 0;
	int 	numSolutionsToShow = 5; //Maximum number of crop-mode solutions to display
	int		roiWidth, roiHeight;
	int 	paddingMin = 0; //Minimum padding parameter for crop-mode
	char	imageName[31];
	float	figureOfMerit;

	printf("Now we'll investigate the use of crop-mode\n");
	printf("A number of solutions are available for a given ROI width at different X-offsets.\n\n");

	//Get ROI size
	error = ncCamGetMRoiSize(camera, 0, &roiWidth, &roiHeight);
	if (error) {
		return error;
	}

	//To preview the optimum locations for the ROI when used with crop-mode we create an NcCropModeSolutions object
	paddingMin = 0;
	error = ncCropModeSolutionsOpen(&solutionSet, roiWidth, roiHeight, CROP_MODE_ENABLE_X, paddingMin, 0, camera);
	if (error) {
		return error;
	}

	error = listBestCropModeSolutions(solutionSet, numSolutionsToShow);
	if (error) {
		return error;
	}

	printf("\nThe above solutions are valid for crop-mode's default padding (0) \n");
	printf(" which maximises readout rate.\n");
	printf("However, this may introduce non - uniform noise into the lines of the image.\n");
	printf("Uniform noise can be imposed upon the crop-mode solutions by specifying\n");
	printf(" negative padding (-1), the effect of which is seen below : \n");
	//We can modify the parameters we intend to use with crop-mode without creating a new NcCropModeSolutions object
	paddingMin = -1;
	error = ncCropModeSolutionsSetParameters(solutionSet, roiWidth, roiHeight, CROP_MODE_ENABLE_X, paddingMin, 0);
	if (error) {
		return error;
	}

	error = listBestCropModeSolutions(solutionSet, numSolutionsToShow);
	if (error) {
		return error;
	}

	//We're finished with the NcCropModeSolutions object
	error = ncCropModeSolutionsClose(solutionSet);
	if (error) {
		return error;
	}

	//For the rest of this example we will use default padding (0)
	printf("Proceeding with default padding.\n");
	paddingMin = 0;


	//======================================== 
	printf("Enabling crop-mode\n");
	error = ncCamSetCropMode(camera, CROP_MODE_ENABLE_X, paddingMin, 0);
	if (error) {
		return error;
	}

	//The Figure of Merit is an approximation of the maximum possible speed-up relative to the standard ROI readout
	error = ncCamGetCropMode(camera, 0, 0, 0, &figureOfMerit);
	if (error) {
		return error;
	}
	printf("This readout is up to %.2f times faster than standard ROI above.\n", figureOfMerit);

	//Recover the readout time for comparison later
	error = ncCamGetReadoutTime(camera, readoutTimeCrop);
	if (error) {
		return error;
	}

	printf("We'll now launch a continuous acquisition to validate it.\n\n");
	//The timedAcquisitionLoop will save, for reference, the last ten images acquired in the sequence
	sprintf(imageName, "Image-CropMode_");	//Number of image will be appended.

	overrunTotal = 0;
	error = timedAcquisitionLoop(camera, myNcImage, saveFormat, imageName, acquisitionTimeCrop, &overrunTotal);
	if (error) {
		return error;
	}

	if (overrunTotal > 0) {
		int droppedTotal;
		error = ncCamGetNbrDroppedImages(camera, &droppedTotal);
		if (error) {
			return error;
		}
		printf("WARNING: Over-run occurred (%d times with %d dropped!) during the acquisition.\n", overrunTotal, droppedTotal);
	}

	//We disable crop mode
	error = ncCamSetCropMode(camera, CROP_MODE_DISABLE, 0, 0);
	if (error) {
		return error;
	}

	return error;
}



//Provides a continuous acquisition with measurement of time per image
//The last ten images are saved for reference
int timedAcquisitionLoop(NcCam myCam, NcImage* myNcImg, enum ImageFormat saveFormat, char* imageNameRoot, double* acquisitionTime_msec, int* overrun) {

	int 	error = NC_SUCCESS;		//We initialize an error flag variable
	int		overrunFlag = 0, overrunTotal = 0;

	int 	numberImages = 1000;
	int 	runInImages = 10;
	int		i = 0 - runInImages;	//Image index; starts negative for warm-up
	char	imageName[35], status[8] = "WARMUP";

	//Variables for timing
	double		start_msec = 0., stop_msec = 0.;
	short 		controllerTimestampAvailable = 1;
	struct tm 	imageTime;	//For use with the controller timestamp: CTRL_TIMESTAMP
	double 		imageTimeFraction = 0.; //For use with the controller timestamp: CTRL_TIMESTAMP
	double 		imageTime_msec = 0.;

	error = ncCamParamAvailable(myCam, CTRL_TIMESTAMP, 0);
	if (error == NC_ERROR_CAM_NO_FEATURE || error == NC_ERROR_GRAB_FIRMWARE_VERSION) {	//Can't use internal timestamp, act accordingly; not a fatal error
		controllerTimestampAvailable = 0;
	}
	else if (error) {	//Some other error occured 
		return error;
	}

	if (controllerTimestampAvailable == 1) {
		printf("Using internal timestamp for timing.\n");
		error = ncCamSetTimestampMode(myCam, INTERNAL_TIMESTAMP);
		if (error) {
			return error;
		}
	}
	else {
		printf("Using host-system timestamp for timing.\n");
		error = ncCamResetTimer(myCam, 0.0);
		if (error) {
			return error;
		}
	}

	printf("We will acquire %d images for timing\n", numberImages);

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(myCam, OPEN);
	if (error) {
		return error;
	}

	printf("Images received for timing:\n");

	//Launches a continuous acquisition by the frame grabber and the camera (this function does not wait
	//for the acquisition to be complete before returning)
	error = ncCamStart(myCam, 0);
	if (error) {
		return error;
	}

	//Loop to read the acquired images
	for (i = 0; i <= numberImages; ++i) {
		//Reads the image received, returning when the image is received; if a timeout occurs an error code will be returned
		error = ncCamRead(myCam, &myNcImg);
		if (error) {
			return error;
		}

		//Deal with timestamps
		if (controllerTimestampAvailable == 1) {
			error = ncCamGetCtrlTimestamp(myCam, myNcImg, &imageTime, &imageTimeFraction, 0);
			if (error == NC_ERROR_GRAB_NO_TIMESTAMP) {	//Can't use internal timestamp, fall-back to host timestamp; not a fatal error
				controllerTimestampAvailable = 0;
				printf("\n");	//Preserve the number of images so far
				printf("Using host-system timestamp for timing for remaining images ...\n");
				error = ncCamResetTimer(myCam, 0.0);
				if (error) {
					return error;
				}
			}
			else if (error)	{ //Some other error occured 
				return error;
			}
			else {	//Controller timestamp is good 
				imageTime_msec = 1000. * (mk_secsInDay(&imageTime) + imageTimeFraction);
			}
		}

		if (controllerTimestampAvailable == 0) {
			error = ncCamGetHostSystemTimestamp(myCam, myNcImg, &imageTime_msec);
			if (error) {
				return error;
			}
		}

		//Monitor progress
		if (i == 0) { //The acquisition sequence is well under way; start timing
			sprintf(status, "TIMING");
			start_msec = imageTime_msec;
		}
		else if (i == numberImages) { //That's enough images; stop timing 
			sprintf(status, "TIMED ");
			stop_msec = imageTime_msec;
		}

		//Checks if an overrun occured on the last image (implying that the buffer we are reading has
		//been overwritten prior to this "read" call)
		error = ncCamGetOverrun(myCam, &overrunFlag);
		if (error) {
			return error;
		}
		if (overrunFlag == 1) {
			overrunTotal++;
		}

		//At the end of each filename a number will be appended; the number recycles every ten numbers
		sprintf(imageName, "%s%d", imageNameRoot, abs(i % 10));

		//Saves the image acquired
		error = ncCamSaveImage(myCam, myNcImg, imageName, saveFormat, "This is one of the last 10 images grabbed", 1);
		if (error) {
			return error;
		}

		printf("%3d\t%s", i, status); //Print index for number so far
		printf("\r"); //Re-use this line next time
		if (i % 10 == 0) {
			fflush(stdout); //Keep relatively up-to-date  
		}
	}

	printf("\n");	//Preserve the number of images
	printf("%.3g ms to acquire these images.\n", stop_msec - start_msec);

	*acquisitionTime_msec = (stop_msec - start_msec) / numberImages;

	*overrun = overrunTotal;

	//Close the shutter, now that the acquistion is complete
	error = ncCamSetShutterMode(myCam, CLOSE);
	if (error) {
		return error;
	}

	//We have acquired enough images so we tell the frame grabber to stop acquiring frames
	//and the camera to stop sending images to the frame grabber
	error = ncCamAbort(myCam);
	if (error) {
		return error;
	}

	return error;
}


//Converts tm to seconds in day.  We assume the example won't run for more than a day. ;)
int mk_secsInDay(struct tm * time) {
	return time->tm_sec + 60 * (time->tm_min + 60 * time->tm_hour);
}


//Provides a print out of the best crop-mode solutions
int listBestCropModeSolutions(NcCropModeSolutions solutions, int maxNumberDisplayed) {

	int error = NC_SUCCESS;		//We initialize an error flag variable
	int k; //Loop index for solutions to be displayed
	int numberSolutions = 0; //To recover the number of solutions

	//Recover the total number of crop-mode solutions available
	error = ncCropModeSolutionsGetTotal(solutions, &numberSolutions);
	if (error == NC_SUCCESS && numberSolutions > 0) {
		printf("-------------------------------------------------------\n");
		printf("%d crop-mode solutions available.\n", numberSolutions);
		numberSolutions = (numberSolutions > maxNumberDisplayed) ? maxNumberDisplayed : numberSolutions; //How many solutions do we want to display? 
		printf("The best %d are as follows :\n", numberSolutions);
		printf("Figure of Merit\t\tX-offset (MIN, MAX)\n");
		printf("-------------------------------------------------------\n");
		for (k = 0; k < numberSolutions; ++k) {
			float fom; //Figure of Merit for this solution
			int offX_min, offX_max; //X-offset range for this solution
			//Recover properties of this solution; they are ordered by decreasing Figure of Merit
			error = ncCropModeSolutionsGetResult(solutions, k, &fom, &offX_min, &offX_max, 0, 0);
			if (error) {
				break;
			}
			printf("%7.4f\t\t\t(%d, %d)\n", fom, offX_min, offX_max);
		}
		printf("-------------------------------------------------------\n");
	}
	else if (numberSolutions == 0) {
		printf("No crop-mode solutions available!  Will use standard ROI.\n");
	}

	return error;
}
