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

/**
 * Accepts an output value. Parser functions only use their return value to
 * indicate success or failure. To output data, such as AST nodes, they call
 * the current output routine. Calling the routine multiple times allows a
 * parser function to produce multiple output items, which is not possible
 * with ordinary return values.
 */
typedef struct {
  int (*code)(void *data, Dynamic value);
  void *data;
} OutRoutine;

static int out_dynamic_fn(void *data, Dynamic value)
{
  Dynamic *out = data;
  if (dynamic_ok(*out)) return 0;
  if (!dynamic_ok(value)) return 0;
  *out = value;
  return 1;
}

/**
 * Produces an output routine which captures a single return value in a
 * Dynamic variable. Attempting to return more than one value will produce
 * an error in the output routine.
 */
OutRoutine out_dynamic(Dynamic *out)
{
  OutRoutine self;
  self.code = out_dynamic_fn;
  self.data = out;
  *out = dynamic_none();
  return self;
}

static int out_list_fn(void *data, Dynamic value)
{
  list_builder_add(data, value);
  return 1;
}

OutRoutine out_list_builder(ListBuilder *b)
{
  OutRoutine self;
  self.code = out_list_fn;
  self.data = b;
  return self;
}
