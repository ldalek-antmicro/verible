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

#include "verilog/analysis/checkers/forbid_implicit_declarations_rule.h"

#include <initializer_list>

#include "gtest/gtest.h"
#include "common/analysis/linter_test_utils.h"
#include "common/analysis/syntax_tree_linter_test_utils.h"
#include "common/text/symbol.h"
#include "verilog/CST/verilog_nonterminals.h"
#include "verilog/CST/verilog_treebuilder_utils.h"
#include "verilog/analysis/verilog_analyzer.h"

namespace verilog {
namespace analysis {
namespace {

using verible::LintTestCase;
using verible::RunLintTestCases;

TEST(ForbidImplicitDeclarationsRule, FunctionFailures) {
  auto kToken = SymbolIdentifier;
  const std::initializer_list<LintTestCase> ForbidImplicitDeclarationsTestCases = {
      {""},
      {"module m;\nendmodule\n"},
      {"module m;\nassign ", {kToken, "a1"}, " = 1'b0;\nendmodule"},
      {"module m;\nwire a1; assign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nmodule foo;\nassign ", {kToken, "a1"}, " = 1'b0;\nendmodule;\nendmodule"},
      {"module m;\nwire a1;\nmodule foo;\nendmodule;\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nend\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nassign ", {kToken, "a1"}, " = 1'b0;\nend\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nwire a1; assign a1 = 1'b0;\nend\nassign a1 = 1'b0;\nendmodule"},
  };

  RunLintTestCases<VerilogAnalyzer, ForbidImplicitDeclarationsRule>(
      ForbidImplicitDeclarationsTestCases);
}

}  // namespace
}  // namespace analysis
}  // namespace verilog
