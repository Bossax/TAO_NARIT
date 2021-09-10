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
   size_t num_feature = 9;

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

 // save image
void SaveImage(spinImage *hGrabbedImage, char *fname){
    spinError err = spinImageSave(*hGrabbedImage, fname, JPEG);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
       printf("Unable to save image. Non-fatal error %d...\n\n", err);
    }
    else
    {
       printf("Image saved at %s\n\n", fname);
    }

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

spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap)
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


    const unsigned int k_numImages = 2;
    unsigned int imageCnt = 0;
    spinImage hResultImage = NULL;
    bool8_t failDetect = False;
    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        // Retrieve next received image

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
            sprintf(filename, "ROI_Image-%d.jpg", imageCnt);

            // convert and save image
            SaveImage(&hResultImage, filename);
        }


        // release the image memory and ready for the next image
        ReleaseImage(&hResultImage);
        printf("Image buffer released . .\n");
    }


    // End Acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }
    return err;

}

spinError set_format(spinNodeMapHandle *hhNodeMap, char *PixelFormat){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hPixelFormat;
    spinNodeHandle hPixelFormatValue;
    int64_t PixelFormatValue = 0;
    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "PixelFormat", &hPixelFormat );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node PixelFormat ..\n");
         return err;
     }

     // Get the enumeration number of the format from the node
    if (IsAvailableAndReadable(hPixelFormat, "PixelFormat")){

        err = spinEnumerationGetEntryByName(hPixelFormat, PixelFormat,&hPixelFormatValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set pixel format. Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }

    // Retrieve node value
   if (IsAvailableAndReadable(hPixelFormat, "PixelFormat")){

       err = spinEnumerationEntryGetIntValue(hPixelFormatValue,  &PixelFormatValue);
       if (err != SPINNAKER_ERR_SUCCESS)
       {
           printf(
               "Unable to set pixel format node value. Aborting with error "
               "%d...\n\n",
               err);
           return err;
       }
   }
    // set the node value
    if (IsAvailableAndWritable(hPixelFormat, "PixelFormat"))
    {
        err = spinEnumerationSetIntValue(hPixelFormat, PixelFormatValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set pixel format. Aborting with error %d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "PixelFormat");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Pixel format is set to %s ..\n", PixelFormat);

    return err;
}

spinError set_coding(spinNodeMapHandle *hhNodeMap, char *PixelCoding){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hPixelCoding;
    spinNodeHandle hPixelCodingValue;
    int64_t PixelCodingValue = 0;

    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "PixelCoding", &hPixelCoding );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node PixelCoding ..\n");
         return err;
     }

     // Get the enumeration number of the format from the node
    if (IsAvailableAndReadable(hPixelCoding, "PixelCoding")){

        err = spinEnumerationGetEntryByName(hPixelCoding, PixelCoding, &hPixelCodingValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set pixel coding. Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }

    // Retrieve node value
   if (IsAvailableAndReadable(hPixelCoding, "PixelCoding")){

       err = spinEnumerationEntryGetIntValue(hPixelCodingValue,  &PixelCodingValue);
       if (err != SPINNAKER_ERR_SUCCESS)
       {
           printf(
               "Unable to set pixel format node value. Aborting with error "
               "%d...\n\n",
               err);
           return err;
       }
   }

    // set the node value
    if (IsAvailableAndWritable(hPixelCoding, "PixelCoding"))
    {
        err = spinEnumerationSetIntValue(hPixelCoding, PixelCodingValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set pixel format. Aborting with error %d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "PixelCoding");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    printf("Pixel coding is set to %s ..\n", PixelCoding);
    return err;
}

spinError set_width(spinNodeMapHandle *hhNodeMap, int Width){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hWidth;
    int64_t MaxWidth = 2048;
    int64_t WidthValue = (int64_t)Width;
    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "Width", &hWidth );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node Width ..\n");
         return err;
     }

     // Read the maximum value of the node
    if (IsAvailableAndReadable(hWidth, "Width")){

        err = spinIntegerGetMax(hWidth, &MaxWidth);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set width. Aborting with error %d...\n\n",err);
            return err;
        }
    }
    // cap the value
    if (WidthValue > MaxWidth){
        WidthValue = MaxWidth;
    }

    // set the node value
    if (IsAvailableAndWritable(hWidth, "Width"))
    {
        err = spinIntegerSetValue(hWidth, WidthValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set pixel format. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "Width");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Image width is set to %ld ..\n", WidthValue);
    return err;
}

spinError set_height(spinNodeMapHandle *hhNodeMap, int Height){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hHeight;
    int64_t MaxHeight = 2048;
    int64_t HeightValue = (int64_t)Height;
    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "Height", &hHeight );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node Height ..\n");
         return err;
     }

     // Read the maximum value of the node
    if (IsAvailableAndReadable(hHeight, "Height")){

        err = spinIntegerGetMax(hHeight, &MaxHeight);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set Height. Aborting with error %d...\n\n",err);
            return err;
        }
    }
    // cap the value
    if (HeightValue > MaxHeight){
        HeightValue = MaxHeight;
    }

    // set the node value
    if (IsAvailableAndWritable(hHeight, "Height"))
    {
        err = spinIntegerSetValue(hHeight, HeightValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set pixel format. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "Height");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Image height is set to %ld ..\n", HeightValue);
    return err;
}

