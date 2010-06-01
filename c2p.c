/* $Id: c2p.c,v 1.29 2010/05/31 02:20:12 ak Exp $ */
/*                                                           
**                                                            
**                                                            
**                      ###############                       
**                     #:::::::::::::::##                     
**                     #::::::######:::::#                    
**                     #######     #:::::#                    
**     ################            #:::::######   #########   
**   ##:::::::::::::::#            #:::::##::::###:::::::::#  
**  #:::::::::::::::::#         ####::::# #:::::::::::::::::# 
** #:::::::######:::::#    #####::::::##  ##::::::#####::::::#
** #::::::#     #######  ##::::::::###     #:::::#     #:::::#
** #:::::#              #:::::#####        #:::::#     #:::::#
** #:::::#             #:::::#             #:::::#     #:::::#
** #::::::#     ########:::::#             #:::::#    #::::::#
** #:::::::######:::::##:::::#       #######:::::#####:::::::#
**  #:::::::::::::::::##::::::#######:::::##::::::::::::::::# 
**   ##:::::::::::::::##::::::::::::::::::##::::::::::::::##  
**     #####################################::::::########    
**                                         #:::::#            
**                                         #:::::#            
**                                        #:::::::#           
**                                        #:::::::#           
**                                        #:::::::#           
**                                        #########           
**                                                            
** Copyright (c) 1997,2002,2003,2006-2010, azumakuniyuki All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
** 
** Redistributions of source code must retain the above copyright notice, this list
** of conditions and the following disclaimer.
** 
** Redistributions in binary form  must reproduce the above copyright notice,  this
** list of conditions and the following disclaimer in the documentation and/or oth-
** er materials provided with the distribution.
** 
** Neither the name of the ``azumakuniyuki''  nor the names of its contributors may
** be used to endorse or promote products  derived from this software  without spe-
** cific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED  BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,  THE IMPLIED
** WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR A  PARTICULAR PURPOSE  ARE DIS-
** CLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR  CONTRIBUTORS  BE LIABLE FOR
** ANY DIRECT, INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
** ( INCLUDING,  BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS  OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS;  OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON
** ANY THEORY OF LIABILITY,  WHETHER IN CONTRACT,  STRICT LIABILITY,  OR  TORT (IN-
** CLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFT-
** WARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#include "jp.h"
#include "en.h"

#define SHOW_CONTROLS	((unsigned int)(1<<0))	/* Show control characters */
#define REQ_WHITESPACE	((unsigned int)(1<<1))	/* Require whitespace */
#define LANG_JAPANESE	((unsigned int)(1<<8))	/* Default is Japanese */
#define LANG_ENGLISH	((unsigned int)(1<<9))	/* Default is English */

#define MAX_LENGTH	127
#define MAX_STDIN	127
#define C2P_OPTIONS	"Cd:hL:sv"

/* Default language */
#ifdef JA
#define DEFAULT_LANGUAGE	LANG_JAPANESE
#elif EN
#define DEFAULT_LANGUAGE	LANG_ENGLISH
#else
#define DEFAULT_LANGUAGE	LANG_ENGLISH
#endif

static const char *MY_PROGNAME[] = {"c2p"};	/* My name */
/* static const char *MY_DESCRIPTION[] = {"Convert from character to phonetic code"}; */
static const char *MY_VERSION[] = {"1.1.6"};	/* Version number */
static const char CHAR_DELIMITER_C = ',';	/* Delimiter(,) Comma */
static const char CHAR_DELIMITER_P = '|';	/* Delimiter(|) Pipe line */
static const char CHAR_DELIMITER_W = ' ';	/* Delimiter( ) whitespace */
static const char CHAR_DELIMITER_H = '-';	/* Delimiter(-) Hyphen */

extern const char *PHONETIC_JAPANESE[];
extern const char *PHONETIC_ENGLISH[];
static int OPERATION = 0;
static char DELIMITER;

void putHelp(void);
void putVersion(void);

