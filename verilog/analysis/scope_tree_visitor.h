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

#ifndef VERIBLE_VERILOG_ANALYSIS_SCOPE_TREE_VISITOR_H_
#define VERIBLE_VERILOG_ANALYSIS_SCOPE_TREE_VISITOR_H_

#include <map>

#include "common/text/concrete_syntax_leaf.h"
#include "common/text/syntax_tree_context.h"
#include "common/text/tree_context_visitor.h"
#include "common/util/auto_pop_stack.h"

namespace verilog {
namespace analysis {

// This tree visitor traverse CST tree and mantains scope stack
class ScopeTreeVisitor : public verible::TreeContextVisitor {
 public:
  class ScopeInfo {
   public:
    typedef std::map<absl::string_view, const verible::Symbol*> symbol_map_type;

    ScopeInfo() = default;

    symbol_map_type declared_nets_;
    symbol_map_type referenced_nets_;
  };

  typedef verible::AutoPopStack<ScopeInfo> ScopeTreeContext;

  ScopeTreeVisitor() = default;

  virtual void HandleScope(const verible::Symbol& symbol,
                           const verible::SyntaxTreeContext& context,
                           const ScopeTreeContext& scope) {};

  virtual void HandleNetDeclaration(const verible::Symbol& symbol,
                                    const verible::SyntaxTreeContext& context,
                                    const ScopeTreeContext& scope) {};

  virtual void HandleNetReference(const verible::Symbol& symbol,
                                  const verible::SyntaxTreeContext& context,
                                  const ScopeTreeContext& scope) {};

 private:
  void Visit(const verible::SyntaxTreeNode& node) override;
  void Visit(const verible::SyntaxTreeLeaf& node) override;

  ScopeTreeContext current_scope_;
};

}  // namespace analysis
}  // namespace verilog

#endif  // VERIBLE_VERILOG_ANALYSIS_SCOPE_TREE_VISITOR_H_
