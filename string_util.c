#include "string_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 64

char *read_line(FILE *in)
{
  // allocate initial space and null-terminate it
  char *line = malloc(sizeof(char) * (INITIAL_CAPACITY + 1));
  int len = 0;
  int capacity = line != NULL ? INITIAL_CAPACITY : 0;
  if (line != NULL)
    {
      line[0] = '\0';
    }

  // loop until end of line or end of file
  int ch;
  while ((ch = fgetc(in)) != EOF && ch != '\n')
    {
      // if we have a place to store the character, store it
      if (line != NULL)
	{
	  // check whether we need to resize
	  if (len == capacity)
	    {
	      char *bigger = malloc(sizeof(char) * (capacity * 2 + 1));
	      if (bigger != NULL)
		{
		  strcpy(bigger, line);
		  free(line);
		  line = bigger;
		  capacity *= 2;
		}
	      else
		{
		  // resize failed
		  free(line);
		  line = NULL;
		}
	    }

	  // add new character and move null character
	  if (line != NULL)
	    {
	      line[len++] = ch;
	      line[len] = '\0';
	    }
	}
    }

  // strip trailing carriage returns
  char *end = line + len - 1;
  while (end >= line && *end == '\r')
    {
      *(end--) = '\0';
    }

  if (ch == EOF && len == 0)
    {
      // nothing to read; return NULL
      free(line);
      line = NULL;
    }

  return line;
}
