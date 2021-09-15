
#ifndef TAO_SPINNAKER_H_
#define TAO_SPINNAKER_H_ 1

#include <tao.h>
#include <SpinnakerC.h>

/*---------------------------------------------------------------------------*/
/* ERRORS */

extern void tao_spinnaker_error_push(
    const char* func,
    spinError err);

/*---------------------------------------------------------------------------*/
/* SYSTEM */

/**
 * Get an instance of the Spinnaker system.
 *
 * This function yields an instance of the Spinnaker system singleton.  The
 * caller is responsible of calling tao_spinnaker_system_release_instance() to
 * eventually release this instance.
 *
 * @param system_ptr  Address of Spinnaker system handle.
 */
extern tao_status tao_spinnaker_system_get_instance(
    spinSystem* system_ptr);

/**
 * Release an instance of the Spinnaker system.
 *
 * This function releases an instance of the Spinnaker system singleton.
 *
 * @param system   Spinnaker system handle (see
 *                 tao_spinnaker_system_get_instance()).
 */
extern tao_status tao_spinnaker_system_release_instance(
    spinSystem system);

/*---------------------------------------------------------------------------*/
/* INTERFACES */

/**
 * Create a list of Spinnaker interfaces.
 *
 * This function creates a list of Spinnaker interfaces available in the
 * system.  The caller is responsible of calling
 * tao_spinnaker_interface_list_destroy() to eventually destroy the list.
 *
 * @param system              An instance of the Spinnaker system (see
 *                            tao_spinnaker_system_get_instance()).
 * @param interface_list_ptr  Address of list of Spinnaker interfaces.
 */
extern tao_status tao_spinnaker_interface_list_create(
    spinSystem system,
    spinInterfaceList* interface_list_ptr);

/**
 * Get the number of Spinnaker interfaces.
 *
 * This function yields the number of interfaces in a list of Spinnaker
 * interfaces.
 *
 * @param interface_list   List of Spinnaker interfaces (created by
 *                         tao_spinnaker_interface_list_create()).
 */
extern long tao_spinnaker_interface_list_get_size(
    spinInterfaceList interface_list);

/**
 * Destroy a list of Spinnaker interfaces.
 *
 * This function destroys a list of Spinnaker interfaces created by
 * tao_spinnaker_interface_list_create().
 */
extern tao_status tao_spinnaker_interface_list_destroy(
    spinInterfaceList interface_list);

extern tao_status tao_spinnaker_interface_list_get(
    spinInterfaceList interface_list,
    long index,
    spinInterface* interface_ptr);

extern tao_status tao_spinnaker_interface_release(
    spinInterface interface);

/*---------------------------------------------------------------------------*/
/* CAMERA LISTS */

/**
  * Get camera list from the interface module
  */
extern tao_status tao_spinnaker_camera_list_create_from_interface(
    spinSystem system,
    long interface_index,
    spinInterfaceList* interface_list_ptr);

/**
  * Create camera list
  */
extern tao_status tao_spinnaker_camera_list_create_empty(
      spinCameraList* camera_list_ptr);

/**
  * Get camera list from the interface module
  */
extern tao_status tao_spinnaker_get_cameras_from_system(
              spinSystem system,
              spinCameraList cam_list);


/**
 * Clear and destroy a Spinnaker camera list.
 */
extern tao_status tao_spinnaker_camera_list_destroy(
    spinCameraList camera_list);


/**
  * Get the number of cameras in the list
  */

extern long tao_spinnaker_camera_list_get_size(
    spinCameraList camera_list);

/*---------------------------------------------------------------------------*/
/* GenICam*/

/**
 * Get a Spinnaker camera.
 *
 * The caller is responsible of eventually calling
 * tao_spinnaker_camera_release() to release the camera.
 */
extern tao_status tao_spinnaker_camera_list_get(
    spinCameraList camera_list,
    long camera_index,
    spinCamera* camera_ptr);



/**
  Initialize the camera
*/
extern tao_status tao_spinnaker_camera_init(
    spinCamera camera);


/**
  Get camera nodemap
**/
extern tao_status tao_spinnaker_camera_get_nodemap(
      spinCamera camera,
      spinNodeMapHandle* cameraNodemap_ptr);

  /**
   * Deinitialize camera.
   */
extern tao_status tao_spinnaker_camera_deinit(spinCamera camera);

/**
 * Release a Spinnaker camera.
 */
extern tao_status tao_spinnaker_camera_release(
    spinCamera camera);


/*---------------------------------------------------------------------------*/
/* Acquisition */

// extern tao_status tao_spinnaker_



/*---------------------------------------------------------------------------*/
/* Utils */

/* Node */
extern tao_status tao_spinnaker_node_is_available(
    spinNodeHandle node,
    int* flag_ptr);

extern tao_status tao_spinnaker_node_is_readable(
    spinNodeHandle node,
    int* flag_ptr);

extern tao_status tao_spinnaker_node_is_writable(
    spinNodeHandle node,
    int* flag_ptr);

extern tao_status tao_spinnaker_node_is_available_and_readable(
    spinNodeHandle node,
    int* flag_ptr);

extern tao_status tao_spinnaker_node_is_available_and_writable(
    spinNodeHandle node,
    int* flag_ptr);

extern tao_status tao_spinnaker_node_map_get_node(
    spinNodeMapHandle node_map,
    const char* node_name,
    spinNodeHandle* node_ptr);

extern tao_status tao_spinnaker_node_map_release_node(
    spinNodeMapHandle node_map,
    spinNodeHandle node);

/* Getter */
// Enumeration node
extern tao_status tao_spinnaker_enumeration_get_entry(
    spinNodeHandle enumNode,
    const char* entry_name,
    spinNodeHandle* entry_ptr);

extern tao_status tao_spinnaker_enumeration_get_int(
    spinNodeHandle nodeValueHandle,
    long* value_ptr);


/* Setter */
// Enumeration node
extern tao_status tao_spinnaker_enumeration_set_int(
    spinNodeHandle nodeEnumeration,
    long value);


#endif /* TAO_SPINNAKER_H_ */
