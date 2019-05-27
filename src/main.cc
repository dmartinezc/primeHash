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

/// poor man's testing

#include "include/primeHash.hh"

#include <string_view>
#include <functional> // hash
#include <chrono>
#include <string.h> // for strlen()
#include "stdio.h"

std::size_t
stdHash(const char* str)
{
     return std::hash<std::string_view>{}(str);
}

int
main(int argc, char *argv[])
{
     if (argv[1]) {
	  using namespace std::chrono;

	  high_resolution_clock::time_point t1 = high_resolution_clock::now();
	  std::size_t h1 =  stdHash(argv[1]);
	  high_resolution_clock::time_point t2 = high_resolution_clock::now();
	  std::size_t h2 = primeHash(argv[1]);
	  high_resolution_clock::time_point t3 = high_resolution_clock::now();
	  auto l = strlen(argv[1]);
	  high_resolution_clock::time_point tl = high_resolution_clock::now();
	  std::size_t h3 = primeHash(argv[1], l);
	  high_resolution_clock::time_point t4 = high_resolution_clock::now();

	  auto stdTime = duration_cast<std::chrono::nanoseconds>(t2 - t1);
	  auto primeTimeStr = duration_cast<std::chrono::nanoseconds>(t3 - t2);
	  auto primeTimeBytes = duration_cast<std::chrono::nanoseconds>(t4 - t3);
	  auto strLength = duration_cast<std::chrono::nanoseconds>(tl - t3);
	  printf("stdhash (%ld ns):\t  %zx , myHashStr (%ld ns):\t %zx , myHashBytes (%ld ns (%ld) (%ld) ):\t%zx \n",
		 stdTime.count(), h1,
		 primeTimeStr.count(), h2,
		 primeTimeBytes.count(), strLength.count(), (primeTimeBytes.count() - strLength.count()),
		 h3);
     }
     return 0;
}
