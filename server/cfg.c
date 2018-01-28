/*
 *  cfg.c - functions for configuration information
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2014 Arthur de Jong
 *  Copyright (C) 2017-2018 Shintaro Fujiwara
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA
 *
 *  Has borrowed some from libaudit.c. These guys will be happy to be mentioned here.
 *    Authors of libaudit.c 
 *      Steve Grubb <sgrubb@redhat.com>
 *      Rickard E. (Rik) Faith <faith@redhat.com>
*/

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <syslog.h>
#include <math.h>
#include <sys/stat.h>
#include "../segatexd.h"
#include "cfg.h"

/*
 *  initializing object member contents
 *  this could be overrided by another function
*/
/*
struct segatex_ng_config segatexd_cfg_raw = 
    {
        10, // number of threads
    };
*/

//struct segatex_ng_config *segatexd_cfg = &segatexd_cfg_raw;

/* should be set to NULL, this will be checked in cfg_init() */
struct segatex_ng_config *segatexd_cfg = NULL;

const char msg_cfg_read [ ] ="cfg_read was called by SIG_VALUE !\n";
const char msg_cfg_read_ok [ ] ="file value was reloaded by SIG_VALUE !\n";

/* the maximum line length in the configuration file */
#define MAX_LINE_LENGTH    4096

/* the delimiters of tokens */
#define TOKEN_DELIM " \t\n\r"

/* set the configuration information to the defaults after memory is allocated*/

void cfg_defaults ( struct segatex_ng_config *cfg )
{
    memset ( cfg, 0, sizeof ( struct segatex_ng_config ) );
    if ( cfg == NULL )
    {
        printf("cfg.c: cfg_defaults() failed to memset");
        exit(EXIT_FAILURE);
    }
    //for debug
    printf("cfg_defaults was called !\n");

    cfg->threads = 5;
}

/* check that the condition is true and otherwise log an error
   and bail out */
/* This function is only called from this file, so set static.*/

static inline void check_argumentcount ( const char *filename, int lnr,
                                       const char *keyword, int condition )
{
    if ( !condition )
    {
        segatex_msg ( LOG_ERR, "%s:%d: %s: wrong number of arguments",
                filename, lnr, keyword );
        exit ( EXIT_FAILURE );
    }
}

/* This function works like strtok() except that the original string is
 *  not modified and a pointer within str to where the next token begins
 *  is returned (this can be used to pass to the function on the next
 *  iteration). If no more tokens are found or the token will not fit in
 *  the buffer, NULL is returned. 
*/
/* This function is only called from this file, so set static.*/

static char *get_token ( char **line, char *buf, size_t buflen )
{
    //for debug
    //printf("get_token was called !\n");

    size_t len;

    if ( ( line == NULL ) || ( *line == NULL ) || ( **line == '\0' ) || ( buf == NULL ) )
        return NULL;

    /* find the beginning and length of the token */

    *line += strspn ( *line, TOKEN_DELIM );
    len = strcspn ( *line, TOKEN_DELIM );

    /* check if there is a token */

    if ( len == 0 )
    {
        *line = NULL ;

        return NULL;
    }

    /* limit the token length */

    if ( len >= buflen )
        len = buflen - 1;

    //for debug
    //printf("len is %d\n",len);
    
    /* copy the token */
    strncpy ( buf, *line, len );
    buf [ len ] = '\0';

    //for debug
    //printf("buf is %s\n",buf);
    
    /* skip to the next token */
    *line += len;
    *line += strspn ( *line, TOKEN_DELIM );

    /* return the token */

    return buf;
}

/* This function is only called from this file, so set static.*/

static int get_int ( const char *filename, int lnr,
                    const char *keyword, char **line )
{
    char token [ 32 ];
    //for debug
    //printf("get_init was called !\n");
    //printf("filename is %s\n", filename);
    //printf("lnr is %d\n", lnr);
    //printf("keyword is %s\n", keyword);
    //printf("line is %s\n", *line);
    
    check_argumentcount ( filename, lnr, keyword,
                    get_token ( line, token, sizeof ( token ) ) != NULL );

    /* TODO: replace with correct numeric parse */

    return atoi ( token );
}

/* This function is only called from this file, so set static.*/

static void get_eol ( const char *filename, int lnr,
                    const char *keyword, char **line )
{
    //for debug
    //printf("get_eol was called !\n");

    if ( ( line != NULL ) && ( *line != NULL ) && ( **line != '\0' ) )
    {
        segatex_msg ( LOG_ERR, "%s:%d: %s: too may arguments", filename, lnr, keyword );
        exit ( EXIT_FAILURE );
    }
}

/* read configuration file of segatexd */

