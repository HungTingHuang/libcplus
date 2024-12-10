#include "internal.h"

static struct test1
{
	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;
} t1 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

static struct test2
{
	int8_t A;
	int8_t B;
	int8_t C;
	int8_t D;
	int8_t E;
} t2 = { .A = 0x0a, .B = 0x0b, .C = 0x0c, .D = 0x0d, .E = 0x0e };

static struct test3
{
	int16_t A;
	int16_t B;
	int16_t C;
	int16_t D;
	int16_t E;
} t3 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

static struct test4
{
	int64_t A;
	int64_t B;
	int64_t C;
	int64_t D;
	int64_t E;
} t4 = { .A = 0x01234567, .B = 0x12345678, .C = 0x23456789, .D = 0x3456789A, .E = 0x456789AB };

static struct test5
{
	int32_t A;
	int8_t B;
	int16_t C;
	int8_t D;
	int64_t E;
} t5 = { .A = 0x0123, .B = 0x12, .C = 0x2345, .D = 0x67, .E = 0x01234567 };

static struct test6
{
	int8_t A;
	int16_t B;
	int8_t C;
	int64_t D;
	int32_t E;
} t6 = { .A = 0x13, .B = 0x2478, .C = 0x56, .D = 0x45670123, .E = 0x01234567 };

TEST(CPLUS_OFFSETOF, functionity)
{
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t1.A) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t1.B) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t1.C) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t1.D) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t1.E) - (intptr_t)(&t1)) == CPLUS_OFFSETOF(struct test1, E));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t2.A) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t2.B) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t2.C) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t2.D) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t2.E) - (intptr_t)(&t2)) == CPLUS_OFFSETOF(struct test2, E));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t3.A) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t3.B) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t3.C) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t3.D) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t3.E) - (intptr_t)(&t3)) == CPLUS_OFFSETOF(struct test3, E));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t4.A) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t4.B) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t4.C) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t4.D) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t4.E) - (intptr_t)(&t4)) == CPLUS_OFFSETOF(struct test4, E));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t5.A) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t5.B) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t5.C) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t5.D) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t5.E) - (intptr_t)(&t5)) == CPLUS_OFFSETOF(struct test5, E));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t6.A) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, A));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t6.B) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, B));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t6.C) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, C));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t6.D) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, D));
	EXPECT_EQ(CPLUS_TRUE, ((intptr_t)(&t6.E) - (intptr_t)(&t6)) == CPLUS_OFFSETOF(struct test6, E));
}

