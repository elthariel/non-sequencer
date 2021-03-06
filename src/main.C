/**********************************************************************************/
/* Copyright (C) 2007,2008 Jonathan Moore Liles                                   */
/*                                                                                */
/* This program is free software; you can redistribute it and/or modify it        */
/*  under the terms of the GNU General Public License as published by the         */
/*  Free Software Foundation; either version 2 of the License, or (at your        */
/*  option) any later version.                                                    */
/*                                                                                */
/*  This program is distributed in the hope that it will be useful, but WITHOUT   */
/*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or ;       */
/*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for     */
/*  more details.                                                                 */
/*                                                                                */
/*  You should have received a copy of the GNU General Public License along       */
/*  with This program; see the file COPYING.  If not,write to the Free Software ; */
/*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.    */
/**********************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "non.H"
// #include "gui/input.H"
#include "gui/ui.H"
#include "jack.H"
#include "lash.H"
#include "transport_jack.H"
#include "transport_midi_clock.H"

#include "pattern.H"
#include "phrase.H"

extern const char *BUILD_ID;
extern const char *VERSION;

Canvas *pattern_c, *phrase_c, *trigger_c;

sequence *playlist;

//global_settings config;
song_settings song;

Lash lash;

/* default to pattern mode */

UI *ui;

void
quit ( void )
{
    /* clean up, only for valgrind's sake */

    delete ui;

    delete pattern_c;
    delete phrase_c;
    delete trigger_c;

    midi_all_sound_off();

    // wait for it...
    sleep( 1 );

    midi_shutdown();

    MESSAGE( "Your fun is over" );

    exit( 0 );
}

void
init_song ( void )
{
    song.filename = NULL;

    pattern_c->grid( NULL );
    phrase_c->grid( NULL );

    playlist->reset();
    playlist->insert( 0, 1 );
    pattern_c->grid( new pattern );
    phrase_c->grid( new phrase );

    song.dirty( false );
}

void
handle_midi_input ( void )
{
    midievent *e;
    while ( ( e = midi_input_event( PERFORMANCE ) ) )
    {
        pattern::record_event( e );
        delete e;
    }
}

bool
load_song ( const char *name )
{
    MESSAGE( "loading song \"%s\"", name );

    Grid *pattern_grid = pattern_c->grid();
    Grid *phrase_grid = phrase_c->grid();

    pattern_c->grid( NULL );
    phrase_c->grid( NULL );

    if ( ! playlist->load( name ) )
    {
        WARNING( "failed to load song file" );
        goto failed;
    }

    pattern_c->grid( pattern::pattern_by_number( 1 ) );
    phrase_c->grid( phrase::phrase_by_number( 1 ) );

    song.filename = strdup( name );

    song.dirty( false );

    return true;

failed:

    pattern_c->grid( pattern_grid );
    phrase_c->grid( phrase_grid );

    return false;
}

bool
save_song ( const char *name )
{
    playlist->save( name );

    song.filename = strdup( name );
    song.dirty( false );

    return true;
}

int
main ( int argc, char **argv )
{
    song.play_mode = PATTERN;
    song.random.feel = 8;
    song.random.probability = 0.33;

    printf( "%s %s %s -- %s\n", APP_TITLE, VERSION, BUILD_ID, COPYRIGHT );

    playlist = new sequence;

    pattern_c = new Canvas;
    phrase_c = new Canvas;
    trigger_c = new Canvas;

    init_song();

    pattern::signal_create_destroy.connect( mem_fun( phrase_c,  &Canvas::v_zoom_fit ) );
    pattern::signal_create_destroy.connect( mem_fun( song, &song_settings::set_dirty ) );
    phrase::signal_create_destroy.connect( mem_fun( song, &song_settings::set_dirty ) );

    const char *jack_name;
    jack_client_t *client;

    jack_name = midi_init( &client );
    if ( ! jack_name )
        ASSERTION( "Could not initialize MIDI system! (is Jack running and with MIDI ports enabled?)" );

//    TransportJack my_transport( *client );
    // FIXME Implement run time switch of TransportMethod
    TransportMidiClock my_transport( *client );
    transport.set_transport_method( my_transport );

    if ( ! transport.valid )
    {
        if ( transport.master )
            ASSERTION( "The version of JACK you are using does not appear to be capable of passing BBT positional information." );
        else
            ASSERTION( "Either the version of JACK you are using does pass BBT information, or the current timebase master does not provide it." );
    }

    song.dirty( false );

    init_colors();

    ui = new UI;

    if ( ! lash.init( &argc, &argv, jack_name ) )
        WARNING( "error initializing LASH" );

    if ( argc > 1 )
    {
        /* maybe a filename on the commandline */
        if ( ! load_song( argv[ 1 ] ) )
            ASSERTION( "Could not load song \"%s\" specified on command line", argv[ 1 ] );
    }

    MESSAGE( "Initializing GUI" );

    ui->run();

    return 0;
}
