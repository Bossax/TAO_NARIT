#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"
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

int main(int argc, char** argv)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

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

      // Reset deviceSerialNumber
      spinNodeHandle hDeviceReset = NULL;
      err = spinNodeMapGetNode(hNodeMap, "DeviceReset", &hDeviceReset);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to get Device reset node error %d...\n\n", err);

      }
      err = spinCommandExecute(hDeviceReset);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
          printf("Unable to reset the device. error %d...\n\n", err);

      }
      else {
        printf("Device is reset successfully .. .\n" );
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

    return err;
}
