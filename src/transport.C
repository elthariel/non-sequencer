
/*******************************************************************************/
/* Copyright (C) 2008 Jonathan Moore Liles                                     */
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

#include <jack/jack.h>

#include <stdlib.h>
#include <stdio.h>

#include "transport.H"
#include "jack.H"
#include "common.h"
#include "const.h"

/* FIXME: use JackSyncCallback instead? (sync-callback) */

Transport transport;

Transport::Transport ( void )
  :_master_beats_per_minute ( 120 ),
   _master_beats_per_bar ( 4 ),
   _master_beat_type ( 4 ),
   transport_method ( 0 )
{
}

void
Transport::poll ( jack_nframes_t frames )
{
  if ( transport_method )
    transport_method->transport_update( *this, frames );
}

void
Transport::start ( void )
{
  if ( transport_method )
  {
    MESSAGE( "Starting transport" );
    transport_method->start(*this);
  }
}

void
Transport::stop ( void )
{
  if ( transport_method )
  {
    MESSAGE( "Stopping transport" );
    transport_method->stop(*this);
  }
}

void
Transport::toggle ( void )
{
    if ( rolling )
        stop();
    else
        start();
}

void
Transport::locate ( tick_t ticks )
{
  if ( transport_method )
  {
    MESSAGE( "Relocating transport to tick: %lu", ticks);
    transport_method->locate(*this, ticks);
  }
}

void
Transport::set_beats_per_minute ( double n )
{
  if ( transport_method )
    transport_method->set_beats_per_minute( *this, n );
}

void
Transport::set_beats_per_bar ( int n )
{
    if ( n < 2 )
        return;

  if ( transport_method )
    transport_method->set_beats_per_bar( *this, n );
}

void
Transport::set_beat_type ( int n )
{
    if ( n < 4 )
        return;

  if ( transport_method )
    transport_method->set_beat_type( *this, n );
}

void
Transport::set_transport_method( iTransportStrategy &tm )
{
  transport_method = &tm;
  transport_method->init(*this);
}

iTransportStrategy *
Transport::get_transport_method ( void )
{
  return ( transport_method );
}
