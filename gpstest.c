#include <stdio.h>
#include <gps.h>
#include <errno.h>

#include "gpsdclient.h"

static struct fixsource_t source;

void print_info(struct gps_data_t *gpsdata) {

	char scr[128];

	/* Fill in the latitude. */
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.latitude) == 0) {
  	(void)snprintf(scr, sizeof(scr), "%s %c",
    	deg_to_str(deg_dd, fabs(gpsdata->fix.latitude)),
      (gpsdata->fix.latitude < 0) ? 'S' : 'N');
	} else
		(void)snprintf(scr, sizeof(scr), "n/a");

	fprintf(stdout, "%s ", scr);

	/* Fill in the longitude. */
  if (gpsdata->fix.mode >= MODE_2D && isnan(gpsdata->fix.longitude) == 0) {
		(void)snprintf(scr, sizeof(scr), "%s %c",
    	deg_to_str(deg_dd, fabs(gpsdata->fix.longitude)),
    	(gpsdata->fix.longitude < 0) ? 'W' : 'E');
	} else
		(void)snprintf(scr, sizeof(scr), "n/a");

	fprintf(stdout, "%s\n", scr);
}

int main(int argc, char *argv[])
{

		static struct gps_data_t gps_data;

		gpsd_source_spec(NULL, &source);

    if ( gps_open(source.server, source.port, &gps_data) != 0 ) {
			fprintf(stderr, "No gpsd running or network error: %s.\n", gps_errstr(errno));
			return -1;
		} else {

    	gps_stream(&gps_data, WATCH_ENABLE | WATCH_NEWSTYLE, source.device);

			for(;;) {
				if(!gps_waiting(&gps_data, 5000000)) {
					fprintf(stderr, "Timeout\n");
					return -1;
				} else {
					errno = 0;
					if (gps_read(&gps_data) == -1) {
						fprintf(stderr, "Socket error\n");
						return -1;
					} else {
						print_info(&gps_data);
					}
				}
			} // for(;;)
		}

		return 0;
}		
