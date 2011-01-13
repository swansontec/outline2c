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

compile = gcc -g -std=c89 -pedantic -o $@ $<

default: outline2c

outline2c: ../source/outline2c.c \
	../source/ast.c \
	../source/debug.c \
	../source/filter.c \
	../source/generate.c \
	../source/lexer.c \
	../source/list.c \
	../source/lwl.c \
	../source/main.c \
	../source/options.c \
	../source/out.c \
	../source/parser.c \
	../source/pool.c \
	../source/scope.c \
	../source/source.c \
	../source/string.c \
	../source/type.c \
	; $(compile)

test: outline2c
	./outline2c ../build-gcc/test.c.ol
	diff test.c.ref test.c

run: outline2c
	./outline2c -d test.c.ol

install: outline2c
	cp outline2c /usr/local/bin/

clean:
	rm -f outline2c
