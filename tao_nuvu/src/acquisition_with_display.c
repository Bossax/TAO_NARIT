#include "tao_nuvu.h"
#include <gtk/gtk.h>
#include <math.h>
#include <malloc.h>
// #include <unistd.h>

#define WIDTH 128
#define HEIGHT 128
#define BYTE_PER_PIXEL 1


// data struct
typedef struct imageBuffer{
  unsigned char* data;
  int stride;
  pthread_mutex_t mutexBuffer;
  pthread_cond_t waitdata;
} imageBuffer;

// global
static int uint16size = (int) pow(2,16);
imageBuffer buff;
static int hasRun = 0;

// ---Utility funcitonsd---- //
// Fatal error
static void fatal_error()
{
    fprintf(stderr, "Some fatal error has been encountered...\n");
    if (tao_any_errors()) {
        tao_report_errors();
    }
    exit(EXIT_FAILURE);
}

// scale 16bbp to 8bbp
unsigned char scale_operation(uint16_t val)
{
		unsigned char h = (unsigned char) floor(((double) val)/uint16size * 255.0);
		return h;

}

void scale_image(uint16_t *image, unsigned char *new_image)
{
	unsigned char	val = 0;
	for(int i = 0; i < HEIGHT*WIDTH; i++){
				val = scale_operation(*(image+i));
				*(new_image+i) = val;
	}
}
//
void printData1(unsigned char * data)
{
	for (int j =0; j < WIDTH*HEIGHT; j++){
		printf("%d ", *(data + j));
		if (j%WIDTH == 0){
			printf("\n");
		}
	}
}

void printData2(unsigned short * data)
{
	for (int j =0; j < WIDTH*HEIGHT; j++){
		printf("%d ", *(data + j));
		if (j%WIDTH == 0){
			printf("\n");
		}
	}
}

// resize array
void resize_array (unsigned char** init_array,  unsigned char* new_array,
	 								int rows, int cols)
{
	for(int row = 0; row < rows; row++)
	 {
		  for(int col = 0; col < cols; col++)
			{
				*(new_array+row*WIDTH+col) = *(*(init_array+row)+col);
			}
	 }
}

/*------ Camera operations ------- */
tao_status initialize(NcCam* cam, double exposureTime, int gain){
  tao_status st = TAO_OK;			//We initialize an error flag variable
	double readoutTime, waitingTime;

  printf("Open camera...\n" );
  st = cam_open(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, cam);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_readout_mode(*cam, 1);
  if( st != TAO_OK){
     fatal_error();
  }

  st = get_readout_time(*cam, &readoutTime);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_exposure_time(*cam, exposureTime);
  if( st != TAO_OK){
     fatal_error();
  }

  waitingTime = 0.1 * exposureTime;
  st = set_waiting_time(*cam, waitingTime);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_timeout(*cam ,  (int)(waitingTime + readoutTime + exposureTime) + 1000);
  if( st != TAO_OK){
     fatal_error();


  }

	// set image size
	set_ROI(*cam, WIDTH, HEIGHT);

	// set gain
	change_analog_gain(*cam, 0);

  printf("initialization is complete.\n" );

  return st;

}

// acquisition
// grab NcImage (unsigned short *)
// downsample
// update the pointer to the 8 bpp image
// No rounding up to 4-byte integer (strid === width)
tao_status acquisition(NcCam cam, unsigned char	*image_handle){
  tao_status  st = TAO_OK;
	NcImage* _image_handle; // 1-D array pointer 16 bpp

  // start acquisition
  st = cam_take_image(cam);
  if (st != TAO_OK) {
    fatal_error();
  }
	// wait image
	int isacquiring = 1;
	while (isacquiring){
		ncCamIsAcquiring(cam, &isacquiring);
	}

	// read image
	st = read_uint16_image(cam, &_image_handle)  ;
	if(st != TAO_OK){
		fatal_error();
	}
	scale_image((uint16_t*) _image_handle, image_handle);

  return st;

}


