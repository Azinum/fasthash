// fasthash.c

#include "common.h"

#define BENCHMARK 1

typedef size_t Hash;

Hash djb2(const u8* data, const size_t size);
Hash sdbm(const u8* data, const size_t size);
Hash basic(const u8* data, const size_t size);
Hash basic_simd(const u8* data, const size_t size);
Hash basic_v2(const u8* data, const size_t size);
Hash basic_v2_simd(const u8* data, const size_t size);

typedef enum {
  HASH_DJB2 = 0,
  HASH_SDBM,
  HASH_BASIC,
  HASH_BASIC_SIMD,
  HASH_BASIC_V2,
  HASH_BASIC_V2_SIMD,

  MAX_HASH_TYPE,
} Hash_type;

typedef Hash (*hash_func)(const u8*, const size_t);

static const hash_func hash_funcs[MAX_HASH_TYPE] = {
  djb2,
  sdbm,
  basic,
  basic_simd,
  basic_v2,
  basic_v2_simd,
};

static const char* hash_funcs_str[MAX_HASH_TYPE] = {
  "djb2",
  "sdbm",
  "basic",
  "basic_simd",
  "basic_v2",
  "basic_v2_simd",
};

static const char* hash_funcs_str_fmt[MAX_HASH_TYPE] = {
  "         djb2",
  "         sdbm",
  "        basic",
  "   basic_simd",
  "     basic_v2",
  "basic_v2_simd",
};

typedef enum {
  V_INT32,
  V_INT64,
  V_FLOAT32,
  V_FLOAT64
} Vector_type;

void benchmarks_run();
void collisions_run();
f32 benchmark(hash_func func, const u8* data, const size_t size, const size_t n);
size_t calculate_collisions(hash_func func, const u8* data, const size_t size, const size_t n, size_t* collision_map, size_t collision_map_size);
void vector_print(Vector_type type, void* v);
void vector_printline(Vector_type type, void* v);

#define BUFFER_SIZE 128
size_t buffer[BUFFER_SIZE] = {
  22209, 35084, 14403, 53369, 8182,  26351, 10703, 7852,  36446, 32467, 23145, 29189, 29799, 3675,  59335, 39577,
  18128, 8246,  25003, 22155, 56203, 42543, 8796,  61916, 2371,  46641, 60998, 30057, 51286, 61374, 20820, 42799,
  39485, 63172, 34580, 65450, 14329, 4679,  62075, 5887,  42873, 656,   44182, 39326, 47225, 3430,  28608, 34128,
  15614, 54189, 38856, 63997, 23014, 31536, 53567, 19804, 41280, 24540, 24153, 4392,  46305, 47027, 24903, 24626,
  9288,  20034, 30896, 64667, 4424,  57398, 37279, 6659,  40571, 9711,  8049,  49257, 7296,  49740, 8717,  53992,
  12363, 39617, 53784, 55887, 5674,  7590,  16895, 33775, 45129, 56816, 22166, 7178,  31735, 48070, 25555, 61579,
  3624,  8906,  23398, 35573, 17325, 5182,  32425, 57459, 46669, 64560, 56424, 29056, 15012, 46821, 17643, 48880,
  64670, 1713,  39583, 32868, 7837,  17377, 7647,  16615, 17775, 25579, 13761, 59412, 27054, 46447, 11631, 14170,
};

i32 main(void) {
  ASSERT(hash_funcs[HASH_BASIC]((u8*)buffer, sizeof(buffer)) == hash_funcs[HASH_BASIC_SIMD]((u8*)buffer, sizeof(buffer)));
  ASSERT(hash_funcs[HASH_BASIC_V2]((u8*)buffer, sizeof(buffer)) == hash_funcs[HASH_BASIC_V2_SIMD]((u8*)buffer, sizeof(buffer)));
#if BENCHMARK
  benchmarks_run();
#endif
  collisions_run();
  return 0;
}

