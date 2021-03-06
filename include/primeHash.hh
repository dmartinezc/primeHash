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


#include <cstddef> // for size_t
#include <cstdint> // for uint32_t

std::size_t primeHash(const void* ptr, std::size_t size);
std::size_t primeHash(const char* str);

void primeHash32(const void* key, int len, uint32_t seed, void* out);
void primeHash64(const void* key, int len, uint32_t seed, void* out);
