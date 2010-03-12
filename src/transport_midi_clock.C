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
/* jack */
#include <jack/jack.h>
#include <jack/midiport.h>

#include "transport_midi_clock.H"
#include "jack.H"

#define MIDI_CLOCK_PPQN 24

TransportMidiClock::TransportMidiClock( jack_client_t &jack_client )
  :client(&jack_client)
{
  midi_port = jack_port_register( client, "midi_clock_in",  JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 );
}

TransportMidiClock::~TransportMidiClock ( void )
{
  jack_port_unregister( client, midi_port );
}

void
TransportMidiClock::init ( Transport &t )
{
  /* FIXME We only base validity of the transport on the existence of
   * the jack midi port */
  t.valid = midi_port != NULL;

  t.rolling = false;
  t.master = true;
  t.beats_per_minute = t._master_beats_per_minute;
  t.beats_per_bar = t._master_beats_per_bar;
  t.beat_type = t._master_beat_type;
  t.ticks_per_beat = PPQN;
  t.signal_tempo_change( t.beats_per_minute );
  t.signal_bpb_change( t.beats_per_bar );
  t.signal_beat_change( t.beat_type );
  t.bar = 1;
  t.beat = 1;
  t.tick = 1;
}

void
TransportMidiClock::transport_update ( Transport &t, jack_nframes_t frames )
{
//  static midievent      e;
  jack_midi_event_t     ev;
  jack_nframes_t        count;
  void                  *raw_buffer;
  uint                  received_ticks = 0;

  raw_buffer = jack_port_get_buffer( midi_port, frames );
  count = jack_midi_get_event_count ( raw_buffer );

  for ( uint i = 0; i < count; i++ )
  {
    jack_midi_event_get ( &ev, raw_buffer, i);
    switch ( ev.buffer[0] )
    {
    case 0xfa:
      MESSAGE( "Midi Start" );
      t.rolling = true;
      // Midi start means (re)start from the beginning.
      t.bar = 1;
      t.beat = 1;
      t.tick = 1;
      break;
    case 0xfb:
      MESSAGE( "Midi Continue" );
      t.rolling = true;
      break;
    case 0xfc:
      MESSAGE( "Midi Stop" );
      t.rolling = false;
      break;
    case 0xf8:
      // Handle ticks
      ++received_ticks;
      break;
    default:
      break;
    }
  }

  if ( received_ticks && t.rolling )
  {
    //MESSAGE( "Received %lu ticks", received_ticks );

    // The received ticks in non-sequencer ppqn, beware of truncating if PPQN changes
    tick_t new_ticks = received_ticks * ( PPQN / MIDI_CLOCK_PPQN);

    // Updating position

    t.beat--; t.bar--;

    t.ticks = (t.bar * t.beats_per_bar + t.beat) * PPQN + t.tick;
    t.ticks_per_period = new_ticks;
    t.frames_per_tick = new_ticks / (double) frames;

    t.tick += new_ticks;
    t.beat += t.tick / PPQN;
    t.tick %= PPQN;
    t.bar += t.beat / t.beats_per_bar;
    t.beat %= t.beats_per_bar;

    t.beat++; t.bar++;

    // FIXME Update beat_per_minute
    // tick_per_period
    // frame_per_tick
  }
//     jack_position_t pos;

//     jack_transport_query( client, &pos );

//     t.valid = pos.valid & JackPositionBBT;

//     t.bar = pos.bar;
//     t.beat = pos.beat;
//     t.tick = pos.tick;

//     /* bars and beats start at 1.. */
//     pos.bar--;
//     pos.beat--;

//     /* FIXME: these probably shouldn't be called from the RT
//        thread... Anyway, it happens infrequently. */
//     if ( pos.beats_per_minute != t.beats_per_minute )
//         t.signal_tempo_change( pos.beats_per_minute );

//     if ( pos.beats_per_bar != t.beats_per_bar )
//         t.signal_bpb_change( pos.beats_per_bar );

//     if ( pos.beat_type != t.beat_type )
//         t.signal_beat_change( pos.beat_type );

//     t.ticks_per_beat = pos.ticks_per_beat;
//     t.beats_per_bar = pos.beats_per_bar;
//     t.beat_type = pos.beat_type;
//     t.beats_per_minute = pos.beats_per_minute;

//     t.frame = pos.frame;
//     t.frame_rate = pos.frame_rate;

//     /* FIXME: this only needs to be calculated if bpm or framerate changes  */
//     {
//         const double frames_per_beat = t.frame_rate * 60 / t.beats_per_minute;

//         t.frames_per_tick = frames_per_beat / (double)PPQN;
//         t.ticks_per_period = t.nframes / t.frames_per_tick;
//     }

//     tick_t abs_tick = (pos.bar * pos.beats_per_bar + pos.beat) * pos.ticks_per_beat + pos.tick;
// //    tick_t abs_tick = pos.bar_start_tick + (pos.beat * pos.ticks_per_beat) + pos.tick;

//     /* scale Jack's ticks to our ticks */

//     const double pulses_per_tick = PPQN / pos.ticks_per_beat;

//     t.ticks = abs_tick * pulses_per_tick;
//     t.tick = t.tick * pulses_per_tick;

//     t.ticks_per_beat = PPQN;
}

void
TransportMidiClock::start ( Transport &t )
{
}

void
TransportMidiClock::stop ( Transport &t )
{
}

void
TransportMidiClock::locate ( Transport &t, tick_t ticks )
{
}

void
TransportMidiClock::set_beats_per_minute ( Transport &t, double n )
{
  t._master_beats_per_minute = n;
}

void
TransportMidiClock::set_beats_per_bar ( Transport &t, int n )
{
  t._master_beats_per_bar = n;
}

void
TransportMidiClock::set_beat_type ( Transport &t, int n )
{
  t._master_beat_type = n;
}

