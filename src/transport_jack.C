/*******************************************************************************/
/* Copyright (C) 2007-2008 Jonathan Moore Liles                                */
/* Copyright (C) 2010 Julien 'Lta' BALLET                                      */
/*                                                                             */
/* This program is free software; you can redistribute it and/or modify it     */
/* under the terms of the GNU General Public License as published by the       */
/* Free Software Foundation; either version 2 of the License, or (at your      */
/* option) any later version.                                                  */
/*                                                                             */
/* This program is distributed in the hope that it will be useful, but WITHOUT */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   */
/* more details.                                                               */
/*                                                                             */
/* You should have received a copy of the GNU General Public License along     */
/* with This program; see the file COPYING.  If not,write to the Free Software */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*******************************************************************************/

#include <math.h>

#include "transport_jack.H"
#include "jack.H"


static volatile bool _done;
/* A condition to wait for the first jack synchronization call */
static pthread_cond_t sync_started_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sync_started_cond_mutex = PTHREAD_MUTEX_INITIALIZER;


static int sync ( jack_transport_state_t state, jack_position_t *pos, void * );
static void _jack_timebase_proxy( jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg );


TransportJack::TransportJack( jack_client_t &jack_client )
  :client(&jack_client)
{
  _done = false;
}

void
TransportJack::init ( Transport &t )
{
  /* We need to wait until jack finally calls our timebase and
   * process callbacks in order to be able to test for valid
   * transport info. We need to lock before activating jack to
   * avoid a race condition */
  pthread_mutex_lock(&sync_started_cond_mutex);

  jack_set_sync_callback( client, sync, 0 );

  if ( jack_set_timebase_callback( client, 1, _jack_timebase_proxy, &t) == 0 )
  {
    MESSAGE( "Waiting for JACK..." );
    pthread_cond_wait(&sync_started_cond, &sync_started_cond_mutex);
    MESSAGE( "running as timebase master" );
    transport.master = true;
  }
  else
    WARNING( "could not take over as timebase master" );

  pthread_mutex_unlock(&sync_started_cond_mutex);
}

void
TransportJack::transport_update ( Transport &t )
{
    static bool catched_transport = true;
    jack_transport_state_t ts;
    jack_position_t pos;

    ts = jack_transport_query( client, &pos );

    t.rolling = ts == JackTransportRolling;

    t.valid = pos.valid & JackPositionBBT;

    t.bar = pos.bar;
    t.beat = pos.beat;
    t.tick = pos.tick;

    /* bars and beats start at 1.. */
    pos.bar--;
    pos.beat--;

    /* FIXME: these probably shouldn't be called from the RT
       thread... Anyway, it happens infrequently. */
    if ( pos.beats_per_minute != t.beats_per_minute )
        t.signal_tempo_change( pos.beats_per_minute );

    if ( pos.beats_per_bar != t.beats_per_bar )
        t.signal_bpb_change( pos.beats_per_bar );

    if ( pos.beat_type != t.beat_type )
        t.signal_beat_change( pos.beat_type );

    t.ticks_per_beat = pos.ticks_per_beat;
    t.beats_per_bar = pos.beats_per_bar;
    t.beat_type = pos.beat_type;
    t.beats_per_minute = pos.beats_per_minute;

    t.frame = pos.frame;
    t.frame_rate = pos.frame_rate;

    /* FIXME: this only needs to be calculated if bpm or framerate changes  */
    {
        const double frames_per_beat = t.frame_rate * 60 / t.beats_per_minute;

        t.frames_per_tick = frames_per_beat / (double)PPQN;
        t.ticks_per_period = t.nframes / t.frames_per_tick;
    }

    tick_t abs_tick = (pos.bar * pos.beats_per_bar + pos.beat) * pos.ticks_per_beat + pos.tick;
//    tick_t abs_tick = pos.bar_start_tick + (pos.beat * pos.ticks_per_beat) + pos.tick;

    /* scale Jack's ticks to our ticks */

    const double pulses_per_tick = PPQN / pos.ticks_per_beat;

    t.ticks = abs_tick * pulses_per_tick;
    t.tick = t.tick * pulses_per_tick;

    t.ticks_per_beat = PPQN;

    /* Unlocking midi_init function at the first call of this callback */
    if (catched_transport && t.valid)
    {
      pthread_mutex_lock(&sync_started_cond_mutex);
      pthread_cond_signal(&sync_started_cond);
      pthread_mutex_unlock(&sync_started_cond_mutex);
      catched_transport = false;
    }
}

