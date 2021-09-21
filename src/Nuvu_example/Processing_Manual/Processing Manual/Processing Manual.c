//This is a header file that needs to be added to all your projects as all the nc functions are defined here
#include "nc_driver.h"

//This header defines the initialize and cleanUp functions
#include "../Utility/utility.h"

#ifdef WINDOWS
//Include windows header for the Sleep function
#include <Windows.h>
#endif

#include <stdio.h>

int processingManual(NcCam camera, NcProcCtx* ctx);
int acquireBias(NcCam camera, NcProcCtx* ctx);
int produceLmProcessedImage(NcCam camera, NcProcCtx* ctx);
int producePcProcessedImage(NcCam camera, NcProcCtx* ctx);

int main() {

	NcCam		myCam = NULL;
	NcProcCtx	*ctx = NULL;

	int error =  NC_SUCCESS;	//We initialize an error flag variable
	int width, height, overscanLines;

	error = initialize(&myCam);

	if (error == NC_SUCCESS) {
		//Searches for the size of the processing context wanted
		error = ncCamGetSize(myCam, &width, &height);
	}

	if (error == NC_SUCCESS) {
		//Searches for overscan lines of the processing context wanted
		error = ncCamGetOverscanLines(myCam, &overscanLines);
	}

	if (error == NC_SUCCESS) {
		//Opens the context that will be used
		error = ncProcOpen(width, height + overscanLines, &ctx);
	}

	if (error == NC_SUCCESS) {
		// Specify the number of overscan lines
		error = ncProcSetOverscanLines(ctx, overscanLines);
	}

	if (error == NC_SUCCESS) {
		error = processingManual(myCam, ctx);
	}
	
	if (error) {
		printf("The error %d happened during the example.  For more information about this error, the file nc_error.h can be used\n", error);
	}

	if (ctx != NULL) {
		ncProcClose(ctx);
	}

	cleanUp(myCam);

	return error;
}


int processingManual(NcCam camera, NcProcCtx* ctx) {

	NcImage *image;

	int error = NC_SUCCESS;		//We initialize an error flag variable

	//We capture an image without processing for reference
	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &image);
	if (error) {
		return error;
	}

	//Saves the corrected image in TIF format
	//There is no need to specify a width and height as the function will take the ones defined in the ncCam structure
	//"Not processed reference image" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, image, "ncImage", FITS, "Not processed reference image", 1);
	if (error) {
		return error;
	}
	printf("Reference image acquired\n");

	//We take the bias
	error = acquireBias(camera, ctx);
	if (error) {
		return error;
	}
	printf("Bias calculated\n");

	//We take a LM-processed image
	printf("Calculating LM-processed image\n");
	error = produceLmProcessedImage(camera, ctx);
	if (error) {
		return error;
	}
	printf("LM-processed image produced\n");

	//We take a PC-processed image
	printf("Calculating PC-processed image\n");
	error = producePcProcessedImage(camera, ctx);
	if (error) {
		return error;
	}
	printf("PC-processed image produced\n");

	return error;
}


int acquireBias(NcCam camera, NcProcCtx* ctx) {

	/***Procedure to take the Bias****/

	NcImage *image;

	int			i, error = NC_SUCCESS;		//We initialize an error flag variable
	const int	biasStackSize = 50;			//Number of images to acquire the bias

	//Closes the shutter as this is to acquire the bias/black images
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	//Launches a continuous acquisitions by the framegrabber and the camera (this function does not wait
	//for the acquisition to be complete before returning)
	error = ncCamStart(camera, 0);
	if (error) {
		return error;
	}

	//Accumulates Bias images
	printf("Capturing bias image :\n");
	for (i = 0; i<biasStackSize; ++i)
	{
		printf("%d\r", i + 1);
		//printf("Capturing bias image %d\n", i + 1);
		error = ncCamRead(camera, &image);
		if (error) {
			return error;
		}

		error = ncProcAddBiasImage(ctx, image);
		if (error) {
			return error;
		}
	}
	printf("\n");	//Preserve the number of images

	//Uses the "abort" function to tell the frame grabber to stop acquiring frames from the
	//camera, also tells the camera to stop sending images to the frame grabber as we already have all the
	//required images in order to calculate the bias
	error = ncCamAbort(camera);
	if (error) {
		return error;
	}

	//Since extra images could have arrived prior to the ncCamAbort to be effective, we check for dummy images since we don't want to read them afterward
	while (!ncCamReadChronologicalNonBlocking(camera, &image, NULL));

	printf("Calculating bias\n");

	error = ncProcComputeBias(ctx);
	if (error) {
		return error;
	}

	return error;
}


