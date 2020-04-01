// Copyright 2017-2020 The Verible Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VERIBLE_LINT_WAIVER_CONFIG_LEXER_H__
#define VERIBLE_LINT_WAIVER_CONFIG_LEXER_H__

// verilog.lex has "%prefix=verilog", meaning the class flex creates is
// verilogFlexLexer. Unfortunately, FlexLexer.h doesn't have proper ifdefs
// around its inclusion, so we have to put a bar around it here.
#include "common/lexer/flex_lexer_adapter.h"
#include "common/text/token_info.h"
#ifndef _WAIVER_FLEXLEXER_H_
#undef yyFlexLexer  // this is how FlexLexer.h says to do things
#define yyFlexLexer veribleFlexLexer
#include <FlexLexer.h>
#endif

#include "absl/strings/string_view.h"

namespace verible {

class LintWaiverConfig : public verible::FlexLexerAdapter<veribleFlexLexer> {
  using parent_lexer_type = verible::FlexLexerAdapter<veribleFlexLexer>;

 public:
  explicit LintWaiverConfig(const absl::string_view config);

  // Main lexing function. Will be defined by Flex.
  int yylex() override;

  // Restart lexer with new input stream.
  void Restart(absl::string_view) override;

  // Returns true if token is invalid.
  bool TokenIsError(const verible::TokenInfo&) const override;
};

}  // namespace verilog

#endif  // VERIBLE_LINT_WAIVER_CONFIG_LEXER_H__

