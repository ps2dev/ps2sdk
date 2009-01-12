#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <testsuite.h>

static const char *test_clock(void *arg)
{
    clock_t c = clock();
    
    if (c == 0)
        return "clock not working";
        
    printf("\n(clock start: %d)", (int) c);
    printf("\nwaiting 5 secs");

    clock_t w = c + 5 * CLOCKS_PER_SEC;
    while (clock() < w)
        ;
        
    clock_t e = clock();
    int diff = (int) ((e - c) / CLOCKS_PER_SEC);
        
    printf("\n(clock end: %d)", (int) e);
    printf("\n(elapsed: %d sec)", diff);
    
    if (diff != 5)
        return "clock pause not working";
        
    return NULL;
}

static const char *test_sleep(void *arg)
{
    printf("\nwaiting 5 secs");
    
    clock_t c = clock();
    sleep(5);
    clock_t e = clock();
    
    int diff = (int) ((e - c) / CLOCKS_PER_SEC);
        
    printf("\n(elapsed: %d sec)", diff);
    
    if (diff != 5)
        return "sleep not working";
        
    return NULL;
}

int time_add_tests(test_suite *p)
{
    add_test(p, "clock", test_clock, NULL);
    add_test(p, "sleep", test_sleep, NULL);
	return 0;
}
