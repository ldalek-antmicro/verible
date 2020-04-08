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

#include <set>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "common/analysis/citation.h"
#include "common/analysis/lint_rule_status.h"
#include "common/analysis/matcher/bound_symbol_manager.h"
#include "common/text/symbol.h"
#include "common/text/syntax_tree_context.h"
#include "verilog/analysis/descriptions.h"
#include "verilog/analysis/lint_rule_registry.h"
#include "verilog/CST/identifier.h"

namespace verilog {
namespace analysis {

using verible::GetStyleGuideCitation;
using verible::LintRuleStatus;
using verible::LintViolation;
using verible::SyntaxTreeContext;

// Register ForbidImplicitDeclarationsRule
VERILOG_REGISTER_LINT_RULE(ForbidImplicitDeclarationsRule);

// forbid-implicit-net-declarations?
absl::string_view ForbidImplicitDeclarationsRule::Name() {
  return "forbid-implicit-declarations";
}
const char ForbidImplicitDeclarationsRule::kTopic[] = "implicit-declarations";
const char ForbidImplicitDeclarationsRule::kMessage[] =
    "Nets must be declared explicitly.";

std::string ForbidImplicitDeclarationsRule::GetDescription(DescriptionType description_type) {
  return absl::StrCat("Checks that there are no occurrences of "
                      "implicitly declared nets.");
}

const absl::string_view ForbidImplicitDeclarationsRule::GetIdentifier(
    const verible::matcher::BoundSymbolManager& manager,
    const std::string& id) const {
  const auto* identifier_symbol = manager.FindSymbol(id);
  const auto* identifier_leaf = AutoUnwrapIdentifier(*ABSL_DIE_IF_NULL(identifier_symbol));
  return ABSL_DIE_IF_NULL(identifier_leaf)->get().text;
}

// checks that we are in correct scope
void ForbidImplicitDeclarationsRule::CheckAndPopScope(const verible::Symbol& symbol,
                                                const SyntaxTreeContext& context) {
  while ((scopes_.size() > 0) &&
         (!ContainsAncestor(scopes_.back().symbol_, context))) {
    scopes_.pop_back();
  }
}

void ForbidImplicitDeclarationsRule::DetectScope(const verible::Symbol& symbol,
                                                 const SyntaxTreeContext& context) {
  verible::matcher::BoundSymbolManager manager;

  if (net_scope_matcher_.Matches(symbol, &manager)) {
    CheckAndPopScope(symbol, context);
    scopes_.push_back(ScopeType(&symbol));
  }
}

void ForbidImplicitDeclarationsRule::DetectDeclarations(const verible::Symbol& symbol,
                                                        const SyntaxTreeContext& context) {
  verible::matcher::BoundSymbolManager manager;
  // If matched variable in scope
  if (net_declaration_matcher_.Matches(symbol, &manager)) {
    // Top of the matched tree stack
    CHECK_GE(context.size(), 1);
    const auto& top = context.top(); // scope
    const auto top_tag = static_cast<verilog::NodeEnum>(top.Tag().tag);

    CHECK((top_tag == NodeEnum::kModuleItemList) ||
          (top_tag == NodeEnum::kGenerateItemList));

    // This could be omitted if we had matchers for begining and ending of scopes
    // e.g. something like kBegin/kEnd. But we would have to modify parser grammar for
    // some of them, e.g. "module". Leaving like this for now.
    CheckAndPopScope(symbol, context);

    const auto* decls = ABSL_DIE_IF_NULL(manager.GetAs<verible::SyntaxTreeNode>("decls"));
    for (const auto& itr : decls->children()) {
      const auto& kind = itr.get()->Kind();
      if (kind != verible::SymbolKind::kNode) continue;
      const auto& tag = static_cast<verilog::NodeEnum>(itr.get()->Tag().tag);
      if ((tag != verilog::NodeEnum::kNetVariable) &&
          (tag != verilog::NodeEnum::kNetDeclarationAssignment)) continue;
      const auto& leaf = ABSL_DIE_IF_NULL(GetLeftmostLeaf(*itr))->get();
      CHECK_EQ(leaf.token_enum, SymbolIdentifier);
      const auto& identifier = leaf.text;

      // TODO: check parent scope for net names overlap?
      CHECK_GE(scopes_.size(), 1);
      auto& scope = scopes_.back().declared_nets_;
      scope[identifier] = &top;
    }
  }
}

void ForbidImplicitDeclarationsRule::AnalyzeLHS(const verible::SyntaxTreeLeaf& lval,
                                                const verible::SyntaxTreeContext& context) {
    const verible::TokenInfo& lval_token = lval.get();
    CHECK_EQ(lval_token.token_enum, SymbolIdentifier);
    const auto& identifier = lval_token.text;

    for (auto itr = scopes_.rbegin() ; itr != scopes_.rend() ; ++itr) {
      const auto& scope = itr->declared_nets_;
      //const auto* node = scope.at(identifier);
      const auto& node = scope.find(identifier);

      // If there's decleration or exists common ancestor then it's explicit declaration.
      // TODO: Check necessity of call to ContainsAncestor().
      // CheckAndPopScope() is propably enough.
      if ((node != scope.end()) && (ContainsAncestor(node->second, context))) {
        // Found declaration, stop searching
        return ;
      }
    }

    violations_.insert(LintViolation(lval, kMessage, context));
}

void ForbidImplicitDeclarationsRule::DetectReference(const verible::Symbol& symbol,
                                                     const SyntaxTreeContext& context) {
  verible::matcher::BoundSymbolManager manager;

  if (net_assignment_matcher_.Matches(symbol, &manager)) {
    if (!context.IsInside(NodeEnum::kContinuousAssignmentStatement)) {
      return ;
    }

    const auto* lpval = ABSL_DIE_IF_NULL(
        manager.GetAs<verible::SyntaxTreeNode>("lpval"));

    const auto& children = lpval->children();
    CHECK_EQ(children.size(), 1);
    const auto& lhs = *ABSL_DIE_IF_NULL(children[0]);

    CheckAndPopScope(symbol, context);

    if (net_reference_matcher_.Matches(lhs, &manager)) {
      // Matches simple LPvalue, e.g. assign a = 1'b0;
      const auto& lval = *ABSL_DIE_IF_NULL(
          manager.GetAs<verible::SyntaxTreeLeaf>("lval"));
      AnalyzeLHS(lval, context);
    } else if (net_openrange_matcher_.Matches(lhs, &manager)) {
      // Matches concatenated LPvalue, e.g. assign {a,b,c} = 3'b101;
      const auto* clist = ABSL_DIE_IF_NULL(
          manager.GetAs<verible::SyntaxTreeNode>("clist"));

      for (const auto& itr : clist->children()) {
        if (net_expression_reference_matcher_.Matches(*itr, &manager)) {
          const auto& lval = *ABSL_DIE_IF_NULL(
              manager.GetAs<verible::SyntaxTreeLeaf>("lval"));
          AnalyzeLHS(lval, context);
        }
      }
    } // TODO: Should log if we don't get any matches?
  }
}

void ForbidImplicitDeclarationsRule::HandleSymbol(const verible::Symbol& symbol,
                                                  const SyntaxTreeContext& context) {
  // Detects and pushes on stack new scopes
  DetectScope(symbol, context);

  // Detects nets declarations
  DetectDeclarations(symbol, context);

  // Detect referenced nets and checks for declarations
  DetectReference(symbol, context);
}

LintRuleStatus ForbidImplicitDeclarationsRule::Report() const {
  return LintRuleStatus(violations_, Name(), GetStyleGuideCitation(kTopic));
}

}  // namespace analysis
}  // namespace verilog
