#include <stdlib.h>
#include <tao-spinnaker.h>

#define ACQUISITION_MODE "Continuous"

static void fatal_error()

{
    fprintf(stderr, "Some fatal error has been encountered...\n");
    if (tao_any_errors()) {
        tao_report_errors();
    }
    exit(EXIT_FAILURE);
}

int main(
    int argc,
    char* argv[])
{
    spinSystem system = NULL;
    spinInterfaceList interface_list = NULL;

    if (tao_spinnaker_system_get_instance(&system) != TAO_OK) {
        fatal_error();
    }

    if (tao_spinnaker_interface_list_create(system, &interface_list) != TAO_OK) {
       fatal_error();
    }

    long number_interfaces = tao_spinnaker_interface_list_get_size(interface_list);
    if (number_interfaces < 0) {
        fatal_error();
    }
    printf("Number of Spinnaker interface(s): %ld\n", number_interfaces);


    // Get camera list from the SYSTEM
    spinCameraList camera_list = NULL;
    if(tao_spinnaker_camera_list_create_empty(&camera_list)){
        fatal_error();
    }

    if(tao_spinnaker_get_cameras_from_system(system, camera_list)){
        fatal_error();
    }
    // Get number of cameras
    long numCameras = 0;

    numCameras = tao_spinnaker_camera_list_get_size(camera_list);
    if(numCameras == 0){
      goto finalize;
    }

    printf("Number of cameras = %ld \n", numCameras);

    #if 0


        // Count number of cameras.
        {
            spinCameraList camera_list = NULL;
            if (tao_spinnaker_camera_list_create_from_interface(
                    system, -1, &interface_list) != TAO_OK) {
                fatal_error();
            }
            long number_cameras = tao_spinnaker_camera_list_get_size(
                camera_list);
            printf("Number of Spinnaker camera(s): %ld\n", number_cameras);
            if (tao_spinnaker_camera_list_destroy(camera_list) != TAO_OK) {
                fatal_error();
            }
        }
    #endif
    #if 0
        for (long interface_index = 0; interface_index < number_interfaces; ++interface_index) {
            spinCameraList camera_list = NULL;
            if (tao_spinnaker_camera_list_create(
                    system, interface_index, &interface_list) != TAO_OK) {
                fatal_error();
            }
            long number_cameras = tao_spinnaker_camera_list_get_size(
                camera_list);
            printf("Spinnaker interface %ld has %ld camera(s)\n",
                   interface_index, number_cameras);
            if (tao_spinnaker_camera_list_destroy(camera_list) != TAO_OK) {
                fatal_error();
            }
        }
    #endif

    /* --------------------- Initialize camera-----------------------------*/

    printf("Please select camera number to operate: ");
    long camera_index = 0;
    char c;
    c  = (char)getchar();
    camera_index = atoi(&c);
    printf("Camera number %ld will be used.. \n", camera_index);

    if(camera_index > numCameras-1){
      printf("Camera number must not greater than %ld \n", numCameras-1);
      goto finalize;
    }

    spinCamera camera = NULL;
    // get the camera from the list
    if(tao_spinnaker_camera_list_get(camera_list, camera_index, &camera)!= TAO_OK){
      fatal_error();
    }

    // Initialize camera
    if(tao_spinnaker_camera_init(camera) != TAO_OK){
      fatal_error();
    }

    // Get camera node map
    spinNodeMapHandle cameraNodeMap = NULL ;
    if(tao_spinnaker_camera_get_nodemap(camera, &cameraNodeMap)){
      fatal_error();
    }

    /* --------------------- Acquire image(s)-----------------------------*/
    // set mode to acquisition mode to continuous

    spinNodeHandle AcquisitionMode = NULL;
    spinNodeHandle AcquisitionModeValue= NULL;
    long acquisitionModeValue = 0;
    const char nodename[256] = "AcquisitionMode";

    if(tao_spinnaker_node_map_get_node(cameraNodeMap, nodename, &AcquisitionMode) != TAO_OK){
      fatal_error();
    }

    int nodeST = 0;
    if(tao_spinnaker_node_is_available_and_readable(AcquisitionMode, &nodeST) == TAO_OK){

      if(tao_spinnaker_enumeration_get_entry(AcquisitionMode, ACQUISITION_MODE, &AcquisitionModeValue) != TAO_OK){
        fatal_error();
      }
      printf("AcquisitionNode obtained...\n");

    }else
    {
      printf("Cannot access to acquisition node .. \n");
      fatal_error();
    }

    // get integer from the entry
    if(tao_spinnaker_node_is_available_and_readable(AcquisitionModeValue, &nodeST) == TAO_OK){

      if(tao_spinnaker_enumeration_get_int(AcquisitionModeValue, &acquisitionModeValue) != TAO_OK)
      {

        fatal_error();
      }

    }else
    {

      fatal_error();
    }

    // set integer to the enumeration node
    if(tao_spinnaker_node_is_available_and_writable(AcquisitionModeValue, &nodeST) == TAO_OK){
      if(tao_spinnaker_enumeration_set_int(AcquisitionMode, acquisitionModeValue) != TAO_OK){

        fatal_error();
      }
    }
    else{
      printf("Cannot set the acquisition mode \n" );
      fatal_error();
    }

    printf("Acquisition mode is set to Continuos..\n");

      /* --------------------- Take images ----------------------------*/





    // deinitialize camera
    if(tao_spinnaker_camera_deinit(camera) != TAO_OK){

      fatal_error();
    }

    // release camera
    if(tao_spinnaker_camera_release(camera) != TAO_OK){

      fatal_error();
    }
    /* --------------------- Finalize handles ----------------------------*/
finalize:


    if(tao_spinnaker_camera_list_destroy(camera_list) != TAO_OK){
      fatal_error();
    }

    if (tao_spinnaker_interface_list_destroy(interface_list) != TAO_OK) {
        fatal_error();
    }

    if (tao_spinnaker_system_release_instance(system) != TAO_OK) {
        fatal_error();
    }
    if (tao_any_errors()) {
        fatal_error();
    }
    printf("Done... \n" );
    return  EXIT_SUCCESS;
}
