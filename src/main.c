#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <bot_core/bot_core.h>

#include <lcmtypes/erlcm_nmea_t.h>

#include "gps.h"
#include "gps_display.h"

void callback(void *_context, int64_t ts, const char *buf);

typedef struct _state_t state_t;

struct _state_t
{
    //getopt_t *gopt;
    gps_t *g;
    gps_display_t *gd;
    FILE *logf;
    lcm_t *lcm;
};


static void
print_usage (char **argv)
{
    fprintf (stderr, "Usage: %s --device <pathtodev> [OPTIONS]\n"
             "Driver for the Garmin GPS 18..\n"
             "\n"
             "  Options:\n"
             "    -h, --help                       shows this help and exits\n"
             "    -d <device>, --device <device>   path to the gps device file\n"
             "    -b N, --baud N,                  serial baud rate\n"
             "    -p N, --port N,                  tcp port [default: 7337]\n"
             "    -a, --allowremote                allow remote connections\n"
             "    --display                        enable curses display\n"
             , argv[0]);
}




int main(int argc, char *argv[])
{
    state_t *self = (state_t*) calloc(1, sizeof(state_t));


    char *portname = NULL;
    int baudrate = 38400;
    int display = 0;

    int port = 7337;
    int allowremote = 1;


    char *optstring = "had:b:p:";
    char c;
    struct option long_opts[] = {
            { "help", no_argument, NULL, 'h' },
            { "allowremote", no_argument, NULL, 'a' },
            { "device", required_argument, NULL, 'd' },
            { "baud", required_argument, NULL, 'b' },
            { "port", required_argument, NULL, 'p' },
            { "display", no_argument, NULL, 'y' },
            { 0, 0, 0, 0 }
    };


    while ((c = getopt_long (argc, argv, optstring, long_opts, 0)) >= 0)
    {
        switch (c) {
            case 'd':
                portname = strdup(optarg);
                break;
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'a':
                allowremote = 1;
                break;
            case 'y':
                display = 1;
                break;
            case 'h':
            default:
                print_usage (argv);
                return 1;
        }
    }


    self->lcm = bot_lcm_get_global(NULL);


    self->g = gps_create(portname);
    if (self->g==NULL) {
        fprintf (stderr, "Error accessing the gps at device port %s\n", portname);
        return -1;
    }

    gps_autonegotiate_baud(self->g, baudrate);
    gps_set_baud(self->g, baudrate);

/*     char *logpath = getopt_get_string(self->gopt, "log"); */
/*     if (strlen(logpath)>0) { */
/*         self->logf = fopen(logpath, "w"); */
/*         if (self->logf == NULL) { */
/*             perror(logpath); */
/*             return -1; */
/*         } */
/*     } */

    // A = use DGPS when available
    // 8 = 38.4kbps on next reboot
    // 0 = no velocity filter
    // 0.5 = dead reckoning time-out
    gps_command(self->g, "$PGRMC,,,,,,,,,A,,,0,,,0.5*");

    // the above has a typo? This might actually turn off the velocity
    // filter
    // gps_command(self->g, "$PGRMC,,,,,,,,,A,,0,,,0.5*");

    // output sentences we want:
    //   GPRMC : basic position information
    // (-) GSA : active satelite data (better precision data)
    // (-) GSV : verbose info about satelites in view
    //   PGRME : position error information
    //   PGRMV : velocity information

    // enable all output sentences
    gps_command(self->g, "$PGRMO,,3*");

    // start the reader thread.
    gps_start(self->g);

    if (0)
    {
        // turn off everything
        gps_command(self->g, "$PGRMO,,2*");

        // enable what we want...
        gps_command(self->g, "$PGRMO,GPRMC,1*");
        gps_command(self->g, "$PGRMO,PGRME,1*");
        gps_command(self->g, "$PGRMO,PGRMV,1*");
        gps_command(self->g, "$PGRMO,GPGSA,1*");
    }

    //int disp = getopt_get_bool(self->gopt, "display");

    if (display)
        self->gd = gps_display_create();

    gps_set_readline_callback(self->g, callback, self);

    gps_display_start(self->gd);

    while (1)
        sleep(1);

    gps_destroy(self->g);

    //fclose(self->logf);
    free(self);
}

void callback(void *_context, int64_t ts, const char *buf)
{
    state_t *self = (state_t*) _context;

    if (self->gd != NULL)
        gps_display_process_nmea(self->gd, buf);

    erlcm_nmea_t nm;
    nm.utime = ts;
    nm.nmea = (char*) buf;

    erlcm_nmea_t_publish (self->lcm, "NMEA", &nm);
}