TEST(CPLUS_CONTAINER_OF, functionity)
{
	struct test1 * t1p = CPLUS_NULL;
	int32_t * t1pA = &(t1.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t1p = CPLUS_CONTAINER_OF(t1pA, struct test1, A)));
	EXPECT_EQ(CPLUS_TRUE, t1p->A == t1.A);
	EXPECT_EQ(CPLUS_TRUE, t1p->B == t1.B);
	EXPECT_EQ(CPLUS_TRUE, t1p->C == t1.C);
	EXPECT_EQ(CPLUS_TRUE, t1p->D == t1.D);
	EXPECT_EQ(CPLUS_TRUE, t1p->E == t1.E);
	int32_t * t1pB = &(t1.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t1p = CPLUS_CONTAINER_OF(t1pB, struct test1, B)));
	EXPECT_EQ(CPLUS_TRUE, t1p->A == t1.A);
	EXPECT_EQ(CPLUS_TRUE, t1p->B == t1.B);
	EXPECT_EQ(CPLUS_TRUE, t1p->C == t1.C);
	EXPECT_EQ(CPLUS_TRUE, t1p->D == t1.D);
	EXPECT_EQ(CPLUS_TRUE, t1p->E == t1.E);
	int32_t * t1pC = &(t1.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t1p = CPLUS_CONTAINER_OF(t1pC, struct test1, C)));
	EXPECT_EQ(CPLUS_TRUE, t1p->A == t1.A);
	EXPECT_EQ(CPLUS_TRUE, t1p->B == t1.B);
	EXPECT_EQ(CPLUS_TRUE, t1p->C == t1.C);
	EXPECT_EQ(CPLUS_TRUE, t1p->D == t1.D);
	EXPECT_EQ(CPLUS_TRUE, t1p->E == t1.E);
	int32_t * t1pD = &(t1.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t1p = CPLUS_CONTAINER_OF(t1pD, struct test1, D)));
	EXPECT_EQ(CPLUS_TRUE, t1p->A == t1.A);
	EXPECT_EQ(CPLUS_TRUE, t1p->B == t1.B);
	EXPECT_EQ(CPLUS_TRUE, t1p->C == t1.C);
	EXPECT_EQ(CPLUS_TRUE, t1p->D == t1.D);
	EXPECT_EQ(CPLUS_TRUE, t1p->E == t1.E);
	int32_t * t1pE = &(t1.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t1p = CPLUS_CONTAINER_OF(t1pE, struct test1, E)));
	EXPECT_EQ(CPLUS_TRUE, t1p->A == t1.A);
	EXPECT_EQ(CPLUS_TRUE, t1p->B == t1.B);
	EXPECT_EQ(CPLUS_TRUE, t1p->C == t1.C);
	EXPECT_EQ(CPLUS_TRUE, t1p->D == t1.D);
	EXPECT_EQ(CPLUS_TRUE, t1p->E == t1.E);

	struct test2 * t2p = CPLUS_NULL;
	int8_t * t2pA = &(t2.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t2p = CPLUS_CONTAINER_OF(t2pA, struct test2, A)));
	EXPECT_EQ(CPLUS_TRUE, t2p->A == t2.A);
	EXPECT_EQ(CPLUS_TRUE, t2p->B == t2.B);
	EXPECT_EQ(CPLUS_TRUE, t2p->C == t2.C);
	EXPECT_EQ(CPLUS_TRUE, t2p->D == t2.D);
	EXPECT_EQ(CPLUS_TRUE, t2p->E == t2.E);
	int8_t * t2pB = &(t2.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t2p = CPLUS_CONTAINER_OF(t2pB, struct test2, B)));
	EXPECT_EQ(CPLUS_TRUE, t2p->A == t2.A);
	EXPECT_EQ(CPLUS_TRUE, t2p->B == t2.B);
	EXPECT_EQ(CPLUS_TRUE, t2p->C == t2.C);
	EXPECT_EQ(CPLUS_TRUE, t2p->D == t2.D);
	EXPECT_EQ(CPLUS_TRUE, t2p->E == t2.E);
	int8_t * t2pC = &(t2.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t2p = CPLUS_CONTAINER_OF(t2pC, struct test2, C)));
	EXPECT_EQ(CPLUS_TRUE, t2p->A == t2.A);
	EXPECT_EQ(CPLUS_TRUE, t2p->B == t2.B);
	EXPECT_EQ(CPLUS_TRUE, t2p->C == t2.C);
	EXPECT_EQ(CPLUS_TRUE, t2p->D == t2.D);
	EXPECT_EQ(CPLUS_TRUE, t2p->E == t2.E);
	int8_t * t2pD = &(t2.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t2p = CPLUS_CONTAINER_OF(t2pD, struct test2, D)));
	EXPECT_EQ(CPLUS_TRUE, t2p->A == t2.A);
	EXPECT_EQ(CPLUS_TRUE, t2p->B == t2.B);
	EXPECT_EQ(CPLUS_TRUE, t2p->C == t2.C);
	EXPECT_EQ(CPLUS_TRUE, t2p->D == t2.D);
	EXPECT_EQ(CPLUS_TRUE, t2p->E == t2.E);
	int8_t * t2pE = &(t2.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t2p = CPLUS_CONTAINER_OF(t2pE, struct test2, E)));
	EXPECT_EQ(CPLUS_TRUE, t2p->A == t2.A);
	EXPECT_EQ(CPLUS_TRUE, t2p->B == t2.B);
	EXPECT_EQ(CPLUS_TRUE, t2p->C == t2.C);
	EXPECT_EQ(CPLUS_TRUE, t2p->D == t2.D);
	EXPECT_EQ(CPLUS_TRUE, t2p->E == t2.E);

	struct test3 * t3p = CPLUS_NULL;
	int16_t * t3pA = &(t3.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t3p = CPLUS_CONTAINER_OF(t3pA, struct test3, A)));
	EXPECT_EQ(CPLUS_TRUE, t3p->A == t3.A);
	EXPECT_EQ(CPLUS_TRUE, t3p->B == t3.B);
	EXPECT_EQ(CPLUS_TRUE, t3p->C == t3.C);
	EXPECT_EQ(CPLUS_TRUE, t3p->D == t3.D);
	EXPECT_EQ(CPLUS_TRUE, t3p->E == t3.E);
	int16_t * t3pB = &(t3.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t3p = CPLUS_CONTAINER_OF(t3pB, struct test3, B)));
	EXPECT_EQ(CPLUS_TRUE, t3p->A == t3.A);
	EXPECT_EQ(CPLUS_TRUE, t3p->B == t3.B);
	EXPECT_EQ(CPLUS_TRUE, t3p->C == t3.C);
	EXPECT_EQ(CPLUS_TRUE, t3p->D == t3.D);
	EXPECT_EQ(CPLUS_TRUE, t3p->E == t3.E);
	int16_t * t3pC = &(t3.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t3p = CPLUS_CONTAINER_OF(t3pC, struct test3, C)));
	EXPECT_EQ(CPLUS_TRUE, t3p->A == t3.A);
	EXPECT_EQ(CPLUS_TRUE, t3p->B == t3.B);
	EXPECT_EQ(CPLUS_TRUE, t3p->C == t3.C);
	EXPECT_EQ(CPLUS_TRUE, t3p->D == t3.D);
	EXPECT_EQ(CPLUS_TRUE, t3p->E == t3.E);
	int16_t * t3pD = &(t3.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t3p = CPLUS_CONTAINER_OF(t3pD, struct test3, D)));
	EXPECT_EQ(CPLUS_TRUE, t3p->A == t3.A);
	EXPECT_EQ(CPLUS_TRUE, t3p->B == t3.B);
	EXPECT_EQ(CPLUS_TRUE, t3p->C == t3.C);
	EXPECT_EQ(CPLUS_TRUE, t3p->D == t3.D);
	EXPECT_EQ(CPLUS_TRUE, t3p->E == t3.E);
	int16_t * t3pE = &(t3.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t3p = CPLUS_CONTAINER_OF(t3pE, struct test3, E)));
	EXPECT_EQ(CPLUS_TRUE, t3p->A == t3.A);
	EXPECT_EQ(CPLUS_TRUE, t3p->B == t3.B);
	EXPECT_EQ(CPLUS_TRUE, t3p->C == t3.C);
	EXPECT_EQ(CPLUS_TRUE, t3p->D == t3.D);
	EXPECT_EQ(CPLUS_TRUE, t3p->E == t3.E);

	struct test4 * t4p = CPLUS_NULL;
	int64_t * t4pA = &(t4.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t4p = CPLUS_CONTAINER_OF(t4pA, struct test4, A)));
	EXPECT_EQ(CPLUS_TRUE, t4p->A == t4.A);
	EXPECT_EQ(CPLUS_TRUE, t4p->B == t4.B);
	EXPECT_EQ(CPLUS_TRUE, t4p->C == t4.C);
	EXPECT_EQ(CPLUS_TRUE, t4p->D == t4.D);
	EXPECT_EQ(CPLUS_TRUE, t4p->E == t4.E);
	int64_t * t4pB = &(t4.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t4p = CPLUS_CONTAINER_OF(t4pB, struct test4, B)));
	EXPECT_EQ(CPLUS_TRUE, t4p->A == t4.A);
	EXPECT_EQ(CPLUS_TRUE, t4p->B == t4.B);
	EXPECT_EQ(CPLUS_TRUE, t4p->C == t4.C);
	EXPECT_EQ(CPLUS_TRUE, t4p->D == t4.D);
	EXPECT_EQ(CPLUS_TRUE, t4p->E == t4.E);
	int64_t * t4pC = &(t4.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t4p = CPLUS_CONTAINER_OF(t4pC, struct test4, C)));
	EXPECT_EQ(CPLUS_TRUE, t4p->A == t4.A);
	EXPECT_EQ(CPLUS_TRUE, t4p->B == t4.B);
	EXPECT_EQ(CPLUS_TRUE, t4p->C == t4.C);
	EXPECT_EQ(CPLUS_TRUE, t4p->D == t4.D);
	EXPECT_EQ(CPLUS_TRUE, t4p->E == t4.E);
	int64_t * t4pD = &(t4.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t4p = CPLUS_CONTAINER_OF(t4pD, struct test4, D)));
	EXPECT_EQ(CPLUS_TRUE, t4p->A == t4.A);
	EXPECT_EQ(CPLUS_TRUE, t4p->B == t4.B);
	EXPECT_EQ(CPLUS_TRUE, t4p->C == t4.C);
	EXPECT_EQ(CPLUS_TRUE, t4p->D == t4.D);
	EXPECT_EQ(CPLUS_TRUE, t4p->E == t4.E);
	int64_t * t4pE = &(t4.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t4p = CPLUS_CONTAINER_OF(t4pE, struct test4, E)));
	EXPECT_EQ(CPLUS_TRUE, t4p->A == t4.A);
	EXPECT_EQ(CPLUS_TRUE, t4p->B == t4.B);
	EXPECT_EQ(CPLUS_TRUE, t4p->C == t4.C);
	EXPECT_EQ(CPLUS_TRUE, t4p->D == t4.D);
	EXPECT_EQ(CPLUS_TRUE, t4p->E == t4.E);

	struct test5 * t5p = CPLUS_NULL;
	int32_t * t5pA = &(t5.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t5p = CPLUS_CONTAINER_OF(t5pA, struct test5, A)));
	EXPECT_EQ(CPLUS_TRUE, t5p->A == t5.A);
	EXPECT_EQ(CPLUS_TRUE, t5p->B == t5.B);
	EXPECT_EQ(CPLUS_TRUE, t5p->C == t5.C);
	EXPECT_EQ(CPLUS_TRUE, t5p->D == t5.D);
	EXPECT_EQ(CPLUS_TRUE, t5p->E == t5.E);
	int8_t * t5pB = &(t5.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t5p = CPLUS_CONTAINER_OF(t5pB, struct test5, B)));
	EXPECT_EQ(CPLUS_TRUE, t5p->A == t5.A);
	EXPECT_EQ(CPLUS_TRUE, t5p->B == t5.B);
	EXPECT_EQ(CPLUS_TRUE, t5p->C == t5.C);
	EXPECT_EQ(CPLUS_TRUE, t5p->D == t5.D);
	EXPECT_EQ(CPLUS_TRUE, t5p->E == t5.E);
	int16_t * t5pC = &(t5.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t5p = CPLUS_CONTAINER_OF(t5pC, struct test5, C)));
	EXPECT_EQ(CPLUS_TRUE, t5p->A == t5.A);
	EXPECT_EQ(CPLUS_TRUE, t5p->B == t5.B);
	EXPECT_EQ(CPLUS_TRUE, t5p->C == t5.C);
	EXPECT_EQ(CPLUS_TRUE, t5p->D == t5.D);
	EXPECT_EQ(CPLUS_TRUE, t5p->E == t5.E);
	int8_t * t5pD = &(t5.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t5p = CPLUS_CONTAINER_OF(t5pD, struct test5, D)));
	EXPECT_EQ(CPLUS_TRUE, t5p->A == t5.A);
	EXPECT_EQ(CPLUS_TRUE, t5p->B == t5.B);
	EXPECT_EQ(CPLUS_TRUE, t5p->C == t5.C);
	EXPECT_EQ(CPLUS_TRUE, t5p->D == t5.D);
	EXPECT_EQ(CPLUS_TRUE, t5p->E == t5.E);
	int64_t * t5pE = &(t5.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t5p = CPLUS_CONTAINER_OF(t5pE, struct test5, E)));
	EXPECT_EQ(CPLUS_TRUE, t5p->A == t5.A);
	EXPECT_EQ(CPLUS_TRUE, t5p->B == t5.B);
	EXPECT_EQ(CPLUS_TRUE, t5p->C == t5.C);
	EXPECT_EQ(CPLUS_TRUE, t5p->D == t5.D);
	EXPECT_EQ(CPLUS_TRUE, t5p->E == t5.E);

	struct test6 * t6p = CPLUS_NULL;
	int8_t * t6pA = &(t6.A);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t6p = CPLUS_CONTAINER_OF(t6pA, struct test6, A)));
	EXPECT_EQ(CPLUS_TRUE, t6p->A == t6.A);
	EXPECT_EQ(CPLUS_TRUE, t6p->B == t6.B);
	EXPECT_EQ(CPLUS_TRUE, t6p->C == t6.C);
	EXPECT_EQ(CPLUS_TRUE, t6p->D == t6.D);
	EXPECT_EQ(CPLUS_TRUE, t6p->E == t6.E);
	int16_t * t6pB = &(t6.B);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t6p = CPLUS_CONTAINER_OF(t6pB, struct test6, B)));
	EXPECT_EQ(CPLUS_TRUE, t6p->A == t6.A);
	EXPECT_EQ(CPLUS_TRUE, t6p->B == t6.B);
	EXPECT_EQ(CPLUS_TRUE, t6p->C == t6.C);
	EXPECT_EQ(CPLUS_TRUE, t6p->D == t6.D);
	EXPECT_EQ(CPLUS_TRUE, t6p->E == t6.E);
	int8_t * t6pC = &(t6.C);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t6p = CPLUS_CONTAINER_OF(t6pC, struct test6, C)));
	EXPECT_EQ(CPLUS_TRUE, t6p->A == t6.A);
	EXPECT_EQ(CPLUS_TRUE, t6p->B == t6.B);
	EXPECT_EQ(CPLUS_TRUE, t6p->C == t6.C);
	EXPECT_EQ(CPLUS_TRUE, t6p->D == t6.D);
	EXPECT_EQ(CPLUS_TRUE, t6p->E == t6.E);
	int64_t * t6pD = &(t6.D);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t6p = CPLUS_CONTAINER_OF(t6pD, struct test6, D)));
	EXPECT_EQ(CPLUS_TRUE, t6p->A == t6.A);
	EXPECT_EQ(CPLUS_TRUE, t6p->B == t6.B);
	EXPECT_EQ(CPLUS_TRUE, t6p->C == t6.C);
	EXPECT_EQ(CPLUS_TRUE, t6p->D == t6.D);
	EXPECT_EQ(CPLUS_TRUE, t6p->E == t6.E);
	int32_t * t6pE = &(t6.E);
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (t6p = CPLUS_CONTAINER_OF(t6pE, struct test6, E)));
	EXPECT_EQ(CPLUS_TRUE, t6p->A == t6.A);
	EXPECT_EQ(CPLUS_TRUE, t6p->B == t6.B);
	EXPECT_EQ(CPLUS_TRUE, t6p->C == t6.C);
	EXPECT_EQ(CPLUS_TRUE, t6p->D == t6.D);
	EXPECT_EQ(CPLUS_TRUE, t6p->E == t6.E);
}