// --- worker functions --- //
// createImage
// grab unsigned short 1-D array from NcImage
// downsample to 8 bpp unsigned char 1-D array
// put data in the buffer struct
void* createImage(void* arg){

		NcCam	cam = NULL;
	  tao_status st = TAO_OK;
		// pointer to the final data which will be stored in the buffer
		unsigned char	*final_image_array = (unsigned char *) malloc(HEIGHT * WIDTH *
																											sizeof(unsigned char));

		double exposuretime = 100.0;
		st = initialize(&cam, exposuretime , 10);

	  double detector_temp = 0.0;
	 	st =  detector_temperature(cam, &detector_temp);
		printf("Temperature = %ld \n", (long)detector_temp );
//
	  // open shutter
	  enum ShutterMode mode = OPEN;
	  st = set_shuttermode(cam, mode);
	  if (st != TAO_OK) {
	    fatal_error();
	  }
		while (1){
		 /* experiment
			uint32_t *imgg;
			ncCamAllocUInt32Image(cam, &imgg);
			size_t p = malloc_usable_size(imgg);
			printf("UInt32 image size = %ld\n", p );

			int x0,x1,y0,y1 = 0;
			ncCamGetRoi(cam, &x0, &x1, &y0, &y1);
			printf("width = %d, height = %d\n",(x1-x0+1), (y1-y0+1) );


			*/

			st = acquisition(cam, final_image_array);
			if (st != TAO_OK) {
		    fatal_error();
		  }
			// printf("Image is acquired.. \n");

			pthread_mutex_lock(&(buff.mutexBuffer));
			memcpy((void *) buff.data, (void*) final_image_array,HEIGHT * WIDTH *
																												sizeof(unsigned char));
			pthread_mutex_unlock(&(buff.mutexBuffer));


		  // pthread_cond_signal(&(buff.waitdata));
		}

		// /* print raw data
		printData1(final_image_array);
		// */
		// --- Finalizer --- //
		printf("Finishing...\n");
		// close shutter
	  st = set_shuttermode(cam, CLOSE);
	  if (st != TAO_OK) {
	    fatal_error();
	  }

		st = cam_close(cam);
	  if(st != TAO_OK){
	    fatal_error();
	  }

		return NULL;
}


// GTK callback functions
gboolean display(gpointer arg){
  if (hasRun == 0){
    pthread_t imageThread;
    // spawn image generating thread
    if(pthread_create(&imageThread, NULL, createImage, NULL) != 0){
      printf("Failed to create a GTK routine..\n");
    }
		 hasRun = 1;

  }

 GtkImage* img = (GtkImage*) arg;
  static cairo_surface_t *surface;
  // pthread_cond_wait(&(buff.waitdata), &(buff.mutexBuffer));
  pthread_mutex_lock(&(buff.mutexBuffer));

  // create surface
  surface = cairo_image_surface_create_for_data (
    buff.data,                    // unsigned char
    CAIRO_FORMAT_A8,             // cairo_format_t
    HEIGHT,
    WIDTH,
    buff.stride);

  pthread_mutex_unlock(&(buff.mutexBuffer));
  // pthread_cond_signal(&(buff.waitdata));

  // update image
  gtk_image_set_from_surface(img,surface);

  return TRUE;
}


int main(int argc, char **argv) {

	gtk_init (&argc, &argv);
  // initialize global struct
  buff.stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8, WIDTH);
	printf("stride =%d\n", buff.stride);
  buff.data = (unsigned char*) calloc(buff.stride*HEIGHT, sizeof(unsigned char));
  pthread_cond_init(&(buff. waitdata), NULL);
  // GTK initialization
  GtkWidget *img; // canvas object
  GtkWidget *window;

  cairo_surface_t *surface = cairo_image_surface_create_for_data (
    buff.data,                    // unsigned char
    CAIRO_FORMAT_A8,             // cairo_format_t
    HEIGHT,
    WIDTH,
    buff.stride);

  img = gtk_image_new_from_surface(surface);

	// GTK setup
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//title
	gtk_window_set_title(GTK_WINDOW(window), "Nuvu Camera");
	// size
	gtk_window_set_default_size(GTK_WINDOW(window), 800,800);
	// initial position
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// kill signal
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	// add container
  gtk_container_add(GTK_CONTAINER(window), img);
  gtk_widget_show_all(window);
  gdk_threads_add_idle(display,img);
	gtk_main();



  return EXIT_SUCCESS;
}
