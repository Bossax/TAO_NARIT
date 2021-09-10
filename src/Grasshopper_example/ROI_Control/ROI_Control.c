#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"
#include <unistd.h>
#include <stdlib.h>

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256


bool8_t IsAvailableAndReadable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hNode, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbReadable && pbAvailable;
}


bool8_t IsAvailableAndWritable(spinNodeHandle hNode, char nodeName[])
{
    bool8_t pbAvailable = False;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinNodeIsAvailable(hNode, &pbAvailable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, err);
    }

    bool8_t pbWritable = False;
    err = spinNodeIsWritable(hNode, &pbWritable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node writability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbWritable && pbAvailable;
}

void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}

spinError PrintSensorInfo(spinNodeMapHandle *hhNodeMap)
 {
    spinError err = SPINNAKER_ERR_SUCCESS;
   // takes hcam pointer and  print out sensor info by enumeartion
   printf("\n\n=== Sensor Information ===\n");
   spinNodeHandle hImageFormatControl = NULL;
   size_t num_feature = 30;

   err = spinNodeMapGetNode(*hhNodeMap,"ImageFormatControl", &hImageFormatControl);
   if (err != SPINNAKER_ERR_SUCCESS)
   {
       printf("Unable to set ImageFormatControl (node retrieval). Aborting with error %d...\n\n", err);
       return err;
   }

   // enumerate to print sensor information
   for(int i = 0 ; i <= num_feature; i++ ){
     spinNodeHandle hFeatureNode = NULL;

     err = spinCategoryGetFeatureByIndex(hImageFormatControl, i, &hFeatureNode);
     if (err != SPINNAKER_ERR_SUCCESS)
     {
         printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
         continue;
     }

     char featureName[MAX_BUFF_LEN];
     size_t lenFeatureName = MAX_BUFF_LEN;
     err = spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
     if (err != SPINNAKER_ERR_SUCCESS)
     {
         strcpy(featureName, "Unknown name");
     }

     char featureValue[MAX_BUFF_LEN];
     size_t lenFeatureValue = MAX_BUFF_LEN;
     err = spinNodeToString(hFeatureNode, featureValue, &lenFeatureValue);
     if (err != SPINNAKER_ERR_SUCCESS)
     {
         strcpy(featureValue, "Unknown value");
     }

     printf("%s: %s\n", featureName, featureValue);

   }
   printf("\n");
   return err;

 }

spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    // Set acquisition mode to continuous
    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeContinuous = NULL;
    int64_t acquisitionModeContinuous = 0;

    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
        return err;
    }

    if (IsAvailableAndReadable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquistionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    if (IsAvailableAndReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
    {
        err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
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
        PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // set acquisition mode to continuous
    if (IsAvailableAndWritable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
                err);
            return err;
        }
        printf("Acquisition mode set to continuous...\n");
    }
    else
    {
        PrintRetrieveNodeFailure("node", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // Retrieve device serial number for filename
    spinNodeHandle hDeviceSerialNumber = NULL;
    char deviceSerialNumber[MAX_BUFF_LEN];
    size_t lenDeviceSerialNumber = MAX_BUFF_LEN;

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceSerialNumber", &hDeviceSerialNumber);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        strcpy(deviceSerialNumber, "");
        lenDeviceSerialNumber = 0;
    }
    else
    {
        if (IsAvailableAndReadable(hDeviceSerialNumber, "DeviceSerialNumber"))
        {
            err = spinStringGetValue(hDeviceSerialNumber, deviceSerialNumber, &lenDeviceSerialNumber);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }

            printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
        }
        else
        {
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
        }
    }
    printf("\n");

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        // Retrieve next received image
        spinImage hResultImage = NULL;
        bool8_t isIncomplete = False;
        bool8_t hasFailed = False;

        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to get next image. Non-fatal error %d...\n\n", err);
            continue;
        }

        // Ensure image completion
        err = spinImageIsIncomplete(hResultImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        if (isIncomplete)
        {
            spinImageStatus imageStatus = IMAGE_NO_ERROR;

            err = spinImageGetStatus(hResultImage, &imageStatus);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve image status. Non-fatal error %d...\n\n", err);
            }
            else
            {
                printf("Image incomplete with image status %d...\n", imageStatus);
            }

            hasFailed = True;
        }

        // Release incomplete or failed image
        if (hasFailed)
        {
            err = spinImageRelease(hResultImage);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to release image. Non-fatal error %d...\n\n", err);
            }

            continue;
        }

        // Print image information
        size_t width = 0;
        size_t height = 0;

        err = spinImageGetWidth(hResultImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image width. Non-fatal error %d...\n", err);
        }

        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image height. Non-fatal error %d...\n", err);
        }

        printf("Grabbed image %u, width = %u, height = %u\n", imageCnt, (unsigned int)width, (unsigned int)height);

        // Convert image to mono 8
        spinImage hConvertedImage = NULL;

        err = spinImageCreateEmpty(&hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to create image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        err = spinImageConvert(hResultImage, PixelFormat_Mono8, hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to convert image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        // Create unique file name
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "ImageFormatControl-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "ImageFormatControl-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        // Save image
        err = spinImageSave(hConvertedImage, filename, JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n", err);
        }
        else
        {
            printf("Image saved at %s\n\n", filename);
        }

        // Destroy converted image
        err = spinImageDestroy(hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy image. Non-fatal error %d...\n\n", err);
        }

        // Release image
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release image. Non-fatal error %d...\n\n", err);
        }
    }

    // End Acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }

    return err;
}

