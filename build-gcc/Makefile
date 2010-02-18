#
# Copyright 2010 William R. Swanson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

objects = debug.o file.o lexer.o match.o outline.o parser.o pattern.o string.o main.o
cflags = -g -std=c89 -pedantic

all: outline2c

debug.o: ../source/debug.c ../source/string.h ../source/debug.h ../source/match.h ../source/pattern.h ../source/outline.h ../source/typedefs.h
	gcc $(cflags) -c -o debug.o ../source/debug.c

file.o: ../source/file.c ../source/file.h ../source/typedefs.h
	gcc $(cflags) -c -o file.o ../source/file.c

lexer.o: ../source/lexer.c ../source/lexer.h ../source/string.h ../source/typedefs.h
	gcc $(cflags) -c -o lexer.o ../source/lexer.c

match.o: ../source/match.c ../source/match.h ../source/pattern.h ../source/file.h ../source/string.h ../source/outline.h ../source/typedefs.h
	gcc $(cflags) -c -o match.o ../source/match.c

outline.o: ../source/outline.c ../source/outline.h ../source/typedefs.h
	gcc $(cflags) -c -o outline.o ../source/outline.c

parser.o: ../source/parser.c ../source/parser.h ../source/file.h ../source/string.h ../source/debug.h ../source/lexer.h ../source/match.h  ../source/pattern.h ../source/outline.h ../source/typedefs.h
	gcc $(cflags) -c -o parser.o ../source/parser.c

pattern.o: ../source/pattern.c ../source/pattern.h ../source/string.h ../source/outline.h ../source/typedefs.h
	gcc $(cflags) -c -o pattern.o ../source/pattern.c

string.o: ../source/string.c ../source/string.h ../source/typedefs.h
	gcc $(cflags) -c -o string.o ../source/string.c

main.o: ../source/main.c ../source/lexer.h ../source/parser.h ../source/outline.h ../source/file.h ../source/string.h ../source/typedefs.h
	gcc $(cflags) -c -o main.o ../source/main.c

outline2c: $(objects)
	gcc -g -o outline2c $(objects)

clean: 
	rm -f $(objects) outline2c

run: outline2c
	./outline2c test.c.o2c
