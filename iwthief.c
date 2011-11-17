#include <stdio.h>
#include <iwlib.h>

#include "iwspy.h"

typedef struct iwscan_state
{
	/* State */
	int     ap_num;   /* Access Point number 1->N */
	int     val_index;  /* Value in table 0->(N-1) */
} iwscan_state;

int print_scan_token(struct stream_descr * stream,
		struct iw_event	* event,
		struct iwscan_state * state,
		struct iw_range * iwrange,
		int has_range)
{
	char buffer[128];

	switch (event->cmd)
	{
		case SIOCGIWAP:
			printf("Cell %02d - Address: %s\n", state->ap_num, iw_saether_ntop(&event->u.ap_addr, buffer));
			state->ap_num++;
			break;
		case SIOCGIWESSID:
			{
				char essid[IW_ESSID_MAX_SIZE+1];
				memset(essid, '\0', sizeof(essid));
				if ((event->u.essid.pointer) && (event->u.essid.length))
					memcpy(essid, event->u.essid.pointer, event->u.essid.length);
				if (event->u.essid.flags) {
					if ((event->u.essid.flags & IW_ENCODE_INDEX) > 1)
						printf("ESSID: \"%s\" [%d]\n", essid, (event->u.essid.flags & IW_ENCODE_INDEX));
					else
						printf("ESSID: \"%s\"\n", essid);
				} else
					printf("ESSID: hidden\n");
			}
			break;
	}
	return 0;
}

int scan_device(int skfd, char *ifname)
{
	struct iw_range range;
	int has_range;
	struct timeval tv;
	struct iw_scan_req scanopt;
	struct iwreq wrq;
	int timeout = 15000000; // 15s
	unsigned char *buffer = NULL;
	int buflen = IW_SCAN_MAX_DATA;

	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

	if (!has_range) {
		fprintf(stderr, "Interface %s doesn't support scanning\n", ifname);
		return -1;
	}

	fprintf(stdout, "Interface %s support scanning\n", ifname);

	tv.tv_sec = 0;
	tv.tv_usec = 25000;

	memset(&scanopt, 0, sizeof(scanopt));

	wrq.u.data.pointer = NULL;
	wrq.u.data.flags = 0;
	wrq.u.data.length = 0;

	if (iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0) {
		fprintf(stderr, "iw_set_ext() error: %s\n", strerror(errno));
		return -1;
	}

	timeout -= tv.tv_usec;

	while(1) {
		fd_set rfds;
		int last_fd;
		int ret;

		FD_ZERO(&rfds);
		last_fd = -1;

		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);
		if (ret < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "Unhandled signal, exiting...\n");
			return -1;
		}
		if (ret == 0) {
			unsigned char *newbuf;
		realloc:
			newbuf = realloc(buffer, buflen);
			if(newbuf == NULL) {
				if(buffer) { free(buffer); }
				fprintf(stderr, "Allocation failed, %s", __FUNCTION__);
				return -1;
			}
			buffer = newbuf;

			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;

			if (iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0) {
				/* E2BIG */
				if (errno == E2BIG) {
					if (wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *=2;
				}
				/* Try again */
				goto realloc;
				/* EAGAIN */
				if (errno == EAGAIN) {
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					if (timeout > 0)
						continue;
				}
				/* Other error - we dont'want wat to do... */
				fprintf(stderr, "Error occured in iw_get_ext(). %s\nEiting...\n", strerror(errno));
				return -1;
			} else /* iw_get_ext() > 0 */
				break;
		}
	} /* while(1) */

	if (wrq.u.data.length) {
		int i;
		struct iw_event iwe;
		struct stream_descr stream;
		struct iwscan_state state = { .ap_num = 1, .val_index = 0 };
		int ret;

		//printf("Scan result %d [%02X", wrq.u.data.length, buffer[0]);
		//for(i = 1; i < wrq.u.data.length; i++) printf(":%02X", buffer[i]);
		//printf("]\n");

		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		do {
			ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
			if (ret > 0) {
				print_scan_token(&stream, &iwe, &state, &range, has_range);
			}
		} while (ret > 0);
		printf("\n");
	} else {
		fprintf(stderr, "No scan results ;(\n");
		return -1;
	}


	return 0;
}

int main (int argc, char *argv[])
{
  int skfd;

  printf("IWSpy, version 0.01 alpha\n");

  if ((skfd = iw_sockets_open()) < 0) {
    printf("Cannot open socket\n");
    return -1;
  }

	int result = scan_device(skfd, "wlan0");

	printf("Scan result = %d\n", result);

  return 0;
}