void
TransportJack::start ( Transport &t )
{
  jack_transport_start( client );
}

void
TransportJack::stop ( Transport &t )
{
  jack_transport_stop( client );
}

void
TransportJack::locate ( Transport &t, tick_t ticks )
{
  jack_nframes_t frame = trunc( ticks * t.frames_per_tick );

  MESSAGE( "JackTransport: Relocating transport to frame: %lu", frame);
  jack_transport_locate( client, frame );
}

void
TransportJack::set_beats_per_minute ( Transport &t, double n )
{
  t._master_beats_per_minute = n;
  _done = false;
}

void
TransportJack::set_beats_per_bar ( Transport &t, int n )
{
  t._master_beats_per_bar = n;
  _done = false;
}

void
TransportJack::set_beat_type ( Transport &t, int n )
{
  t._master_beat_type = n;
  _done = false;
}

/** callback for when we're Timebase Master, mostly taken from
 * transport.c in Jack's example clients. */
/* FIXME: there is a subtle interaction here between the tempo and
 * JACK's buffer size. Inflating ticks_per_beat (as jack_transport
 * does) diminishes the effect of this correlation, but does not
 * eliminate it... This is caused by the accumulation of a precision
 * error, and all timebase master routines I've examined appear to
 * suffer from this same tempo distortion (and all use the magic
 * number of 1920 ticks_per_beat in an attempt to reduce the magnitude
 * of the error. Currently, we keep this behaviour. */
void
TransportJack::timebase ( jack_transport_state_t, jack_nframes_t nframes, jack_position_t *pos, int new_pos)
{
    if ( new_pos || ! _done )
    {
        pos->valid = JackPositionBBT;
        pos->beats_per_bar = transport._master_beats_per_bar;
        pos->ticks_per_beat = 1920.0;                           /* magic number means what? */
        pos->beat_type = transport._master_beat_type;
        pos->beats_per_minute = transport._master_beats_per_minute;

        double wallclock = (double)pos->frame / (pos->frame_rate * 60);

        unsigned long abs_tick = wallclock * pos->beats_per_minute * pos->ticks_per_beat;
        unsigned long abs_beat = abs_tick / pos->ticks_per_beat;

        pos->bar = abs_beat / pos->beats_per_bar;
        pos->beat = abs_beat - (pos->bar * pos->beats_per_bar) + 1;
        pos->tick = abs_tick - (abs_beat * pos->ticks_per_beat);
        pos->bar_start_tick = pos->bar * pos->beats_per_bar * pos->ticks_per_beat;
        pos->bar++;

        _done = true;
    }
    else
    {
        pos->tick += nframes * pos->ticks_per_beat * pos->beats_per_minute / (pos->frame_rate * 60);

        while ( pos->tick >= pos->ticks_per_beat )
        {
            pos->tick -= pos->ticks_per_beat;

            if ( ++pos->beat > pos->beats_per_bar )
            {
                pos->beat = 1;

                ++pos->bar;

                pos->bar_start_tick += pos->beats_per_bar * pos->ticks_per_beat;
            }
        }
    }
}

static void
_jack_timebase_proxy( jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg )
{
  TransportJack *tj = (TransportJack *)arg;

  tj->timebase( state, nframes, pos, new_pos );
}

/* Jack's transport sync callback*/
static int
sync ( jack_transport_state_t state, jack_position_t *pos, void * )
{
    static bool seeking = false;

    switch ( state )
    {
        case JackTransportStopped:           /* new position requested */
            /* JACK docs lie. This is only called when the transport
               is *really* stopped, not when starting a slow-sync
               cycle */
            stop_all_patterns();
            return 1;
        case JackTransportStarting:          /* this means JACK is polling slow-sync clients */
        {
            stop_all_patterns();
            return 1;
        }
        case JackTransportRolling:           /* JACK's timeout has expired */
            /* FIXME: what's the right thing to do here? */
//            request_locate( pos->frame );
            return 1;
            break;
        default:
            WARNING( "unknown transport state" );
    }

    return 0;
}