spinError set_offsetx(spinNodeMapHandle *hhNodeMap, int OffsetX){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hOffsetX;
    int64_t MaxOffsetX = 2048;
    int64_t OffsetXValue = (int64_t)OffsetX;
    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "OffsetX", &hOffsetX );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node OffsetX ..\n");
         return err;
     }

     // Read the maximum value of the node
    if (IsAvailableAndReadable(hOffsetX, "OffsetX")){

        err = spinIntegerGetMax(hOffsetX, &MaxOffsetX);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set OffsetX. Aborting with error %d...\n\n",err);
            return err;
        }
    }
    // cap the value
    if (OffsetXValue > MaxOffsetX){
        OffsetXValue = MaxOffsetX;
    }

    // set the node value
    if (IsAvailableAndWritable(hOffsetX, "OffsetX"))
    {
        err = spinIntegerSetValue(hOffsetX, OffsetXValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set x offset. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "OffsetX");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Offset X is set to %ld ..\n", OffsetXValue);
    return err;
}

spinError set_offsety(spinNodeMapHandle *hhNodeMap, int OffsetY){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hOffsetY;
    int64_t MaxOffsetY = 2048;
    int64_t OffsetYValue = (int64_t)OffsetY;
    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "OffsetY", &hOffsetY );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node OffsetY ..\n");
         return err;
     }

     // Read the maximum value of the node
    if (IsAvailableAndReadable(hOffsetY, "OffsetY")){

        err = spinIntegerGetMax(hOffsetY, &MaxOffsetY);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set OffsetY. Aborting with error %d...\n\n",err);
            return err;
        }
    }
    // cap the value
    if (OffsetYValue > MaxOffsetY){
        OffsetYValue = MaxOffsetY;
    }

    // set the node value
    if (IsAvailableAndWritable(hOffsetY, "OffsetY"))
    {
        err = spinIntegerSetValue(hOffsetY, OffsetYValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set Y offset. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "OffsetY");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Offset Y is set to %ld ..\n", OffsetYValue);
    return err;
}

spinError set_reverse(spinNodeMapHandle *hhNodeMap, bool8_t reverse){

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hReverse;

    bool8_t reverseX = reverse;

    // Get the node from nodemap
    err = spinNodeMapGetNode(*hhNodeMap, "ReverseX", &hReverse );
    if (err != SPINNAKER_ERR_SUCCESS){
         printf("Unable to get node Reverse ..\n");
         return err;
     }

    // set the node value
    if (IsAvailableAndWritable(hReverse, "ReverseX"))
    {
        err = spinBooleanSetValue(hReverse, reverseX);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to reverse image. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "ReverseX");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    if(reverseX) printf("Image is reversed!..\n");
    else printf("Image is NOT reversed!..\n");
    return err;
}

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
    while((ch = getopt(argc,argv,"f:c:w:h:x:y:r::")) != EOF){
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
              printf("%s \n",optarg);
              if(optarg != NULL){
                if(strcmp(optarg,"False") == 0){
                  continue;

                }
                else if(strcmp(optarg, "True") == 0){
                  reverseX = True;
                }
                else{
                  printf("Reverse X: True or False ?");
                  exit(2);
                }

              }
              else if(optarg == NULL){
                printf("optarg is NULL \n" );
                reverseX = True;
              }

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
      err = spinCameraInit(hCamera);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
          return err;
      }

      // Retrieve GenICam nodemap
      spinNodeMapHandle hNodeMap = NULL;

      err = spinCameraGetNodeMap(hCamera, &hNodeMap);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
          return err;
      }

      // Configure Image
      if(format != NULL) set_format(&hNodeMap, format);
      if(coding != NULL) set_coding(&hNodeMap, coding);
      if(width != 0) set_width(&hNodeMap, width);
      if(height != 0) set_height(&hNodeMap, height);
      if(offsetx != 0) set_offsetx(&hNodeMap, offsetx);
      if(offsety != 0) set_offsety(&hNodeMap, offsety);
      if(reverseX) set_reverse(&hNodeMap, reverseX);

      // Sensor INFORMATION
      PrintSensorInfo(&hNodeMap);

      // Acquire images
      err = AcquireImages(hCamera, hNodeMap);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          return err;
      }

      // Deinitialize camera
      err = spinCameraDeInit(hCamera);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to deinitialize camera. Non-fatal error %d...\n\n", err);

      }

      // Release camera
      err = spinCameraRelease(hCamera);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          return err;
      }
      printf("Complete... Cleaning up stuff \n\n");


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
