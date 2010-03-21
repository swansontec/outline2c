/*
 * Copyright 2010 William R. Swanson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GENERATE_H_INCLUDED
#define GENERATE_H_INCLUDED

#include "ast.h"

int generate(FileW *out, char const *filename);

int generate_code(FileW *out, Scope *s, AstCode *p);
int generate_for(FileW *out, Scope *s, AstFor *p);
int generate_symbol(FileW *out, Scope *s, AstSymbol *p);
int generate_lookup(FileW *out, Scope *s, AstLookup *p);
int generate_lookup_tag(FileW *out, Scope *s, AstLookup *p);
int generate_lookup_builtin(FileW *out, Scope *s, AstLookup *p);

#endif
