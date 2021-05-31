// primeHash: a hash function rotating by prime numbers.
// Copyright (C) 2019  Diego Martinez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "../include/primeHash.hh"

#include <utility> // for std::move
#include <cstddef> // for size_t
#include <cstdint> // for uint64_t
#include <climits> // CHAR_BIT
#include <cassert> // for assert


namespace {

template <typename T>
struct Hash {
     Hash() : h() { }
     Hash(const T val) : h(val) { }

     explicit Hash(const T w, const unsigned bytes) :
	  h(bytes == sizeof(T) ?
	    w :
	    w & (~(((T)(-1)) << (bytes * CHAR_BIT))))
	  {
	  assert(bytes <= sizeof(T) &&
		 "packing too many bits");
     }
     explicit Hash(const char* str) :
	  h(*reinterpret_cast<const T*>(str))
     {
#ifndef NDEBUG
	  for (int i = 0; i < sizeof(T); i++) {
	       assert(str[i] && "not enough data");
	  }
#endif
     }
     explicit Hash(const char* str, const unsigned bytes) :
	  Hash(*reinterpret_cast<const T*>(str), bytes)
     {
	  assert(bytes <= sizeof(T) &&
		 "packing too many bits");
     }
     void operator=(T val) {
	  h = val;
     }

     T h;
}; // struct Hash

/// single precision data type
template <std::size_t bytes> struct S;
template<> struct S<4> { typedef std::uint32_t S_t; };
template<> struct S<8> { typedef std::uint64_t S_t; };

/// double precision data type
template <std::size_t bytes> struct D;
template<> struct D<4> { typedef std::uint64_t D_t; };
template<> struct D<8> {
     // non std :(, supported by gcc and clang
     typedef unsigned __int128 D_t;
};

template <unsigned N_CHARS>
struct Double {
     using S_t =  typename S<N_CHARS>::S_t;
     using D_t =  typename D<N_CHARS>::D_t;
     Double(const Double<N_CHARS>& other) :
	  u(other.u.h)
	  { }
     Double(const D_t val) :
	  u(val)
	  { }
     explicit Double(const S_t val, const S_t magic) :
	  u(((D_t)val) * ((D_t)magic))
	  { }
     explicit Double(const Hash<S_t> val, const S_t magic) :
	  Double(val.h, magic)
	  { }
     explicit Double(const Hash<S_t> val1, const Hash<S_t> val2) :
	  u(val1.h, val2.h)
	  { }
     explicit Double(const Double<N_CHARS>&& a) :
	  u(a.u.h)
	  { }
     explicit Double(const Double<N_CHARS>&& a,
		     const Double<N_CHARS>&& b) :
	  u(a.u.h ^ b.u.h)
	  { }
     explicit Double(const Double&& a, const Double&& b,
		     const Double&& c) :
	  u(a.u.h ^ b.u.h ^ c.u.h)
	  { }

     S_t reduce() const {
	  return u.ab[0] ^ u.ab[1];
     }

