#include "tao_nuvu.h"

/* Operation to calculate EM gain*/
static int emGainOp(int* nums){
  size_t size = sizeof(nums);
  int  num_array = (int)size/sizeof(int);

  if (num_array !=3){
     printf("The array should has 3 integers, not %d \n", num_array );
   }

  /* median value */
  return (int) (*(nums)+ *(nums+ 1))/2;

  /* user value */
  // return *(num+2);
}

/* Fatal error */
static void fatal_error()
{
    fprintf(stderr, "Some fatal error has been encountered...\n");
    if (tao_any_errors()) {
        tao_report_errors();
    }
    exit(EXIT_FAILURE);
}

/*Initialize*/
tao_status initialize(NcCam* cam){
  tao_status st = TAO_OK;			//We initialize an error flag variable
	double readoutTime, waitingTime, exposureTime;

  printf("Open camera...\n" );
  st = tao_nuvu_cam_open(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, cam);
  if( st != TAO_OK){
     fatal_error();
  }

  st = tao_nuvu_readout_mode(*cam, 1);
  if( st != TAO_OK){
     fatal_error();
  }

  st = tao_nuvu_get_readout_time(*cam, &readoutTime);
  if( st != TAO_OK){
     fatal_error();
  }

	exposureTime = readoutTime;
  st = tao_nuvu_exposure_time(*cam, exposureTime);
  if( st != TAO_OK){
     fatal_error();
  }

  waitingTime = 0.1 * exposureTime;
  st = tao_nuvu_set_waiting_time(*cam, waitingTime);
  if( st != TAO_OK){
     fatal_error();
  }

  st = tao_nuvu_set_timeout(*cam ,  (int)(waitingTime + readoutTime + exposureTime) + 1000);
  if( st != TAO_OK){
     fatal_error();
  }

  printf("initialization is complete.\n" );

  return st;

}


/*Acquisition*/
tao_status continuousAcquisition(NcCam cam, int nbrImagesToSave){
  tao_status  st = TAO_OK;
  int		i;
  char	imageName[31];

  NcImage	*ncImage;

  // open shutter
  enum ShutterMode mode = OPEN;
  st = tao_nuvu_set_shuttermode(cam, OPEN);
  if (st != TAO_OK) {
    fatal_error();
  }
  printf("Acquiring images...\n" );
  // start acquisition
  st = tao_nuvu_cam_start(cam, nbrImagesToSave);
  if (st != TAO_OK) {
    fatal_error();
  }

  // Loop to read images
  for(i = 0; i< nbrImagesToSave; i++){
    // create image name
  	sprintf(imageName, "Image_%d", i);
    printf("Reading image %d \n", i );
    st = tao_nuvu_read_image(cam,&ncImage)  ;
    if(st != TAO_OK){
      fatal_error();
    }

    printf("Saving image \"%s\"\n", imageName);
    st = tao_nuvu_save_image(cam, ncImage, imageName, FITS, "Image acquired in continuous acquisition", 1);

    if(st != TAO_OK){
      fatal_error();
    }

  }

  // abort acquisition
  st = tao_nuvu_cam_abort(cam);
  if(st != TAO_OK){
    printf("Cannot abort acquisition \n" );
  }

  return st;

}


int main(int argc, char const *argv[]) {
  NcCam	cam = NULL;
  tao_status status = TAO_OK;

  const int	nbrImagesToSave = 2;	//The number of images we wish to read from the acquisition
  printf("Initializing .... \n" );
  status = initialize(&cam);

  double detector_temp = 0.0;
  tao_nuvu_detector_temperature(cam, &detector_temp);

  printf("Temperature = %ld \n",(long)detector_temp );
  if (status == TAO_OK) {
	status = continuousAcquisition(cam, nbrImagesToSave);

	}

  if (status != TAO_OK) {
    fatal_error();
  }

  status = tao_nuvu_cam_close(cam);
  if(status != TAO_OK){
    fatal_error();
  }

  printf("Complete... \n" );
  return EXIT_SUCCESS;
}
