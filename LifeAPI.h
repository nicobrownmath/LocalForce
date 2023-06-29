// LifeAPI provide comfortable functions (API) to manipulate, iterate, evolve,
// compare and report Life objects. This is mainly done in order to provide fast
// (using C) but still comfortable search utility. Contributor Chris Cain.
// Written by Michael Simkin 2014

//The modifications to normal LifeAPI in here are from CatForce because I've grown attached to them

#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <tuple>

#include <immintrin.h>

#define N 64

#define SUCCESS 1
#define FAIL 0

// GCC
#ifdef __GNUC__
#ifndef __clang__
#include <x86intrin.h>

uint64_t reverse_uint64_t(uint64_t x) {
  const uint64_t h1 = 0x5555555555555555ULL;
  const uint64_t h2 = 0x3333333333333333ULL;
  const uint64_t h4 = 0x0F0F0F0F0F0F0F0FULL;
  const uint64_t v1 = 0x00FF00FF00FF00FFULL;
  const uint64_t v2 = 0x0000FFFF0000FFFFULL;
  x = ((x >> 1) & h1) | ((x & h1) << 1);
  x = ((x >> 2) & h2) | ((x & h2) << 2);
  x = ((x >> 4) & h4) | ((x & h4) << 4);
  x = ((x >> 8) & v1) | ((x & v1) << 8);
  x = ((x >> 16) & v2) | ((x & v2) << 16);
  x = (x >> 32) | (x << 32);
  return x;
}

#define __builtin_rotateleft64 __rolq
#define __builtin_rotateright64 __rorq
#define __builtin_bitreverse64 ::reverse_uint64_t
#endif
#endif

// MSVC
#ifdef __MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt64
#define __builtin_rotateleft64 _rotl64
#define __builtin_rotateright64 _rotr64

inline int __builtin_ctzll(uint64_t x) {
  unsigned long log2;
  _BitScanReverse64(&log2, x);
  return log2;
}
#endif

inline uint64_t convolve_uint64_t(uint64_t x, uint64_t y) {
  if(y == 0)
    return 0;

  uint64_t result = 0;
  while (x != 0) {
    int lsb = __builtin_ctzll(x);
    result |= __builtin_rotateleft64(y, lsb);
    x &= ~(((uint64_t)1) << lsb);
  }
  return result;
}

uint64_t convolve_uint64_t2(uint64_t x, uint64_t y) {
  if(x == 0 || y == 0)
    return 0;

  // power of 2:
  if((x & (x - 1)) == 0) {
    int lsb = __builtin_ctzll(x);
    return __builtin_rotateleft64(x, lsb);
  }
  if((y & (y - 1)) == 0) {
    int lsb = __builtin_ctzll(y);
    return __builtin_rotateleft64(y, lsb);
  }

  return 0
    | __builtin_rotateleft64(y, 0) * ((x & (1ULL << 0)) != 0)
    | __builtin_rotateleft64(y, 1) * ((x & (1ULL << 1)) != 0)
    | __builtin_rotateleft64(y, 2) * ((x & (1ULL << 2)) != 0)
    | __builtin_rotateleft64(y, 3) * ((x & (1ULL << 3)) != 0)
    | __builtin_rotateleft64(y, 4) * ((x & (1ULL << 4)) != 0)
    | __builtin_rotateleft64(y, 5) * ((x & (1ULL << 5)) != 0)
    | __builtin_rotateleft64(y, 6) * ((x & (1ULL << 6)) != 0)
    | __builtin_rotateleft64(y, 7) * ((x & (1ULL << 7)) != 0)
    | __builtin_rotateleft64(y, 8) * ((x & (1ULL << 8)) != 0)
    | __builtin_rotateleft64(y, 9) * ((x & (1ULL << 9)) != 0)
    | __builtin_rotateleft64(y, 10) * ((x & (1ULL << 10)) != 0)
    | __builtin_rotateleft64(y, 11) * ((x & (1ULL << 11)) != 0)
    | __builtin_rotateleft64(y, 12) * ((x & (1ULL << 12)) != 0)
    | __builtin_rotateleft64(y, 13) * ((x & (1ULL << 13)) != 0)
    | __builtin_rotateleft64(y, 14) * ((x & (1ULL << 14)) != 0)
    | __builtin_rotateleft64(y, 15) * ((x & (1ULL << 15)) != 0)
    | __builtin_rotateleft64(y, 16) * ((x & (1ULL << 16)) != 0)
    | __builtin_rotateleft64(y, 17) * ((x & (1ULL << 17)) != 0)
    | __builtin_rotateleft64(y, 18) * ((x & (1ULL << 18)) != 0)
    | __builtin_rotateleft64(y, 19) * ((x & (1ULL << 19)) != 0)
    | __builtin_rotateleft64(y, 20) * ((x & (1ULL << 20)) != 0)
    | __builtin_rotateleft64(y, 21) * ((x & (1ULL << 21)) != 0)
    | __builtin_rotateleft64(y, 22) * ((x & (1ULL << 22)) != 0)
    | __builtin_rotateleft64(y, 23) * ((x & (1ULL << 23)) != 0)
    | __builtin_rotateleft64(y, 24) * ((x & (1ULL << 24)) != 0)
    | __builtin_rotateleft64(y, 25) * ((x & (1ULL << 25)) != 0)
    | __builtin_rotateleft64(y, 26) * ((x & (1ULL << 26)) != 0)
    | __builtin_rotateleft64(y, 27) * ((x & (1ULL << 27)) != 0)
    | __builtin_rotateleft64(y, 28) * ((x & (1ULL << 28)) != 0)
    | __builtin_rotateleft64(y, 29) * ((x & (1ULL << 29)) != 0)
    | __builtin_rotateleft64(y, 30) * ((x & (1ULL << 30)) != 0)
    | __builtin_rotateleft64(y, 31) * ((x & (1ULL << 31)) != 0)
    | __builtin_rotateleft64(y, 32) * ((x & (1ULL << 32)) != 0)
    | __builtin_rotateleft64(y, 33) * ((x & (1ULL << 33)) != 0)
    | __builtin_rotateleft64(y, 34) * ((x & (1ULL << 34)) != 0)
    | __builtin_rotateleft64(y, 35) * ((x & (1ULL << 35)) != 0)
    | __builtin_rotateleft64(y, 36) * ((x & (1ULL << 36)) != 0)
    | __builtin_rotateleft64(y, 37) * ((x & (1ULL << 37)) != 0)
    | __builtin_rotateleft64(y, 38) * ((x & (1ULL << 38)) != 0)
    | __builtin_rotateleft64(y, 39) * ((x & (1ULL << 39)) != 0)
    | __builtin_rotateleft64(y, 40) * ((x & (1ULL << 40)) != 0)
    | __builtin_rotateleft64(y, 41) * ((x & (1ULL << 41)) != 0)
    | __builtin_rotateleft64(y, 42) * ((x & (1ULL << 42)) != 0)
    | __builtin_rotateleft64(y, 43) * ((x & (1ULL << 43)) != 0)
    | __builtin_rotateleft64(y, 44) * ((x & (1ULL << 44)) != 0)
    | __builtin_rotateleft64(y, 45) * ((x & (1ULL << 45)) != 0)
    | __builtin_rotateleft64(y, 46) * ((x & (1ULL << 46)) != 0)
    | __builtin_rotateleft64(y, 47) * ((x & (1ULL << 47)) != 0)
    | __builtin_rotateleft64(y, 48) * ((x & (1ULL << 48)) != 0)
    | __builtin_rotateleft64(y, 49) * ((x & (1ULL << 49)) != 0)
    | __builtin_rotateleft64(y, 50) * ((x & (1ULL << 50)) != 0)
    | __builtin_rotateleft64(y, 51) * ((x & (1ULL << 51)) != 0)
    | __builtin_rotateleft64(y, 52) * ((x & (1ULL << 52)) != 0)
    | __builtin_rotateleft64(y, 53) * ((x & (1ULL << 53)) != 0)
    | __builtin_rotateleft64(y, 54) * ((x & (1ULL << 54)) != 0)
    | __builtin_rotateleft64(y, 55) * ((x & (1ULL << 55)) != 0)
    | __builtin_rotateleft64(y, 56) * ((x & (1ULL << 56)) != 0)
    | __builtin_rotateleft64(y, 57) * ((x & (1ULL << 57)) != 0)
    | __builtin_rotateleft64(y, 58) * ((x & (1ULL << 58)) != 0)
    | __builtin_rotateleft64(y, 59) * ((x & (1ULL << 59)) != 0)
    | __builtin_rotateleft64(y, 60) * ((x & (1ULL << 60)) != 0)
    | __builtin_rotateleft64(y, 61) * ((x & (1ULL << 61)) != 0)
    | __builtin_rotateleft64(y, 62) * ((x & (1ULL << 62)) != 0)
    | __builtin_rotateleft64(y, 63) * ((x & (1ULL << 63)) != 0) ;
}

