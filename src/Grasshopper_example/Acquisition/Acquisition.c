/*
Grasshopper Spinnaker C API
Image acquisition
- SingleFrame
- Continuous
**/
#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

// Acquisition mode
typedef enum _acquisition_mode{
    SINGLE_FRAME,
    CONTINUOUS
}acquisition_mode;


// This function helps to check if a node is available and readable
bool8_t IsAvailableAndReadable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable); //GenAPI
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node  availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hNode, &pbReadable); //GenAPI
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbReadable && pbAvailable;
}

// This function helps to check if a node is available and writable
bool8_t IsAvailableAndWritable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable);  //GenAPI
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbWritable = False;
    err = spinNodeIsWritable(hNode, &pbWritable); //GenAPI
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node writability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbWritable && pbAvailable;
}

// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}

// convert and save image
void SaveImage(spinImage *hGrabbedImage, char *fname)
{
    // Convert image to mono 8
    // create a holder and destroy when saved
    spinError err;
    spinImage hConvertedImage = NULL;
    bool8_t hasFailed = False;                  // if failed, clear stuff
    err = spinImageCreateEmpty(&hConvertedImage);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create image. Non-fatal error %d...\n\n", err);
        return;
    }

    err = spinImageConvert(*hGrabbedImage, PixelFormat_Mono8, hConvertedImage);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to convert image. Non-fatal error %d...\n\n", err);
        hasFailed = True;
    }

    // Save image
    if (!hasFailed) // skip if has failed
    {
        err = spinImageSave(hConvertedImage, fname, JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n\n", err);
        }
        else
        {
            printf("Image saved at %s\n\n", fname);
        }
    }

    // Destroy converted image
    err = spinImageDestroy(hConvertedImage);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy image. Non-fatal error %d...\n\n", err);
    }
    return;
}

//Release grabbed image
void ReleaseImage(spinImage * hGrabbedImage)
{
    spinError err;
    err = spinImageRelease(*hGrabbedImage);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to release image. Non-fatal error %d...\n\n", err);

    }
    return;
}

bool8_t IsImageIncomplete(spinImage *hGrabbedImage)
{
    bool8_t isIncomplete = False;

    spinError err = spinImageIsIncomplete(*hGrabbedImage, &isIncomplete);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
        isIncomplete = True;
    }

    // Check image status if incomplete
    if (isIncomplete)
    {
        spinImageStatus imageStatus = IMAGE_NO_ERROR;

        err = spinImageGetStatus(*hGrabbedImage, &imageStatus);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image status. Non-fatal error %d...\n\n", imageStatus);
        }
        else
        {
            printf("Image incomplete with image status %d...\n", imageStatus);
        }

    }

    return isIncomplete;

}

// This function acts as the body of the example; please see NodeMapInfo_C
// example for more in-depth comments on setting up cameras.
spinError RunSingleCamera(spinCamera hCam, acquisition_mode mode, int arg_num, ...)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Initialize camera
    
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve GenICam nodemap
    spinNodeMapHandle hNodeMap = NULL;
    err = spinCameraGetNodeMap(hCam, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
        return err;
    }

    // check mode
    unsigned int k_numImages = 0;
    unsigned int frame_rate = 0;

    char * mode_name;
    if(mode == SINGLE_FRAME) {
        mode_name = "SingleFrame";
        printf("Single Frame acquisition ... \n");
    }
    else if (mode == CONTINUOUS){
        mode_name = "Continuous";
        printf(
            "Continuous acquisition ... \n"
        );

        va_list valist;
        if(arg_num == 2){
            va_start(valist, 2);
            k_numImages = va_arg(valist, int);
            frame_rate = va_arg(valist, int);
        }
        else if(arg_num == 1){
            va_start(valist, 1);
            k_numImages = va_arg(valist, int);
        }
        va_end(valist);
    }

    // ================ Set acquisition mode ======================= //
    printf("\n*** IMAGE ACQUISITION ***\n\n");
    /* ==================================================*/

    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeValue= NULL;
    int64_t acquisitionModeValue = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve entry node from enumeration node
    if (IsAvailableAndReadable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationGetEntryByName(hAcquisitionMode, mode_name, &hAcquisitionModeValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to %s. Aborting with error "
                "%d...\n\n",
                mode_name, err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer from entry node
    if (IsAvailableAndReadable(hAcquisitionModeValue, "AcquisitionModeValue"))
    {
        err = spinEnumerationEntryGetIntValue(hAcquisitionModeValue, &acquisitionModeValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        char str_handle[MAX_BUFF_LEN];
        sprintf(str_handle, "AcquisitionMode '%s'", mode_name);
        PrintRetrieveNodeFailure("entry", str_handle);
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    // Set integer as new value of enumeration node
    if (IsAvailableAndWritable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }


    printf("Acquisition mode set to %s...\n", mode_name);

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // image buffer handler used for the entire image acquisition process
    spinImage hResultImage = NULL;
    unsigned int imageCnt = 0;
    // ==================== Acquire images ============================ //
    if (mode == SINGLE_FRAME)
    {
        //Get image
        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to get next image. Non-fatal error %d...\n\n", err);
        }
        // Ensure image completion
        // inquire info from imageControl node
        bool8_t failDetect = IsImageIncomplete(&hResultImage);

        // If the image is complete
        // proceed to convert and save
        if(!failDetect){
            char *filename;
            filename = "Acquisition_SingleFrame.jpg";

            // convert and save image
            SaveImage(&hResultImage, filename);

        }

        // Release the image handler
        ReleaseImage(&hResultImage);
    }
    else if(mode == CONTINUOUS)
    {
        bool8_t failDetect = False;  // flag if image has problems
        for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            // Get image
            err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to get next image. Non-fatal error %d...\n\n", err);
                continue;
            }

            // Ensure image completion
            // inquire info from imageControl node
             failDetect = IsImageIncomplete(&hResultImage);

            // if the image is complete
            // proceed to convert and save image
            if(!failDetect){
                char filename[MAX_BUFF_LEN];
                sprintf(filename, "Acquisition-Continuous-%d.jpg", imageCnt);

                // convert and save image
                SaveImage(&hResultImage, filename);
            }


            // release the image memory and ready for the next image
            ReleaseImage(&hResultImage);


        }   // for loop
    }       // end continuous acquisition

    // End acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    return err;
}

