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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "global_settings.H"


GlobalSettings *GlobalSettings::_instance = 0;

GlobalSettings::GlobalSettings()
  :record_mode( MERGE ),
   transport_mode( JACK_TRANSPORT ),
   follow_playhead( true ),
   record_filtered( false ), //unused
   visual_metronome( false ) //unused
{
  asprintf( &user_config_dir, "%s/%s", getenv( "HOME" ), USER_CONFIG_DIR );
  mkdir( user_config_dir, 0777 );
}

GlobalSettings &
GlobalSettings::get()
{
  if ( _instance )
    return *_instance;
  else
    return *( _instance = new GlobalSettings );
}

