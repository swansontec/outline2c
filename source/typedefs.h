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
/*
 * Because C doesn't seem to allow duplicated typedefs, it makes sense to put
 * all the "typedef struct" definitions in one place to avoid clashes.
 */

#ifndef TYPEDEFS_H_INCLUDED
#define TYPEDEFS_H_INCLUDED

/* string.h */
typedef struct string String;

/* file.h */
typedef struct file_r FileR;
typedef struct file_w FileW;

/* outline.h */
typedef struct outline_item OutlineItem;
typedef struct outline      Outline;

/* pattern.h */
typedef struct pattern          Pattern;
typedef struct pattern_wild     PatternWild;
typedef struct pattern_word     PatternWord;
typedef struct pattern_replace  PatternReplace;

/* match.h */
typedef struct code             Code;
typedef struct code_code        CodeCode;
typedef struct code_replace     CodeReplace;
typedef struct code_match       CodeMatch;
typedef struct match            Match;
typedef struct match_builder    MatchBuilder;

#endif
