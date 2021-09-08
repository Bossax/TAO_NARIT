#include "SpinnakerC.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

#define CONFIGURE 1
#define RESET 0

// This function helps to check if a node is available and readable
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

// This function helps to check if a node is available and writable
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

// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}

spinError ConfigureExposure(spinNodeMapHandle hNodeMap, double exposureTimeToSet)
{
    // return
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Turn off automatic exposure mode
    spinNodeHandle hExposureAuto = NULL;
    spinNodeHandle hExposureAutoOff = NULL;
    int64_t exposureAutoOff = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "ExposureAuto", &hExposureAuto);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable automatic exposure (node retrieval). Aborting with error %d...\n", err);
        return err;
    }

    // Retrieve entry node from enumeration node
    if (IsAvailableAndReadable(hExposureAuto, "ExposureAuto"))
    {
        err = spinEnumerationGetEntryByName(hExposureAuto, "Off", &hExposureAutoOff);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to disable automatic exposure (enum entry retrieval). Aborting with error %d...\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureAuto");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer value from entry node
    if (IsAvailableAndReadable(hExposureAutoOff, "ExposureAutoOff"))
    {
        err = spinEnumerationEntryGetIntValue(hExposureAutoOff, &exposureAutoOff);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to disable automatic exposure (enum entry int value retrieval). Aborting with error %d...\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "ExposureAuto 'Off'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Set integer as new value for enumeration node
    if (IsAvailableAndWritable(hExposureAuto, "ExposureAuto"))
    {
        err = spinEnumerationSetIntValue(hExposureAuto, exposureAutoOff);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to disable automatic exposure (enum entry setting). Aborting with error %d...\n", err);
            return err;
        }
        printf("Automatic exposure disabled...\n");
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureAuto");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    //
    // Set exposure time manually; exposure time recorded in microseconds
    //
    // *** NOTES ***
    // It is ensured that the desired exposure time does not exceed the maximum.
    // Exposure time is counted in microseconds - this can be found out either
    // by retrieving the unit with the spinFloatGetUnit() methods or by
    // checking SpinView.
    //
    spinNodeHandle hExposureTime = NULL;
    double exposureTimeMax = 0.0;
    // double exposureTimeToSet = 2000000.0;

    // Retrieve exposure time node
    err = spinNodeMapGetNode(hNodeMap, "ExposureTime", &hExposureTime);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set exposure time. Aborting with error %d...\n", err);
        return err;
    }

    // Retrieve maximum exposure time value from the float node
    if (IsAvailableAndReadable(hExposureTime, "ExposureTime"))
    {
        err = spinFloatGetMax(hExposureTime, &exposureTimeMax);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set exposure time. Aborting with error %d...\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureTime");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Ensure desired exposure time does not exceed maximum
    if (exposureTimeToSet > exposureTimeMax)
    {
        exposureTimeToSet = exposureTimeMax;
    }

    // Set desired exposure time as new value
    if (IsAvailableAndWritable(hExposureTime, "ExposureTime"))
    {
        err = spinFloatSetValue(hExposureTime, exposureTimeToSet);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to set exposure time. Aborting with error %d...\n", err);
            return err;
        }

        printf("Exposure time set to %f us...\n", exposureTimeToSet);
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureTime");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    return err;
}

spinError ResetExposure(spinNodeHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Turn automatic exposure back on
    //
    // *** NOTES ***
    // It is recommended to have automatic exposure enabled whenever manual
    // exposure settings are not required.
    //
    spinNodeHandle hExposureAuto = NULL;
    spinNodeHandle hExposureAutoContinuous = NULL;
    int64_t exposureAutoContinuous = 0;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "ExposureAuto", &hExposureAuto);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable automatic exposure (node retrieval). Aborting with error %d...\n", err);
        return err;
    }

    // Retrieve entry node from enumeration node
    if (IsAvailableAndReadable(hExposureAuto, "ExposureAuto"))
    {
        err = spinEnumerationGetEntryByName(hExposureAuto, "Continuous", &hExposureAutoContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to enable automatic exposure (enum entry retrieval). Aborting with error %d...\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureAuto");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer value from entry node
    if (IsAvailableAndReadable(hExposureAutoContinuous, "ExposureAutoContinuous"))
    {
        err = spinEnumerationEntryGetIntValue(hExposureAutoContinuous, &exposureAutoContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to enable automatic exposure (enum entry int value retrieval). Aborting with error %d...\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "ExposureAuto 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Set integer as new value for enumeration node
    if (IsAvailableAndWritable(hExposureAuto, "ExposureAuto"))
    {
        err = spinEnumerationSetIntValue(hExposureAuto, exposureAutoContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to enable automatic exposure (enum entry setting). Aborting with error %d...\n", err);
            return err;
        }

        printf("Automatic exposure enabled...\n\n");
    }
    else
    {
        PrintRetrieveNodeFailure("node", "ExposureAuto");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    return err;
}


int main(int argc, char* argv[])
{
    spinError err =SPINNAKER_ERR_SUCCESS;

    if(argc > 3){
        printf("Wrong flag inputs, Configure -c or Reset -r ...\n");
    }
    // FLags
    // -c [time in micro sec] : configure exposure time
    // -r : reset to default
    int cmd;
    double exposure_time;
    int nargs = 0;
    // Configure or Reset
    if(argc > 0){
        if (nargs == 0) {
            if (strcmp(argv[1], "-c") == 0) {
                cmd = CONFIGURE;
                nargs++;
            }
            else if(strcmp(argv[1], "-r") == 0) {
                cmd = RESET;
            }
            else{
                printf("Wrong flag inputs, Configure -c or Reset -r ...\n");
                return EXIT_FAILURE;
            }

        }
    }
    else{
        cmd = RESET;
    }


    // extract exposure time
    if(nargs == 1){
        assert(cmd == CONFIGURE);
        exposure_time = (double)atoi(argv[2]);
    }

    // =========== Instantination of NodeMaps ========= //
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

    // =============  Access the camera and execute the command ========= //
    // Select camera
    spinCamera hCamera = NULL;
    // spinError errReturn = SPINNAKER_ERR_SUCCESS;
    int cam_id = 0;

    err = spinCameraListGet(hCameraList, cam_id, &hCamera);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", err);
        return err;
    }
    else
    {
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
        if(cmd == CONFIGURE){
            // Configure exposure
            err = ConfigureExposure(hNodeMap, exposure_time);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                return err;
            }
        }
        else if(cmd == RESET){
            err = ResetExposure(hNodeMap);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                return err;
            }
        }

    }

    printf("Complete...\n\n");

    // ==================== Cleaning up =====================//
    // Deinitialize camera
    err = spinCameraDeInit(hCamera);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    // Release camera buffer
    err = spinCameraRelease(hCamera);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
      printf("Unable to release camera. Aborting with error %d...\n\n", err);
      return err;
    }

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

    printf("\nDone! Press Enter to exit...\n");

    return err;

}
