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

#include "common/analysis/lint_waiver_config.h"

#include "common/text/token_info.h"
#include "absl/strings/string_view.h"

namespace verible {

LintWaiverConfig::LintWaiverConfig(const absl::string_view config)
    : parent_lexer_type(config) {}

void LintWaiverConfig::Restart(absl::string_view config) {
  parent_lexer_type::Restart(config);
}

bool LintWaiverConfig::TokenIsError(const verible::TokenInfo& token) const {
  return false;
}

}  // namespace verilog

