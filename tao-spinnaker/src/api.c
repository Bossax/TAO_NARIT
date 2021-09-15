#include "tao-spinnaker.h"

/*---------------------------------------------------------------------------*/
/* ERRORS */

void tao_spinnaker_error_push(
    const char* func,
    spinError err)
{
    fprintf(stderr, "error %ld in function %s\n", (long) err, func);
}

/*---------------------------------------------------------------------------*/
/* SYSTEM */

tao_status tao_spinnaker_system_get_instance(
    spinSystem* system_ptr)
{
    spinError err = spinSystemGetInstance(system_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinSystemGetInstance", err);
        *system_ptr = NULL;
        return TAO_ERROR;
    }
    return TAO_OK;
}

tao_status tao_spinnaker_system_release_instance(
    spinSystem system)
{
    spinError err = spinSystemReleaseInstance(system);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinSystemReleaseInstance", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}

/*---------------------------------------------------------------------------*/
/* INTERFACES */

tao_status tao_spinnaker_interface_list_create(
    spinSystem system,
    spinInterfaceList* interface_list_ptr)
{
    spinError err;
    err = spinInterfaceListCreateEmpty(interface_list_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceListCreateEmpty", err);
        goto error;
    }
    err = spinSystemGetInterfaces(system, *interface_list_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinSystemGetInterfaces", err);
        tao_spinnaker_interface_list_destroy(*interface_list_ptr);
        goto error;
    }
    return TAO_OK;
error:
    *interface_list_ptr = NULL;
    return TAO_ERROR;
}

long tao_spinnaker_interface_list_get_size(
    spinInterfaceList interface_list)
{
    size_t size = 0;
    spinError err = spinInterfaceListGetSize(interface_list, &size);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceListGetSize", err);
        return -1L;
    }
    long result = (long)size;
    if (result < 0 || result != size) {
        tao_push_error(__func__, TAO_BAD_VALUE); // FIXME: TAO_INTEGER_OVERFLOW
        return -1L;
    }
    return result;
}

tao_status tao_spinnaker_interface_list_destroy(
    spinInterfaceList interface_list)
{
    tao_status status = TAO_OK;
    spinError err;
    err = spinInterfaceListClear(interface_list);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceListClear", err);
        status = TAO_ERROR;
    }
    err = spinInterfaceListDestroy(interface_list);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceListDestroy", err);
        status = TAO_ERROR;
    }
    return status;
}

tao_status tao_spinnaker_interface_list_get(
    spinInterfaceList interface_list,
    long index,
    spinInterface* interface_ptr)
{
    spinError err = spinInterfaceListGet(interface_list, index, interface_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceListGet", err);
        *interface_ptr = NULL;
        return TAO_ERROR;
    }
    return TAO_OK;
}

tao_status tao_spinnaker_interface_release(
    spinInterface interface)
{
    spinError err = spinInterfaceRelease(interface);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinInterfaceRelease", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}

/*---------------------------------------------------------------------------*/
/* CAMERA LISTS */

tao_status tao_spinnaker_camera_list_create_from_interface(
    spinSystem system,
    long interface_index,
    spinCameraList* camera_list_ptr)
{
    tao_status result = TAO_OK;
    spinInterfaceList interface_list = NULL;
    spinInterface interface = NULL;
    int level = 0; // to keep of level reached for cleanup
    spinError err;

    // Create empty camera list.
    err = spinCameraListCreateEmpty(camera_list_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraListCreateEmpty", err);
        goto error;
    }
    level = 1;

    // Fill camera list.
    if (index < 0) {
        // Count cameras on all interfaces.
        err = spinSystemGetCameras(system, *camera_list_ptr);
        if (err != SPINNAKER_ERR_SUCCESS) {
            tao_spinnaker_error_push("spinSystemGetCameras", err);
            goto error;
        }
    } else {
        // Count cameras on a given interface.
        if (tao_spinnaker_interface_list_create(
                &interface_list, system) != TAO_OK) {
            goto error;
        }
        level = 2;
        if (tao_spinnaker_interface_list_get(
                interface_list, interface_index, &interface) != TAO_OK) {
            goto error;
        }
        level = 3;
        err = spinInterfaceGetCameras(interface, *camera_list_ptr);
        if (err != SPINNAKER_ERR_SUCCESS) {
            tao_spinnaker_error_push("spinInterfaceGetCameras", err);
            goto error;
        }
    }

done:
    if (level >= 3) {
        if (tao_spinnaker_interface_release(interface) != TAO_OK) {
            result = TAO_ERROR;
        }
    }
    if (level >= 2) {
        if (tao_spinnaker_interface_list_destroy(interface_list) != TAO_OK) {
            result = TAO_ERROR;
        }
    }
    if (level >= 1 && result != TAO_OK) {
        tao_spinnaker_camera_list_destroy(*camera_list_ptr);
        result = TAO_ERROR;
    }
    if (result != TAO_OK) {
        *camera_list_ptr = NULL;
    }
    return result;
error:
    result = TAO_ERROR;
    goto done;
}


tao_status tao_spinnaker_camera_list_create_empty(spinCameraList* camera_list_ptr)
{
  tao_status status = TAO_OK;
  spinError err = SPINNAKER_ERR_SUCCESS;
  err = spinCameraListCreateEmpty(camera_list_ptr);
  if (err != SPINNAKER_ERR_SUCCESS)
  {
      tao_spinnaker_error_push(__func__, err);
      status = TAO_ERROR;
  }
  return status;
}

tao_status tao_spinnaker_get_cameras_from_system(
        spinSystem system,
        spinCameraList cam_list)
{
    tao_status status = TAO_OK;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinSystemGetCameras(system, cam_list);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        tao_spinnaker_error_push(__func__, err);
        status = TAO_ERROR;
    }
    return status;
}


