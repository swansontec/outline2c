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

all: outline2c

debug.o: ../source/debug.c ../source/string.h ../source/debug.h ../source/match.h ../source/outline.h ../source/typedefs.h
	gcc -c -o debug.o ../source/debug.c -g -std=c89 -pedantic

file.o: ../source/file.c ../source/file.h ../source/typedefs.h
	gcc -c -o file.o ../source/file.c -g -std=c89 -pedantic

lexer.o: ../source/lexer.c ../source/lexer.h ../source/string.h ../source/typedefs.h
	gcc -c -o lexer.o ../source/lexer.c -g -std=c89 -pedantic

match.o: ../source/match.c ../source/match.h ../source/file.h ../source/string.h ../source/outline.h ../source/typedefs.h
	gcc -c -o match.o ../source/match.c -g -std=c89 -pedantic

outline.o: ../source/outline.c ../source/outline.h ../source/typedefs.h
	gcc -c -o outline.o ../source/outline.c -g -std=c89 -pedantic

parser.o: ../source/parser.c ../source/parser.h ../source/file.h ../source/string.h ../source/debug.h ../source/lexer.h ../source/match.h ../source/outline.h ../source/typedefs.h
	gcc -c -o parser.o ../source/parser.c -g -std=c89 -pedantic

string.o: ../source/string.c ../source/string.h ../source/typedefs.h
	gcc -c -o string.o ../source/string.c -g -std=c89 -pedantic

main.o: ../source/main.c ../source/lexer.h ../source/parser.h ../source/outline.h ../source/file.h ../source/string.h ../source/typedefs.h
	gcc -c -o main.o ../source/main.c -g -std=c89 -pedantic

outline2c: debug.o file.o lexer.o match.o outline.o parser.o string.o main.o
	gcc -o outline2c debug.o file.o lexer.o match.o outline.o parser.o string.o main.o -g

clean: 
	rm -f debug.o file.o lexer.o match.o outline.o parser.o string.o main.o outline2c

run: outline2c
	./outline2c test.c.o2c
