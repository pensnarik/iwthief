#include <stdio.h>
#include <gps.h>
#include <errno.h>

int main(int argc, char *argv[])
{

		struct gps_data_t * gps_data;

    gps_data = gps_open("localhost", "2974");

		if(!gps_data) {
			fprintf(stderr, "No gpsd running or network error.\n");
			return -1;
		} else {

    	gps_stream(gps_data, WATCH_ENABLE | WATCH_NEWSTYLE, NULL);
		}

		return 0;
}		
