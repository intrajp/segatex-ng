#include <stdlib.h>
#include <stdio.h>

int main ()
{
    int bytes;
    char * text;
    printf ("How many bytes you want to book:");
    scanf ("%i", & bytes);
    text = (char *) malloc (bytes);
    /* Check if the operation was successful */
    if (text)
    {
        printf ("Reserved memory:% i bytes =% i =% i Kbytes Mbytes\n", bytes, bytes/1024, bytes / (1024 * 1024 )) ;
        printf ("The block starts at:%p\n", text);
        /* Now free the memory */
        free (text);
        text = NULL;
        //printf ("What will happen after freeing the memory blocks?:%p\n", text);
    }
    else
    printf ("Could not allocate memory\n") ;
    printf ("What will happen after freeing the memory blocks?:%p\n", text);
}
