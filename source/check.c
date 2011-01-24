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
 * All functions return 1 for success and 0 for failure. This macro checks a
 * return code and bails out if it indicates an error.
 */
#define CHECK(r) do { if (!(r)) return 0; } while(0)

/**
 * Verifies that a memory-allocating call succeeds, and prints an error message
 * otherwise.
 */
#define CHECK_MEM(r) do { \
  if (!(r)) { \
    fprintf(stderr, "error: Out of memory at %s:%d\n", __FILE__, __LINE__); \
    return 0; \
  } \
} while(0)