// ========= Main =============== //
// TO Do: Set frame rate enabling?
int main(int argc, char* argv[])
{
    // Flags
    // i) -s singleframe
    // ii) -c [n] [fr] continuous with number of images and frame rate

    acquisition_mode mode;
    unsigned int k_numImages = 1;
    unsigned int frame_rate = 0;
    int nargs = 0;

    if(argc > 5){
        printf("Wrong positional arguments for Continuous mode...\n");
        return EXIT_FAILURE;
    }
    // Single or continuous?
    if (nargs == 0) {
        if (strcmp(argv[1], "-s") == 0) {
            mode = SINGLE_FRAME;
        }
        else if(strcmp(argv[1], "-c") == 0) {
            mode = CONTINUOUS;
            nargs++;
        }
        else{
            printf("Wrong flag inputs, Single Frame -s or Continuous -c \n");
            return EXIT_FAILURE;
        }

    }
    // extract continuous mode parameters
    if(nargs == 1){
        assert(mode == CONTINUOUS);
        k_numImages = atoi(argv[2]);
        if (argc == 3) nargs++;
    }
    if(nargs == 2){
        frame_rate = atoi(argv[3]);
    }

    spinError errReturn = SPINNAKER_ERR_SUCCESS; // Non-fatal error
    spinError err = SPINNAKER_ERR_SUCCESS;


    // Retrieve singleton reference to system object
    spinSystem hSystem = NULL;

    err = spinSystemGetInstance(&hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve system instance. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve list of cameras from the system
    spinCameraList hCameraList = NULL;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve number of cameras
    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with error %d...\n\n", err);
        return err;
    }

    // =========== Finish and exit if there are no cameras ====== //
    if (numCameras == 0)
    {
        // Clear and destroy camera list before releasing system
        err = spinCameraListClear(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
            return err;
        }

        err = spinCameraListDestroy(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release system
        err = spinSystemReleaseInstance(hSystem);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release system instance. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("Not enough cameras! Exit\n");
        return -1;
    }

    // ============  Run camera  ===============//

    spinCamera hCamera = NULL;
    // Get camera from the list
    err = spinCameraListGet(hCameraList, 0, &hCamera);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", err);
        errReturn = err;
    }
    else // Actual execution of image acquisition
    {
        // Run
        // single frame
        if(mode == SINGLE_FRAME){
            err = RunSingleCamera(hCamera, mode, 0);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                errReturn = err;
            }
        }
        // continuous
        else if(mode == CONTINUOUS){
            // no fame rate input
            if(frame_rate == 0){
                err = RunSingleCamera(hCamera, mode, 1, k_numImages);
                if (err != SPINNAKER_ERR_SUCCESS)
                {
                    errReturn = err;
                }
            }
            // with frame rate input
            else{
                err = RunSingleCamera(hCamera, mode, 2, k_numImages, frame_rate);
                if (err != SPINNAKER_ERR_SUCCESS)
                {
                    errReturn = err;
                }
            }
        }

    }

    // Release camera
    err = spinCameraRelease(hCamera);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        errReturn = err;
    }

    printf("Complete...\n\n");

    // =============== Clean  up ==================== //
    // Clear and destroy camera list before releasing system
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Release system
    err = spinSystemReleaseInstance(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to release system instance. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("\nDone!...\n");
    return errReturn;
}
