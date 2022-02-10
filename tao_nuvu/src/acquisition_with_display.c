#include "tao_nuvu.h"
#include <gtk/gtk.h>
#include <math.h>
// #include <unistd.h>

#define WIDTH 800
#define HEIGHT 800
#define BYTE_PER_PIXEL 1

// global
static int uint16size = (int) pow(2,16);


struct callback_arguments{
	NcCam cam;
	GtkImage *img;
};

/* Fatal error */
static void fatal_error()
{
    fprintf(stderr, "Some fatal error has been encountered...\n");
    if (tao_any_errors()) {
        tao_report_errors();
    }
    exit(EXIT_FAILURE);
}

unsigned char scale_operation(uint16_t val)
{
	return (unsigned char) floor(val/uint16size * 255);
}

/* scale 16bbp to 8bbp*/
void scale_image(uint16_t **image, unsigned char **new_image)
{
	for(int row = 0; row < HEIGHT; row++){
			for(int col = 0; col < WIDTH; col++){
					*(*(new_image+row) + col) = scale_operation(*(*(image+row) + col));
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




/*Initialize*/
tao_status initialize(NcCam cam, double exposureTime, int gain){
  tao_status st = TAO_OK;			//We initialize an error flag variable
	double readoutTime, waitingTime;

  printf("Open camera...\n" );
  st = cam_open(NC_AUTO_UNIT, NC_AUTO_CHANNEL, 4, &cam);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_readout_mode(cam, 1);
  if( st != TAO_OK){
     fatal_error();
  }

  st = get_readout_time(cam, &readoutTime);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_exposure_time(cam, exposureTime);
  if( st != TAO_OK){
     fatal_error();
  }

  waitingTime = 0.1 * exposureTime;
  st = set_waiting_time(cam, waitingTime);
  if( st != TAO_OK){
     fatal_error();
  }

  st = set_timeout(cam ,  (int)(waitingTime + readoutTime + exposureTime) + 1000);
  if( st != TAO_OK){
     fatal_error();


  }

	// set image size
	set_ROI(cam, WIDTH, HEIGHT);

	// set gain
	change_analog_gain(cam, 0);

  printf("initialization is complete.\n" );

  return st;

}

tao_status acquisition(NcCam cam, unsigned char	**image_handle){
  tao_status  st = TAO_OK;
	uint16_t** _image_handle = malloc(WIDTH*HEIGHT*sizeof(uint16_t));

  // open shutter
  enum ShutterMode mode = OPEN;
  st = set_shuttermode(cam, mode);
  if (st != TAO_OK) {
    fatal_error();
  }

  // start acquisition
  st = cam_set_ready(cam);
  if (st != TAO_OK) {
    fatal_error();
  }

	st = read_uint16_image(cam, _image_handle)  ;
	if(st != TAO_OK){
		fatal_error();
	}

  // abort acquisition
  st = cam_abort(cam);
  if(st != TAO_OK){
    printf("Cannot abort acquisition \n" );
  }

  // close shutter
  st = set_shuttermode(cam, CLOSE);
  if (st != TAO_OK) {
    fatal_error();
  }

	scale_image(_image_handle, image_handle);

	free(_image_handle);

  return st;

}


// GTK callback functions

gboolean display_callback(gpointer user_data)
{
	struct callback_arguments *arg = user_data;
  cairo_surface_t *surface;
  int stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8, WIDTH);
  unsigned char* img_data = (unsigned char*) malloc(stride*HEIGHT*sizeof(unsigned char));
	unsigned char	**image_handle;

	acquisition(arg->cam, image_handle);
	resize_array(image_handle, img_data, HEIGHT, stride);

	surface = cairo_image_surface_create_for_data (
				 img_data,                    // unsigned char
				 CAIRO_FORMAT_A8,             // cairo_format_t
				 HEIGHT,
				 WIDTH,
				 stride);

	gtk_image_set_from_surface(arg -> img, surface);

	return TRUE;
}



int main(int argc, char **argv) {
	NcCam	cam = NULL;
  tao_status status = TAO_OK;

	double exposuretime = 100.0;
	status = initialize(cam, exposuretime , 10);

  double detector_temp = 0.0;
  detector_temperature(cam, &detector_temp);

	gtk_init (&argc, &argv);
	GtkWidget *window;
	GtkWidget *img;



	struct callback_arguments *pack;
	pack->cam = cam;
	pack->img = (GtkImage *) img;


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
	g_timeout_add (10, display_callback, pack);
	gtk_main();

  status = cam_close(cam);
  if(status != TAO_OK){
    fatal_error();
  }

  return EXIT_SUCCESS;
}
