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

#include "stdio.h"

#include <cstddef> // for size_t
#include <cstdint> // for uint64_t
#include <climits> // CHAR_BIT
#include <cassert> // for assert

#include <string_view>
#include <functional> // hash
#include <chrono>

namespace {

constexpr std::size_t N_CHARS = sizeof(std::size_t);

struct Hash {
     Hash() : h(0) { }
     explicit Hash(const std::size_t val) : h(val) { }

     explicit Hash(const std::size_t w, const unsigned bytes) :
	  h(w &
	    ~(((std::size_t)(-1)) << (bytes * CHAR_BIT)))
     {
	  assert(bytes <= N_CHARS &&
		 "packing too many bits");
	  assert(bytes >= 1 &&
		 "redundant, should be calling the other ctor");
     }
     explicit Hash(const char* str) :
	  h(*reinterpret_cast<const std::size_t*>(str))
     {
#ifndef NDEBUG
	  for (int i = 0; i < N_CHARS; i++) {
	       assert(str[i] && "not enough data");
	  }
#endif
     }
     explicit Hash(const char* str, const unsigned bytes) :
	  h((*reinterpret_cast<const std::size_t*>(str)) &
	    ~(((std::size_t)(-1)) << (bytes * CHAR_BIT)))
     {
	  assert(bytes <= N_CHARS &&
		 "packing too many bits");
	  assert(bytes >= 1 &&
		 "redundant, should be calling the other ctor");
     }
     void operator=(std::size_t val) {
	  h = val;
     }

     std::size_t h;
}; // struct Hash

/// single pressition data type
template <std::size_t bytes> struct S;
template<> struct S<4> { typedef std::uint32_t S_t; };
template<> struct S<8> { typedef std::uint64_t S_t; };

/// double pressition data type
template <std::size_t bytes> struct D;
template<> struct D<4> { typedef std::uint64_t D_t; };
template<> struct D<8> { typedef unsigned __int128 D_t; };

struct Double {
     using D_t =  D<N_CHARS>::D_t;

     explicit Double(const Hash val, const std::size_t magic) :
	  u(((D_t)val.h) * ((D_t)magic))
	  { }

     std::size_t reduce() const {
	  return u.ab[0] ^ u.ab[1];
     }
     union U {
	  U(D_t val) : h(val) { }
	  D_t h;
	  std::size_t ab[2]; // [msb, lsb]
	  static_assert(sizeof(D_t) == (2*sizeof(std::size_t)),
	       "data size miss alignment");
     } u;
};

/// Prime numbers
struct P {
     // 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61
     static constexpr std::size_t p3 = 1u << 3;
     static constexpr std::size_t p5 = 1u << 5;
     static constexpr std::size_t p7 = 1u << 7;
     static constexpr std::size_t p11 = 1u << 11;
     static constexpr std::size_t p13 = 1u << 13;
     static constexpr std::size_t p17 = 1u << 17;
     static constexpr std::size_t p19 = 1u << 19;
     static constexpr std::size_t p23 = 1u << 23;
     static constexpr std::size_t p29 = 1u << 29;
     static constexpr std::size_t p31 = 1u << 31;
};

/// magic numbers
template <std::size_t bytes> struct M;
template<> struct M<4> : P {
     static constexpr std::size_t hi = 0x80808080;
     static constexpr std::size_t lo = 0x01010101;

     static constexpr std::size_t Magic1 =
	  p3 | p5 | p7 | p11 | p13 | p17 | p19 | p23| p29 | p31;

     static constexpr std::size_t Magic2a = p3 | p7 | p13 | p19 | p29;
     static constexpr std::size_t Magic2b = p5 | p11 | p17 | p23 | p31;

     static constexpr std::size_t Magic3a = p3 | p11 | p19; // | p31?
     static constexpr std::size_t Magic3b = p5 | p13 | p23;
     static constexpr std::size_t Magic3c = p7 | p17 | p29;

     static constexpr std::size_t Magic4a = p3 | p13; // | p29 ?
     static constexpr std::size_t Magic4b = p5 | p17; // | p31 ?
     static constexpr std::size_t Magic4c = p7 | p19;
     static constexpr std::size_t Magic4d = p11 | p23;

     static constexpr std::size_t Magic5a = p3 | p17;
     static constexpr std::size_t Magic5b = p5 | p19;
     static constexpr std::size_t Magic5c = p7 | p23;
     static constexpr std::size_t Magic5d = p11 | p29;
     static constexpr std::size_t Magic5e = p13 | p31;
};
template<> struct M<8> : P {
     static constexpr std::size_t hi = 0x8080808080808080;
     static constexpr std::size_t lo = 0x0101010101010101;

     static constexpr S<8>::S_t p37 = ((S<8>::S_t)1) << 37;
     static constexpr S<8>::S_t p41 = ((S<8>::S_t)1) << 41;
     static constexpr S<8>::S_t p43 = ((S<8>::S_t)1) << 43;
     static constexpr S<8>::S_t p47 = ((S<8>::S_t)1) << 47;
     static constexpr S<8>::S_t p53 = ((S<8>::S_t)1) << 53;
     static constexpr S<8>::S_t p59 = ((S<8>::S_t)1) << 59;
     static constexpr S<8>::S_t p61 = ((S<8>::S_t)1) << 61;

     static constexpr std::size_t Magic1 = (p3 | p5 | p7 | p11 | p13 |
					    p17 | p19 | p23| p29 |
					    p31 | p37 | p41 | p43 |
					    p47 | p53 | p59 | p61);

     static constexpr S<8>::S_t Magic2a =
            (p3 | p7 | p13 | p19 | p29 | p37 | p43 | p53); // p61?
     static constexpr S<8>::S_t Magic2b =
            (p5 | p11 | p17 | p23 | p31 | p41 | p47 | p59);

     static constexpr S<8>::S_t Magic3a = p3 | p11 | p19 | p31 | p43; // | p59 ?
     static constexpr S<8>::S_t Magic3b = p5 | p13 | p23 | p37 | p47; // | p61 ?
     static constexpr S<8>::S_t Magic3c = p7 | p17 | p29 | p41 | p53;

     static constexpr S<8>::S_t Magic4a = p3 | p13 | p29 | p43; // | p61 ?
     static constexpr S<8>::S_t Magic4b = p5 | p17 | p31 | p47;
     static constexpr S<8>::S_t Magic4c = p7 | p19 | p37 | p53;
     static constexpr S<8>::S_t Magic4d = p11 | p23 | p41 | p59;

     static constexpr std::size_t Magic5a = p3 | p17 | p37; // | p59 ?
     static constexpr std::size_t Magic5b = p5 | p19 | p41; // | p61 ?
     static constexpr std::size_t Magic5c = p7 | p23 | p43;
     static constexpr std::size_t Magic5d = p11 | p29 | p47;
     static constexpr std::size_t Magic5e = p13 | p31 | p53;
};

std::size_t
combine(Hash a, Hash b)
{
     using Mn = M<N_CHARS>;
     return (Double{a, Mn::Magic2a}.reduce() ^
	     Double{b, Mn::Magic2b}.reduce());
}

std::size_t
combine(Hash a, Hash b, Hash c)
{
     using Mn = M<N_CHARS>;
     return (Double{a, Mn::Magic3a}.reduce() ^
	     Double{b, Mn::Magic3b}.reduce() ^
	     Double{c, Mn::Magic3c}.reduce());
}

std::size_t
combine(Hash a, Hash b, Hash c, Hash d)
{
     using Mn = M<N_CHARS>;
     return (Double{a, Mn::Magic4a}.reduce() ^
	     Double{b, Mn::Magic4b}.reduce() ^
	     Double{c, Mn::Magic4c}.reduce() ^
	     Double{d, Mn::Magic4d}.reduce());
}

} // anonymous namespace

