/*
   cfg.c - functions for configuration information
   This file contains the contents of segatex-ng.

   Copyright (C) 2017 Shintaro Fujiwara

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA
*/

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <syslog.h>

#include "cfg.h"

struct segatex_ng_config *segatexd_cfg=NULL;

/* the maximum line length in the configuration file */
#define MAX_LINE_LENGTH          4096

/* the delimiters of tokens */
#define TOKEN_DELIM " \t\n\r"

/* set the configuration information to the defaults */
/* This function is only called from this file, so set static.*/

static void cfg_defaults(struct segatex_ng_config *cfg)
{
    //for debug
    //printf("cfg_defaults was called !\n");

    memset(cfg,0,sizeof(struct segatex_ng_config));
    cfg->sgx_threads=5;
}


/* check that the condition is true and otherwise log an error
   and bail out */

static inline void check_argumentcount(const char *filename, int lnr,
                                       const char *keyword, int condition)
{
    if (!condition)
    {
        printf("%s:%d: %s: wrong number of arguments",
                filename, lnr, keyword);
        //log_log(LOG_ERR, "%s:%d: %s: wrong number of arguments",
        //        filename, lnr, keyword);
        exit(EXIT_FAILURE);
    }
}

/* This function works like strtok() except that the original string is
   not modified and a pointer within str to where the next token begins
   is returned (this can be used to pass to the function on the next
   iteration). If no more tokens are found or the token will not fit in
   the buffer, NULL is returned. */
/* This function is only called from this file, so set static.*/

static char *get_token(char **line,char *buf,size_t buflen)
{
    //for debug
    //printf("get_token was called !\n");

    size_t len;
    if ((line==NULL)||(*line==NULL)||(**line=='\0')||(buf==NULL))
        return NULL;
    /* find the beginning and length of the token */
    *line+=strspn(*line,TOKEN_DELIM);
    len=strcspn(*line,TOKEN_DELIM);
    /* check if there is a token */
    if (len==0)
    {
        *line=NULL;
        return NULL;
    }
    /* limit the token length */
    if (len>=buflen)
        len=buflen-1;
    //for debug
    //printf("len is %d\n",len);
    /* copy the token */
    strncpy(buf,*line,len);
    buf[len]='\0';
    //for debug
    //printf("buf is %s\n",buf);
    /* skip to the next token */
    *line+=len;
    *line+=strspn(*line,TOKEN_DELIM);

    /* return the token */
    return buf;
}

/* This function is only called from this file, so set static.*/

static int get_int(const char *filename, int lnr,
                    const char *keyword, char **line)
{
    char token[32];
    //for debug
    //printf("get_init was called !\n");
    //printf("filename is %s\n", filename);
    //printf("lnr is %d\n", lnr);
    //printf("keyword is %s\n", keyword);
    //printf("line is %s\n", *line);
    check_argumentcount(filename, lnr, keyword,
                    get_token(line, token, sizeof(token)) != NULL);
    /* TODO: replace with correct numeric parse */
    return atoi(token);
}

/* This function is only called from this file, so set static.*/

static void get_eol(const char *filename,int lnr,
                    const char *keyword,char **line)
{
    //for debug
    //printf("get_eol was called !\n");

    if ((line!=NULL)&&(*line!=NULL)&&(**line!='\0'))
    {
        //log_log(LOG_ERR,"%s:%d: %s: too may arguments",filename,lnr,keyword);
        printf("%s:%d: %s: too many arguments\n",filename,lnr,keyword);
        exit(EXIT_FAILURE);
    }
}

/* This function is only called from this file, so set static.*/

static void cfg_read(const char *filename,struct segatex_ng_config *cfg)
{
    //for debug
    //printf("cfg_read was called !\n");

    FILE *fp;
    int lnr=0;
    char linebuf[MAX_LINE_LENGTH];
    char *line;
    char keyword[32];
    char token[64];
    int i;
    /* open config file */
    if ((fp=fopen(filename,"r"))==NULL)
    {
        //log_log(LOG_ERR,"cannot open config file (%s): %s",filename,strerror(errno));
        printf("cannot open config file %s\n",filename);
        syslog(LOG_INFO,"cannot open config file %s\n",filename);
        exit(EXIT_FAILURE);
    }
    /* read file and parse lines */
    while (fgets(linebuf,sizeof(linebuf),fp)!=NULL)
    {
        lnr++;
        line=linebuf;
        /* strip newline */
        i=(int)strlen(line);
        if ((i<=0)||(line[i-1]!='\n'))
        {
            //log_log(LOG_ERR,"%s:%d: line too long or last line missing newline",filename,lnr);
            printf("%s:%d:  line too long or last line missing newline\n",filename,lnr);
            exit(EXIT_FAILURE);
        }
        line[i-1]='\0';
        /* ignore comment lines */
        if (line[0]=='#')
            continue;
        /* strip trailing spaces */
        for (i--;(i>0)&&isspace(line[i-1]);i--)
            line[i-1]='\0';
        /* get keyword from line and ignore empty lines */
        if (get_token(&line, keyword, sizeof(keyword)) == NULL)
            continue;
        //for debug
        //printf("keyword is %s\n",keyword);
        /* runtime options */
        if (strcasecmp(keyword,"sgx_threads")==0)
        {
            cfg->sgx_threads = get_int(filename, lnr, keyword, &line);
            get_eol(filename, lnr, keyword, &line);
        }
        /* fallthrough */
        else
        {
            //log_log(LOG_ERR,"%s:%d: unknown keyword: '%s'",filename,lnr,keyword);
            printf("%s:%d: unknown keyword: '%s'\n",filename,lnr,keyword);
            exit(EXIT_FAILURE);
        }
    }
    //for debug
    printf("cfg->sgx_threads is %d\n",cfg->sgx_threads);
    /* we're done reading file, close */
    fclose(fp);
}

/* This function is called anywhere, so not set static.*/

void cfg_init(const char *fname)
{
    //for debug
    //printf("cfg_init was called !\n");
    /* check if we were called before */
    if (segatexd_cfg!=NULL)
    {
        //log_log(LOG_CRIT,"cfg_init() may only be called once");
        printf("cfg_init() may only be called once\n");
        exit(EXIT_FAILURE);
    }
    /* allocate the memory (this memory is not freed anywhere) */
    segatexd_cfg=(struct segatex_ng_config *)malloc(sizeof(struct segatex_ng_config));
    if (segatexd_cfg==NULL)
    {
        //log_log(LOG_CRIT,"malloc() failed to allocate memory");
        printf("malloc() failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    /* clear configuration */
    cfg_defaults(segatexd_cfg);
    /* read configfile */
    cfg_read(fname,segatexd_cfg);
}
