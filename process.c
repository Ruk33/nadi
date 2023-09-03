#include <stdio.h> // FILE, popen, fgets, pclose

int process_run(char *dest, 
                unsigned long long n,
                char *to_run)
{
    if (!to_run)
        return 0;
    FILE *fp = popen(to_run, "r");
    if (!fp)
        return 0;
    if (dest)
        fgets(dest, n, fp);
    pclose(fp);
    return 1;
}