uint64_t convolve_uint8_t(uint8_t x, uint64_t y) {
  if(x == 0 || y == 0)
    return 0;

  return 0
    | __builtin_rotateleft64(y, 0) * ((x & (1ULL << 0)) != 0)
    | __builtin_rotateleft64(y, 1) * ((x & (1ULL << 1)) != 0)
    | __builtin_rotateleft64(y, 2) * ((x & (1ULL << 2)) != 0)
    | __builtin_rotateleft64(y, 3) * ((x & (1ULL << 3)) != 0)
    | __builtin_rotateleft64(y, 4) * ((x & (1ULL << 4)) != 0)
    | __builtin_rotateleft64(y, 5) * ((x & (1ULL << 5)) != 0)
    | __builtin_rotateleft64(y, 6) * ((x & (1ULL << 6)) != 0)
    | __builtin_rotateleft64(y, 7) * ((x & (1ULL << 7)) != 0);
}

uint64_t convolve_uint64_t3(uint64_t x, uint64_t y) {
  if(x == 0 || y == 0)
    return 0;

  uint8_t x0 = x >> (8 * 0) & 0xFF;
  uint8_t x1 = x >> (8 * 1) & 0xFF;
  uint8_t x2 = x >> (8 * 2) & 0xFF;
  uint8_t x3 = x >> (8 * 3) & 0xFF;
  uint8_t x4 = x >> (8 * 4) & 0xFF;
  uint8_t x5 = x >> (8 * 5) & 0xFF;
  uint8_t x6 = x >> (8 * 6) & 0xFF;
  uint8_t x7 = x >> (8 * 7) & 0xFF;

  return 0
    | __builtin_rotateleft64(convolve_uint8_t(x0, y), (8 * 0))
    | __builtin_rotateleft64(convolve_uint8_t(x1, y), (8 * 1))
    | __builtin_rotateleft64(convolve_uint8_t(x2, y), (8 * 2))
    | __builtin_rotateleft64(convolve_uint8_t(x3, y), (8 * 3))
    | __builtin_rotateleft64(convolve_uint8_t(x4, y), (8 * 4))
    | __builtin_rotateleft64(convolve_uint8_t(x5, y), (8 * 5))
    | __builtin_rotateleft64(convolve_uint8_t(x6, y), (8 * 6))
    | __builtin_rotateleft64(convolve_uint8_t(x7, y), (8 * 7));

}


namespace PRNG {

// Public domain PRNG by Sebastian Vigna 2014, see http://xorshift.di.unimi.it

uint64_t s[16] = {0x12345678};
int p = 0;

uint64_t rand64() {
  uint64_t s0 = s[p];
  uint64_t s1 = s[p = (p + 1) & 15];
  s1 ^= s1 << 31; // a
  s1 ^= s1 >> 11; // b
  s0 ^= s0 >> 30; // c
  return (s[p] = s0 ^ s1) * 1181783497276652981ULL;
}

void seed(const uint64_t &seed) {
  s[0] ^= seed * 1181783497276652981ULL;
}

} // namespace PRNG

// void fastMemcpy(void *pvDest, void *pvSrc, size_t nBytes) {
//   assert(nBytes % 32 == 0);
//   assert((intptr_t(pvDest) & 31) == 0);
//   assert((intptr_t(pvSrc) & 31) == 0);
//   const __m256i *pSrc = reinterpret_cast<const __m256i*>(pvSrc);
//   __m256i *pDest = reinterpret_cast<__m256i*>(pvDest);
//   int64_t nVects = nBytes / sizeof(*pSrc);
//   for (; nVects > 0; nVects--, pSrc++, pDest++) {
//     const __m256i loaded = _mm256_stream_load_si256(pSrc);
//     _mm256_stream_si256(pDest, loaded);
//   }
//   _mm_sfence();
// }

enum CopyType { COPY, OR, XOR, AND, ANDNOT, ORNOT };

enum SymmetryTransform {
  Identity,
  ReflectAcrossXEven,
  ReflectAcrossX,
  ReflectAcrossYEven,
  ReflectAcrossY,
  Rotate90Even,
  Rotate90,
  Rotate270Even,
  Rotate270,
  Rotate180OddBoth,
  Rotate180EvenHorizontal,
  Rotate180EvenVertical,
  Rotate180EvenBoth,
  ReflectAcrossYeqX,
  ReflectAcrossYeqNegX,
  // reflect across y = -x+3/2, fixing (0,0), instead of y=-x+1/2,
  // sending (0,0) to (-1,-1). Needed for D4x_1 symmetry.
  ReflectAcrossYeqNegXP1
};

enum StaticSymmetry {
  C1,
  D2AcrossX,
  D2AcrossXEven,
  D2AcrossY,
  D2AcrossYEven,
  D2negdiagodd,
  D2diagodd,
  C2,
  C2even,
  C2verticaleven,
  C2horizontaleven,
  C4,
  C4even,
  D4,
  D4even,
  D4verticaleven,
  D4horizontaleven,
  D4diag,
  D4diageven,
  D8,
  D8even,
  D2negdiagevenSubgroupsOnly,
};

inline uint64_t RotateLeft(uint64_t x, unsigned int k) {
  return __builtin_rotateleft64(x, k);
}

inline uint64_t RotateRight(uint64_t x, unsigned int k) {
  return __builtin_rotateright64(x, k);
}

inline uint64_t RotateLeft(uint64_t x) { return RotateLeft(x, 1); }
inline uint64_t RotateRight(uint64_t x) { return RotateRight(x, 1); }

class LifeTarget;

class LifeState {
public:
  uint64_t state[N];

  int min;
  int max;
  int gen;

  LifeState() : state{0}, min(0), max(N - 1), gen(0) {}

  void Set(int x, int y) { state[x] |= (1ULL << (y)); }
  void Erase(int x, int y) { state[x] &= ~(1ULL << (y)); }
  int Get(int x, int y) const { return (state[x] & (1ULL << y)) >> y; }
  void SetCell(int x, int y, int val) {
    if (val == 1) {
      Set((x + 64) % N, (y + 64) % 64);
    }
    if (val == 0)
      Erase((x + 64) % N, (y + 64) % 64);
  }
  int GetCell(int x, int y) const {
    return Get((x + 64) % N, (y + 64) % 64);
  }
  uint64_t GetHash() const {
    uint64_t result = 0;

    for (int i = 0; i < N; i++) {
      result += RotateLeft(state[i], (int)(i / 2));
    }

    return result;
  }

