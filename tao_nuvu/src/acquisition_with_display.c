// acquisition_with_display.c
// options
// 1. -e [exposuretime in msec]
// 2. -g [analog gain value]
// 4. -go [analog gain offset]
// 3. -gem [em gain value]
// 5. -t [target temperature]
// 6. -r [degree to rotate]
// 7. -w [waiting time in msec]

// -----------------------------------------

#include "tao_nuvu.h"
#include <gtk/gtk.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#define WIDTH 128
#define HEIGHT 128
#define BYTE_PER_PIXEL 1
#define W_WIDTH 800
#define W_HEIGHT 800
#define SCALE_FACTOR 5
#define BYTES_PER_PIXEL 1



// data struct
typedef struct imageBuffer{
  unsigned char* data;
  int stride;
  pthread_mutex_t mutexBuffer;
  pthread_cond_t waitdata;
} imageBuffer;

// global
NcCam	cam = NULL;
imageBuffer buff;
int isRotate =  FALSE;
int degree = 0;
double fps = 0;
static int uint16size = (int) pow(2,16);
static int hasRun = 0;

// Man page
void man(){
	printf(
"______ start_nuvu program ______ \n\
syntax: -[option] [numeric value] \n\
1. -e [exposuretime in msec] \n\
2. -g [analog gain value] \n\
4. -go [analog gain offset] \n\
3. -gem [em gain value] (not done yet)\n\
5. -t [target temperature] \n\
6. -r [degree to rotate] \n\
7. -w [waiting time in msec] \n\
\n");

}

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


