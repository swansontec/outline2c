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

objects = ast.o ast-builder.o debug.o file.o generate.o lexer.o main.o parser.o pool.o search.o string.o

# Inter-header dependencies:
ast_h      = ../source/ast.h $(typedefs_h) $(string_h)
ast_builder_h = ../source/ast-builder.h $(ast_h) $(pool_h)
debug_h    = ../source/debug.h $(typedefs_h)
file_h     = ../source/file.h $(typedefs_h)
generate_h = ../source/generate.h $(typedefs_h)
lexer_h    = ../source/lexer.h
parser_h   = ../source/parser.h $(typedefs_h)
pool_h     = ../source/pool.h $(typedefs_h)
search_h   = ../source/search.h $(typedefs_h) $(ast_h)
string_h   = ../source/string.h $(typedefs_h)
typedefs_h = ../source/typedefs.h

all: outline2c

ast.o: ../source/ast.c $(ast_h) $(pool_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/ast.c

ast-builder.o: ../source/ast-builder.c $(ast_builder_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/ast-builder.c

debug.o: ../source/debug.c $(debug_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/debug.c

file.o: ../source/file.c $(file_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/file.c

generate.o: ../source/generate.c $(generate_h) $(parser_h) $(ast_builder_h) $(search_h) $(debug_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/generate.c

lexer.o: ../source/lexer.c $(lexer_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/lexer.c

main.o: ../source/main.c $(file_h) $(string_h) $(generate_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/main.c

parser.o: ../source/parser.c $(parser_h) $(lexer_h) $(ast_builder_h) $(file_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/parser.c

pool.o: ../source/pool.c $(pool_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/pool.c

search.o: ../source/search.c $(search_h) $(file_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/search.c

string.o: ../source/string.c $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/string.c

outline2c: $(objects)
	gcc -g -o $@ $(objects)

clean: 
	rm -f $(objects) outline2c

install:
	cp outline2c /usr/bin/outline2c

run: outline2c
	./outline2c test.c.o2c
	diff test.c test.c.ref