  void ExpandMinMax(int &min, int &max) {
    min = min - 2;
    max = max + 2;

    if (min <= 0 || max >= N - 1) {
      min = 0;
      max = N - 1;
    }
  }

#ifdef __AVX2__
  void RecalculateMinMax() {
    min = 0;
    max = N - 1;

    const char *p = (const char *)state;
    size_t len = 8*N;
    const char *p_init = p;
    const char *endp = p + len;
    do {
      __m256i v1 = _mm256_loadu_si256((const __m256i*)p);
      __m256i v2 = _mm256_loadu_si256((const __m256i*)(p+32));
      __m256i vor = _mm256_or_si256(v1,v2);
      if (!_mm256_testz_si256(vor, vor)) {
        min = (p-p_init)/8;
        break;
      }
      p += 64;
    } while(p < endp);

    p = endp-64;
    do {
      __m256i v1 = _mm256_loadu_si256((const __m256i*)p);
      __m256i v2 = _mm256_loadu_si256((const __m256i*)(p+32));
      __m256i vor = _mm256_or_si256(v1,v2);
      if (!_mm256_testz_si256(vor, vor)) {
        max = (p-p_init+64)/8;
        break;
      }
      p -= 64;
    } while(p >= p_init);

    ExpandMinMax(min, max);
  }
#else
  void RecalculateMinMax() {
    min = 0;
    max = N - 1;

    for (int i = 0; i < N; i++) {
      if (state[i] != 0) {
        min = i;
        break;
      }
    }

    for (int i = N - 1; i >= 0; i--) {
      if (state[i] != 0) {
        max = i;
        break;
      }
    }

    ExpandMinMax(min, max);
  }
#endif

public:
  void Print() const;

  void Copy(const LifeState &delta, CopyType op) {
    if (op == COPY) {
      for (int i = 0; i < N; i++)
        state[i] = delta.state[i];

      min = delta.min;
      max = delta.max;
      gen = delta.gen;
      return;
    }
    if (op == OR) {
      for (int i = 0; i < N; i++)
        state[i] |= delta.state[i];
      min = std::min(min, delta.min);
      max = std::max(max, delta.max);
      return;
    }
    if (op == AND) {
      for (int i = 0; i < N; i++)
        state[i] &= delta.state[i];
    }
    if (op == ANDNOT) {
      for (int i = 0; i < N; i++)
        state[i] &= ~delta.state[i];
    }
    if (op == ORNOT) {
      for (int i = 0; i < N; i++)
        state[i] |= ~delta.state[i];
      RecalculateMinMax();
    }
    if (op == XOR) {
      for (int i = 0; i < N; i++)
        state[i] ^= delta.state[i];
      RecalculateMinMax();
    }
  }

  void Copy(const LifeState &delta) { Copy(delta, COPY); }

  inline void Copy(const LifeState &delta, int x, int y) {
    uint64_t temp1[N] = {0};

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = delta.min; i <= delta.max; i++)
      temp1[i] = RotateLeft(delta.state[i], y);

    memmove(state, temp1 + (N - x), x * sizeof(uint64_t));
    memmove(state + x, temp1, (N - x) * sizeof(uint64_t));