long tao_spinnaker_camera_list_get_size(
    spinCameraList camera_list)
{
    size_t size = 0;
    spinError err = spinCameraListGetSize(camera_list, &size);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraListGetSize", err);
        return -1L;
    }
    long result = (long)size;
    if (result < 0 || result != size) {
        tao_push_error(__func__, TAO_BAD_VALUE); // FIXME: TAO_INTEGER_OVERFLOW
        return -1L;
    }
    return result;
}

tao_status tao_spinnaker_camera_list_destroy(
    spinCameraList camera_list)
{
    tao_status status = TAO_OK;
    spinError err;
    err = spinCameraListClear(camera_list);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraListClear", err);
        status = TAO_ERROR;
    }
    err = spinCameraListDestroy(camera_list);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraListDestroy", err);
        status = TAO_ERROR;
    }
    return status;
}



/*---------------------------------------------------------------------------*/
/* GenICam*/

tao_status tao_spinnaker_camera_list_get(
    spinCameraList camera_list,
    long camera_index,
    spinCamera* camera_ptr)
{
    spinError err = spinCameraListGet(camera_list, camera_index, camera_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraListGet", err);
        *camera_ptr = NULL;
        return TAO_ERROR;
    }
    return TAO_OK;
}

tao_status tao_spinnaker_camera_init(spinCamera camera)
{

  tao_status status = TAO_OK;
  spinError err = SPINNAKER_ERR_SUCCESS;

  err = spinCameraInit(camera);
  if (err != SPINNAKER_ERR_SUCCESS)
  {
      tao_spinnaker_error_push("spinCameraInit", err);
      status = TAO_ERROR;
  }
  return status;
}


tao_status tao_spinnaker_camera_get_nodemap(
                                            spinCamera camera,
                                            spinNodeMapHandle* cameraNodemap_ptr)
{
    tao_status status = TAO_OK;
    spinError err = SPINNAKER_ERR_SUCCESS;
    err = spinCameraGetNodeMap(camera, cameraNodemap_ptr);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
      tao_spinnaker_error_push("spinCameraGetNodeMap", err);
      status = TAO_ERROR;
    }
    return status;
}

tao_status tao_spinnaker_camera_deinit(spinCamera camera)
{
  tao_status status = TAO_OK;
  spinError err = SPINNAKER_ERR_SUCCESS;

  err = spinCameraDeInit(camera);
  if (err != SPINNAKER_ERR_SUCCESS)
  {
      tao_spinnaker_error_push("spinCameraInit", err);
      status = TAO_ERROR;
  }
  return status;
}


tao_status tao_spinnaker_camera_release(
    spinCamera camera)
{
    spinError err = spinCameraRelease(camera);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinCameraRelease", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}
/*---------------------------------------------------------------------------*/
/* ACQUISITION */



/*---------------------------------------------------------------------------*/
/* NODES */
// util functions
static int node_is_available(
    spinNodeHandle node)
{
    bool8_t flag = False;
    spinError err = spinNodeIsAvailable(node, &flag);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinNodeIsAvailable", err);
        return -1;
    }
    return (flag != False ? 1 : 0);
}

