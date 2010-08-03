/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: libgen.c $
# libgen implementation
*/
#include <string.h>
#include <libgen.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

#ifdef F_basename
char *basename(char *path)
{
    static char base[PATH_MAX];
    char all_slashes = 1;
    char *start,*end;

    strcpy(base,".");

    if (path == NULL || *path == '\0')
        return base;

    // If no directory specified, return to :
    if ((end = strrchr(path, '/')) != NULL);
    else if ((end = strrchr(path, '\\')) != NULL);
    else if ((end = strrchr(path,':')) != NULL);
    else { strcpy(base,path); return base; }

    if (end == path)
    {
        if (*end != ':')
        {
            base[0] = *end;
            base[1] = '\0';
        }

        return base;
    }

    end++;

    // if we're not at the end of a path, 
    // then the path ended in a '/','\\', or ':'
    if (*end != '\0')
    {
        strcpy(base,end);
    }
    else
    {

        end--;

        // The path most likely ends in a '/'
        // so search for a previous '/','\\', or':'
        start = end;

        while(start != path)
        {
            start--;

            if (*start == '/')
            {
                if (!all_slashes)
                {
                    start++;
                    memcpy(base,start,end-start);
                    base[end-start] = '\0';
                    return base;
                }
                else
                {
                    end--;
                    continue;
                }
            }
            else if (*start == '\\')
            {
                if (!all_slashes)
                {
                    start++;
                    memcpy(base,start,end-start);
                    base[end-start] = '\0';
                    return base;
                }
                else
                {
                    end--;
                    continue;
                }
            }
            else if (*start == ':')
            {
                if (!all_slashes)
                {
                    start++;
                    memcpy(base,start,end-start);
                    base[end-start] = '\0';
                    return base;
                }
                else
                {
                    end--;
                    continue;
                }
            }
            else
            {
                all_slashes = 0;
            }
        }
    }

    return base;
}
#endif

#ifdef F_dirname
char *dirname(char *path)
{
    static char dir[PATH_MAX];
    char all_slashes = 1;
    char *end;

    strcpy(dir,".");

    if (path == NULL || *path == '\0')
        return dir;

    // If no device specifier, treat whole string as invalid
    //if ((end = strrchr(path,':')) == NULL) { return dir; }

    if ((end = strrchr(path, '/')) != NULL);
    else if ((end = strrchr(path, '\\')) != NULL);
    else if ((end = strrchr(path, ':')) != NULL);
    else return dir;    

    // Only a "device:" or "device:file" type path
    if ((*end == ':') && (end != path))
    {
        end++;
        memcpy(dir,path,end-path);
        dir[end-path] = '\0';
        return dir;
    }

    if ((*end == '/') || (*end == '\\'))
    {
        if (*(end-1) == ':')
        {
            // Only a "device:/file" or "device:/" type path
            end++;
            memcpy(dir,path,end-path);
            dir[end-path] = '\0';
            return dir;
        }
    }

    // Take care of paths that only have one '/' at the beginning
    if (end == path)
    {
        if (*end != ':')
        {
            dir[0] = *end;
            dir[1] = '\0';
        }

        return dir;
    }

    end++;

    if (*end == '\0')
    {

        while((end != path))
        {

            end--;

            if (*end == '/')
            {
                while (*end == '/') end--;
                end++;
                if (!all_slashes)
                {
                    if (end > path)
                    {
                        if (*(end-1) == ':') end++;
                    }
                    break;
                }
            }
            else if (*end == '\\')
            {
                while (*end == '\\') end--;
                end++;
                if (!all_slashes)
                {
                    if (end > path)
                    {
                        if (*(end-1) == ':') end++;
                    }
                    break;
                }
            }
            else if (*end == ':')
            {
                if (!all_slashes)
                {
                    end++;
                    break;
                }
            }
            else all_slashes = 0;

        }

    }
    else
    {
        end--;
        memcpy(dir,path,end-path);
        dir[end-path] = '\0';
        return dir;
    }

    if (all_slashes)
    {
        return dir;
    }

    if (end != path)
    {
    memcpy(dir,path,end-path);
    dir[end-path] = '\0';
    }

    return dir;
}
#endif