inline Hash djb2(const u8* data, const size_t size) {
  Hash result = 5381;
  for (size_t i = 0; i < size; ++i) {
    result = ((result << 5) + result) + data[i];
  }
  return result;
}

inline Hash sdbm(const u8* data, const size_t size) {
  Hash result = 0;
  for (size_t i = 0; i < size; ++i) {
    result = data[i] + (result << 6) + (result << 16) - result;
  }
  return result;
}

inline Hash basic(const u8* data, const size_t size) {
  Hash result[] = {
    7253,
    5381,
  };
  size_t chunk_size = sizeof(result);
  const Hash* it = (Hash*)data;
  for (size_t i = 0; i < size; i += chunk_size, it += 2) {
    result[0] += (result[0] << 7) + it[0];
    result[1] += (result[1] << 7) + it[1];
  }
  return result[0] + result[1];
}

inline Hash basic_simd(const u8* data, const size_t size) {
#ifdef USE_SIMD
  if (size < sizeof(__m128i)) {
    return basic(data, size);
  }
  __m128i result = _mm_set_epi64x(5381, 7253);
  const __m128i shift = _mm_set1_epi64x(7);
  const __m128i* it = (__m128i*)data;
  size_t chunk_size = sizeof(result);
  for (size_t i = 0; i < size; i += chunk_size, it += 1) {
    result = _mm_add_epi64(
      result,
      _mm_add_epi64(
        _mm_sll_epi64(result, shift),
        *it
      )
    );
  }
  Hash* hash = (Hash*)&result;
  return hash[0] + hash[1];
#else
  return basic(data, size);
#endif
}

inline Hash basic_v2(const u8* data, const size_t size) {
  i32 inputs[4] = {
    5381,
    7253,
    3433,
    6673
  };
  size_t chunk_size = sizeof(inputs);
  const i32* it = (i32*)data;
  for (size_t i = 0; i < size; i += chunk_size, it += 4) {
    inputs[0] += (inputs[0] << 7) + it[0];
    inputs[1] += (inputs[1] << 7) + it[1];
    inputs[2] += (inputs[2] << 7) + it[2];
    inputs[3] += (inputs[3] << 7) + it[3];
  }
  Hash hash = inputs[0] + inputs[1] + inputs[2] + inputs[3];
  return hash;
}

inline Hash basic_v2_simd(const u8* data, const size_t size) {
#ifdef USE_SIMD
  if (size < sizeof(__m128i)) {
    return basic_v2(data, size);
  }
  __m128i inputs = _mm_set_epi32(
    6673,
    3433,
    7253,
    5381
  );
  size_t chunk_size = sizeof(inputs);
  const __m128i* it = (__m128i*)data;
  for (size_t i = 0; i < size; i += chunk_size, it += 1) {
    inputs = _mm_add_epi32(
      inputs,
      _mm_add_epi32(
        _mm_slli_epi32(inputs, 7),
        *it
      )
    );
  }
  i32* in = (i32*)&inputs;
  Hash hash = in[0] + in[1] + in[2] + in[3];
  return hash;
#else
  return basic_v2(data, size);
#endif
}

void benchmarks_run() {
  FILE* fp = stdout;

  #define NS_PER_BENCHMARK 4
  const size_t ns[NS_PER_BENCHMARK] = {
    1000, 10000, 100000, 1000000
  };
  f32 bench_result[MAX_HASH_TYPE * NS_PER_BENCHMARK] = {0};
  for (size_t i = 0; i < MAX_HASH_TYPE; ++i) {
    for (size_t n = 0; n < NS_PER_BENCHMARK; ++n) {
      const size_t N = ns[n];
      bench_result[i * NS_PER_BENCHMARK + n] = benchmark(hash_funcs[i], (u8*)buffer, sizeof(buffer), N);
    }
  }

  // hashrate * bytes hashed per hash
  for (size_t i = 0; i < MAX_HASH_TYPE; ++i) {
    const char* hash_name = hash_funcs_str[i];
    fprintf(fp, "%s:\n", hash_name);
    for (size_t n = 0; n < NS_PER_BENCHMARK; ++n) {
      const size_t N = ns[n];
      f32 dt = bench_result[i * NS_PER_BENCHMARK + n];
      f32 dt_per_hash = dt / N;
      size_t hashrate = (size_t)(1.0f / dt_per_hash);
      f32 mib_speed = hashrate * sizeof(buffer) / (f32)(1024 * 1024 * 1024);
      fprintf(fp, "   %.7f ms (N = %zu, %zu H/s, %g MH/s, %.3g GiB/s)\n", dt * 1000.0f, N, hashrate, hashrate / 1000000.0f, mib_speed);
    }
  }
}