     union U {
	  U(D_t val) : h(val) { }
	  U(S_t val1, S_t val2) : ab({val1, val2}) { }
	  D_t h;
	  S_t ab[2]; // [msb, lsb]
	  static_assert(sizeof(D_t) == (2*sizeof(S_t)),
	       "data size miss alignment");
     } u;
};

/// 1 bit at the position of eah prime numbers
// 1 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61
template <std::size_t bytes> struct P;
template <> struct P<4> {
     // 32 bits 1 to 61
     static constexpr std::size_t p1 = 1u << (1 - 1);
     static constexpr std::size_t p2 = 1u << (2 - 1);
     static constexpr std::size_t p3 = 1u << (3 - 1);
     static constexpr std::size_t p5 = 1u << (5 - 1);
     static constexpr std::size_t p7 = 1u << (7 - 1);
     static constexpr std::size_t p11 = 1u << (11 - 1);
     static constexpr std::size_t p13 = 1u << (13 - 1);
     static constexpr std::size_t p17 = 1u << (17 - 1);
     static constexpr std::size_t p19 = 1u << (19 - 1);
     static constexpr std::size_t p23 = 1u << (23 - 1);
     static constexpr std::size_t p29 = 1u << (29 - 1);
     static constexpr std::size_t p31 = 1u << (31 - 1);
};

template <>struct P<8> : P<4> {
     // 64 bits previous + 37 to 61
     static constexpr S<8>::S_t p37 = ((S<8>::S_t)1) << (37 - 1);
     static constexpr S<8>::S_t p41 = ((S<8>::S_t)1) << (41 - 1);
     static constexpr S<8>::S_t p43 = ((S<8>::S_t)1) << (43 - 1);
     static constexpr S<8>::S_t p47 = ((S<8>::S_t)1) << (47 - 1);
     static constexpr S<8>::S_t p53 = ((S<8>::S_t)1) << (53 - 1);
     static constexpr S<8>::S_t p59 = ((S<8>::S_t)1) << (59 - 1);
     static constexpr S<8>::S_t p61 = ((S<8>::S_t)1) << (61 - 1);

};

/// magic numbers
template <std::size_t bytes> struct M;
template<> struct M<4> : P<4> {
     static constexpr S<4>::S_t hi = 0x80808080;
     static constexpr S<4>::S_t lo = 0x01010101;

     static constexpr S<4>::S_t Magic1 = p1 |
         p3 | p5 | p7 | p11 | p13 | p17 | p19 | p23| p29;

     static constexpr S<4>::S_t Magic2a = p3 | p7 | p13 | p19 | p29;
     static constexpr S<4>::S_t Magic2b = p5 | p11 | p17 | p23 | p31;

     static constexpr S<4>::S_t Magic3a = p7 | p17 | p29;
     static constexpr S<4>::S_t Magic3b = p5 | p13 | p23;
     static constexpr S<4>::S_t Magic3c = p3 | p11 | p19 | p31;

};
template<> struct M<8> : P<8> {
     static constexpr S<8>::S_t hi = 0x8080808080808080;
     static constexpr S<8>::S_t lo = 0x0101010101010101;

     static constexpr S<8>::S_t Magic1 = (p1 | p3 | p5 | p7 | p11 | p13 |
					  p17 | p19 | p23| p29 |
					  p31 | p37 | p41 | p43 |
					  p47 | p53 | p59 | p61);

     static constexpr S<8>::S_t Magic2a =
          (p5 | p11 | p17 | p23 | p31 | p41 | p47 | p59);
     static constexpr S<8>::S_t Magic2b =
          (p3 | p7 | p13 | p19 | p29 | p37 | p43 | p53 | p61);

     static constexpr S<8>::S_t Magic3a = p7 | p17 | p29 | p41 | p53;
     static constexpr S<8>::S_t Magic3b = p5 | p13 | p23 | p37 | p47 | p61;
     static constexpr S<8>::S_t Magic3c = p3 | p11 | p19 | p31 | p43 | p59;
};


template <std::size_t bytes> struct Seed;
/// 1 / (golden ratio) in 32 bits.
template<> struct Seed<4> {
     // single precision. 32 bits.
     static constexpr S<4>::S_t Single = 0x7a371e3f;
     // double precision. 64 bits.
     static constexpr D<4>::D_t Double = 0x4fe92f37efc6e33f;
};
/// 1 / (golden ratio) in 64 bits.
template<> struct Seed<8> {
     // single precision. 64 bits.
     static constexpr S<8>::S_t Single =  0x4fe92f37efc6e33f;

     // double precision. 128 bits. bummer, the compiler wont allow a 128 bits constexpr.
     static constexpr S<8>::S_t Doublea = 0xdbb9c08039e72bf8;
     static constexpr S<8>::S_t Doubleb = 0x94fe72f36e3cfe3f;
};

S<4>::S_t getSeed(S<4>::S_t /*ignore*/) {
     return S<4>::S_t{0x7a371e3f};
}
S<8>::S_t getSeed(S<8>::S_t /*ignore*/) {
     return S<8>::S_t{0x4fe92f37efc6e33f};
}

///@note bummer, c++ don't accept constexprs that cannot be
/// represented with native datatype. use a function call returning an
/// expression and let the compiler optimize it
Double<4> getSeed(Double<4> /*ignore*/) {
     return Double<4>{0x4fe92f37efc6e33f};
}
Double<8> getSeed(Double<8> /*ignore*/) {
     // return 0xdbb9c08039e72bf894fe72f36e3cfe3f;
     return Double<8>{(D<8>::D_t)0xdbb9c08039e72bf8 << 64u | 0x94fe72f36e3cfe3f};
}


std::size_t
scramble(const Hash<std::size_t> h)
{
     constexpr unsigned N_CHARS = sizeof(std::size_t);
     return Double<N_CHARS>{h, M<N_CHARS>::Magic1}.reduce();
}

template<typename T>
T
combine(const Hash<T> a, const Hash<T> b)
{
     constexpr unsigned N_CHARS = sizeof(T);
//     T h = Double<N_CHARS>{b, M<N_CHARS>::Magic1}.reduce();
//     return a.h ^ h;
     /*

     return Double<N_CHARS>{Double<N_CHARS>{a, M<N_CHARS>::Magic2a},
			    Double<N_CHARS>{b, M<N_CHARS>::Magic2b}
			    }.reduce();
     */
     return Double<N_CHARS>{Double<N_CHARS>{a, M<N_CHARS>::Magic1},
			    Double<N_CHARS>{b, M<N_CHARS>::Magic1}
			    }.reduce();

//     Double<N_CHARS>{b, M<N_CHARS>::Magic1}.reduce();
}

/// generic version implementing the interface of SMHasher
/// basically std::size_t primeHash(const void* ptr, std::size_t size)
/// with a seed (that is also considered for the initial hash value)
template<typename T>
void
primeHash(const void* key, int len, uint32_t seed, void* out)
{
     constexpr unsigned N_CHARS = sizeof(T);
     if (len <= 0) {
	  // bad arguments, nothing to do.
	  *(T*)out = seed;
	  return;
     }
     assert((key != nullptr) && "bad key ptr.");

     // internal state, start with a seed.
     Hash<T> u{getSeed(typename S<N_CHARS>::S_t(0))};
     unsigned size = len;
     u = combine(u, Hash<T>{size});
     u = combine(u, Hash<T>{seed});

     T* wordData = (T*)(key);
     // Consume N_CHARS at a time while doing the combine.
     ///@note for 32 bits hash we could still consume every 64 bits if
     /// supported, but lets make it fair.
     while (size > N_CHARS) {
	  size -= N_CHARS;
	  u = combine(u, Hash<T>{*wordData++});
     }

     // handle the bits in the tail.
     if (size > 0) {
	  u = combine(u, Hash<T>{*wordData, size});
     }
     

     *(T*)out = u.h;
}

} // anonymous namespace


