
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
  :_master_beats_per_bar ( 4 ),
   _master_beat_type ( 4 ),
   _master_beats_per_minute ( 120 ),
   transport_method ( 0 )
{
}

void
Transport::poll ( void )
{
    transport_method->transport_update( *this );
}

void
Transport::start ( void )
{
    MESSAGE( "Starting transport" );
    transport_method->start(*this);
}

void
Transport::stop ( void )
{
    MESSAGE( "Stopping transport" );
    transport_method->stop(*this);
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
  MESSAGE( "Relocating transport to tick: %lu", ticks);
  transport_method->locate(*this, ticks);
}

void
Transport::set_beats_per_minute ( double n )
{
    transport_method->set_beats_per_minute( *this, n );
}

void
Transport::set_beats_per_bar ( int n )
{
    if ( n < 2 )
        return;

    transport_method->set_beats_per_bar( *this, n );
}

void
Transport::set_beat_type ( int n )
{
    if ( n < 4 )
        return;

    transport_method->set_beat_type( *this, n );
}

void
Transport::set_transport_method( iTransportStrategy &tm )
{
  transport_method = &tm;
  transport_method->init(*this);
}
