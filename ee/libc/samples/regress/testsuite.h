#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

/* a test function receives an optional argument, as given in add_test()
 * and must return string describing the error, or why the test failed.
 * Tests that complete successfully should return NULL.
 */
typedef const char *(* testfunc_t)(void *arg);

typedef struct test_t
{
	const char *name;
	testfunc_t func;
	void *arg;
} test_t;

typedef struct test_suite
{
	int ntests;
	test_t *tests;
} test_suite;

void init_testsuite(test_suite *p);
int add_test(test_suite *p, const char *name, testfunc_t func, void *arg);
int run_testsuite(test_suite *p);

#endif
