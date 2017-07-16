#include <stdio.h>
#include "main.h"

int main(void)
{
    int i;

    i = is_selinux_enabled();

    if (i = 1)
        printf("SELinux is enabled.\n");
    else
        printf("SELinux is not enabled.\n");

    return 0;
}