void cfg_read ( const char *filename, struct segatex_ng_config *cfg )
{
    char line_break [ ]= "\n" ;
    char struct_str [ MAX_LINE_LENGTH ] = "";
    char struct_pre [ 17 ] = "cfg->threads is ";
    char threads_str [ 24 ];
    char filename_str [ MAX_LINE_LENGTH ];
    char filename_pre [ 14 ] = "file name is ";

    strncpy ( filename_str, filename_pre, 16 );
    strcat ( filename_str, filename );
    strcat ( filename_str, line_break );

    /*if signal was caught, printf is unsafe, so change it to write*/
    if ( SIG_VALUE == 2 )
    {
        write ( STDOUT_FILENO, msg_cfg_read, sizeof ( msg_cfg_read ) - 1 );
    }
    else
    {
        printf("cfg_read was called !\n");
    }

    FILE *fp;
    int fd, rc, lnr = 0;
    struct stat st;
    char linebuf [ MAX_LINE_LENGTH ];
    char *line;
    char keyword [ 32 ];
    //char token [ 64 ];
    int i;

    /* open the file */
    rc = open ( filename, O_NOFOLLOW | O_RDONLY );
    if ( rc < 0 ) {
        if ( errno != ENOENT ) {
            segatex_msg ( LOG_ERR, "Error opening %s (%s)",
                filename, strerror ( errno ) );
            exit ( EXIT_FAILURE );
        }
        segatex_msg ( LOG_WARNING,
            "Config file %s doesn't exist", filename );
        exit ( EXIT_FAILURE );
    }

    fd = rc;

    /* check the file's permissions: owned by root, not world writable,
     * not symlink.
     */
    segatex_msg ( LOG_DEBUG, "Config file %s opened for parsing", filename );

    if ( fstat ( fd, &st ) < 0) {
        segatex_msg ( LOG_ERR, "Error fstat'ing %s (%s)",
            filename, strerror ( errno ) );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    if ( st.st_uid != 0 ) {
        segatex_msg ( LOG_ERR, "Error - %s isn't owned by root", filename );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    if ( ( st.st_mode & S_IWOTH ) == S_IWOTH ) {
        segatex_msg ( LOG_ERR, "Error - %s is world writable", filename );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    /* using macro to check file*/
    if ( !S_ISREG ( st.st_mode ) ) {
        segatex_msg ( LOG_ERR, "Error - %s is not a regular file", filename );
        close ( fd );
        exit ( EXIT_FAILURE );
    }

    /* open config file */
    if ( ( fp=fopen ( filename,"r" ) ) == NULL )
    {
        segatex_msg ( LOG_ERR, "cannot open config file (%s): %s", filename, strerror ( errno ) );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    /* read file and parse lines */
    while ( fgets ( linebuf, sizeof ( linebuf ), fp ) != NULL )
    {
        lnr++;
        line = linebuf;
        /* strip newline */
        i = ( int ) strlen ( line );
        if ( ( i <= 0 ) || ( line [ i - 1 ] != '\n' ) )
        {
            segatex_msg ( LOG_ERR, "%s:%d: line too long or last line missing newline", filename, lnr );
            close ( fd );
            exit ( EXIT_FAILURE );
        }
        line [ i - 1 ] = '\0';
        /* ignore comment lines */
        if ( line [ 0 ] == '#' )
            continue;
        /* strip trailing spaces */
        for ( i--; ( i > 0 ) && isspace ( line [ i - 1 ] ); i-- )
            line [ i - 1 ] = '\0';
        /* get keyword from line and ignore empty lines */
        if ( get_token ( &line, keyword, sizeof ( keyword ) ) == NULL )
            continue;
        //for debug
        //printf("keyword is %s\n",keyword);
        /* runtime options */
        if ( strcasecmp ( keyword, "threads" ) == 0 )
        {
            cfg->threads = get_int ( filename, lnr, keyword, &line );
            get_eol ( filename, lnr, keyword, &line );
        }
        /* fallthrough */
        else
        {
            segatex_msg ( LOG_ERR, "%s:%d: unknown keyword: '%s'", filename, lnr, keyword );
            close ( fd );
            exit ( EXIT_FAILURE );
        }
    }
    //for debug
    /*setting int to char string*/
    sprintf ( threads_str, "%d", cfg->threads );
    strcpy ( struct_str, struct_pre );
    strcat ( struct_str, threads_str );
    strcat ( struct_str, line_break );

    /*if signal was caught, printf is unsafe, so change it to write*/
    //show file contents
    if ( SIG_VALUE == 2 )
    {
        write ( STDOUT_FILENO, msg_cfg_read_ok, sizeof ( msg_cfg_read_ok ) );
        write ( STDOUT_FILENO, struct_str, sizeof ( struct_str ) - 1 );
    }
    else
    {
        printf ( "cfg->threads is %d\n", cfg->threads );
    }
    /* we're done reading file, close */
    fclose ( fp );
}

/* This function is called anywhere, so not set static.*/

void cfg_init ( const char *fname )
{
    //for debug
    //printf("cfg_init was called !\n");
    /* check if we were called before */
    if ( segatexd_cfg != NULL )
    {
        segatex_msg ( LOG_CRIT, "cfg_init() may only be called once" );
        exit ( EXIT_FAILURE );
    }
    /* allocate the memory (this memory is not freed anywhere, it's ok, outside this socpe, it's NULL) */
    segatexd_cfg = (struct segatex_ng_config * ) malloc ( sizeof ( struct segatex_ng_config ) );
    //for debug
    //free(segatexd_cfg);
    if ( segatexd_cfg == NULL )
    {
        segatex_msg ( LOG_CRIT, "malloc() failed to allocate memory" );
        exit ( EXIT_FAILURE );
    }

    /* clear configuration */
    cfg_defaults ( segatexd_cfg );
    //for debug
    //free(segatexd_cfg);
    /* read configfile */
    cfg_read ( fname, segatexd_cfg );
    printf("segatexd_cfg:%p\n",segatexd_cfg);
}

/*
 * after outside cfg_init(), segatexd_cfg should be null, so, no need this function? 
*/

void cfg_clear ( )
{
    printf("freeing the config pointer, cause 've already read stuff -- segatexd_cfg:%p\n",segatexd_cfg);
    free ( segatexd_cfg );
    segatexd_cfg = NULL;
    printf("should be NULL -- segatexd_cfg:%p\n",segatexd_cfg);
}