    min = 0;
    max = N - 1;
  }

  void Join(const LifeState &delta) { Copy(delta, OR); }

  inline void Join(const LifeState &delta, int x, int y) {
    uint64_t temp1[N] = {0};
    uint64_t temp2[N];

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = delta.min; i <= delta.max; i++)
      temp1[i] = RotateLeft(delta.state[i], y);

    memmove(temp2, temp1 + (N - x), x * sizeof(uint64_t));
    memmove(temp2 + x, temp1, (N - x) * sizeof(uint64_t));

    for (int i = 0; i < N; i++) {
      state[i] |= temp2[i];
    }

    min = 0;
    max = N - 1;
  }

  inline void JoinXOR(const LifeState &delta, int x, int y) {
    uint64_t temp1[N] = {0};
    uint64_t temp2[N];

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = delta.min; i <= delta.max; i++)
      temp1[i] = RotateLeft(delta.state[i], y);

    memmove(temp2, temp1 + (N - x), x * sizeof(uint64_t));
    memmove(temp2 + x, temp1, (N - x) * sizeof(uint64_t));

    for (int i = 0; i < N; i++) {
      state[i] ^= temp2[i];
    }
  }

  inline void JoinAND(const LifeState &delta, int x, int y) {
    uint64_t temp1[N] = {0};
    uint64_t temp2[N];

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = delta.min; i <= delta.max; i++)
      temp1[i] = RotateLeft(delta.state[i], y);

    memmove(temp2, temp1 + (N - x), x * sizeof(uint64_t));
    memmove(temp2 + x, temp1, (N - x) * sizeof(uint64_t));

    for (int i = 0; i < N; i++) {
      state[i] &= temp2[i];
    }
  }

  inline void JoinANDNOT(const LifeState &delta, int x, int y) {
    uint64_t temp1[N] = {0};
    uint64_t temp2[N];

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = delta.min; i <= delta.max; i++)
      temp1[i] = RotateLeft(delta.state[i], y);

    memmove(temp2, temp1 + (N - x), x * sizeof(uint64_t));
    memmove(temp2 + x, temp1, (N - x) * sizeof(uint64_t));

    for (int i = 0; i < N; i++) {
      state[i] &= ~temp2[i];
    }
  }

  void JoinWSymChain(const LifeState &state, int x, int y,
                     const std::vector<SymmetryTransform> &symChain) {
    LifeState transformed = state;
    transformed.Move(x, y);

    for (auto sym : symChain) {
      LifeState soFar = transformed;
      soFar.Transform(sym);
      transformed.Join(soFar);
    }
    Join(transformed);
  }

  void JoinWSymChain(const LifeState &state,
                     const std::vector<SymmetryTransform> &symChain) {
    LifeState transformed = state;

    for (auto sym : symChain) {
      LifeState soFar = transformed;
      soFar.Transform(sym);
      transformed.Join(soFar);
    }
    Join(transformed);
  }

  LifeState GetSymChain(int x, int y, const std::vector<SymmetryTransform> &symChain) const {
    LifeState transformed = *this;
    transformed.Move(x, y);
    for (auto &transform : symChain) {
      LifeState soFar = transformed;
      soFar.Transform(transform);
      transformed.Join(soFar);
    }
    return transformed;
  }

  LifeState GetSymChain(const std::vector<SymmetryTransform> &symChain) const {
    LifeState transformed = *this;
    for (auto &transform : symChain) {
      LifeState soFar = transformed;
      soFar.Transform(transform);
      transformed.Join(soFar);
    }
    return transformed;
  }

  void SymChain(const std::vector<SymmetryTransform> &symChain) {
    for (auto &transform : symChain) {
      LifeState copy = *this;
      copy.Transform(transform);
      Join(copy);
    }
  }

  unsigned GetPop() const {
    unsigned pop = 0;

    for (int i = min; i <= max; i++) {
      pop += __builtin_popcountll(state[i]);
    }

    return pop;
  }

  // bool IsEmpty() const {
  //   for (int i = 0; i < N; i++) {
  //     if(state[i] != 0)
  //       return false;
  //   }

  //   return true;
  // }

  bool IsEmpty() const {
    uint64_t all = 0;
    for (int i = 0; i < N; i++) {
      all |= state[i];
    }

    return all == 0;
  }

  bool IsFull() const {
    uint64_t all = ~0ULL;
    for (int i = 0; i < N; i++) {
      all &= state[i];
    }

    return all == ~0ULL;
  }

  void Inverse() {
    for (int i = 0; i < N; i++) {
      state[i] = ~state[i];
    }
  }

  bool operator==(const LifeState &b) const {
    for (int i = 0; i < N; i++)
      if (state[i] != b.state[i])
        return false;

    return true;
  }

  bool operator!=(const LifeState &b) const {
    return !(*this == b);
  }

  LifeState operator~() const {
    LifeState result;
    for (int i = 0; i < N; i++) {
      result.state[i] = ~state[i];
    }
    return result;
  }

  LifeState operator&(const LifeState &other) const {
    LifeState result;
    for (int i = 0; i < N; i++) {
      result.state[i] = state[i] & other.state[i];
    }
    return result;
  }

  LifeState& operator&=(const LifeState &other) {
    for (int i = 0; i < N; i++) {
      state[i] = state[i] & other.state[i];
    }
    return *this;
  }

  LifeState operator|(const LifeState &other) const {
    LifeState result;
    for (int i = 0; i < N; i++) {
      result.state[i] = state[i] | other.state[i];
    }
    return result;
  }

  LifeState& operator|=(const LifeState &other) {
    for (int i = 0; i < N; i++) {
      state[i] = state[i] | other.state[i];
    }
    return *this;
  }

  LifeState operator^(const LifeState &other) const {
    LifeState result;
    for (int i = 0; i < N; i++) {
      result.state[i] = state[i] ^ other.state[i];
    }
    return result;
  }

  LifeState& operator^=(const LifeState &other) {
    for (int i = 0; i < N; i++) {
      state[i] = state[i] ^ other.state[i];
    }
    return *this;
  }

  friend bool operator <(const LifeState& a, const LifeState& b) {
      for (unsigned i = 0; i < 64; i++) {
          if(a.state[i] < b.state[i]) return true;
          if(a.state[i] > b.state[i]) return false;
      }
      return false;
  }

  inline bool AreDisjoint(const LifeState &pat) const {
    int min = 0;
    int max = N - 1;

    uint64_t differences = 0;
    //#pragma clang loop vectorize(enable)
    #pragma GCC ivdep
    for (int i = min; i <= max; i++) {
      uint64_t difference = (~state[i] & pat.state[i]) ^ (pat.state[i]);
      differences |= difference;
    }

    return differences == 0;
  }

  inline bool Contains(const LifeState &pat) const {
    int min = 0;
    int max = N - 1;

    uint64_t differences = 0;
    //#pragma clang loop vectorize(enable)
    #pragma GCC ivdep
    for (int i = min; i <= max; i++) {
      uint64_t difference = (state[i] & pat.state[i]) ^ (pat.state[i]);
      differences |= difference;
    }

    return differences == 0;
  }

  bool Contains(const LifeState &pat, int targetDx, int targetDy) const {
    int min = pat.min;
    int max = pat.max;

    int dy = (targetDy + 64) % 64;

    for (int i = min; i <= max; i++) {
      int curX = (N + i + targetDx) % N;

      if ((RotateRight(state[curX], dy) & pat.state[i]) != (pat.state[i]))
        return false;
    }
    return true;
  }

  bool AreDisjoint(const LifeState &pat, int targetDx, int targetDy) const {
    int min = pat.min;
    int max = pat.max;
    int dy = (targetDy + 64) % 64;

    for (int i = min; i <= max; i++) {
      int curX = (N + i + targetDx) % N;

      if (((~RotateRight(state[curX], dy)) & pat.state[i]) != pat.state[i])
        return false;
    }

    return true;
  }

  inline bool Contains(const LifeTarget &target, int dx, int dy) const;
  inline bool Contains(const LifeTarget &target) const;

  void Reverse(int idxS, int idxE) {
    for (int i = 0; idxS + i < idxE - i; i++) {
      int l = idxS + i;
      int r = idxE - i;

      uint64_t temp = state[l];
      state[l] = state[r];
      state[r] = temp;
    }
  }

  void Move(int x, int y) {
    uint64_t temp[N];

    if (x < 0)
      x += N;
    if (y < 0)
      y += 64;

    for (int i = 0; i < N; i++)
      temp[i] = RotateLeft(state[i], y);

    memmove(state, temp + (N - x), x * sizeof(uint64_t));
    memmove(state + x, temp, (N - x) * sizeof(uint64_t));

    if ((min + x) % N < (max + x) % N) {
      min = (min + x) % N;
      max = (max + x) % N;
    } else {
      min = 0;
      max = N - 1;
    }
  }

  void BitReverse() {
    for (int i = 0; i < N; i++) {
      state[i] = __builtin_bitreverse64(state[i]);
    }
  }

  void FlipY() { // even reflection across y-axis, ie (0,0) maps to (0, -1)
    Reverse(0, N - 1);
  }

  void Transpose(bool whichDiagonal) {
    int j, k;
    uint64_t m, t;

    for (j = 32, m = 0x00000000FFFFFFFF; j; j >>= 1, m ^= m << j) {
      for (k = 0; k < 64; k = ((k | j) + 1) & ~j) {
        if (whichDiagonal) {
          t = (state[k] ^ (state[k | j] >> j)) & m;
          state[k] ^= t;
          state[k | j] ^= (t << j);
        } else {
          t = (state[k] >> j ^ (state[k | j])) & m;
          state[k] ^= (t << j);
          state[k | j] ^= t;
        }
      }

      RecalculateMinMax();
    }
  }

  void Transpose() { Transpose(true); }

  // even reflection across x-axis, ie (0,0) maps to (0, -1)
  void FlipX() { BitReverse(); }

  void Transform(SymmetryTransform transf);

  void Transform(int dx, int dy, SymmetryTransform transf) {
    Move(dx, dy);
    Transform(transf);
    RecalculateMinMax();
  }

  LifeState ZOI() const {
    LifeState temp;
    LifeState boundary;
    for (int i = 0; i < N; i++) {
      uint64_t col = state[i];
      temp.state[i] = col | RotateLeft(col) | RotateRight(col);
    }

    boundary.state[0] = temp.state[N - 1] | temp.state[0] | temp.state[1];

    for (int i = 1; i < N - 1; i++)
      boundary.state[i] = temp.state[i - 1] | temp.state[i] | temp.state[i + 1];

    boundary.state[N - 1] = temp.state[N - 2] | temp.state[N - 1] | temp.state[0];

    boundary.RecalculateMinMax();
    return boundary;
  }

  LifeState Inflate(const int &x) const {
    if (x >= 32) {
      LifeState output = LifeState();
      output.Inverse();
      return output;
    }
    return Convolve(SolidRect(-x, -x, 2 * x + 1, 2 * x + 1));
  }

  LifeState GetBoundary() const {
    LifeState boundary = ZOI();
    boundary.Copy(*this, ANDNOT);
    return boundary;
  }

  LifeState BigZOI() const {
    LifeState b;
    b.state[0] = state[0] | RotateLeft(state[0]) | RotateRight(state[0]) |
                 state[N - 1] | state[0 + 1];
    for (int i = 1; i < N-1; i++) {
      b.state[i] = state[i] | RotateLeft(state[i]) | RotateRight(state[i]) | state[i-1] | state[i+1];
    }
    b.state[N-1] = state[N-1] | RotateLeft(state[N-1]) | RotateRight(state[N-1]) |
                 state[N-1 - 1] | state[0];

    LifeState c;
    c.state[0] = b.state[0] | b.state[N - 1] | b.state[0 + 1];
    for (int i = 1; i < N - 1; i++) {
      c.state[i] = b.state[i] | b.state[i - 1] | b.state[i + 1];
    }
    c.state[N - 1] = b.state[N - 1] | b.state[N - 1 - 1] | b.state[0];

    LifeState zoi;

    zoi.state[0] =
      c.state[0] | RotateLeft(c.state[0]) | RotateRight(c.state[0]);
    for (int i = 1; i < N - 1; i++) {
      zoi.state[i] =
        c.state[i] | RotateLeft(c.state[i]) | RotateRight(c.state[i]);
    }
    zoi.state[N - 1] = c.state[N - 1] | RotateLeft(c.state[N - 1]) |
      RotateRight(c.state[N - 1]);

    zoi.RecalculateMinMax();
    return zoi;
  }

  static inline void ConvolveInner(LifeState &result, const uint64_t (&doubledother)[N*2], uint64_t x, unsigned int k, unsigned int postshift) {
    for (int i = 0; i < N; i++) {
      result.state[i] |= __builtin_rotateleft64(convolve_uint64_t(x, doubledother[i+k]), postshift);
    }
  }

  LifeState Convolve(const LifeState &other) const {
    LifeState result;
    uint64_t doubledother[N*2];
    memcpy(doubledother,     other.state, N * sizeof(uint64_t));
    memcpy(doubledother + N, other.state, N * sizeof(uint64_t));

    for (int j = 0; j < N; j++) {
      uint64_t x = state[j];
      if(x == 0)
        continue;

      unsigned int postshift;

      if((x & 1) == 0) { // Possibly wrapped
        int lsb = __builtin_ctzll(x);
        x = __builtin_rotateright64(x, lsb);
        postshift = lsb;
      } else{
        int lead = __builtin_clzll(~x);
        x = __builtin_rotateleft64(x, lead);
        postshift = 64-lead;
      }

      switch(x) {
      case (1 << 1) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 2) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 3) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 4) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 5) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 6) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 7) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      case (1 << 8) - 1: ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      default:           ConvolveInner(result, doubledother, x, 64-j, postshift); break;
      }
    }

    result.min = 0;
    result.max = N - 1;

    return result;
  }

  //I don't know why this is (marginally) slower than the other one
  LifeState MyConvolve(const LifeState &other) const {
    LifeState output;

    uint64_t doubledother[N*2];
    memcpy(doubledother,     other.state, N * sizeof(uint64_t));
    memcpy(doubledother + N, other.state, N * sizeof(uint64_t));

    //#pragma clang loop vectorize(enable)
    #pragma GCC ivdep
    for (unsigned myX = 1; myX <= 64; myX++) {
      uint64_t xVal = state[64 - myX];
      if (xVal != 0) {
        //#pragma clang loop vectorize(enable)
        #pragma GCC ivdep
        for (unsigned outputX = 0; outputX < 64; outputX++) {
          output.state[outputX] |= convolve_uint64_t(xVal, doubledother[outputX + myX]);
        }
      }
    }

    return output;
  }

  inline uint64_t findmatches_uint64_t(uint64_t x, uint64_t y) {
    uint64_t result = ~0ULL;
    while (x != 0) {
      int lsb = __builtin_ctzll(x);
      result &= __builtin_rotateright64(y, lsb);
      x &= ~(((uint64_t)1) << lsb);
    }
    return result;
  }

  LifeState FindMatches(const LifeState &testState) {
    LifeState output;
    output.Inverse();

    uint64_t doubledother[N*2];
    memcpy(doubledother,     testState.state, N * sizeof(uint64_t));
    memcpy(doubledother + N, testState.state, N * sizeof(uint64_t));

    #pragma GCC ivdep
    for (unsigned myX = 0; myX < 64; myX++) {
      uint64_t xVal = state[myX];
      #pragma GCC ivdep
      for (unsigned outputX = 0; outputX < 64; outputX++) {
        output.state[outputX] &= findmatches_uint64_t(xVal, doubledother[outputX + myX]);
      }
    }

    output.min = 0;
    output.max = N - 1;

    return output;
  }

  LifeState OneNeighbor() const {
    uint64_t tempxor[N];
    uint64_t tempand[N];

    LifeState output;

    for (int i = 0; i < N; i++) {
      uint64_t l = RotateLeft(state[i]);
      uint64_t r = RotateRight(state[i]);
      tempxor[i] = l ^ r ^ state[i]; //parity of neighborhood
      tempand[i] = ((l | r) & state[i]) | (l & r); //are there at least 2 in neighborhood
    }

    //#pragma clang loop unroll(full)
    #pragma GCC unroll 64
    for (int i = 0; i < N; i++) {
      int idxU;
      int idxB;
      if (i == 0)
        idxU = N - 1;
      else
        idxU = i - 1;

      if (i == N - 1)
        idxB = 0;
      else
        idxB = i + 1;

      output.state[i] = Rokicki1Neighbor(state[i], tempxor[idxU], tempand[idxU], tempxor[idxB],
                            tempand[idxB]);
    }

    output.min = 0;
    output.max = N - 1;
    
    return output;
  }

  LifeState TwoNeighbors() const {
    uint64_t tempxor[N];
    uint64_t tempand[N];

    LifeState output;

    for (int i = 0; i < N; i++) {
      uint64_t l = RotateLeft(state[i]);
      uint64_t r = RotateRight(state[i]);
      tempxor[i] = l ^ r ^ state[i]; //parity of neighborhood
      tempand[i] = ((l | r) & state[i]) | (l & r); //are there at least 2 in neighborhood
    }

    //#pragma clang loop unroll(full)
    #pragma GCC unroll 64
    for (int i = 0; i < N; i++) {
      int idxU;
      int idxB;
      if (i == 0)
        idxU = N - 1;
      else
        idxU = i - 1;

      if (i == N - 1)
        idxB = 0;
      else
        idxB = i + 1;

      output.state[i] = Rokicki2Neighbors(state[i], tempxor[idxU], tempand[idxU], tempxor[idxB],
                            tempand[idxB]);
    }

    output.min = 0;
    output.max = N - 1;
    
    return output;
  }

  LifeState ThreeNeighbors() const {
    uint64_t tempxor[N];
    uint64_t tempand[N];

    LifeState output;

    for (int i = 0; i < N; i++) {
      uint64_t l = RotateLeft(state[i]);
      uint64_t r = RotateRight(state[i]);
      tempxor[i] = l ^ r ^ state[i]; //parity of neighborhood
      tempand[i] = ((l | r) & state[i]) | (l & r); //are there at least 2 in neighborhood
    }

    //#pragma clang loop unroll(full)
    #pragma GCC unroll 64
    for (int i = 0; i < N; i++) {
      int idxU;
      int idxB;
      if (i == 0)
        idxU = N - 1;
      else
        idxU = i - 1;

      if (i == N - 1)
        idxB = 0;
      else
        idxB = i + 1;

      output.state[i] = Rokicki3Neighbors(state[i], tempxor[idxU], tempand[idxU], tempxor[idxB],
                            tempand[idxB]);
    }

    output.min = 0;
    output.max = N - 1;
    
    return output;
  }

  void Clear() {
    for (int i = 0; i < N; i++)
      state[i] = 0;

    min = 0;
    max = 0;
    gen = 0;
  }

  void Fill() {
    for (int i = 0; i < N; i++)
      state[i] = ~0ULL;
  }

  // From Page 5 of
  // https://www.gathering4gardner.org/g4g13gift/math/RokickiTomas-GiftExchange-LifeAlgorithms-G4G13.pdf
  uint64_t inline Rokicki(
      const uint64_t &a, //state row
      const uint64_t &bU0, //above row parity
      const uint64_t &bU1, //above row has at least 2
      const uint64_t &bB0, //below row parity
      const uint64_t &bB1 //below row has at least 2
    ) {
      uint64_t aw = RotateLeft(a); //left neighbor
      uint64_t ae = RotateRight(a); //right neighbor
      uint64_t s0 = aw ^ ae; //samerow has 1 neighbor
      uint64_t s1 = aw & ae; //samerow has 2 neighbors
      uint64_t ts0 = bB0 ^ bU0; //parity of diffrow neighbors
      uint64_t ts1 = (bB0 & bU0) | (ts0 & s0); //exactly 2 rows have neighbor parity 1
      return  ((bB1 ^ bU1 ^ ts1 ^ s1) & 
              ((bB1 | bU1) ^ (ts1 | s1)) &
              ((ts0 ^ s0) | a));
              //| (a & ~bU0 & ~bU1 & ~bB0 & ~bB1 & ~aw & ~ae);
              //| (~a & (ts0 ^ s0) & (bB1 ^ bU1 ^ ts1 ^ s1) & ~((bB1 | bU1) ^ (ts1 | s1))); //For B7 TEMPORARY
              //~(ts0 ^ s0): neighbor parity is 0
              //
  }

  uint64_t inline Rokicki1Neighbor(
    const uint64_t &a, //state row
    const uint64_t &bU0, //above row parity
    const uint64_t &bU1, //above row has at least 2
    const uint64_t &bB0, //below row parity
    const uint64_t &bB1 //below row has at least 2
  ) const {
    uint64_t aw = RotateLeft(a); //left neighbor
    uint64_t ae = RotateRight(a); //right neighbor
    uint64_t s0 = aw ^ ae; //samerow has 1 neighbor
    uint64_t s1 = aw & ae; //samerow has 2 neighbors

    uint64_t ts0 = bB0 ^ bU0; //parity of diffrow neighbors
    uint64_t ts1 = (bB0 & bU0) | (ts0 & s0); //at least 2 rows have neighbor parity 1

    return  (s0 ^ ts0) & //bit 0 of neighbor count
            ~(s1 | bU1 | bB1 | ts1); //not bit 1 or 2 of neighbor count
  }
  
  uint64_t inline Rokicki2Neighbors(
    const uint64_t &a, //state row
    const uint64_t &bU0, //above row parity
    const uint64_t &bU1, //above row has at least 2
    const uint64_t &bB0, //below row parity
    const uint64_t &bB1 //below row has at least 2
  ) const  {
    uint64_t aw = RotateLeft(a); //left neighbor
    uint64_t ae = RotateRight(a); //right neighbor
    uint64_t s0 = aw ^ ae; //samerow has 1 neighbor
    uint64_t s1 = aw & ae; //samerow has 2 neighbors

    uint64_t ts0 = bB0 ^ bU0; //parity of diffrow neighbors
    uint64_t ts1 = (bB0 & bU0) | (ts0 & s0); //at least 2 rows have neighbor parity 1

    return  (bB1 ^ bU1 ^ ts1 ^ s1) & //bit 1 of neighbor count
            ((bB1 | bU1) ^ (ts1 | s1)) & //not bit 2 of neighbor count
            ~(ts0 ^ s0); //not bit 2 of neighbor count
  }
  
  uint64_t inline Rokicki3Neighbors(
    const uint64_t &a, //state row
    const uint64_t &bU0, //above row parity
    const uint64_t &bU1, //above row has at least 2
    const uint64_t &bB0, //below row parity
    const uint64_t &bB1 //below row has at least 2
  ) const  {
    uint64_t aw = RotateLeft(a); //left neighbor
    uint64_t ae = RotateRight(a); //right neighbor
    uint64_t s0 = aw ^ ae; //samerow has 1 neighbor
    uint64_t s1 = aw & ae; //samerow has 2 neighbors

    uint64_t ts0 = bB0 ^ bU0; //parity of diffrow neighbors
    uint64_t ts1 = (bB0 & bU0) | (ts0 & s0); //at least 2 rows have neighbor parity 1

    return  (bB1 ^ bU1 ^ ts1 ^ s1) & //bit 1 of neighbor count
            ((bB1 | bU1) ^ (ts1 | s1)) & //not bit 2 of neighbor count
            (ts0 ^ s0); //not bit 2 of neighbor count
  }

  void Step();

  void Step(int numIters) {
    for (int i = 0; i < numIters; i++) {
      Step();
      // RemoveGliders();
    }
  }

  static int Parse(LifeState &state, const char *rle, int starti);

  static int Parse(LifeState &state, const char *rle, int dx, int dy) {
    if (Parse(state, rle, 0) == -1) {
      state.Move(dx, dy);
      return SUCCESS;
    } else {
      return FAIL;
    }
  }

  static int Parse(LifeState &state, const char *rle) {
    return Parse(state, rle, 0, 0);
  }

  static int Parse(LifeState &state, const char *rle, int dx, int dy,
                   SymmetryTransform transf) {
    int result = Parse(state, rle);

    if (result == SUCCESS)
      state.Transform(dx, dy, transf);

    return result;
  }

  static LifeState Parse(const char *rle, int dx, int dy,
                         SymmetryTransform trans) {
    LifeState result;
    Parse(result, rle);
    result.Transform(dx, dy, trans);

    return result;
  }

  static LifeState Parse(const char *rle, int dx, int dy) {
    LifeState result;
    Parse(result, rle, dx, dy);

    return result;
  }

  static LifeState Parse(const char *rle) { return Parse(rle, 0, 0); }

  static LifeState RandomState() {
    LifeState result;
    for (int i = 0; i < N; i++)
      result.state[i] = PRNG::rand64();

    result.RecalculateMinMax();

    return result;
  }

