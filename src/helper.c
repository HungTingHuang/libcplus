/******************************************************************
* @file: helper.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/
#include "common.h"
#include "cplus_helper.h"

#ifdef __CPLUS_UNITTEST__
#include "cplus_memmgr.h"

struct test1
{
    int32_t A;
    int32_t B;
    int32_t C;
    int32_t D;
    int32_t E;
} t1 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

struct test2
{
    int8_t A;
    int8_t B;
    int8_t C;
    int8_t D;
    int8_t E;
} t2 = { .A = 0x0a, .B = 0x0b, .C = 0x0c, .D = 0x0d, .E = 0x0e };

struct test3
{
    int16_t A;
    int16_t B;
    int16_t C;
    int16_t D;
    int16_t E;
} t3 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

struct test4
{
    int64_t A;
    int64_t B;
    int64_t C;
    int64_t D;
    int64_t E;
} t4 = { .A = 0x01234567, .B = 0x12345678, .C = 0x23456789, .D = 0x3456789A, .E = 0x456789AB };

struct test5
{
    int32_t A;
    int8_t B;
    int16_t C;
    int8_t D;
    int64_t E;
} t5 = { .A = 0x0123, .B = 0x12, .C = 0x2345, .D = 0x67, .E = 0x01234567 };

struct test6
{
    int8_t A;
    int16_t B;
    int8_t C;
    int64_t D;
    int32_t E;
} t6 = { .A = 0x13, .B = 0x2478, .C = 0x56, .D = 0x45670123, .E = 0x01234567 };

CPLUS_UNIT_TEST(CPLUS_OFFSETOF, functionity)
{
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t1.A) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t1.B) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t1.C) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t1.D) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t1.E) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, E));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t2.A) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t2.B) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t2.C) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t2.D) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t2.E) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, E));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t3.A) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t3.B) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t3.C) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t3.D) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t3.E) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, E));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t4.A) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t4.B) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t4.C) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t4.D) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t4.E) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, E));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t5.A) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t5.B) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t5.C) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t5.D) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t5.E) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, E));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t6.A) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, A));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t6.B) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, B));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t6.C) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, C));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t6.D) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, D));
    UNITTEST_EXPECT_EQ(true, ((intptr_t)(&t6.E) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, E));
}

CPLUS_UNIT_TEST(CPLUS_CONTAINER_OF, functionity)
{
    struct test1 * t1p = NULL;
    int32_t * t1pA = &(t1.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t1p = CPLUS_CONTAINER_OF(t1pA, struct test1, A)));
    UNITTEST_EXPECT_EQ(true, t1p->A == t1.A);
    UNITTEST_EXPECT_EQ(true, t1p->B == t1.B);
    UNITTEST_EXPECT_EQ(true, t1p->C == t1.C);
    UNITTEST_EXPECT_EQ(true, t1p->D == t1.D);
    UNITTEST_EXPECT_EQ(true, t1p->E == t1.E);
    int32_t * t1pB = &(t1.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t1p = CPLUS_CONTAINER_OF(t1pB, struct test1, B)));
    UNITTEST_EXPECT_EQ(true, t1p->A == t1.A);
    UNITTEST_EXPECT_EQ(true, t1p->B == t1.B);
    UNITTEST_EXPECT_EQ(true, t1p->C == t1.C);
    UNITTEST_EXPECT_EQ(true, t1p->D == t1.D);
    UNITTEST_EXPECT_EQ(true, t1p->E == t1.E);
    int32_t * t1pC = &(t1.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t1p = CPLUS_CONTAINER_OF(t1pC, struct test1, C)));
    UNITTEST_EXPECT_EQ(true, t1p->A == t1.A);
    UNITTEST_EXPECT_EQ(true, t1p->B == t1.B);
    UNITTEST_EXPECT_EQ(true, t1p->C == t1.C);
    UNITTEST_EXPECT_EQ(true, t1p->D == t1.D);
    UNITTEST_EXPECT_EQ(true, t1p->E == t1.E);
    int32_t * t1pD = &(t1.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t1p = CPLUS_CONTAINER_OF(t1pD, struct test1, D)));
    UNITTEST_EXPECT_EQ(true, t1p->A == t1.A);
    UNITTEST_EXPECT_EQ(true, t1p->B == t1.B);
    UNITTEST_EXPECT_EQ(true, t1p->C == t1.C);
    UNITTEST_EXPECT_EQ(true, t1p->D == t1.D);
    UNITTEST_EXPECT_EQ(true, t1p->E == t1.E);
    int32_t * t1pE = &(t1.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t1p = CPLUS_CONTAINER_OF(t1pE, struct test1, E)));
    UNITTEST_EXPECT_EQ(true, t1p->A == t1.A);
    UNITTEST_EXPECT_EQ(true, t1p->B == t1.B);
    UNITTEST_EXPECT_EQ(true, t1p->C == t1.C);
    UNITTEST_EXPECT_EQ(true, t1p->D == t1.D);
    UNITTEST_EXPECT_EQ(true, t1p->E == t1.E);

    struct test2 * t2p = NULL;
    int8_t * t2pA = &(t2.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t2p = CPLUS_CONTAINER_OF(t2pA, struct test2, A)));
    UNITTEST_EXPECT_EQ(true, t2p->A == t2.A);
    UNITTEST_EXPECT_EQ(true, t2p->B == t2.B);
    UNITTEST_EXPECT_EQ(true, t2p->C == t2.C);
    UNITTEST_EXPECT_EQ(true, t2p->D == t2.D);
    UNITTEST_EXPECT_EQ(true, t2p->E == t2.E);
    int8_t * t2pB = &(t2.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t2p = CPLUS_CONTAINER_OF(t2pB, struct test2, B)));
    UNITTEST_EXPECT_EQ(true, t2p->A == t2.A);
    UNITTEST_EXPECT_EQ(true, t2p->B == t2.B);
    UNITTEST_EXPECT_EQ(true, t2p->C == t2.C);
    UNITTEST_EXPECT_EQ(true, t2p->D == t2.D);
    UNITTEST_EXPECT_EQ(true, t2p->E == t2.E);
    int8_t * t2pC = &(t2.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t2p = CPLUS_CONTAINER_OF(t2pC, struct test2, C)));
    UNITTEST_EXPECT_EQ(true, t2p->A == t2.A);
    UNITTEST_EXPECT_EQ(true, t2p->B == t2.B);
    UNITTEST_EXPECT_EQ(true, t2p->C == t2.C);
    UNITTEST_EXPECT_EQ(true, t2p->D == t2.D);
    UNITTEST_EXPECT_EQ(true, t2p->E == t2.E);
    int8_t * t2pD = &(t2.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t2p = CPLUS_CONTAINER_OF(t2pD, struct test2, D)));
    UNITTEST_EXPECT_EQ(true, t2p->A == t2.A);
    UNITTEST_EXPECT_EQ(true, t2p->B == t2.B);
    UNITTEST_EXPECT_EQ(true, t2p->C == t2.C);
    UNITTEST_EXPECT_EQ(true, t2p->D == t2.D);
    UNITTEST_EXPECT_EQ(true, t2p->E == t2.E);
    int8_t * t2pE = &(t2.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t2p = CPLUS_CONTAINER_OF(t2pE, struct test2, E)));
    UNITTEST_EXPECT_EQ(true, t2p->A == t2.A);
    UNITTEST_EXPECT_EQ(true, t2p->B == t2.B);
    UNITTEST_EXPECT_EQ(true, t2p->C == t2.C);
    UNITTEST_EXPECT_EQ(true, t2p->D == t2.D);
    UNITTEST_EXPECT_EQ(true, t2p->E == t2.E);

    struct test3 * t3p = NULL;
    int16_t * t3pA = &(t3.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t3p = CPLUS_CONTAINER_OF(t3pA, struct test3, A)));
    UNITTEST_EXPECT_EQ(true, t3p->A == t3.A);
    UNITTEST_EXPECT_EQ(true, t3p->B == t3.B);
    UNITTEST_EXPECT_EQ(true, t3p->C == t3.C);
    UNITTEST_EXPECT_EQ(true, t3p->D == t3.D);
    UNITTEST_EXPECT_EQ(true, t3p->E == t3.E);
    int16_t * t3pB = &(t3.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t3p = CPLUS_CONTAINER_OF(t3pB, struct test3, B)));
    UNITTEST_EXPECT_EQ(true, t3p->A == t3.A);
    UNITTEST_EXPECT_EQ(true, t3p->B == t3.B);
    UNITTEST_EXPECT_EQ(true, t3p->C == t3.C);
    UNITTEST_EXPECT_EQ(true, t3p->D == t3.D);
    UNITTEST_EXPECT_EQ(true, t3p->E == t3.E);
    int16_t * t3pC = &(t3.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t3p = CPLUS_CONTAINER_OF(t3pC, struct test3, C)));
    UNITTEST_EXPECT_EQ(true, t3p->A == t3.A);
    UNITTEST_EXPECT_EQ(true, t3p->B == t3.B);
    UNITTEST_EXPECT_EQ(true, t3p->C == t3.C);
    UNITTEST_EXPECT_EQ(true, t3p->D == t3.D);
    UNITTEST_EXPECT_EQ(true, t3p->E == t3.E);
    int16_t * t3pD = &(t3.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t3p = CPLUS_CONTAINER_OF(t3pD, struct test3, D)));
    UNITTEST_EXPECT_EQ(true, t3p->A == t3.A);
    UNITTEST_EXPECT_EQ(true, t3p->B == t3.B);
    UNITTEST_EXPECT_EQ(true, t3p->C == t3.C);
    UNITTEST_EXPECT_EQ(true, t3p->D == t3.D);
    UNITTEST_EXPECT_EQ(true, t3p->E == t3.E);
    int16_t * t3pE = &(t3.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t3p = CPLUS_CONTAINER_OF(t3pE, struct test3, E)));
    UNITTEST_EXPECT_EQ(true, t3p->A == t3.A);
    UNITTEST_EXPECT_EQ(true, t3p->B == t3.B);
    UNITTEST_EXPECT_EQ(true, t3p->C == t3.C);
    UNITTEST_EXPECT_EQ(true, t3p->D == t3.D);
    UNITTEST_EXPECT_EQ(true, t3p->E == t3.E);

    struct test4 * t4p = NULL;
    int64_t * t4pA = &(t4.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t4p = CPLUS_CONTAINER_OF(t4pA, struct test4, A)));
    UNITTEST_EXPECT_EQ(true, t4p->A == t4.A);
    UNITTEST_EXPECT_EQ(true, t4p->B == t4.B);
    UNITTEST_EXPECT_EQ(true, t4p->C == t4.C);
    UNITTEST_EXPECT_EQ(true, t4p->D == t4.D);
    UNITTEST_EXPECT_EQ(true, t4p->E == t4.E);
    int64_t * t4pB = &(t4.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t4p = CPLUS_CONTAINER_OF(t4pB, struct test4, B)));
    UNITTEST_EXPECT_EQ(true, t4p->A == t4.A);
    UNITTEST_EXPECT_EQ(true, t4p->B == t4.B);
    UNITTEST_EXPECT_EQ(true, t4p->C == t4.C);
    UNITTEST_EXPECT_EQ(true, t4p->D == t4.D);
    UNITTEST_EXPECT_EQ(true, t4p->E == t4.E);
    int64_t * t4pC = &(t4.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t4p = CPLUS_CONTAINER_OF(t4pC, struct test4, C)));
    UNITTEST_EXPECT_EQ(true, t4p->A == t4.A);
    UNITTEST_EXPECT_EQ(true, t4p->B == t4.B);
    UNITTEST_EXPECT_EQ(true, t4p->C == t4.C);
    UNITTEST_EXPECT_EQ(true, t4p->D == t4.D);
    UNITTEST_EXPECT_EQ(true, t4p->E == t4.E);
    int64_t * t4pD = &(t4.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t4p = CPLUS_CONTAINER_OF(t4pD, struct test4, D)));
    UNITTEST_EXPECT_EQ(true, t4p->A == t4.A);
    UNITTEST_EXPECT_EQ(true, t4p->B == t4.B);
    UNITTEST_EXPECT_EQ(true, t4p->C == t4.C);
    UNITTEST_EXPECT_EQ(true, t4p->D == t4.D);
    UNITTEST_EXPECT_EQ(true, t4p->E == t4.E);
    int64_t * t4pE = &(t4.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t4p = CPLUS_CONTAINER_OF(t4pE, struct test4, E)));
    UNITTEST_EXPECT_EQ(true, t4p->A == t4.A);
    UNITTEST_EXPECT_EQ(true, t4p->B == t4.B);
    UNITTEST_EXPECT_EQ(true, t4p->C == t4.C);
    UNITTEST_EXPECT_EQ(true, t4p->D == t4.D);
    UNITTEST_EXPECT_EQ(true, t4p->E == t4.E);

    struct test5 * t5p = NULL;
    int32_t * t5pA = &(t5.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t5p = CPLUS_CONTAINER_OF(t5pA, struct test5, A)));
    UNITTEST_EXPECT_EQ(true, t5p->A == t5.A);
    UNITTEST_EXPECT_EQ(true, t5p->B == t5.B);
    UNITTEST_EXPECT_EQ(true, t5p->C == t5.C);
    UNITTEST_EXPECT_EQ(true, t5p->D == t5.D);
    UNITTEST_EXPECT_EQ(true, t5p->E == t5.E);
    int8_t * t5pB = &(t5.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t5p = CPLUS_CONTAINER_OF(t5pB, struct test5, B)));
    UNITTEST_EXPECT_EQ(true, t5p->A == t5.A);
    UNITTEST_EXPECT_EQ(true, t5p->B == t5.B);
    UNITTEST_EXPECT_EQ(true, t5p->C == t5.C);
    UNITTEST_EXPECT_EQ(true, t5p->D == t5.D);
    UNITTEST_EXPECT_EQ(true, t5p->E == t5.E);
    int16_t * t5pC = &(t5.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t5p = CPLUS_CONTAINER_OF(t5pC, struct test5, C)));
    UNITTEST_EXPECT_EQ(true, t5p->A == t5.A);
    UNITTEST_EXPECT_EQ(true, t5p->B == t5.B);
    UNITTEST_EXPECT_EQ(true, t5p->C == t5.C);
    UNITTEST_EXPECT_EQ(true, t5p->D == t5.D);
    UNITTEST_EXPECT_EQ(true, t5p->E == t5.E);
    int8_t * t5pD = &(t5.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t5p = CPLUS_CONTAINER_OF(t5pD, struct test5, D)));
    UNITTEST_EXPECT_EQ(true, t5p->A == t5.A);
    UNITTEST_EXPECT_EQ(true, t5p->B == t5.B);
    UNITTEST_EXPECT_EQ(true, t5p->C == t5.C);
    UNITTEST_EXPECT_EQ(true, t5p->D == t5.D);
    UNITTEST_EXPECT_EQ(true, t5p->E == t5.E);
    int64_t * t5pE = &(t5.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t5p = CPLUS_CONTAINER_OF(t5pE, struct test5, E)));
    UNITTEST_EXPECT_EQ(true, t5p->A == t5.A);
    UNITTEST_EXPECT_EQ(true, t5p->B == t5.B);
    UNITTEST_EXPECT_EQ(true, t5p->C == t5.C);
    UNITTEST_EXPECT_EQ(true, t5p->D == t5.D);
    UNITTEST_EXPECT_EQ(true, t5p->E == t5.E);

    struct test6 * t6p = NULL;
    int8_t * t6pA = &(t6.A);
    UNITTEST_EXPECT_EQ(true, NULL != (t6p = CPLUS_CONTAINER_OF(t6pA, struct test6, A)));
    UNITTEST_EXPECT_EQ(true, t6p->A == t6.A);
    UNITTEST_EXPECT_EQ(true, t6p->B == t6.B);
    UNITTEST_EXPECT_EQ(true, t6p->C == t6.C);
    UNITTEST_EXPECT_EQ(true, t6p->D == t6.D);
    UNITTEST_EXPECT_EQ(true, t6p->E == t6.E);
    int16_t * t6pB = &(t6.B);
    UNITTEST_EXPECT_EQ(true, NULL != (t6p = CPLUS_CONTAINER_OF(t6pB, struct test6, B)));
    UNITTEST_EXPECT_EQ(true, t6p->A == t6.A);
    UNITTEST_EXPECT_EQ(true, t6p->B == t6.B);
    UNITTEST_EXPECT_EQ(true, t6p->C == t6.C);
    UNITTEST_EXPECT_EQ(true, t6p->D == t6.D);
    UNITTEST_EXPECT_EQ(true, t6p->E == t6.E);
    int8_t * t6pC = &(t6.C);
    UNITTEST_EXPECT_EQ(true, NULL != (t6p = CPLUS_CONTAINER_OF(t6pC, struct test6, C)));
    UNITTEST_EXPECT_EQ(true, t6p->A == t6.A);
    UNITTEST_EXPECT_EQ(true, t6p->B == t6.B);
    UNITTEST_EXPECT_EQ(true, t6p->C == t6.C);
    UNITTEST_EXPECT_EQ(true, t6p->D == t6.D);
    UNITTEST_EXPECT_EQ(true, t6p->E == t6.E);
    int64_t * t6pD = &(t6.D);
    UNITTEST_EXPECT_EQ(true, NULL != (t6p = CPLUS_CONTAINER_OF(t6pD, struct test6, D)));
    UNITTEST_EXPECT_EQ(true, t6p->A == t6.A);
    UNITTEST_EXPECT_EQ(true, t6p->B == t6.B);
    UNITTEST_EXPECT_EQ(true, t6p->C == t6.C);
    UNITTEST_EXPECT_EQ(true, t6p->D == t6.D);
    UNITTEST_EXPECT_EQ(true, t6p->E == t6.E);
    int32_t * t6pE = &(t6.E);
    UNITTEST_EXPECT_EQ(true, NULL != (t6p = CPLUS_CONTAINER_OF(t6pE, struct test6, E)));
    UNITTEST_EXPECT_EQ(true, t6p->A == t6.A);
    UNITTEST_EXPECT_EQ(true, t6p->B == t6.B);
    UNITTEST_EXPECT_EQ(true, t6p->C == t6.C);
    UNITTEST_EXPECT_EQ(true, t6p->D == t6.D);
    UNITTEST_EXPECT_EQ(true, t6p->E == t6.E);
}

CPLUS_UNIT_TEST(CPLUS_INITIALIZE_STRUCT_POINTER, functionity)
{
    CPLUS_INITIALIZE_STRUCT_POINTER(&t1);
    UNITTEST_EXPECT_EQ(true, 0 == t1.A);
    UNITTEST_EXPECT_EQ(true, 0 == t1.B);
    UNITTEST_EXPECT_EQ(true, 0 == t1.C);
    UNITTEST_EXPECT_EQ(true, 0 == t1.D);
    UNITTEST_EXPECT_EQ(true, 0 == t1.E);

    CPLUS_INITIALIZE_STRUCT_POINTER(&t2);
    UNITTEST_EXPECT_EQ(true, 0 == t2.A);
    UNITTEST_EXPECT_EQ(true, 0 == t2.B);
    UNITTEST_EXPECT_EQ(true, 0 == t2.C);
    UNITTEST_EXPECT_EQ(true, 0 == t2.D);
    UNITTEST_EXPECT_EQ(true, 0 == t2.E);

    CPLUS_INITIALIZE_STRUCT_POINTER(&t3);
    UNITTEST_EXPECT_EQ(true, 0 == t3.A);
    UNITTEST_EXPECT_EQ(true, 0 == t3.B);
    UNITTEST_EXPECT_EQ(true, 0 == t3.C);
    UNITTEST_EXPECT_EQ(true, 0 == t3.D);
    UNITTEST_EXPECT_EQ(true, 0 == t3.E);

    CPLUS_INITIALIZE_STRUCT_POINTER(&t4);
    UNITTEST_EXPECT_EQ(true, 0 == t4.A);
    UNITTEST_EXPECT_EQ(true, 0 == t4.B);
    UNITTEST_EXPECT_EQ(true, 0 == t4.C);
    UNITTEST_EXPECT_EQ(true, 0 == t4.D);
    UNITTEST_EXPECT_EQ(true, 0 == t4.E);

    CPLUS_INITIALIZE_STRUCT_POINTER(&t5);
    UNITTEST_EXPECT_EQ(true, 0 == t5.A);
    UNITTEST_EXPECT_EQ(true, 0 == t5.B);
    UNITTEST_EXPECT_EQ(true, 0 == t5.C);
    UNITTEST_EXPECT_EQ(true, 0 == t5.D);
    UNITTEST_EXPECT_EQ(true, 0 == t5.E);

    CPLUS_INITIALIZE_STRUCT_POINTER(&t6);
    UNITTEST_EXPECT_EQ(true, 0 == t6.A);
    UNITTEST_EXPECT_EQ(true, 0 == t6.B);
    UNITTEST_EXPECT_EQ(true, 0 == t6.C);
    UNITTEST_EXPECT_EQ(true, 0 == t6.D);
    UNITTEST_EXPECT_EQ(true, 0 == t6.E);
}

void unittest_helper(void)
{
    UNITTEST_ADD_TESTCASE(CPLUS_OFFSETOF, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_CONTAINER_OF, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_INITIALIZE_STRUCT_POINTER, functionity);
}

#endif // __CPLUS_UNITTEST__