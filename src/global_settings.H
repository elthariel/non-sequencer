/*
** global_settings.H
** Login : <elthariel@rincevent>
** Started on  Sun Feb 14 15:08:19 2010 elthariel
** $Id$
**
** Author(s):
**  - elthariel <elthariel@gmail.com>
**
** Copyright (C) 2010 elthariel
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
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

#pragma once

#include "non.H"

enum record_mode_e {
    MERGE,
    OVERWRITE,
    LAYER,
    NEW
};

enum transport_mode_e {
    JACK_TRANSPORT,
    FREEWHEEL,
    MIDI_CLOCK_SLAVE
};


/* A signleton to holds global settings, avoiding a global variable */
class GlobalSettings
{
public:
  static GlobalSettings &get();


  record_mode_e         record_mode;
  transport_mode_e      transport_mode;
  bool                  follow_playhead;
  char                  *user_config_dir;


  /* Unused */
  bool                  record_filtered;     /* ignore non-note events while recording */
  bool                  visual_metronome;    /* show visual metronome */

protected:
  GlobalSettings();
  GlobalSettings( const GlobalSettings& ) {}

  static GlobalSettings *_instance;
};