std::size_t
primeHash(const void* ptr, std::size_t size)
{
     constexpr unsigned N_CHARS = sizeof(std::size_t);
     Hash<std::size_t> u{size}; // size is the initial seed.
     if (size <= N_CHARS) {
	  ///@note all the data fits in size_t
	  // Scramble the bits anyway to accommodate for hash
	  // tables growing by powers of 2
	  return combine(u, Hash<std::size_t>{(char*)ptr,
					      (unsigned)size});
     }

     // Consume N_CHARS at a time while doing the combine.
     std::size_t* wordData = (std::size_t*)(ptr);
     while (size > N_CHARS) {
	  size -= N_CHARS;
	  u = combine(u, Hash<std::size_t>{*wordData++});
     }

     // handle the bits in the tail.
     if (size > 0) {
	  u = combine(u, Hash<std::size_t>{*wordData, (unsigned)size});
     }
     
     return u.h;
}

std::size_t
primeHash(const char* str)
{
     constexpr unsigned N_CHARS = sizeof(std::size_t);

     // consume N_CHARS at a time while doing the combine.
     std::size_t* wordPtr = (std::size_t*)(str);
     std::size_t curr = *wordPtr++;
     if ((curr - M<N_CHARS>::lo) & ~curr & M<N_CHARS>::hi) {
	  // current word might have a zero byte
	  for (unsigned i = 1; i < N_CHARS; i++) {
	       if (!str[i]) {
		    ///@note all the string fits in size_t
		    // Scramble the bits anyway to accommodate for
		    // hash tables growing by powers of 2
		    return scramble(Hash<std::size_t>{str, i});
	       }
	  }
     }
     
     Hash<std::size_t> u{curr};
     while (true) {
	  curr = *wordPtr++;
	  if ((curr - M<N_CHARS>::lo) & ~curr & M<N_CHARS>::hi) {
	       // current word might have a zero byte
	       str = (const char*)(wordPtr - 1);
	       if (str[0] == '\0') {
		    break; // no more data.
	       }
	       for (unsigned i = 1; i < N_CHARS; i++) {
		    if (!str[i]) {
			 // found the last '\0'
			 u = combine(u, Hash<std::size_t>{str, i});
			 break;
		    }
	       }
	       // else false positive...
	  }
	  // ...continue with the next word
	  u = combine(u, Hash<std::size_t>{curr});
     }

     return u.h;
}


/// 64 bits hash value implementing the interface of SMHasher
void
primeHash64(const void* key, int len, uint32_t seed, void* out)
{
     primeHash<uint64_t>(key, len, seed, out);
}

/// 32 bits hash value implementing the interface of SMHasher
void
primeHash32(const void* key, int len, uint32_t seed, void* out)
{
     primeHash<uint32_t>(key, len, seed, out);
}

