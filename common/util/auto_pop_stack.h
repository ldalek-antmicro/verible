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

#ifndef VERIBLE_COMMON_UTIL_AUTO_POP_STACK_H_
#define VERIBLE_COMMON_UTIL_AUTO_POP_STACK_H_

#include <vector>

namespace verible {

// AutoPop stack class. Templated version of SyntaxTreeContext::AutoPop
template <typename T>
class AutoPopStack {
 public:
  typedef T value_type;
  typedef AutoPopStack<value_type> this_type;

  class AutoPop {
   public:
    AutoPop(this_type* stack, T& value) {
      stack_ = stack;
      stack->Push(value);
    }
    ~AutoPop() {
      stack_->Pop();
    }

   private:
    this_type* stack_;
  };

  bool empty() {
    return stack_.empty();
  }

  value_type& top() {
    CHECK(!stack_.empty());
    return *ABSL_DIE_IF_NULL(stack_.back());
  }

  typedef std::vector<value_type*> stack_type;
  typedef typename stack_type::const_iterator const_iterator;
  typedef typename stack_type::const_reverse_iterator const_reverse_iterator;

  // Allow read-only random-access into stack:
  const_iterator begin() const { return stack_.begin(); }
  const_iterator end() const { return stack_.end(); }

  // Reverse iterators be useful for searching from the top-of-stack downward.
  const_reverse_iterator rbegin() const { return stack_.rbegin(); }
  const_reverse_iterator rend() const { return stack_.rend(); }

 private:
  void Push(T& value) {
    stack_.push_back(&value);
  }

  void Pop() {
    CHECK(!stack_.empty());
    stack_.pop_back();
  }

  stack_type stack_;
};

}  // namespace verible

#endif  // VERIBLE_COMMON_UTIL_AUTO_POP_STACK_H_
