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

objects = ast.o ast-builder.o context.o debug.o filter.o generate.o lexer.o list.o main.o parser.o pool.o scope.o string.o

# Inter-header dependencies:
ast_h      = ../source/ast.h $(scope_h) $(list_h)
ast_builder_h = ../source/ast-builder.h $(ast_h)
context_h  = ../source/context.h $(type_h) $(string_h)
debug_h    = ../source/debug.h $(ast_h)
filter_h   = ../source/filter.h $(ast_h)
generate_h = ../source/generate.h $(string_h)
lexer_h    = ../source/lexer.h
list_h     = ../source/list.h $(type_h) $(pool_h)
parser_h   = ../source/parser.h $(context_h) $(ast_builder_h)
pool_h     = ../source/pool.h $(string_h)
scope_h    = ../source/scope.h $(type_h) $(pool_h)
string_h   = ../source/string.h
type_h     = ../source/type.h

all: outline2c

ast.o: ../source/ast.c $(ast_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/ast.c

ast-builder.o: ../source/ast-builder.c $(ast_builder_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/ast-builder.c

context.o: ../source/context.c $(context_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/context.c

debug.o: ../source/debug.c $(debug_h) $(scope_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/debug.c

filter.o: ../source/filter.c $(filter_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/filter.c

generate.o: ../source/generate.c $(generate_h) $(parser_h) $(filter_h) $(debug_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/generate.c

lexer.o: ../source/lexer.c $(lexer_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/lexer.c

list.o: ../source/list.c $(list_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/list.c

main.o: ../source/main.c $(string_h) $(generate_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/main.c

parser.o: ../source/parser.c $(parser_h) $(lexer_h) $(filter_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/parser.c

pool.o: ../source/pool.c $(pool_h) $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/pool.c

scope.o: ../source/scope.c $(scope_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/scope.c

string.o: ../source/string.c $(string_h)
	$(CC) $(CFLAGS) -o $@ -c ../source/string.c

outline2c: $(objects)
	gcc -g -o $@ $(objects)

clean: 
	rm -f $(objects) outline2c

install:
	cp outline2c /usr/bin/outline2c

run: outline2c
	./outline2c test.c.ol
	diff test.c.ref test.c
