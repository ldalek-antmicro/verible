// Copyright 2017-2019 The Verible Authors.
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

#ifndef VERIBLE_VERILOG_ANALYSIS_DEFAULT_RULES_H_
#define VERIBLE_VERILOG_ANALYSIS_DEFAULT_RULES_H_

namespace verilog {
namespace analysis {

// List of rule that are enabled by default in production
// Use --ruleset default
// clang-format off
constexpr const char* kDefaultRuleSet[] = {
    "invalid-system-task-function",
    "module-begin-block",
    "module-parameter",
    "module-port",
    "module-filename",
    "package-filename",
    "void-cast",
    "generate-label",
    "always-comb",
    "v2001-generate-begin",
    "forbidden-macro",
    "create-object-name-match",
    "packed-dimensions-range-ordering",
    "unpacked-dimensions-range-ordering",
    "no-trailing-spaces",
    "no-tabs",
    "posix-eof",
    "line-length",
    "undersized-binary-literal",
    "explicit-function-lifetime",
    "explicit-function-task-parameter-type",
    "explicit-task-lifetime",
    "explicit-parameter-storage-type",
    "plusarg-assignment",
    "macro-name-style",
    "parameter-name-style",
    // (b/138352825): "always-comb-blocking",
    // (b/138352825): "always-ff-non-blocking",
    // TODO(fangism): enable in production:
    // TODO(b/117554159): "case-missing-default",
    // TODO(b/117131903): "proper-parameter-declaration",
    // TODO(b/131637160): "signal-name-style",
    // "endif-comment",
};
}  // namespace analysis
}  // namespace verilog

#endif  // VERIBLE_VERILOG_ANALYSIS_DEFAULT_RULES_H_
