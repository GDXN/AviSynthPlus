// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .


#ifndef __Tokenizer_H__
#define __Tokenizer_H__

#include "internal.h"


/*********************************************************
*********************************************************/


class Tokenizer 
/**
  * Breaks up scripts into tokens
 **/
{
public:
  Tokenizer(const char* pc, IScriptEnvironment* _env);
  explicit Tokenizer(Tokenizer* old);

  void NextToken();

  inline bool IsIdentifier() const { return type == 'd'; }
  inline bool IsOperator() const { return type == 'o'; }
  inline bool IsInt() const { return type == 'i'; }
  inline bool IsFloat() const { return type == 'f'; }
  inline bool IsString() const { return type == 's'; }
  inline bool IsNewline() const { return type == 'n'; }
  inline bool IsEOF() const { return type == 0; }

  inline bool IsIdentifier(const char* id) const 
    { return IsIdentifier() && !lstrcmpi(id, identifier); }
  inline bool IsOperator(int o) const 
    { return IsOperator() && o == op; }

  const char* AsIdentifier() const { AssertType('d'); return identifier; }
  int AsOperator() const { AssertType('o'); return op; }
  int AsInt() const { AssertType('i'); return integer; }
  float AsFloat() const { AssertType('f'); return floating_pt; }
  const char* AsString() const { AssertType('s'); return string; }

  int GetLine() const { return line; }
  int GetColumn(const char* start_of_string) const;

private:
  void SkipWhitespace();
  void SkipNewline();
  void GetNumber();
  void AssertType(char expected_type) const;
  void SetToOperator(int o);
  
  IScriptEnvironment* const env;
  const char* token_start;
  const char* pc;
  int line;
  int type;   // i'd'entifier, 'o'perator, 'i'nt, 'f'loat, 's'tring, 'n'ewline, 0=eof
  union 
  {
    const char* identifier;
    int op;   // '+', '++', '.', ',', '(', ')', 0=eoln
    int integer;
    float floating_pt;
    const char* string;
  };
};



/**** Helper functions ****/

static const char* GetTypeName(short type);
void ThrowTypeMismatch(char expected, char actual, IScriptEnvironment* env);


#endif  // __Tokenizer_H__