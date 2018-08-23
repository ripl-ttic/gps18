// file: gps_lcm_listen.c
// desc: listens for nmea messages transmitted via LCM, and displays some basic
//       GPS information as it arrives over the network.

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <bot_core/bot_core.h>
//#include <common/nmea.h>

#include <lcmtypes/nmea_t.h>

#include "gps_display.h"

typedef struct _state {
    struct gps_display *gd;
    lcm_t *lcm;
    pthread_t gps_tid;
} state_t;

static void
on_nmea(const lcm_recv_buf_t *rbuf, const char *channel,
        const nmea_t *_nmea, void *user)
{
    state_t *self = (state_t*) user;

    gps_display_process_nmea( self->gd, _nmea->nmea );
    return;
}

int main(int argc, char **argv)
{
    state_t *self = (state_t*) calloc(1, sizeof(state_t));

    self->gd = gps_display_create();

    // initialize LCM
    self->lcm = bot_lcm_get_global(NULL);
    if( NULL == self->lcm ) { fprintf(stderr, "error allocating LCM.  "); return 1; }
    //if( 0 != lcm_init(self->lcm, NULL)) {
    //    fprintf(stderr, "error initializing LCM!\n"); return 1; }

    // subscribe to GPS NMEA messages
    nmea_t_subscribe( self->lcm, "NMEA", on_nmea, self );

    self->gps_tid = gps_display_start(self->gd);

    while(1) {
        lcm_handle( self->lcm );
    }


    // cleanup
    free (self);

    // TODO cleanup gps_display

    return 0;
}
