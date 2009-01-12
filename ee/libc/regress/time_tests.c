#include <stdio.h>
#include <time.h>

#include <testsuite.h>

static const char *test_clock(void *arg)
{
    clock_t c = clock();
    clock_t w = c + 5 * CLOCKS_PER_SEC;
    
    if (c == 0)
        return "clock not working";
        
    printf("\n(clock start: %d)", (int) c);
    printf("\nwaiting 5 secs");
    
    while ((c = clock()) < w)
        ;
        
    printf("\n(clock end: %d)", (int) c);
    
    return NULL;
}

int time_add_tests(test_suite *p)
{
    add_test(p, "clock", test_clock, NULL);
	return 0;
}