#ifdef __AVX2__
  // https://stackoverflow.com/questions/56153183/is-using-avx2-can-implement-a-faster-processing-of-lzcnt-on-a-word-array
  std::pair<int, int> FirstOn() const
  {
    const char *p = (const char *)state;
    size_t len = 8*N;
    //assert(len % 64 == 0);
    //optimal if p is 64-byte aligned, so we're checking single cache-lines
    const char *p_init = p;
    const char *endp = p + len;
    do {
      __m256i v1 = _mm256_loadu_si256((const __m256i*)p);
      __m256i v2 = _mm256_loadu_si256((const __m256i*)(p+32));
      __m256i vor = _mm256_or_si256(v1,v2);
      if (!_mm256_testz_si256(vor, vor)) {        // find the first non-zero cache line
        __m256i v1z = _mm256_cmpeq_epi32(v1, _mm256_setzero_si256());
        __m256i v2z = _mm256_cmpeq_epi32(v2, _mm256_setzero_si256());
        uint32_t zero_map = _mm256_movemask_ps(_mm256_castsi256_ps(v1z));
        zero_map |= _mm256_movemask_ps(_mm256_castsi256_ps(v2z)) << 8;

        unsigned idx = __builtin_ctz(~zero_map);  // Use ctzll for GCC, because GCC is dumb and won't optimize away a movsx
        uint32_t nonzero_chunk;
        memcpy(&nonzero_chunk, p+4*idx, sizeof(nonzero_chunk));  // aliasing / alignment-safe load
        if(idx % 2 == 0) {
          return std::make_pair((p-p_init + 4*idx)/8, __builtin_ctz(nonzero_chunk));
        } else {
          return std::make_pair((p-p_init + 4*(idx-1))/8, __builtin_ctz(nonzero_chunk) + 32);
        }
      }
      p += 64;
    } while(p < endp);
    return std::make_pair(-1, -1);
  }
