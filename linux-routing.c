#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <arpa/inet.h>

static void usage(void)
{
	printf("Usage: linux-routing [-a|d|l] [-n ip_range] [-m mask]\n");
}

static int list_route(void)
{
	char iface[17];
	char gate_addr[128], net_addr[128];
	char mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;

	char *fmt = "%15s %127s %127s %X %d %d %d %127s %d %d %d\n";

	num = sscanf(line, fmt,
		     iface, net_addr, gate_addr,
		     &iflags, &refcnt, &use, &metric, mask_addr,
		     &mss, &window, &irtt);

	return 0;
}

static int edit_route(int what, const char *ip, const char *mask,
		      const char *interface)
{
	int fd;
	struct rtentry route;
	struct sockaddr_in *addr;

	if (ip == NULL || mask == NULL || interface == NULL) {
		printf("%d: %s [%d]\n", __LINE__, strerror(errno), errno);
		return -1;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		printf("%d: %s [%d]\n", __LINE__, strerror(errno), errno);
		return -1;
	}

	memset (&route, 0, sizeof (route));


	addr = (struct sockaddr_in *) &route.rt_dst;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("0.0.0.0");

	addr = (struct sockaddr_in *) &route.rt_gateway;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("192.168.1.1");

	addr = (struct sockaddr_in *) &route.rt_genmask;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("0.0.0.0");

	route.rt_flags = RTF_UP | RTF_GATEWAY;
	route.rt_dev = (char *)interface;

	if (what == 1)
		route.rt_metric = 101;

	if (ioctl(fd, what == 1 ? SIOCADDRT : SIOCDELRT, &route) < 0) {
		printf("%d: %s [%d]\n", __LINE__, strerror(errno), errno);
		return -1;
	}

	return 0;
}

int main (int argc, char **argv)
{
	int opt;
	int what = 0;
	char *addr = NULL, *mask = NULL, *interface = NULL;

	while ((opt = getopt(argc, argv, "adi:m:n:")) != -1) {
		switch (opt) {
		case 'a':
			what = 1;
		break;
		case 'd':
			what = 2;
		break;
		case 'i':
			interface = optarg;
		break;
		case 'm':
			mask = optarg;
		break;
		case 'n':
			addr = optarg;
		break;
		default: /* '?' */
			usage();
			exit(-1);
		}
	}

	if (what) {
		edit_route(what, addr, mask, interface);
	} else {
		list_route();
	}

	exit(0);
}
