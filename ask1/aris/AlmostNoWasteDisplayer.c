//For local testing, comment out ioctl line, ioctl library, and change filename from './' to '/' in open.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <inttypes.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "../lunix-chrdev.h"

//*
#define DBG_MODE
//*/
#ifdef DBG_MODE
#define DEBUG_(a) a
#define LOGGING_STR stderr
#else
#define DEBUG_(a) ;
#define LOGGING_STR fopen("./logfile.log", "w");
#endif
//Structure sring sizes.
#define OUTPUT_BUFSZ 20		//Corresponds to LUNIX_CHRDEV_BUFSZ.
#define DEV_NAMESZ 20		//Maximum length of device name string.

//Log file stream (defaults to stderr).
FILE *log_stream;
//Display update rate (signifies event number). This is here to bring refresh frequency to acceptable rates.
size_t updates_every=6;
//Value array for all sensors (in string format). [Single thread: no semaphore necessary]
enum status_type {offline, online, error};
enum data_mode {raw, cooked};
typedef struct _data_ {	//Sensor data structure: Use 'value' for output string - Use 'buffer' for still buffering string.
	//Public
	char *value;						//Ready value pointer.
	char *buffer;						//Buffer value pointer.
	enum status_type status;			//Connection status. [0: Offline - 1: Online - 2: Error]
	enum data_mode read_mode;			//Raw or cooked input mode (needed for reception termination condition).
	//Private
	int file_d;							//File descriptor for data stream.
	size_t buf_last_pos;				//Value of last written character in buffer (access to buffer[buf_last_pos] will yield the first non-written character: '\0' for consistency reasons).
	char _s1[OUTPUT_BUFSZ];				//Private string 1.
	char _s2[OUTPUT_BUFSZ];				//Private string 2.
	//Protected (for debugging)
	char dev_name[DEV_NAMESZ];			//Device name.
	
} data;
//Static messages.
char *static_msgs[]={"Disconnected", "Initializing", "Error"};
//Array dimension sizes for the program file descriptors.
#define MACHINE_NUM 16
#define SENSOR_NUM 3
//Global value array for sensor data.
data MyRA[MACHINE_NUM][SENSOR_NUM];
_Bool MyRA_Updated=1;

