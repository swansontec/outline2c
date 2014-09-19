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

CFLAGS = -g -ansi -pedantic -Wall

default: outline2c

-include outline2c.d
outline2c: ../source/outline2c.c
	$(CC) $(CFLAGS) -MMD -o $@ $<

test: outline2c
	./outline2c ../build-gcc/test.c.ol
	diff test.c.ref test.c

run: outline2c
	./outline2c -d test.c.ol

install: outline2c
	cp outline2c /usr/local/bin/

clean:
	rm -f *.d
	rm -f outline2c
	rm -f test.c