TEST(CPLUS_INITIALIZE_STRUCT_POINTER, functionity)
{
	CPLUS_INITIALIZE_STRUCT_POINTER(&t1);
	EXPECT_EQ(CPLUS_TRUE, 0 == t1.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t1.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t1.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t1.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t1.E);

	CPLUS_INITIALIZE_STRUCT_POINTER(&t2);
	EXPECT_EQ(CPLUS_TRUE, 0 == t2.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t2.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t2.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t2.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t2.E);

	CPLUS_INITIALIZE_STRUCT_POINTER(&t3);
	EXPECT_EQ(CPLUS_TRUE, 0 == t3.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t3.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t3.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t3.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t3.E);

	CPLUS_INITIALIZE_STRUCT_POINTER(&t4);
	EXPECT_EQ(CPLUS_TRUE, 0 == t4.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t4.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t4.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t4.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t4.E);

	CPLUS_INITIALIZE_STRUCT_POINTER(&t5);
	EXPECT_EQ(CPLUS_TRUE, 0 == t5.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t5.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t5.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t5.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t5.E);

	CPLUS_INITIALIZE_STRUCT_POINTER(&t6);
	EXPECT_EQ(CPLUS_TRUE, 0 == t6.A);
	EXPECT_EQ(CPLUS_TRUE, 0 == t6.B);
	EXPECT_EQ(CPLUS_TRUE, 0 == t6.C);
	EXPECT_EQ(CPLUS_TRUE, 0 == t6.D);
	EXPECT_EQ(CPLUS_TRUE, 0 == t6.E);
}