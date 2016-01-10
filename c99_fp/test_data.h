/* Test data. */

/* If you don't specify anything, you get really weaksauce synthetic data. */
#if !defined(TEST_DATASET_4_1) && !defined(TEST_DATASET_36_1)
#define TEST_DATASET_4_1
#endif

/* We use macros here because C makes static initialization hard */
#if defined(TEST_DATASET_4_1)
#define TEST_TOOTH_MAP {2, 1, 1}

#elif defined(TEST_DATASET_36_1)
#define TEST_TOOTH_MAP {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

#endif

extern const uint32_t TEST_SAMPLE_RATE;
extern const float    TEST_MAX_ACCEL;
extern const float    TEST_ERROR_RATE;

extern const size_t   num_sample_engine_ticks;
extern const uint32_t sample_engine_ticks[];
