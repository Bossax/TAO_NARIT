# Spinnaker library for Cameras

TAO interface to Spinnaker SDK is to facilitate writing code using cameras
managed by Spinnaker.

- The wrapped functions are prefixed by `tao_spinnaker_`.

- The wrapped functions use TAO error stack to report errors.



## Handles

The Spinnaker SDK does not directly expose structures but provides opaque
handles which are simple pointers.  It seems that the `NULL` value is suitable
to indicate that any such handle is not yet itialized or have been destroyed or
released.
