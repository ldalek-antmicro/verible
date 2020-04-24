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

#include "verilog/analysis/scope_tree_visitor.h"
#include "verilog/CST/verilog_matchers.h"  // IWYU pragma: keep
#include "verilog/CST/verilog_nonterminals.h"

namespace verilog {
namespace analysis {

void ScopeTreeVisitor::Visit(const verible::SyntaxTreeNode& node) {
  const auto tag = static_cast<NodeEnum>(node.Tag().tag);
  const auto& context = TreeContextVisitor::Context();

  switch (tag) {
    case NodeEnum::kSeqBlock:
    case NodeEnum::kGenerateRegion:
    case NodeEnum::kModuleDeclaration: {
      ScopeInfo scope_info;
      const ScopeTreeContext::AutoPop pp(&current_scope_, scope_info);
      HandleScope(node, context, current_scope_);
      TreeContextVisitor::Visit(node);
      break;
    }

    default: {
      // Visit subtree children.
      TreeContextVisitor::Visit(node);
      break;
    }
  }
}

void ScopeTreeVisitor::Visit(const verible::SyntaxTreeLeaf& leaf) {
  const auto& context = TreeContextVisitor::Context();
  const auto tag = verilog_tokentype(leaf.Tag().tag);

  switch (tag) {
    case verilog_tokentype::SymbolIdentifier: {
      if (context.DirectParentsAre({NodeEnum::kNetVariable,
                                    NodeEnum::kNetVariableDeclarationAssign,
                                    NodeEnum::kNetDeclaration}) &&
           (context.IsInside(NodeEnum::kModuleItemList) ||
            context.IsInside(NodeEnum::kGenerateItemList))) {
        const auto identifier = leaf.get().text; // absl::string_view

        if (!current_scope_.empty()) {
          auto& scope = current_scope_.top();
          auto& declared_nets = scope.declared_nets_;
          declared_nets[identifier] = &leaf;
          HandleNetDeclaration(leaf, context, current_scope_);
        }

      // FIXME: HandleNetReference/HandleNetDeclaration should
      // be matched on SyntaxTreeNode and each net declaration/reference should be
      // extraced using CST/matcher
      } else if (context.DirectParentsAre({NodeEnum::kUnqualifiedId,
                                           NodeEnum::kLocalRoot,
                                           NodeEnum::kReference,
                                           NodeEnum::kReferenceCallBase,

                                           NodeEnum::kLPValue,
                                           NodeEnum::kNetVariableAssignment,
                                           NodeEnum::kAssignmentList,
                                           NodeEnum::kContinuousAssignmentStatement}) ||

                 context.DirectParentsAre({NodeEnum::kUnqualifiedId,
                                           NodeEnum::kLocalRoot,
                                           NodeEnum::kReference,
                                           NodeEnum::kReferenceCallBase,

                                           NodeEnum::kExpression,
                                           NodeEnum::kOpenRangeList,
                                           NodeEnum::kBraceGroup,
                                           NodeEnum::kLPValue,

                                           NodeEnum::kNetVariableAssignment,
                                           NodeEnum::kAssignmentList,
                                           NodeEnum::kContinuousAssignmentStatement})) {
        const auto identifier = leaf.get().text; // absl::string_view

        if (!current_scope_.empty()) {
          auto& scope = current_scope_.top();
          auto& referenced_nets = scope.referenced_nets_;
          referenced_nets[identifier] = &leaf;
          HandleNetReference(leaf, context, current_scope_);
        }
      }
      break;
    }

    default:
      break;
  }
}

}  // namespace analysis
}  // namespace verilog