/*------ Camera operations ------- */
tao_status initialize(NcCam* cam, int argc, char* argv[]){
   int i;
	 int array_index = 0;
	 char* param[argc];
	 char* val[argc];
	 if (argc > 1){
		 // read input arguments
		 for(i = 1; i < argc; i += 2)
		 {
			 param[array_index] = argv[i];
		 		 val[array_index] = argv[i+1];

		 }
	 }

	 //em gain array
	 //emGainOp =

	 // List of parameters
	 	double readoutTime,
				   waitingTime,
					 exposureTime,
					 temperature;
		int 	 analogGain,
					 analogOffset;


	 // do generic initialization
  tao_status st = TAO_OK;			//We initialize an error flag variable


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
	printf("Estimated readout time = %lf msec\n", readoutTime);

	/* =====assign defaults values to parameters ======== */
	exposureTime = readoutTime;
	waitingTime = 0.1 * exposureTime;
	temperature = -60.0;

	int	error = NC_SUCCESS;

		int	analogGainMin, analogGainMax;
	error = ncCamGetAnalogGainRange(*cam, &analogGainMin, &analogGainMax);
	if (error) {return error;}
	analogGain = (analogGainMin + analogGainMax) / 2;

	int	analogOffsetMin, analogOffsetMax;
	error = ncCamGetAnalogOffsetRange(*cam, &analogOffsetMin, &analogOffsetMax);
	if (error) {return error;}
	analogOffset = (analogOffsetMin + analogOffsetMax) / 2;

	//#======== iterate over the list of options ================#/
	char* op;

	for(int j =0; j < (argc-1)/2; j++ ){
		 op = param[j];

		 // exposure time
		if(strcmp(op, "-e") == 0){
			if (sscanf (val[j], "%lf", &exposureTime) != 1){
				printf("exposure time should be a floating point number\n");
			  fatal_error();
			}
		}
		// analog gain
		else if (strcmp(op, "-g") == 0){
			if (sscanf (val[j], "%d", &analogGain) != 1){
				printf("analog gain should be an integer\n");
				fatal_error();
			}
		}
		// em gain: TODO
		else if (strcmp(op, "-gem") == 0){
			// int emGain = (int) val[j];
			// st = set_em_gain(*cam, emGainOp,emGainInput);
			// if( st != TAO_OK){
			// 	 fatal_error();
			// }
		}
		// analog gain offset
		else if (strcmp(op, "-go") == 0){
			if (sscanf (val[j], "%d", &analogGain) != 1){
				printf("analog offset should be an integer\n");
				fatal_error();
			}
		}
		else if (strcmp(op, "-t") == 0){
			if (sscanf (val[j], "%lf", &temperature) != 1){
				printf("temperature should be a floating point number\n");
				fatal_error();
			}

		}
		else if (strcmp(op, "-r") == 0){
			isRotate = TRUE;
			if (sscanf (val[j], "%d", &degree) != 1){
				printf("degree should be an interger\n");
				fatal_error();
			}
			if(degree%90 != 0){
				printf("Degree must be 90-degree interger\n");
				return 0;
			}
			else
			printf("Image will be rotated %d CW\n", degree);
		}
		else if (strcmp(op, "-w") == 0){
			if (sscanf (val[j], "%lf", &waitingTime) != 1){
				printf("waiting time should be a floating point number\n");
				fatal_error();
			}
		}


	} // end option loop

	// # =============== setter functions ============== #//
	st = set_exposure_time(*cam, exposureTime);
	if( st != TAO_OK){
		 fatal_error();
	}
	printf("exposure time is set to %f msec\n", exposureTime);

	st = set_waiting_time(*cam, waitingTime);
	if( st != TAO_OK){
		 fatal_error();
	}
	printf("waiting time is set to %f msec\n", waitingTime);

	st = set_analog_gain(*cam, analogGain);
	if( st != TAO_OK){
		 fatal_error();
	}
	printf("analog gain is set to %d\n", analogGain);

	st = set_analog_gain(*cam, analogOffset);
	if( st != TAO_OK){
		 fatal_error();
	}
	printf("analog offset is set to %d\n", analogOffset);

	st = set_temperature(*cam, temperature);
	if( st != TAO_OK){
		 fatal_error();
	}
	printf("temperature is set to %f\n", temperature);

	// set reasonable timeout
  st = set_timeout(*cam ,  (int)(readoutTime + exposureTime) + 1000);
  if( st != TAO_OK){
     fatal_error();
	}
	// set image size
	set_ROI(*cam, WIDTH, HEIGHT);

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
	// wait image: blocking style Ok for separate thread
	// FIXME: bad practice
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

// camera status update routines
gboolean temperature_update(gpointer user_data){

		double detector_temp = 0.0;
		detector_temperature(cam, &detector_temp);
		printf("Temperature = %ld \n", (long)detector_temp );
		return TRUE;

}

gboolean fps_update(gpointer user_data)
{
	double fps = 0.0;
	get_framerate(cam, &fps);
	printf("Frame rate = %lf \n", fps );
	return TRUE;

}
// --- worker functions --- //
// createImage
// grab unsigned short 1-D array from NcImage
// downsample to 8 bpp unsigned char 1-D array
// put data in the buffer struct
void* createImage(void* arg)
{


	// pointer to the final data which will be stored in the buffer
	unsigned char	*final_image_array = (unsigned char *) malloc(HEIGHT * WIDTH *
																										sizeof(unsigned char));

	  // open shutter
		tao_status st = TAO_OK;
	  enum ShutterMode mode = OPEN;
	  st = set_shuttermode(cam, mode);
	  if (st != TAO_OK) {
	    fatal_error();
	  }
		while (1){


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


// Callbacks
gboolean redraw(gpointer user_data){
	sleep(0.001);
  gtk_widget_queue_draw((GtkWidget *) user_data);
  return TRUE;
}

// GTK callback functions
gboolean draw_callback(GtkWidget*widget,cairo_t* cr, gpointer arg){
  if (hasRun == 0){
    pthread_t imageThread;
    // spawn image generating thread
    if(pthread_create(&imageThread, NULL, createImage, NULL) != 0){
      printf("Failed to create a GTK routine..\n");
    }
		 hasRun = 1;

  }

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_A8, WIDTH, HEIGHT);
	unsigned char* data_ptr = cairo_image_surface_get_data(surface);

  // pthread_cond_wait(&(buff.waitdata), &(buff.mutexBuffer));
  pthread_mutex_lock(&(buff.mutexBuffer));

  // write data to surface
	cairo_surface_flush(surface);
  memcpy(data_ptr, buff.data, buff.stride*HEIGHT*sizeof(unsigned char));
  cairo_surface_mark_dirty(surface);;

  pthread_mutex_unlock(&(buff.mutexBuffer));
  // pthread_cond_signal(&(buff.waitdata));


  // scale the surface
  static int offsetx_rotate = (W_WIDTH)/2 ;
  static int offsety_rotate = (W_HEIGHT)/2 ;

  if (isRotate == TRUE) {
    cairo_translate (cr, offsetx_rotate,offsety_rotate);
    double radian = degree/180.0 * M_PI;

    cairo_rotate(cr, radian);
    cairo_translate (cr, -SCALE_FACTOR*WIDTH/2,-SCALE_FACTOR*HEIGHT/2);


  }
  cairo_scale (cr, SCALE_FACTOR, SCALE_FACTOR);

  cairo_set_source_surface (cr, surface, 0,0);
  cairo_paint (cr);
  cairo_surface_destroy (surface);

  return FALSE;

}

gboolean quit_callback(gpointer arg)
{
		// clean camera
	enum ShutterMode mode = CLOSE;
	set_shuttermode(cam,mode);
	cam_abort(cam);
	cam_close(cam);
  gtk_main_quit();
	return FALSE;
}

int main(int argc, char **argv) {
	// check --help
	if (argc > 1) {
		if(!strcmp("--help",argv[1]) && (argc == 2)){
			man();
			return 0;
		}
		else if (!strcmp("--help",argv[1]) || (argc == 2)){
			printf("Wrong argument. use --help to see the manual\n");
			return 0;
		}
	}



 	initialize(&cam, argc,argv);

	gtk_init (&argc, &argv);
  // initialize global struct
  buff.stride = cairo_format_stride_for_width (CAIRO_FORMAT_A8, WIDTH);
	printf("stride =%d\n", buff.stride);
  buff.data = (unsigned char*) calloc(buff.stride*HEIGHT, sizeof(unsigned char));
  pthread_cond_init(&(buff. waitdata), NULL);
  // GTK initialization
  GtkWidget *area = gtk_drawing_area_new();
  GtkWidget *window;

	// GTK setup
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//title
	gtk_window_set_title(GTK_WINDOW(window), "Nuvu Camera");
	// size
	gtk_window_set_default_size(GTK_WINDOW(window), 800,800);
	// initial position
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// kill signal
	g_signal_connect(window, "destroy", G_CALLBACK(quit_callback), NULL);
	g_signal_connect(G_OBJECT(area),"draw", G_CALLBACK(draw_callback), NULL);

	// add container
  gtk_container_add(GTK_CONTAINER(window), area);
  gtk_widget_show_all(window);
	gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE, redraw, area, NULL);
	gdk_threads_add_timeout_seconds_full(G_PRIORITY_LOW, 10, temperature_update,cam, NULL);
	gdk_threads_add_timeout_seconds_full(G_PRIORITY_DEFAULT, 10, fps_update,cam, NULL);

	gtk_main();


  return EXIT_SUCCESS;
}