#else
  std::pair<int, int> FirstOn() const {
    for (int x = 0; x < N; x++) {
      if (state[x] == 0ULL)
        continue;

      return std::make_pair(x, __builtin_ctzll(state[x]));
    }

    return std::make_pair(-1, -1);
  }
#endif

  static LifeState SolidRect(int x, int y, int w, int h) {
    uint64_t column;
    if (h < 64)
      column = RotateLeft(((uint64_t)1 << h) - 1, y);
    else
      column = ~0ULL;

    unsigned start, end;
    if (w < N) {
      start = (x + N) % N;
      end = (x + w + N) % N;
    } else {
      start = 0;
      end = N;
    }

    LifeState result;
    if (end > start) {
      for (unsigned int i = start; i < end; i++)
        result.state[i] = column;
    } else {
      for (unsigned int i = 0; i < end; i++)
        result.state[i] = column;
      for (unsigned int i = start; i < N; i++)
        result.state[i] = column;
    }
    return result;
  }

  std::tuple<LifeState, int, int> Standardized(StaticSymmetry symmetry) const {
    //get the lexicographically first translation
    std::tuple<LifeState, int, int> output = {*this, 0, 0};
    int maxX;
    int maxY;
    int diagonality;
    switch (symmetry) {
      case C1:
        maxX = 64;
        maxY = 64;
        diagonality = 0;
        break;
      case D2AcrossX:
      case D2AcrossXEven:
        maxX = 1;
        maxY = 64;
        diagonality = 0;
        break;
      case D2AcrossY:
      case D2AcrossYEven:
        maxX = 64;
        maxY = 1;
        diagonality = 0;
        break;
      case D2diagodd:
        maxX = 64;
        maxY = 1;
        diagonality = 1;
        break;
      case D2negdiagodd:
      case D2negdiagevenSubgroupsOnly:
        maxX = 64;
        maxY = 1;
        diagonality = -1;
        break;
      default:
        maxX = 0;
        maxY = 0;
        diagonality = 0;
        break;
    }
    for (int x = 0; x < maxX; x++) {
      for (int baseY = 0; baseY < maxY; baseY++) {
        int y = (baseY + diagonality * x) % 64;
        uint64_t testStateRow0 = RotateRight(state[x], y);
        if (testStateRow0 < std::get<0>(output).state[0]) {
          output = {*this, -x, -y};
          std::get<0>(output).Move(-x, -y);
        }
        else if (testStateRow0 == std::get<0>(output).state[0]) {
          LifeState testState = *this;
          testState.Move(-x, -y);
          if (testState < std::get<0>(output)) output = {std::move(testState), -x, -y};
        }
      }
    }
    return output;
  }

  std::tuple<LifeState, int, int, SymmetryTransform> StandardizedWithTransforms(StaticSymmetry symmetry) const {
    //get the lexicographically first transformation
    std::tuple<LifeState, int, int> output = Standardized(symmetry);
    SymmetryTransform outputTransform = Identity;
    std::vector<SymmetryTransform> useTransforms;
    switch (symmetry) {
      case C1:
        useTransforms = {ReflectAcrossX, ReflectAcrossY, ReflectAcrossYeqX, ReflectAcrossYeqNegXP1, Rotate90, Rotate180OddBoth, Rotate270};
        break;
      case C2:
        useTransforms = {ReflectAcrossX, ReflectAcrossY, Rotate90};
        break;
      case C2even:
        useTransforms = {ReflectAcrossXEven, ReflectAcrossYEven, Rotate90Even};
        break;
      case C2horizontaleven:
        useTransforms = {ReflectAcrossYEven};
        break;
      case C2verticaleven:
        useTransforms = {ReflectAcrossXEven};
        break;
      case C4:
        useTransforms = {ReflectAcrossX};
        break;
      case C4even:
        useTransforms = {ReflectAcrossXEven};
        break;
      case D2AcrossX:
      case D2AcrossXEven:
        useTransforms = {ReflectAcrossY};
        break;
      case D2AcrossY:
      case D2AcrossYEven:
        useTransforms = {ReflectAcrossX};
        break;
      case D2diagodd:
        useTransforms = {ReflectAcrossYeqNegXP1};
        break;
      case D2negdiagodd:
      case D2negdiagevenSubgroupsOnly:
        useTransforms = {ReflectAcrossYeqX};
        break;
      case D4:
      case D4diag:
        useTransforms = {Rotate90};
        break;
      case D4even:
      case D4diageven:
        useTransforms = {Rotate90Even};
        break;
      default:
        break;
    }
    for (auto &transform : useTransforms) {
      LifeState transformed = *this;
      transformed.Transform(transform);
      std::tuple<LifeState, int, int> testOutput = transformed.Standardized(symmetry);
      if (std::get<0>(testOutput) < std::get<0>(output)) {
        output = std::move(testOutput);
        outputTransform = transform;
      }
    }
    return {std::get<0>(output), std::get<1>(output), std::get<2>(output), outputTransform};
  }
};