/* WIP
std::size_t
primeHash(const void* ptr, const std::size_t size) {

     unsigned i;
     for (i = 0;
	  ((std::size_t)(str + i)) & (N_CHARS - 1); // size_t alignment
	  i++) {
	  if (!str[i]) {
	       ///@note scramble the bits anyway
	       return Double{Hash{str, i},
			     M<N_CHARS>::Magic1
			     }.reduce();
	  }
     }

     // Now 'wordPtr' points to a size_t aligned word.
     std::size_t* wordPtr = (std::size_t*)(str + i);

     // Start the initial seed with the first bytes and consume
     // N_CHARS at a time while doing the combine..
     Hash u = (i == 0 ? Hash{*wordPtr++} : Hash{str, i});



     while (true) {
	  std::size_t curr = *wordPtr++;
	  if ((curr - M<N_CHARS>::lo) & ~curr & M<N_CHARS>::hi) {
	       // current word might have a zero byte
	       str = (const char*)(wordPtr - 1);

	       for (i = 0; i < N_CHARS; i++) {
		    if (!str[i]) {
			 // found the last '\0'
			 break;
		    }
	       }
	       if (!str[i]) {
		    return combine(u, Hash{curr, i});
	       }
	       // else false positive...
	  }
	  // ...continue with the next word
	  u = combine(u, Hash{curr});
     }
}
*/


std::size_t
primeHash(const char* str) {

     Hash u{};
     // Handle the first bytes until str is aligned with a full word
     // size_t pointer
     unsigned i;
     for (i = 0; ;i++) {
	  if (!(((std::size_t)(str + i)) & (N_CHARS - 1))) {
	       // size_t alignment
	       if (i > 0) {
		    // Start the initial seed with the first bytes
		    u = Hash{str, i};
	       }
	       break;
	  }
	  if (!str[i]) {
	       ///@note all the data fits in size_t
	       /// scramble the bits anyway
	       return Double{Hash{str, i},
			     M<N_CHARS>::Magic1}.reduce();
	  }
     }

     // Now 'wordPtr' points to a size_t aligned word.
     // consume N_CHARS at a time while doing the combine.
     std::size_t* wordPtr = (std::size_t*)(str + i);

     while (true) {
	  std::size_t curr = *wordPtr++;
	  if ((curr - M<N_CHARS>::lo) & ~curr & M<N_CHARS>::hi) {
	       // current word might have a zero byte
	       str = (const char*)(wordPtr - 1);
	       for (i = 0; i < N_CHARS; i++) {
		    if (!str[i]) {
			 // found the last '\0'
			 return combine(u, Hash{curr, i});
		    }
	       }
	       // else false positive...
	  }
	  // ...continue with the next word
	  u = combine(u, Hash{curr});
     }
}

std::size_t stdHash(const char* str) {

     return std::hash<std::string_view>{}(str);
}

int main(int argc, char *argv[]) {
     if (argv[1]) {
	  using namespace std::chrono;

	  high_resolution_clock::time_point t1 = high_resolution_clock::now();
	  std::size_t h1 =  stdHash(argv[1]);
	  high_resolution_clock::time_point t2 = high_resolution_clock::now();
	  std::size_t h2 = primeHash(argv[1]);
	  high_resolution_clock::time_point t3 = high_resolution_clock::now();

	  printf("stdhash (%d): %zd %zx %zo, myHash (%d): %zd %zx %zo\n",
		 t2 - t1, h1, h1, h1,
		 t3 - t2, h2, h2, h2);
     }
     return 0;
}
