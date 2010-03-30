#ifndef __DRAW_TESTS_H__
#define __DRAW_TESTS_H__

#include <tamtypes.h>

// Alpha Testing
#define ATEST_METHOD_ALLFAIL		0
#define ATEST_METHOD_ALLPASS		1
#define ATEST_METHOD_LESS			2
#define ATEST_METHOD_LESS_EQUAL		3
#define ATEST_METHOD_EQUAL			4
#define ATEST_METHOD_GREATER_EQUAL	5
#define ATEST_METHOD_GREATER		6
#define ATEST_METHOD_NOTEQUAL		7

#define ATEST_KEEP_ALL				0
#define ATEST_KEEP_ZBUFFER			1
#define ATEST_KEEP_FRAMEBUFFER		2
#define ATEST_KEEP_ALPHA			3

// Destination Alpha Testing
#define DTEST_METHOD_PASS_ZERO		0
#define DTEST_METHOD_PASS_ONE		1

// Depth Test
#define ZTEST_METHOD_ALLFAIL		0
#define ZTEST_METHOD_ALLPASS		1
#define ZTEST_METHOD_GREATER_EQUAL	2
#define ZTEST_METHOD_GREATER		3

typedef struct {
	unsigned char enable;
	unsigned char method;
	unsigned char compval;
	unsigned char keep;
} ALPHATEST; 

typedef struct {
	unsigned char enable;
	unsigned char pass;
} DESTTEST;

typedef struct {
	unsigned char enable;
	unsigned char method;
} DEPTHTEST;
#ifdef __cplusplus
extern "C" {
#endif

	// Scissoring pixel test area
	QWORD *draw_scissor_area(QWORD *q, int context, int x0, int x1, int y0, int y1);

	// Pixel Testing
	QWORD *draw_pixel_test(QWORD *q, int context, ALPHATEST *atest, DESTTEST *dtest, DEPTHTEST *ztest);

	// Disable pixel testing defaults
	QWORD *draw_disable_tests(QWORD *q, int context, ZBUFFER *z);

	// Enable pixel testing defaults
	QWORD *draw_enable_tests(QWORD *q, int context, ZBUFFER *z);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_TESTS_H__*/
