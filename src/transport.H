
/*******************************************************************************/
/* Copyright (C) 2008 Jonathan Moore Liles                                     */
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

#pragma once

#include <sigc++/sigc++.h>

using namespace sigc;

#include "event.H" // just for tick_t

#include <jack/transport.h>

typedef double playhead_t;

class Transport;

/* We are using the strategy pattern to allow to change the transport
 * method dynamically */
class iTransportStrategy
{
public:
  virtual ~iTransportStrategy() {}

  /* This method is used to initialize jack callback */
  virtual void init ( Transport &t ) = 0;
  virtual void transport_update ( Transport &t ) = 0;
  virtual void start ( Transport &t ) = 0;
  virtual void stop ( Transport &t ) = 0;
  virtual void locate ( Transport &t, tick_t npos ) = 0;
  virtual void set_beats_per_minute ( Transport &t, double n ) = 0;
  virtual void set_beats_per_bar ( Transport &t, int n ) = 0;
  virtual void set_beat_type ( Transport &t, int n ) = 0;
};

class Transport : public trackable {

    iTransportStrategy *transport_method;

public:

    double _master_beats_per_minute;
    unsigned _master_beats_per_bar;
    unsigned _master_beat_type;

    signal <int, double> signal_tempo_change;
    signal <int, double> signal_bpb_change;
    signal <int, double> signal_beat_change;

    bool master;                                                /* are we driving the transport? */
    bool rolling;
    bool valid;
    volatile bool recording;

    unsigned long bar;
    unsigned beat;
    unsigned tick;

    playhead_t ticks;

    unsigned beats_per_bar;
    unsigned beat_type;
    double ticks_per_beat;
    double beats_per_minute;

    double ticks_per_period;
    double frames_per_tick;

    double frame_rate;

    double frame;
    double nframes;

    Transport ( void );

    void poll ( void );
    void start ( void );
    void stop ( void );
    void toggle ( void );
    void locate ( tick_t ticks );

    void set_beats_per_minute ( double n );
    void set_beats_per_bar ( int n );
    void set_beat_type ( int n );

    /* Changes the transport method */
    void set_transport_method( iTransportStrategy &tm );
};

extern Transport transport;
