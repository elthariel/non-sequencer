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

#pragma once

#include "transport.H"

class TransportMidiClock : public iTransportStrategy
{
public:
  TransportMidiClock( jack_client_t &client );
  ~TransportMidiClock ( void );

  virtual void init ( Transport &t );
  virtual void transport_update ( Transport &t, jack_nframes_t frames );
  virtual void start ( Transport &t );
  virtual void stop ( Transport &t );
  virtual void locate ( Transport &t, tick_t npos );
  virtual void set_beats_per_minute ( Transport &t, double n );
  virtual void set_beats_per_bar ( Transport &t, int n );
  virtual void set_beat_type ( Transport &t, int n );

  void timebase ( jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos );

protected:
  jack_client_t         *client;
  jack_port_t           *midi_port;
};