int produceLmProcessedImage(NcCam camera, NcProcCtx* ctx) {

	/***Procedure to apply image correction****/

	NcImage *image = NULL;
	int error = NC_SUCCESS;		//We initialize an error flag variable

	//Sets the clamp level to 300 (standard value)
	error = ncProcSetBiasClampLevel(ctx, 300);
	if (error) {
		return error;
	}

	//Sets the processing type to LM (this mean the bias image will be subtracted from the image capture)
	error = ncProcSetProcType(ctx, LM);
	if (error) {
		return error;
	}

	//To see how to capture one single image, please refer to "utility.c" or the "simpleAcquisition.c" example
	error = acquireOneImage(&camera, &image);
	if (error) {
		return error;
	}

	//Corrects the image captured at the same address and has an offset of the clamp level (300)
	error = ncProcProcessDataImageInPlace(ctx, image);
	if (error) {
		return error;
	}

	//Saves the corrected image in TIF format
	//There is no need to specify a width and height as the function will take the ones defined in the ncCam structure
	//"LM-Processed image" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncCamSaveImage(camera, image, "ncLmImage", FITS, "LM-Processed image", 1);
	if (error) {
		return error;
	}

	return error;
}


int producePcProcessedImage(NcCam camera, NcProcCtx* ctx) {

	/***Procedure to apply photon counting****/

	NcImage		*image, *PcImage;
	ImageParams	imageParams;
	
	int			error = NC_SUCCESS;		//We initialize an error flag variable
	int			i, timeout;
	const int	nbrFrames = 100;	//The number of frames we want to integrate in PC
	

	//Get the actual timeout value
	error = ncCamGetTimeout(camera, &timeout);
	if (error) {
		return error;
	}

	//Sets the processing type to PC (Photon Counting)
	error = ncProcSetProcType(ctx, PC);
	if (error) {
		return error;
	}

	//If there was enough illumination for the LM image acquired above,  
	//even nominally zero exposure will likely give too much light for photon-counting
	error = ncCamSetExposureTime(camera, 0.0);
	if (error) {
		return error;
	}

	//We set a long enough timeout
	error = ncCamSetTimeout(camera,  timeout * nbrFrames);
	if (error) {
		return error;
	}

	//Open the shutter for the acquisition 
	error = ncCamSetShutterMode(camera, OPEN);
	if (error) {
		return error;
	}

	//Launches an acquisition by the framegrabber and requests an image from the camera (this function
	//does not wait for the acquisition to be complete before returning)
	error = ncCamStart(camera, nbrFrames);
	if (error) {
		return error;
	}

	//Read the first frame
	//This image is read outside the loop because it's image parameters
	//will be used as the image parameters for the output image
	error = ncCamRead(camera, &image);
	if (error) {
		return error;
	}

	//Get the image parameters for this acquisition
	error = ncCamOpenImageParams(&imageParams);
	if (error) {
		return error;
	}

	error = ncCamGetImageParams(camera, image, imageParams);
	if (error) {
		return error;
	}

	//Add the first frame to the PC image
	printf("Capturing images :\n1\r");
	error = ncProcAddDataImage(ctx, image);
	if (error) {
		return error;
	}

	//i starts at 1 because the first frame has already been read (frame 0)
	for (i = 1; i<nbrFrames; ++i)
	{
		printf("%d\r", i + 1);

		error = ncCamRead(camera, &image);
		if (error) {
			return error;
		}

		error = ncProcAddDataImage(ctx, image);
		if (error) {
			return error;
		}
	}
	printf("\n");	//Preserve the number of images

	//Close the shutter, now that the acquisition is complete
	error = ncCamSetShutterMode(camera, CLOSE);
	if (error) {
		return error;
	}

	//The PC image is being retrieved
	error = ncProcGetImage(ctx, &PcImage);
	if (error) {
		return error;
	}

	//Saves the PC image in FITS format
	//There is no need to specify a width and height as the function will take the ones defined in the ncCam structure
	//"PC-Processed image" parameter is used to add a header to our images
	//Overwrite flag set to '1' to overwrite an existing file if it has the same name
	error = ncSaveImage(0, 0, imageParams, PcImage, NC_IMG, "ncPcImage", FITS, NO_COMPRESSION, "PC-Processed image", 1);
	if (error) {
		return error;
	}

	//Frees this image as it will no longer be used
	error = ncProcReleaseImage(ctx, PcImage);
	if (error) {
		return error;
	}

	//Free the image parameters object
	error = ncCamCloseImageParams(imageParams);
	if (error) {
		return error;
	}

	return error;
}