void collisions_run() {
  srand(time(0));
  #define DATA_COUNT 1024
  size_t data_size = sizeof(size_t) * DATA_COUNT;
  size_t* data = malloc(data_size);
  for (size_t i = 0; i < DATA_COUNT; ++i) {
    data[i] = rand();
  }

  #define COLLISION_MAP_SIZE 4421
  #define HASH_INSERTIONS COLLISION_MAP_SIZE/4
  size_t collision_map[COLLISION_MAP_SIZE] = {0};
  for (size_t i = 0; i < MAX_HASH_TYPE; ++i) {
    const char* hash_name = hash_funcs_str[i];
    memset(collision_map, 0, sizeof(collision_map));
    size_t collisions = calculate_collisions(hash_funcs[i], (u8*)data, data_size, HASH_INSERTIONS, collision_map, COLLISION_MAP_SIZE);
    printf("%s:\n", hash_name);
    printf("   collisions: %zu (%.3g%%, insertions: %d, map size: %d)\n", collisions, 100 * (collisions / (f32)COLLISION_MAP_SIZE), HASH_INSERTIONS, COLLISION_MAP_SIZE);
  }
  free(data);
}

f32 benchmark(hash_func func, const u8* data, const size_t size, const size_t n) {
  f32 result = 0.0f;
  TIMER_START();

  for (size_t i = 0; i < n; ++i) {
    Hash hash = func(data, size);
    (void)hash;
  }

  TIMER_END(
    result = dt;
  );
  return result;
}

size_t calculate_collisions(hash_func func, const u8* data, const size_t size, const size_t n, size_t* collision_map, size_t collision_map_size) {
  size_t collisions = 0;
  ASSERT(collision_map != NULL);
  size_t index = 0; // index that maps to collision map
  size_t chunk_size = sizeof(size_t); // size of chunk to hash
  size_t data_index = 0;

  for (; index < n; ++index, data_index += chunk_size) {
    Hash hash = func(&data[data_index], chunk_size);
    collision_map[hash % collision_map_size] += 1;
    if (data_index + chunk_size >= size) {
      data_index = 0;
      chunk_size += sizeof(size_t);
      if (chunk_size >= size) {
        fprintf(stderr, "error: failed to calculate collisions, no data left to hash\n");
        break;
      }
    }
  }

  for (size_t i = 0; i < collision_map_size; ++i) {
    if (collision_map[i] > 1) {
      collisions += 1;
    }
  }
  return collisions;
}

void vector_print(Vector_type type, void* v) {
  switch (type) {
    case V_INT32: {
      i32* vec = (i32*)v;
      printf("%d, %d, %d, %d", vec[0], vec[1], vec[2], vec[3]);
      break;
    }
    case V_INT64: {
      i64* vec = (i64*)v;
      printf("%zu, %zu", vec[0], vec[1]);
      break;
    }
    case V_FLOAT32: {
      f32* vec = (f32*)v;
      printf("%.6f, %.6f, %.6f, %.6f", vec[0], vec[1], vec[2], vec[3]);
      break;
    }
    case V_FLOAT64: {
      f64* vec = (f64*)v;
      printf("%.6f, %.6f, %.6f, %.6f", vec[0], vec[1], vec[2], vec[3]);
      break;
    }
    default:
      break;
  }
}

void vector_printline(Vector_type type, void* v) {
  vector_print(type, v);
  printf("\n");
}
