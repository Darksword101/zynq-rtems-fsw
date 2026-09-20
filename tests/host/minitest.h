#ifndef MINITEST_H
#define MINITEST_H

#include <stdio.h>
#include <stdlib.h>

static int mt_fail = 0, mt_pass = 0;

#define CHECK(c) do { if (c) ++mt_pass; else { ++mt_fail; printf("FAIL: %s:%d    %s\n", __FILE__, __LINE__, #c); } } while(0)
#define CHECK_EQ_U(a,b) CHECK((unsigned long)(a) == (unsigned long)(b))
#define MT_REPORT() do { printf("PASSED: %d, FAILED: %d\n", mt_pass, mt_fail); return mt_fail ? 1 : 0; } while(0)

#endif