tao_status tao_spinnaker_node_is_available(
    spinNodeHandle node,
    int* flag_ptr)
{
    int flag = node_is_available(node);
    if (flag == -1) {
        *flag_ptr = 0;
        return TAO_ERROR;
    } else {
        *flag_ptr = flag;
        return TAO_OK;
    }
}

tao_status tao_spinnaker_node_is_readable(
    spinNodeHandle node,
    int* flag_ptr)
{
    bool8_t flag = False;
    spinError err = spinNodeIsReadable(node, &flag);

    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinNodeIsReadable", err);
        *flag_ptr = 0;
        return TAO_ERROR;
    }
    *flag_ptr = (flag != False);

    return TAO_OK;
}

tao_status tao_spinnaker_node_is_writable(
    spinNodeHandle node,
    int* flag_ptr)
{
    bool8_t flag = False;
    spinError err = spinNodeIsWritable(node, &flag);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinNodeIsWritable", err);
        *flag_ptr = 0;
        return TAO_ERROR;
    }
    *flag_ptr = (flag != False);
    return TAO_OK;
}

tao_status tao_spinnaker_node_is_available_and_readable(
    spinNodeHandle node,
    int* flag_ptr)
{
    // tao_status st = TAO_OK;
    int flag = node_is_available(node);
    if (flag == -1) {
        *flag_ptr = 0;
        return TAO_ERROR;
    } else if (flag)
    {
      return tao_spinnaker_node_is_readable(node, flag_ptr);
    } else {
        *flag_ptr = 0;
        return TAO_OK;
    }
}


tao_status tao_spinnaker_node_is_available_and_writable(
    spinNodeHandle node,
    int* flag_ptr)
{
    int flag = node_is_available(node);
    if (flag == -1) {
        *flag_ptr = 0;
        return TAO_ERROR;
    } else if (flag) {
        return tao_spinnaker_node_is_writable(node, flag_ptr);
    } else {
        *flag_ptr = 0;
        return TAO_OK;
    }
}

tao_status tao_spinnaker_node_map_get_node(
    spinNodeMapHandle node_map,
    const char* node_name,
    spinNodeHandle* node_ptr)
{
    spinError err = spinNodeMapGetNode(node_map, node_name, node_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinNodeMapGetNode", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}

tao_status tao_spinnaker_node_map_release_node(
    spinNodeMapHandle node_map,
    spinNodeHandle node)
{
    spinError err = spinNodeMapReleaseNode(node_map, node);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinNodeMapReleaseNode", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}

/* Getter */
// String node
tao_status tao_spinnaker_string_get_value(
    spinNodeHandle node,
    char* buffer,
    size_t* size_ptr)
{
    spinError err = spinStringGetValue(node, buffer, size_ptr);
    if (err != SPINNAKER_ERR_SUCCESS) {
        tao_spinnaker_error_push("spinStringGetValue", err);
        return TAO_ERROR;
    }
    return TAO_OK;
}


// Enumeration node
tao_status tao_spinnaker_enumeration_get_entry(
    spinNodeHandle enumNode,
    const char* entry_name,
    spinNodeMapHandle* entryNode_ptr)
{
      tao_status status = TAO_OK;
      spinError err =SPINNAKER_ERR_SUCCESS;

      err = spinEnumerationGetEntryByName(enumNode, entry_name, entryNode_ptr);
      if (err != SPINNAKER_ERR_SUCCESS)
      {
        tao_push_error("spinEnumerationGetEntryByName", err);
        status = TAO_ERROR;
      }
      return status;
}

tao_status tao_spinnaker_enumeration_get_int(
    spinNodeHandle nodeValueHandle,
    long* value_ptr)
{
    tao_status status = TAO_OK;
    spinError err =SPINNAKER_ERR_SUCCESS;
    err = spinEnumerationEntryGetIntValue(nodeValueHandle, value_ptr);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
      tao_push_error("spinEnumerationEntryGetIntValue", err);
      status = TAO_ERROR;
    }
    return status;
}

/* Setter */
// Enumeration node
tao_status tao_spinnaker_enumeration_set_int(
    spinNodeHandle nodeEnumeration,
    long value)
{
  tao_status status = TAO_OK;
  spinError err =SPINNAKER_ERR_SUCCESS;

  err = spinEnumerationSetIntValue(nodeEnumeration, value);
  if (err != SPINNAKER_ERR_SUCCESS)
  {
    tao_push_error("spinEnumerationSetIntValue", err);
    status = TAO_ERROR;
  }
  return status;

}