//Data array functions.
_Bool sensor_init (enum data_mode out_mode) {		//Opens the files and initializes the array structures.
	size_t i, j;
	int rv;
	//Initialize log stream.
	log_stream=LOGGING_STR;
	//For all sensors:
	for (i=0; i<MACHINE_NUM; i++) {
		for (j=0; j<SENSOR_NUM; j++) {
			//Create file name.
			snprintf(MyRA[i][j].dev_name, DEV_NAMESZ, "/dev/lunix%u-%s", i, (j==0) ? "batt" : ((j==1) ? "temp" : ((j==2) ? "light" : "error")));
			//Open file.
			MyRA[i][j].file_d=open(MyRA[i][j].dev_name, O_RDONLY);
			if (MyRA[i][j].file_d==-1) {
				fprintf(log_stream, "File %-20s opened for reading (error %3d) : ", MyRA[i][j].dev_name, errno);
				MyRA[i][j].status=error;
			} else {
				fprintf(log_stream, "File %-20s opened for reading (on fd %3d) : ", MyRA[i][j].dev_name, MyRA[i][j].file_d);
				MyRA[i][j].status=online;
			}
			errno=0;
			//Change to proper read mode.
			if (out_mode==raw) {
////				rv=ioctl(MyRA[i][j].file_d, LUNIX_IOC_INMODE, lunix_raw);
				fprintf(log_stream, "Opened in raw mode with value: %d.\n", rv);
			} else {
				fprintf(log_stream, "Opened in default (cooked) mode.\n");
			}
			//Initializing structure fields.
			strcpy(MyRA[i][j]._s1, static_msgs[MyRA[i][j].status]);
			MyRA[i][j].read_mode=out_mode;
			MyRA[i][j].buf_last_pos=0;
			MyRA[i][j].value=MyRA[i][j]._s1;
			MyRA[i][j].buffer=MyRA[i][j]._s2;
			for (rv=0; rv<OUTPUT_BUFSZ; rv++)
				MyRA[i][j].buffer[rv]='\0';
		}
	}
	return 0;
}
void sensor_destr (void) {
	size_t i, j;
	int rv;
	for (i=0; i<MACHINE_NUM; i++) {
		for (j=0; j<SENSOR_NUM; j++) {
			rv=close(MyRA[i][j].file_d);
			fprintf(log_stream, "File %-20s closed ", MyRA[i][j].dev_name);
			if (rv==0)
				fprintf(log_stream, "successfully.\n");
			else
				fprintf(log_stream, "with error %d.\n", errno);
			errno=0;
		}
	}
}
void read_from (int i, int j) {
	int rdrv;
	char ch;
	//We could implement reading of a stream until it doesn't produce anymore (with the help of remembering number of -1s and non-blocking select).
	rdrv=read(MyRA[i][j].file_d, &ch, 1);
	switch (rdrv) {
		case -1	:	//Error.
			if (errno==EAGAIN || errno==EWOULDBLOCK) {
				fprintf(log_stream, "Read of %s (on %d) would block.\n", MyRA[i][j].dev_name, MyRA[i][j].file_d);
			} else {
				fprintf(log_stream, "Error %d appeared upon read of %s (on %d).\n", errno, MyRA[i][j].dev_name, MyRA[i][j].file_d);
				MyRA[i][j].status=error;
				MyRA[i][j].value=static_msgs[error];		//CAN have pointer to external string on error, due to the fact that I don't change it after that.
			}
			errno=0;
			break;
		case 0	:	//EOF.
			fprintf(log_stream, "Device %s (on %d) disconnected.\n", MyRA[i][j].dev_name, MyRA[i][j].file_d);
			MyRA[i][j].status=offline;
			strcpy(MyRA[i][j].value, static_msgs[offline]);
			goto immediate_out;
		default	:	//Assuming read of 1 (also assuming that sizeof(char)=1: Pretty sure that holds for unix systems).
			if (MyRA[i][j].read_mode==raw) {	//If RAW and buf_last_pos=3, output to string and clear buffer.
				//The -2 in the next condition accounts for the fact that is buf_last_pos==2, we will have just read the 4th byte.
				if (MyRA[i][j].buf_last_pos==((sizeof(uint32_t)/sizeof(unsigned char))-2)) {
					uint32_t local_res=0;
					//swap cells (weird module bug ::)))
					size_t i, cell_length=(size_t)(sizeof(uint32_t)/sizeof(unsigned char)), cell_swap_lim=(size_t)(cell_length/2);
					for (i=0; i<cell_swap_lim; i++) {
						MyRA[i][j].buffer[i]^=MyRA[i][j].buffer[cell_length-i-1];
						MyRA[i][j].buffer[cell_length-i-1]^=MyRA[i][j].buffer[i];
						MyRA[i][j].buffer[i]^=MyRA[i][j].buffer[cell_length-i-1];
					}

					local_res=((uint32_t *)MyRA[i][j].buffer)[0];						//wholify:|
					snprintf(MyRA[i][j].buffer, OUTPUT_BUFSZ, "%"PRIu32, local_res);	//create
					char *tp=MyRA[i][j].buffer;											//swap
					MyRA[i][j].buffer=MyRA[i][j].value;
					MyRA[i][j].value=tp;
					MyRA[i][j].buf_last_pos=0;
					fprintf(log_stream, "Value of %s (on %d) updated to '%s'.\n", MyRA[i][j].dev_name, MyRA[i][j].file_d, MyRA[i][j].value);
					MyRA_Updated=1;
				} else {
					MyRA[i][j].buffer[MyRA[i][j].buf_last_pos]=ch;
					MyRA[i][j].buf_last_pos++;
				}
			} else {							//If COOKED and just read '-' or ' ', output to string (swap buffer and value) and clear buffer.
				if (ch=='\0') {
					MyRA[i][j].buffer[MyRA[i][j].buf_last_pos]='\0';	//terminate
					char *tp=MyRA[i][j].buffer;							//swap
					MyRA[i][j].buffer=MyRA[i][j].value;
					MyRA[i][j].value=tp;
					MyRA[i][j].buf_last_pos=0;							//update
					fprintf(log_stream, "Value of %s (on %d) updated to '%s'.\n", MyRA[i][j].dev_name, MyRA[i][j].file_d, MyRA[i][j].value);
					MyRA_Updated=1;										//update array_update indicator
				} else {
					MyRA[i][j].buffer[MyRA[i][j].buf_last_pos]=ch;		//simply update buffer
					MyRA[i][j].buf_last_pos += ((MyRA[i][j].buf_last_pos)<(OUTPUT_BUFSZ-1)) ? 1 : 0;
				}
			}
	}
	//About the looping, here would go the check and reloop (exception: EOF jumps to end).	//...
immediate_out:
	return;
}

