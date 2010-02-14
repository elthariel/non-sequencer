/*
** global_settings.C
** Login : <elthariel@rincevent>
** Started on  Sun Feb 14 15:07:58 2010 elthariel
** $Id$
**
** Author(s):
**  - elthariel <elthariel@gmail.com>
**
** Copyright (C) 2010 elthariel
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "global_settings.H"


GlobalSettings *GlobalSettings::_instance = 0;

GlobalSettings::GlobalSettings()
  :follow_playhead( true ),
   record_mode( MERGE ),
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