void LifeState::Step() {
  uint64_t tempxor[N];
  uint64_t tempand[N];

  for (int i = 0; i < N; i++) {
    uint64_t l = RotateLeft(state[i]);
    uint64_t r = RotateRight(state[i]);
    tempxor[i] = l ^ r ^ state[i]; //parity of neighborhood
    tempand[i] = ((l | r) & state[i]) | (l & r); //are there at least 2 in neighborhood
  }

  //#pragma clang loop unroll(full)
  for (int i = 0; i < N; i++) {
    int idxU;
    int idxB;
    if (i == 0)
      idxU = N - 1;
    else
      idxU = i - 1;

    if (i == N - 1)
      idxB = 0;
    else
      idxB = i + 1;

    state[i] = Rokicki(state[i], tempxor[idxU], tempand[idxU], tempxor[idxB], tempand[idxB]);
  }

  // RecalculateMinMax();
  min = 0;
  max = N - 1;
  gen++;
}

void LifeState::Transform(SymmetryTransform transf) {
  switch (transf) {
  case Identity:
    break;
  case ReflectAcrossXEven:
    FlipX();
    break;
  case ReflectAcrossX:
    FlipX();
    Move(0, 1);
    break;
  case ReflectAcrossYEven:
    FlipY();
    break;
  case ReflectAcrossY:
    FlipY();
    Move(1, 0);
    break;
  case Rotate180EvenBoth:
    FlipX();
    FlipY();
    break;
  case Rotate180EvenVertical:
    FlipX();
    FlipY();
    Move(1, 0);
    break;
  case Rotate180EvenHorizontal:
    FlipX();
    FlipY();
    Move(0, 1);
    break;
  case Rotate180OddBoth:
    FlipX();
    FlipY();
    Move(1, 1);
    break;
  case ReflectAcrossYeqX:
    Transpose(false);
    break;
  case ReflectAcrossYeqNegX:
    Transpose(true);
    break;
  case ReflectAcrossYeqNegXP1:
    Transpose(true);
    Move(1, 1);
    break;
  case Rotate90Even:
    FlipX();
    Transpose(false);
    break;
  case Rotate90:
    FlipX();
    Transpose(false);
    Move(1, 0);
    break;
  case Rotate270Even:
    FlipY();
    Transpose(false);
    break;
  case Rotate270:
    FlipY();
    Transpose(false);
    Move(0, 1);
    break;
  }
}

void LifeState::Print() const {
  for (int j = 0; j < 64; j++) {
    for (int i = 0; i < N; i++) {
      if (GetCell(i - 32, j - 32) == 0) {
        printf(".");
      } else
        printf("O");
    }
    printf("\n");
  }

  printf("\n\n\n\n\n\n");
}

int LifeState::Parse(LifeState &state, const char *rle, int starti) {
  char ch;
  int cnt, i, j;
  int x, y;
  x = 0;
  y = 0;
  cnt = 0;

  i = starti;

  while ((ch = rle[i]) != '\0') {

    if (ch >= '0' && ch <= '9') {
      cnt *= 10;
      cnt += (ch - '0');
    } else if (ch == 'o') {

      if (cnt == 0)
        cnt = 1;

      for (j = 0; j < cnt; j++) {
        state.SetCell(x, y, 1);
        x++;
      }

      cnt = 0;
    } else if (ch == 'b') {
      if (cnt == 0)
        cnt = 1;

      x += cnt;
      cnt = 0;

    } else if (ch == '$') {
      if (cnt == 0)
        cnt = 1;

      if (cnt == 129)
        return i + 1;

      y += cnt;
      x = 0;
      cnt = 0;
    } else if (ch == '!') {
      break;
    } else {
      return -2;
    }

    i++;
  }

  state.RecalculateMinMax();

  return -1;
}

class LifeTarget {
public:
  LifeState wanted;
  LifeState unwanted;

  LifeTarget() {}
  LifeTarget(LifeState &state) {
    wanted = state;
    unwanted = state.GetBoundary();
  }

