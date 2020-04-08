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

#ifndef VERIBLE_VERILOG_ANALYSIS_CHECKERS_FORBID_IMPLICIT_DECLARATIONS_RULE_H_
#define VERIBLE_VERILOG_ANALYSIS_CHECKERS_FORBID_IMPLICIT_DECLARATIONS_RULE_H_

#include <set>
#include <string>

#include "common/analysis/lint_rule_status.h"
#include "common/analysis/matcher/core_matchers.h"
#include "common/analysis/matcher/matcher.h"
#include "common/analysis/matcher/matcher_builders.h"
#include "common/analysis/syntax_tree_lint_rule.h"
#include "common/text/symbol.h"
#include "common/text/syntax_tree_context.h"
#include "verilog/CST/verilog_matchers.h"  // IWYU pragma: keep
#include "verilog/analysis/descriptions.h"

namespace verilog {
namespace analysis {

// ForbidImplicitDeclarationsRule detect implicitly declared nets
class ForbidImplicitDeclarationsRule : public verible::SyntaxTreeLintRule {
 public:
  using rule_type = verible::SyntaxTreeLintRule;
  static absl::string_view Name();

  // Returns the description of the rule implemented formatted for either the
  // helper flag or markdown depending on the parameter type.
  static std::string GetDescription(DescriptionType);

  void HandleSymbol(const verible::Symbol& symbol,
                    const verible::SyntaxTreeContext& context) override;

  verible::LintRuleStatus Report() const override;

 private:
  // Link to style guide rule.
  static const char kTopic[];

  // Diagnostic message.
  static const char kMessage[];

  using Matcher = verible::matcher::Matcher;

  // Checks that top of the stack is our ancestor. If not
  // pops until find one
  void CheckAndPopScope(const verible::Symbol& symbol,
                  const verible::SyntaxTreeContext& context);

  void AnalyzeLHS(const verible::SyntaxTreeLeaf& lval,
                  const verible::SyntaxTreeContext& context);

  // Pushes new scopes on stack
  void DetectScope(const verible::Symbol& symbol,
                   const verible::SyntaxTreeContext& context);

  // Detects nets declarations adds them to map
  void DetectDeclarations(const verible::Symbol& symbol,
                          const verible::SyntaxTreeContext& context);

  void DetectReference(const verible::Symbol& symbol,
                       const verible::SyntaxTreeContext& context);

  bool ContainsAncestor(const verible::Symbol* const symbol,
                        const verible::SyntaxTreeContext& context) const {
    CHECK_NOTNULL(symbol);

    // check for common ancestors
    for (const auto& iter : context) {
      if (iter == symbol) {
        return true;
      }
    }
    return false;
  }

  const absl::string_view GetIdentifier(
      const verible::matcher::BoundSymbolManager& manager,
      const std::string& id) const;

  // Matches scopes with nets
  // TODO: description, other scopes
  const Matcher net_scope_matcher_ = verible::matcher::AnyOf(
      NodekModuleItemList(),
      NodekGenerateItemList());

  // Matches nets declarations
  const Matcher net_declaration_matcher_ =
      NodekNetDeclaration(PathkNetVariableDeclarationAssign().Bind("decls"));

  const Matcher net_assignment_matcher_ =
      NodekNetVariableAssignment(PathkLPValue().Bind("lpval"));

  const Matcher net_openrange_matcher_ =
      NodekBraceGroup(PathkOpenRangeList().Bind("clist"));

  // TODO: Any smart way to merge those two?
  const Matcher net_reference_matcher_ = NodekReferenceCallBase(
      PathkReference(UnqualifiedReferenceHasId().Bind("lval")));
  const Matcher net_expression_reference_matcher_ = NodekExpression(
      PathkReferenceCallBase(PathkReference(
      UnqualifiedReferenceHasId().Bind("lval"))));

  std::set<verible::LintViolation> violations_;

  using DeclType = std::map<absl::string_view, const verible::Symbol*>;

  struct ScopeType {
    ScopeType(const verible::Symbol* symbol) : symbol_(symbol) {};
    const verible::Symbol* const symbol_;
    DeclType declared_nets_;
  };

  std::vector<ScopeType> scopes_;
};

}  // namespace analysis
}  // namespace verilog

#endif  // VERIBLE_VERILOG_ANALYSIS_CHECKERS_FORBID_IMPLICIT_DECLARATIONS_RULE_H_