spin


int main(int argc, char** argv)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;
    char *format = NULL;
    char *coding = NULL;
    int64_t width = 0;
    int64_t height = 0;
    int64_t offsetx = 0;
    int64_t offsety= 0;
    bool8_t reverseX = False;

    // ********* Parsing CML argument ************* //
    if(argc <2){
      printf("Invalid option: Please specify option in the format $[option] [numeric value] \n");
      return EXIT_FAILURE;
    }

    char ch;
    while((ch = getopt(argc,argv,"f:c:w:h:x:y:r")) != EOF){
        switch(ch){
          case 'f':
              format = optarg;
              printf("Pixel Format = %s\n", optarg);
              break;
          case 'c':
              coding = optarg;
              printf("Pixel Coding = %s\n", optarg );
              break;
          case 'w':
            width = (int64_t)atoi(optarg);
            printf("Width = %s\n", optarg);
            break;
          case 'h':
            height = (int64_t)atoi(optarg);
            printf("Height = %s\n", optarg);
            break;

          case 'x':
            offsetx = (int64_t)atoi(optarg);
            printf("Offset X = %s\n", optarg );
            break;

          case 'y':
            offsety = (int64_t)atoi(optarg);
            printf("Offset Y = %s\n", optarg );
            break;

          case 'r':
            reverseX = True;
            printf("Image is reversed\n");
            break;
          default:
            printf("Invalid input..\n" );
            exit(2);
        }
    }

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

    printf("Number of cameras detected: %u\n\n", (unsigned int)numCameras);

    // Finish if there are no cameras
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

        printf("Not enough cameras!\n");
        printf("Done! Press Enter to exit...\n");
        getchar();

        return -1;
    }

      printf("\nRunning example for camera %d...\n", i);

      // Run camera
      spinCamera hCamera = NULL;

      err = spinCameraListGet(hCameraList, 0, &hCamera);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", err);
          return err;
      }

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

      // Configure Image
      if(format != NULL) set_format(format);
      if(coding != NULL) set_coding(coding);
      if(width != 0) set_width(width);
      if(height != 0) set_height(height);
      if(offsetx != 0) set_offsetx(offsetx);
      if(offsety != 0) set_offsety(offsety);
      if(reverseX) set_reverse();

      // Sensor INFORMATION
      PrintSensorInfo(&hNodeMap);

      // Acquire images
      err = AcquireImages(hCam, hNodeMap, hNodeMapTLDevice);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          return err;
      }

      // Deinitialize camera
      err = spinCameraDeInit(hCam);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to deinitialize camera. Non-fatal error %d...\n\n", err);
      }

      return err;

      // Release camera
      err = spinCameraRelease(hCamera);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          return err;
      }
      printf("Complete... Cleaning up stuff \n\n");

    //
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

    printf("\nDone! Press Enter to exit...\n");
    getchar();

    return err;
}