  static int Parse(LifeTarget &target, const char *rle, int x, int y,
                   SymmetryTransform transf) {
    LifeState Temp;
    int result = LifeState::Parse(Temp, rle, x, y, transf);

    if (result == SUCCESS) {
      target.wanted = Temp;
      target.unwanted = Temp.GetBoundary();
      return SUCCESS;
    }

    return FAIL;
  }

  static LifeTarget Parse(const char *rle, int x, int y,
                          SymmetryTransform transf) {
    LifeTarget target;
    Parse(target, rle, x, y, transf);
    return target;
  }

  static LifeTarget Parse(const char *rle, int x, int y) {
    return Parse(rle, x, y, Identity);
  }

  static LifeTarget Parse(const char *rle) { return Parse(rle, 0, 0); }
};

inline bool LifeState::Contains(const LifeTarget &target, int dx,
                                int dy) const {
  return Contains(target.wanted, dx, dy) &&
         AreDisjoint(target.unwanted, dx, dy);
}

inline bool LifeState::Contains(const LifeTarget &target) const {
  return Contains(target.wanted) && AreDisjoint(target.unwanted);
}

// typedef struct {
//   int *xList;
//   int *yList;
//   int len;
//   int allocated;
// } Locator;

// Locator *NewLocator() {
//   Locator *result = (Locator *)(malloc(sizeof(Locator)));

//   result->xList = (int *)(malloc(sizeof(int)));
//   result->yList = (int *)(malloc(sizeof(int)));
//   result->len = 0;
//   result->allocated = 1;

//   return result;
// }

// Locator *Realloc(Locator *locator) {
//   if (locator->allocated <= locator->len) {
//     locator->allocated *= 2;
//     locator->xList =
//         (int *)(realloc(locator->xList, locator->allocated * sizeof(int)));
//     locator->yList =
//         (int *)(realloc(locator->yList, locator->allocated * sizeof(int)));
//   }
//   return locator;
// }

// void Add(Locator *locator, int x, int y) {
//   Realloc(locator);

//   locator->xList[locator->len] = x;
//   locator->yList[locator->len] = y;
//   locator->len++;
// }

// Locator *State2Locator(LifeState *state) {
//   Locator *result = NewLocator();

//   for (int j = 0; j < N; j++) {
//     for (int i = 0; i < N; i++) {
//       int val = state->Get(i, j);

//       if (val == 1)
//         Add(result, i, j);
//     }
//   }

//   return result;
// }

// void ClearAtX(LifeState *state, Locator *locator, int x, uint64_t val) {
//   if (val == 0ULL)
//     return;

//   int len = locator->len;
//   int *xList = locator->xList;
//   int *yList = locator->yList;

//   for (int i = 0; i < len; i++) {
//     int idx = (x + xList[i] + N) % N;
//     int circulate = (yList[i] + 64) % 64;

//     state->state[idx] &= ~RotateLeft(val, circulate);
//   }
// }

// uint64_t LocateAtX(LifeState *state, Locator *locator, int x, int negate) {
//   uint64_t result = ~0ULL;
//   int len = locator->len;
//   int *xList = locator->xList;
//   int *yList = locator->yList;

//   for (int i = 0; i < len; i++) {
//     int idx = (x + xList[i] + N) % N;
//     int circulate = (yList[i] + 64) % 64;

//     if (negate == false)
//       result &= RotateRight(state->state[idx], circulate);
//     else
//       result &= ~RotateRight(state->state[idx], circulate);

//     if (result == 0ULL)
//       break;
//   }

//   return result;
// }

// uint64_t LocateAtX(LifeState *state, Locator *onLocator, Locator *offLocator,
//                    int x) {
//   uint64_t onLocate = LocateAtX(state, onLocator, x, false);

//   if (onLocate == 0)
//     return 0;

//   return onLocate & LocateAtX(state, offLocator, x, true);
// }

// void LocateInRange(LifeState *state, Locator *locator, LifeState *result,
//                    int minx, int maxx, int negate) {
//   for (int i = minx; i <= maxx; i++) {
//     result->state[i] = LocateAtX(state, locator, i, negate);
//   }
// }

// void LocateInRange(LifeState *state, Locator *onLocator, Locator *offLocator,
//                    LifeState *result, int minx, int maxx) {
//   for (int i = minx; i <= maxx; i++) {
//     result->state[i] = LocateAtX(state, onLocator, offLocator, i);
//   }
// }

// void Locate(LifeState *state, Locator *locator, LifeState *result) {
//   LocateInRange(state, locator, result, state->min, state->max, false);
// }

// typedef struct {
//   Locator *onLocator;
//   Locator *offLocator;
// } TargetLocator;

// TargetLocator *NewTargetLocator() {
//   TargetLocator *result = (TargetLocator *)(malloc(sizeof(TargetLocator)));

//   result->onLocator = NewLocator();
//   result->offLocator = NewLocator();

//   return result;
// }

// TargetLocator *Target2Locator(LifeTarget *target) {
//   TargetLocator *result = (TargetLocator *)(malloc(sizeof(TargetLocator)));
//   result->onLocator = State2Locator(target->wanted);
//   result->offLocator = State2Locator(target->unwanted);

//   return result;
// }

// uint64_t LocateAtX(LifeState *state, TargetLocator *targetLocator, int x) {
//   return LocateAtX(state, targetLocator->onLocator,
//   targetLocator->offLocator,
//                    x);
// }

// void LocateInRange(LifeState *state, TargetLocator *targetLocator,
//                    LifeState *result, int minx, int maxx) {
//   return LocateInRange(state, targetLocator->onLocator,
//                        targetLocator->offLocator, result, minx, maxx);
// }

// void LocateTarget(LifeState *state, TargetLocator *targetLocator,
//                   LifeState *result) {
//   LocateInRange(state, targetLocator, result, state->min, state->max);
// }

// static TargetLocator *_glidersTarget[4];

// int RemoveAtX(LifeState *state, int x, int startGiderIdx) {
//   int removed = false;

//   for (int i = startGiderIdx; i < startGiderIdx + 2; i++) {
//     uint64_t gld = LocateAtX(state, _glidersTarget[i], x);

//     if (gld != 0) {
//       removed = true;
//       ClearAtX(state, _glidersTarget[i]->onLocator, x, gld);

//       for (int j = 0; j < 64; j++) {
//         if (gld % 2 == 1) {
//           // Append(state->emittedGliders, "(");
//           // Append(state->emittedGliders, i);
//           // Append(state->emittedGliders, ",");
//           // Append(state->emittedGliders, j);
//           // Append(state->emittedGliders, ",");
//           // Append(state->emittedGliders, state->gen);
//           // Append(state->emittedGliders, ",");
//           // Append(state->emittedGliders, x);
//           // Append(state->emittedGliders, ")");
//         }

//         gld = gld >> 1;

//         if (gld == 0)
//           break;
//       }
//     }
//   }

//   return removed;
// }

// void RemoveGliders(LifeState *state) {
//   int removed = false;

//   if (state->min <= 1)
//     if (RemoveAtX(state, 1, 0) == true)
//       removed = true;

//   if (state->max >= N - 2)
//     if (RemoveAtX(state, N - 2, 2) == true)
//       removed = true;

//   if (removed == true)
//     state->RecalculateMinMax();
// }

// void New() {
//   if (GlobalState == NULL) {
//     GlobalState = NewState();

//     // for (int i = 0; i < CAPTURE_COUNT; i++) {
//     //   Captures[i] = NewState();
//     // }

//     _glidersTarget[0] = NewTargetLocator("2o$obo$o!");
//     _glidersTarget[1] = NewTargetLocator("o$obo$2o!");
//     _glidersTarget[2] = NewTargetLocator("b2o$obo$2bo!", -2, 0);
//     _glidersTarget[3] = NewTargetLocator("2bo$obo$b2o!", -2, 0);

//   } else {
//     Clear(GlobalState);
//   }
// }
