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
      {"module m;\nwire a1;\nmodule foo;\nassign a1 = 1'b0;\nendmodule;\nendmodule"},
      {"module m;\nwire a1;\nmodule foo;\nendmodule;\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nend\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nassign a1 = 1'b0;\nend\nassign a1 = 1'b0;\nendmodule"},
      {"module m;\nwire a1;\nbegin\nwire a1; assign a1 = 1'b0;\nend\nassign a1 = 1'b0;\nendmodule"},

      // multiple declarations
      {"module m;\nwire a0, a1;\nassign a0 = 1'b0;\nassign a1 = 1'b1;\nendmodule"},
      {"module m;\nwire a0, a2;\nassign a0 = 1'b0;\nassign ", {kToken, "a1"}, " = 1'b1;\nendmodule"},

      // multiple net assignments
      {"module m;\nassign ", {kToken, "a"}, " = b, ", {kToken, "c"}, " = d;\nendmodule"},
      {"module m;\nassign ", {kToken, "a1"}, " = 1'b0;wire a1;\nendmodule"},

      // out-of-order
      {"module m;\nassign ", {kToken, "a1"}, " = 1'b0;\nwire a1;\nassign a1 = 1'b1;\nendmodule"},

      // concatenated
      {"module m;\nassign {", {kToken, "a"}, "} = 1'b0;\nendmodule"},
      {"module m;\nassign {", {kToken, "a"}, ",", {kToken, "b"}, "} = 2'b01;\nendmodule"},
      {"module m;\nassign {", {kToken, "a"}, ",", {kToken, "b"}, ",", {kToken, "c"}, "} = 3'b010;\nendmodule"},
      {"module m;\nwire b;assign {", {kToken, "a"}, ", b,", {kToken, "c"}, "} = 3'b010;\nendmodule"},

      // around scope
      {"module m;assign ", {kToken, "a1"}, " = 1'b1;\nbegin\nend\nendmodule"},
      {"module m;begin\nend\nassign ", {kToken, "a1"}, " = 1'b1;endmodule"},

      // declaration and assignement separated by begin-end block
      {"module m;\nwire a1;\nbegin\nend\nassign a1 = 1'b1;\nendmodule"},

      // out-of-scope
      {"module m;\nbegin wire a1;\nend\nassign ", {kToken, "a1"}, " = 1'b1;\nendmodule"},
      {"module m;\nbegin wire a1;\nassign a1 = 1'b0;\nend\nassign ", {kToken, "a1"}, " = 1'b1;\nendmodule"},
      {"module m;\nwire a1;begin assign a1 = 1'b0;\nend\nassign a1 = 1'b1;\nendmodule"},

      // multi-level begin-end blocks
      {"module m;\n"
       "  wire x1;\n"
       "  begin\n"
       "    wire x2;\n"
       "    begin\n"
       "      wire x3;\n"
       "      begin\n"
       "        wire x4;\n"
       "        assign x4 = 1'b0;\n"
       "        assign x3 = 1'b0;\n"
       "        assign x2 = 1'b0;\n"
       "        assign x1 = 1'b0;\n"
       "      end\n"
       "      assign ", {kToken, "x4"}, " = 1'b0;\n"
       "      assign x3 = 1'b1;\n"
       "      assign x2 = 1'b0;\n"
       "      assign x1 = 1'b0;\n"
       "    end\n"
       "    assign ", {kToken, "x4"}, " = 1'b0;\n"
       "    assign ", {kToken, "x3"}, " = 1'b0;\n"
       "    assign x2 = 1'b0;\n"
       "    assign x1 = 1'b0;\n"
       "  end\n"
       "  assign ", {kToken, "x4"}, " = 1'b0;\n"
       "  assign ", {kToken, "x3"}, " = 1'b0;\n"
       "  assign ", {kToken, "x2"}, " = 1'b0;\n"
       "  assign x1 = 1'b1;\n"
       "endmodule"},

       // generate block, TODO: multi-level
       {"module m;\ngenerate\nendgenerate\nendmodule"},
       {"module m;\n"
        "  wire a1;\n"
        "  assign a1 = 1'b1;\n"
        "  generate\n"
        "  endgenerate\n"
        "  assign a1 = 1'b0;\n"
        "endmodule"},
       {"module m;\n"
        "  generate\n"
        "    wire a1;\n"
        "    assign a1 = 1'b1;\n"
        "  endgenerate\n"
        "endmodule"},
       {"module m;\n"
        "  generate\n"
        "    assign ", {kToken, "a1"}, " = 1'b1;\n"
        "  endgenerate\n"
        "endmodule"},
       {"module m;\n"
        "  generate\n"
        "    wire a1;\n"
        "    assign a1 = 1'b1;\n"
        "  endgenerate\n"
        "  assign ", {kToken, "a1"}, " = 1'b1;\n"
        "endmodule"},
       {"module m;\n"
        "  wire a1;\n"
        "  assign a1 = 1'b1;\n"
        "  generate\n"
        "    assign a1 = 1'b1;\n"
        "  endgenerate\n"
        "  assign a1 = 1'b0;\n"
        "endmodule"},
       {"module m;\n"
        "  wire a1;\n"
        "  generate\n"
        "    wire a2\n"
        "    assign a1 = 1'b1;\n"
        "  endgenerate\n"
        "  assign ", {kToken, "a2"}, " = 1'b0;\n"
        "endmodule"},
       {"module m;\n"
        "  wire a1;\n"
        "  generate\n"
        "    wire a2;\n"
        "    assign a1 = 1'b1;\n"
        "    assign a2 = a1;\n"
        "  endgenerate\n"
        "  assign ", {kToken, "a2"}, " = 1'b0;\n"
        "  assign a1 = a2;\n"
        "endmodule"},

      // TODO: module scope

      // TODO: nets declared inside terminal/port connection list
  };

  RunLintTestCases<VerilogAnalyzer, ForbidImplicitDeclarationsRule>(
      ForbidImplicitDeclarationsTestCases);
}

}  // namespace
}  // namespace analysis
}  // namespace verilog