//Display structures.
Display *_D;
Window _W;
int _S;
XEvent _E;
GC _gc;
XGCValues _V;
XFontStruct *font_info;
unsigned long white_pixel, black_pixel;
Colormap _CM;
XColor red, blue;
int W_Height=(29*(MACHINE_NUM+1)), W_Width=189*(SENSOR_NUM+1);	//Window proportions are relative to the font size.
KeySym key;
//Inlines for returning coordinates for a 'cell's' output row and collumn in display.
inline int row (int y, int ymax) { return ((W_Height/ymax)*y+20); }
inline int col (int x, int xmax) { return ((W_Width/xmax)*x+10); }

//Selector functions and structures.
void display_update(void);		//Forwards...
fd_set check_set, backup_set;
enum {loop_end,still_going} service_select_request (int x_fd) {
	size_t i, j;
	size_t selects_num=-1;
	//Create the descriptor set (creation can't be static due to inclusion condition).
	int select_max_arg=0;	//Minimum legal fd value.
	FD_ZERO(&backup_set);
	for (i=0; i<MACHINE_NUM; i++) {
		for (j=0; j<SENSOR_NUM; j++) {
			//Remote testing: MyRA[i][j].status==online  --  Normal operation: MyRA[i][j].status!=error
			if (MyRA[i][j].status==online)	//only non-error streams
				FD_SET(MyRA[i][j].file_d, &backup_set);
			select_max_arg=(MyRA[i][j].file_d>select_max_arg) ? MyRA[i][j].file_d : select_max_arg;
		}
	}
	FD_SET(x_fd, &backup_set);
	select_max_arg=(select_max_arg>x_fd) ? select_max_arg : x_fd;
	select_max_arg++;
	check_set=backup_set;
	//Do the select.
	DEBUG_(fprintf(log_stream, "Select called with max_fd %d.\n", select_max_arg));
	if (select(select_max_arg, &check_set, NULL, NULL, NULL)==-1)
		goto error_check;
	//Check the descriptor set.
	for (i=0; i<MACHINE_NUM; i++) {
		for (j=0; j<SENSOR_NUM; j++) {
			//Normal requests serviced here.
			if (FD_ISSET(MyRA[i][j].file_d, &check_set)) {
				DEBUG_(fprintf(stderr, "Request: Descriptor [%d, %d].\n", i, j));
				read_from(i, j);
			}
		}
	}
	//X requests serviced here.
	if (FD_ISSET(x_fd, &check_set)) {
		DEBUG_(fprintf(stderr, "Request X: "));
		char RetV[2]={0};
		XNextEvent(_D, &_E);
		if (_E.type==Expose) {
			display_update();
		} else if (_E.type == KeyPress) {
			XLookupString(&_E.xkey, RetV, 1, &key, NULL);
			if (key=='q' || key=='Q')
				return loop_end;
		}
	} else if (MyRA_Updated==1 && selects_num>=updates_every) {	//Non-event triggered updates on multiples.
		display_update();
		usleep(10000);
		selects_num=0;
	}
	//Normal exit point.
	return still_going;
	//Return 'loop_end' when all sensor fds are non-responsive.
error_check:
	fprintf(log_stream, "Select call returned with %d: ", errno);
	switch (errno) {
		case EBADF:
			fprintf(log_stream, "EBADF: Terminating.\n");
			break;
		case EINTR:
			fprintf(log_stream, "EINTR: Relooping.\n");
			return still_going;
			break;
		case EINVAL:
			fprintf(log_stream, "EINVAL: Terminating.\n");
			break;
		case ENOMEM:
			fprintf(log_stream, "ENOMEM: Relooping after 5 seconds.\n");
			sleep(5);
			return still_going;
			break;
		default:
			fprintf(log_stream, "Undefined error: Terminating.\n");
	}
	//Default error exit point.
	return loop_end;
}

