/*
Small utility to excersise the HUD from the command line

Copyright 2011 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>
#include <gio/gunixinputstream.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <curses.h>

#include <hud-client.h>


static void print_suggestions(const char * query);
static void update(char *string);
void sighandler(int);

WINDOW *twindow = NULL;
int use_curses = 0;
static HudClientQuery * client_query = NULL;

int
main (int argc, char *argv[])
{
#ifndef GLIB_VERSION_2_36
	g_type_init ();
#endif

	int single_char;
	int pos = 0;
	
	char *line = (char*) malloc(256 * sizeof(char));

	signal(SIGINT, sighandler);

	// reading from pipe
	if (!isatty (STDIN_FILENO) ) {
		size_t a;
		int r;
		r = getline (&line, &a, stdin);
	
		if ( r == -1 ){
			perror("Error reading from pipe");
			exit(EXIT_FAILURE);	
		}

		// get rid of newline
		if( line[r-1] == '\n' )
			line[r-1] = '\0';	
	
		printf("\nsearch token: %s\n", line);
		print_suggestions( line );
	}
	// read command line parameter - hud-cli "search string"
	else if( argc > 1 ){
		printf("\nsearch token: %s\n", argv[1]);
		print_suggestions( argv[1] );
	}
	// interactive mode
     	else{
		use_curses = 1;
		
		twindow = initscr(); 		
		cbreak();
		keypad(stdscr, TRUE);	
		noecho();
		
		/* initialize the query screen */
		update( "" );

		/* interactive shell interface */
		while( 1 ){
		
			single_char = getch();		
			/* need to go left in the buffer */
			if ( single_char == KEY_BACKSPACE ){
				/* don't go too far left */
				if( pos > 0 ){
					pos--;
					line[pos] = '\0';
					update( line );
				}
				else
					; /* we are at the beginning of the buffer already */
			}
			/* ENTER will trigger the action for the first selected suggestion */
			else if ( single_char == '\n' ){

				/* FIXME: execute action on RETURN */
				break;
			}
			/* add character to the buffer and terminate string */
			else if ( single_char != KEY_RESIZE ) {
				if ( pos < 256 -1 ){ // -1 for \0
					line[pos] = single_char;
					line[pos+1] = '\0';
					pos++;
					update( line );
				}
				else {
					
				}
			}
			else{
				// nothing to do
			}
		}
		endwin();
	}
	
	free(line);

	g_clear_object(&client_query);

	return 0;
}

static void 
update( char *string ){
	
	werase(twindow);
	mvwprintw(twindow, 7, 10, "Search: %s", string);

	print_suggestions( string );
	
	// move cursor back to input line
	wmove( twindow, 7, 10 + 8 + strlen(string) );

	refresh();
}

static void
wait_for_sync_notify (GObject * object, GParamSpec * pspec, gpointer user_data)
{
	GMainLoop * loop = (GMainLoop *)user_data;
	g_main_loop_quit(loop);
	return;
}

static gboolean
wait_for_sync (DeeModel * model)
{
	if (dee_shared_model_is_synchronized(DEE_SHARED_MODEL(model))) {
		return TRUE;
	}

	GMainLoop * loop = g_main_loop_new(NULL, FALSE);

	glong sig = g_signal_connect(G_OBJECT(model), "notify::synchronized", G_CALLBACK(wait_for_sync_notify), loop);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_signal_handler_disconnect(G_OBJECT(model), sig);

	return dee_shared_model_is_synchronized(DEE_SHARED_MODEL(model));
}

static void 
print_suggestions (const char *query)
{
	if (client_query == NULL) {
		client_query = hud_client_query_new(query);
	} else {
		hud_client_query_set_query(client_query, query);
	}

	DeeModel * model = hud_client_query_get_results_model(client_query);
	g_return_if_fail(wait_for_sync(model));

	DeeModelIter * iter = NULL;
	int i = 0;
	for (iter = dee_model_get_first_iter(model); !dee_model_is_last(model, iter); iter = dee_model_next(model, iter), i++) {
		const gchar * suggestion = hud_client_query_results_get_command_name(client_query, iter);
		if( use_curses)
			mvwprintw(twindow, 9 + i, 15, "%s", suggestion);
		else{
			gchar * clean_line = NULL;
			pango_parse_markup(suggestion, -1, 0, NULL, &clean_line, NULL, NULL);
			printf("\t%s\n", clean_line);
			free(clean_line);
		}

	}

	return;
}

void sighandler(int signal){
	endwin();
	g_clear_object(&client_query);
	exit(EXIT_SUCCESS);
}
