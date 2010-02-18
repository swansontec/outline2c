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

CC = gcc
CFLAGS = -g -std=c89 -pedantic

objects = debug.o file.o lexer.o main.o match.o outline.o parser.o pattern.o string.o

# Inter-header dependencies:
debug_h    = ../source/debug.h $(typedefs_h)
file_h     = ../source/file.h $(typedefs_h)
lexer_h    = ../source/lexer.h
match_h    = ../source/match.h $(typedefs_h) $(string_h) $(pattern_h)
outline_h  = ../source/outline.h $(typedefs_h)
parser_h   = ../source/parser.h $(typedefs_h) $(lexer_h) $(outline_h) $(string_h)
pattern_h  = ../source/pattern.h $(typedefs_h) $(string_h)
string_h   = ../source/string.h $(typedefs_h)
typedefs_h = ../source/typedefs.h

all: outline2c

debug.o: ../source/debug.c $(debug_h) $(outline_h) $(match_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/debug.c

file.o: ../source/file.c $(file_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/file.c

lexer.o: ../source/lexer.c $(lexer_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/lexer.c

main.o: ../source/main.c $(parser_h) $(file_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/main.c

match.o: ../source/match.c $(match_h) $(string_h) $(outline_h) $(file_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/match.c

outline.o: ../source/outline.c $(outline_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/outline.c

parser.o: ../source/parser.c $(parser_h) $(outline_h) $(match_h) $(lexer_h) $(file_h) $(string_h) $(debug_h) $(file_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/parser.c

pattern.o: ../source/pattern.c $(pattern_h) $(outline_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/pattern.c

string.o: ../source/string.c $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/string.c

outline2c: $(objects)
	gcc -g -o $@ $(objects)

clean: 
	rm -f $(objects) outline2c

run: outline2c
	./outline2c test.c.o2c
