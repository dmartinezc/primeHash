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
	  std::size_t h4 = 0;
	  primeHash32(argv[1], l, 1, &h4);
	  high_resolution_clock::time_point t5 = high_resolution_clock::now();
	  std::size_t h5 = 0;
	  primeHash32(argv[1], l, 2, &h5);
	  high_resolution_clock::time_point t6 = high_resolution_clock::now();
	  for (int i = 0; i<10; i++) {
	       std::size_t v = 0;
	       std::size_t H = 0;
	       primeHash32(&v, i, 0, &H);
	       printf ("%d: %#10x\n", i, H);
	  }

	  auto stdTime = duration_cast<std::chrono::nanoseconds>(t2 - t1);
	  auto primeTimeStr = duration_cast<std::chrono::nanoseconds>(t3 - t2);
	  auto primeTimeBytes = duration_cast<std::chrono::nanoseconds>(t4 - t3);
	  auto strLength = duration_cast<std::chrono::nanoseconds>(tl - t3);
	  printf("stdhash \t %zx \t (%ld ns) \n",
		 h1, stdTime.count());
	  printf("myHashStr \t %zx \t (%ld ns)\n",
		 h2, primeTimeStr.count());
	  printf("myHashBytes \t %zx \t (%ld ns (strlen %ld hash: %ld) ) \n",
		 h3, primeTimeBytes.count(), strLength.count(),
		 (primeTimeBytes.count() - strLength.count()));
     }


     int keycount = 64*1024;

  printf("Keyset 'Zeroes' - %d keys\n",keycount);

  unsigned char * nullblock = new unsigned char[keycount];
  memset(nullblock,0,keycount);

  //----------

  std::vector<unsigned> hashes;

  hashes.resize(keycount);

  for(int i = 0; i < keycount; i++)
  {
       primeHash32(nullblock,i,0,&hashes[i]);
       printf("%#10x\n", hashes[i]);
  }


  printf("\n");

  delete [] nullblock;
     return 0;
}