//Display functions.
void display_init (void) {
	_D=XOpenDisplay(NULL);
	if (_D==NULL) {
		fprintf(stderr, "Could not open display.\n");
		exit(EXIT_FAILURE);
	}
	_S=DefaultScreen(_D);
	white_pixel=WhitePixel(_D, _S);
	black_pixel=BlackPixel(_D, _S);
	_W=XCreateSimpleWindow(_D, RootWindow(_D, _S), 100, 100, W_Width, W_Height, 1, black_pixel, white_pixel);
	XSelectInput(_D, _W, ExposureMask|KeyPressMask);
	XMapWindow(_D, _W);
	if ((font_info=XLoadQueryFont(_D, "9x15"))==NULL) {
		fprintf(stderr, "Could not open 9x15 font.\n");
		exit(EXIT_FAILURE);
	}
	_gc=XCreateGC(_D, _W, 0, &_V);
	XSetFont(_D, _gc, font_info->fid);
	XSetBackground(_D, _gc, white_pixel);
	_CM = DefaultColormap(_D, _S);
	if (!XAllocNamedColor(_D, _CM, "red", &red, &red) || !XAllocNamedColor(_D, _CM, "blue", &blue, &blue)) {
		fprintf(stderr, "Failed to allocated color.\n");
		exit(EXIT_FAILURE);		//Maybe add support for grayscale displays.
	}
}
void display_destr (void) {
	XUnloadFont(_D, font_info->fid);
	XFreeGC(_D, _gc);
	XDestroyWindow(_D, _W);
	XCloseDisplay(_D);
}
void display_update (void) {
	char statS[20], *str;
	char _tabs[4][20]={"[q]________", "BATTERY", "TEMPERATURE", "LIGHT"};
	int i, imx=(MACHINE_NUM+1), j, jmx=(SENSOR_NUM+1);
	for (i=0; i<imx; i++) {
		for (j=0; j<jmx; j++) {
			//point to the proper string
			if (i==0) {				//Top row.
				str=_tabs[j];
			} else if (j==0) {		//Left collumn (except for the first cell).
				snprintf(statS, 19, "Lunix_%02d", i);
				str=statS;
			} else {				//Normal cell.
				str=MyRA[i-1][j-1].value;
			}
			//clear previously occupying text
			XClearArea(_D, _W, col(j, jmx), row(i, imx), (W_Width/jmx), (W_Height/imx), 0);
			//set the correct color for current text
			if (i==0 || j==0)
				XSetForeground(_D, _gc, black_pixel);
			else if (MyRA[i-1][j-1].status==error)
				XSetForeground(_D, _gc, red.pixel);
			else if (MyRA[i-1][j-1].status==online)
				XSetForeground(_D, _gc, blue.pixel);
			else
				XSetForeground(_D, _gc, black_pixel);
			XDrawString(_D, _W, /*DefaultGC(_D, _S)*/_gc, col(j, jmx), row(i, imx), str, strlen(str));
			XFlush(_D);
			DEBUG_(fprintf(stderr, "S: %s @ (%d', %d') <= (%d, %d)  : %p\n", str, col(j, jmx), row(i, imx), i, j, str));
		}
	}
	DEBUG_(fprintf(stderr, "DISPLAY was just updated.\n"));
	DEBUG_(usleep(100000));
	usleep(10000);
}

//Main.
int main (int argc, char *argv[]) {
	//Initialize sensors and structures...
	if (argc>2) {
		fprintf(stderr, "Usage: %s {raw, cooked}\n", argv[0]);
		exit(EXIT_FAILURE);
	} else {
		sensor_init((argc==2 && strcmp(argv[1], "raw")==0) ? raw : cooked);
	}
	//Create X connection.
	display_init();
	int x_fd=XConnectionNumber(_D);	//To be perfectly formal, I have to check POSIX compatibility.
	fprintf(log_stream, "X Connection descriptor opened on fd %3d.\n", x_fd);
	/**/
	while (service_select_request(x_fd)!=loop_end) ;
	/**/
	sleep(2);
	//Terminate X connection.
	display_destr();
	//Close files and structures...
	sensor_destr();
	return 0;
}