int main( int argc, char *argv[] )
{
	int opt_result;				/* The result value of the getopt() */
	int num_of_args = argc - 1;		/* The Number Of Arguments(Exclude command itself and options) */
	int statuscode = 0;			/* return(); */
	int givenstring_length = 0;		/* Length of the string */
	short int print_count = 0;		/* Counter for the printing */
	short int check_count = 0;		/* Counter for the checking */
	bool read_from_stdin = 0;		/* If a string came from STDIN, turns on */
	char printablechar_count = 0;		/* Counter for printable characters */
	char multibytechar_count = 0;		/* Counter for multibyte characters */
	char controlchar_count = 0;		/* Counter for contorl characters */

	unsigned char given_characters[MAX_LENGTH] = "";	/* Characters in the argument */
	unsigned char chars_from_stdin[MAX_STDIN] = "";		/* Characters From STDIN */
	unsigned char **selected_language = NULL;		/* Selected language. */
	unsigned char select_char = '\0';
	unsigned char target_char = 0;

	DELIMITER = CHAR_DELIMITER_C;

	while(1) /* Parse options */
	{
		opt_result = getopt( argc, argv, C2P_OPTIONS );
		if( opt_result == -1 )
		{
			break;
		}

		switch( opt_result )
		{
			case 'C': /* Show control characters */
				OPERATION |= SHOW_CONTROLS;
				num_of_args--;
				break;

			case 'd': /* Select a delimiter */
				switch( *optarg )
				{
					case 'c':	/* Comma */
						DELIMITER = CHAR_DELIMITER_C;
						break;
					case 'h':	/* Hyphen */
						DELIMITER = CHAR_DELIMITER_H;
						break;
					case 'p':	/* Pipe line */
						DELIMITER = CHAR_DELIMITER_P;
						break;
					case 'w':	/* White space */
						DELIMITER = CHAR_DELIMITER_W;
						break;
					default:	/* Noting to do */
						break;
				}
				num_of_args--;
				break;

			case 'h': /* Help message */
				putHelp();
				exit(EXIT_SUCCESS);
				/* break; */

			case 'L': /* Language */
				switch( *optarg )
				{
					case 'e':	/* English */
						OPERATION |= LANG_ENGLISH;
						break;
					case 'j':	/* Japanese */
						OPERATION |= LANG_JAPANESE;
						break;
					default:
						break;
				}
				num_of_args--;
				break;

			case 's': /* Delimiter requires a white space after itself. */
				OPERATION |= REQ_WHITESPACE;
				num_of_args--;
				break;

			case 'v': /* Version number */
				putVersion();
				exit(EXIT_SUCCESS);
				/* break; */

			case '?':
			default:
				(void)fprintf( stderr, "try -h option\n" );
				/* break; */
				exit(EXIT_FAILURE);

		} /* end of switch() */

	} /* end of while */


	if( num_of_args < 1 )
	{
		/* Read from STDIN, if there is no string in the arguemnts. */
		char __character;
		short int __counter = 0;
		read_from_stdin = 1;

		while( ( __character = getchar() ) != EOF )
		{
			if( __counter < ( MAX_STDIN - 1 ) )
			{
				chars_from_stdin[__counter] = __character;
				__counter++;
			}
			else
			{
				break;
			}
		}

		chars_from_stdin[__counter] = '\0';
		givenstring_length = strlen( (char*)chars_from_stdin );
	}
	else
	{
		/* Check the length of the string */
		givenstring_length = strlen( argv[argc-1] );

		/* Check arguments */
		if( strcmp( argv[argc-1], argv[0] ) == 0 )
		{
			/* There is no argument */
			statuscode = 1;
			return(statuscode);
		}
	}

	if( givenstring_length > MAX_LENGTH )
	{
		(void)fprintf( stderr, "Too many characters(%d), max is %d\n", givenstring_length, MAX_LENGTH );
		statuscode = 1;
		return(statuscode);
	}
	else
	{
		/* Check selected language */
		if( ( OPERATION & LANG_JAPANESE ) == 0 && ( OPERATION & LANG_ENGLISH ) == 0 )
		{
			OPERATION |= DEFAULT_LANGUAGE;
		}

		/* Select a language */
		if( OPERATION & LANG_JAPANESE )
		{
			selected_language = (unsigned char **)PHONETIC_JAPANESE;
		}
		else if( OPERATION & LANG_ENGLISH )
		{
			selected_language = (unsigned char **)PHONETIC_ENGLISH;
		}


		/* Copy The String */
		if( read_from_stdin == 1 )
		{
#ifdef NOSTRL		/* If 'strlcpy' does not exist */
			strcpy( given_characters, chars_from_stdin );
#else			/* 'strlcpy' exists */
			strlcpy( (char*)given_characters, (char*)chars_from_stdin, sizeof(given_characters) );
#endif
		}
		else
		{
#ifdef NOSTRL		/* If 'strlcpy' does not exist */
			strcpy( given_characters, argv[argc-1]);
#else			/* 'strlcpy' exists */
			strlcpy( (char*)given_characters, (char*)argv[argc-1], sizeof(given_characters) );
#endif
		}


		/* Count the number of multibyte characters and control characters. */
		for( check_count = 0; check_count < givenstring_length; ++check_count )
		{
			if( isprint(given_characters[check_count]) )
			{
				++printablechar_count;
			}
			else if( iscntrl(given_characters[check_count]) )
			{
				++controlchar_count;
			}
			else if( given_characters[check_count] > 127 )
			{
				++multibytechar_count;
			}
		}

		for( print_count = 0; print_count < givenstring_length; ++print_count )
		{
			select_char = given_characters[print_count];

			/* No printable characters are left. */
			if( OPERATION & SHOW_CONTROLS )
			{
				if( ( print_count + multibytechar_count ) == givenstring_length )
				{
					break;
				}
			}
			else
			{
				if( ( print_count + controlchar_count + multibytechar_count ) == givenstring_length )
				{
					break;
				}
			}


			/* Check the ASCII code.  31 < x < 127 */
			if( isprint(select_char) )
			{
				if( select_char < 'A' )
				{
					/* Some symbols < 65 */
					target_char = select_char;
					printf( "%s", selected_language[target_char] );
				}
				else
				{
					/* Check whether or not a character is upper case. */
					if( isupper(select_char) )
					{
						/* Upper case */
						target_char = tolower(select_char) - 26;
						printf( "%s", selected_language[target_char] );
						printf( "%s", selected_language[0] );
					}
					else
					{
						/* Lower case, some symbols */
						target_char = select_char - 26; 
						printf( "%s", selected_language[target_char] );
					}
				}

				if( ! ( OPERATION & SHOW_CONTROLS ) )
				{
					/* Only non-printable characters are left in the string. */
					if( ( print_count + controlchar_count + multibytechar_count + 1 ) == givenstring_length )
					{
						printf("\n");
						break;
					}
				}
				else
				{
					/* Only multibyte characters are left in the string. */
					if( ( print_count + multibytechar_count + 1 ) == givenstring_length )
					{
						printf("\n");
						break;
					}
				}

				if( print_count + 1 < givenstring_length )
				{
					if( ( OPERATION & REQ_WHITESPACE ) && ( DELIMITER != CHAR_DELIMITER_W ) )
					{
						/* A delimiter character and a white space */
						printf( "%c%c", DELIMITER, ' ' );
					}
					else
					{
						printf( "%c", DELIMITER );
					}
				}
				else
				{
					/* This is the last character. */
					printf("\n");
					break;
				}
			}
			else if( iscntrl(select_char) )
			{
				/* Control character is included. */
				if( OPERATION & SHOW_CONTROLS )
				{
					if( select_char < ' ' )
					{
						/* Control Character's ASCII code is less than 0x20(Space) */
						target_char = select_char;
					}
					else
					{
						/* Control Character is 'Delete'(127) */
						target_char = select_char - 26;
					}
					printf( "%s", selected_language[target_char] );

					/* Only multibyte characters are left in the string. */
					if( ( print_count + multibytechar_count + 1 ) == givenstring_length )
					{
						printf("\n");
						break;
					}
				}
				else
				{
					/* Only non-Printable characters are left in the string. */
					if( ( print_count + controlchar_count + multibytechar_count + 1 ) == givenstring_length )
					{
						printf("\n");
						break;
					}
				}

				if( print_count + 1 < givenstring_length )
				{
					if( OPERATION & SHOW_CONTROLS )
					{
						if( ( OPERATION & REQ_WHITESPACE ) && ( DELIMITER != CHAR_DELIMITER_W ) )
						{
							/* A delimiter character and a white space */
							printf( "%c%c", DELIMITER, ' ' );
						}
						else
						{
							printf( "%c", DELIMITER );
						}
					}
					--controlchar_count;
					continue;
				}
				else
				{
					printf("\n");
					break;
				}
			}
			else if( select_char > 127 )
			{
				/* There is multibyte character. */
				if( print_count + 1 < givenstring_length )
				{
					--multibytechar_count;
					continue;
				}
				else
				{
					printf("\n");
					break;
				}
			}

		} /* End of the loop for() */

	}

	return( statuscode );

} /* end of main */

void putHelp(void)
{
	printf( "Usage: %s [Options] String\n", *MY_PROGNAME );
	puts("Options:");
	puts("");
	puts(" Delimiter:");
	puts("  -d<x>  : c = Comma [,]");
	puts("         : h = Hyphen [-]");
	puts("         : p = Pipeline [|]");
	puts("         : w = White space [ ]");
	puts("");
	puts(" Language:" );
	puts("  -L<x>  : e = English");
	puts("         : j = Japanese(UTF-8)");
	puts("");
	puts("  -s     : Writes a white space after each delimiter.");
	puts("  -C     : Show control characters too.");
	puts("  -h     : Shows help message(This screen).");
	puts("  -v     : Shows version number.");
}

void putVersion(void)
{
	printf( "%s\n", *MY_VERSION );
}

