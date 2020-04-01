%{
#define _WAIVER_FLEXLEXER_H_
#include <iostream>
#include "common/analysis/lint_waiver_config.h"
%}

%option nodefault
%option noyywrap
%option prefix="verible"
%option c++
%option yyclass="verible::LintWaiverConfig"

/* white space */
LineTerminator \r|\n|\r\n
InputCharacter [^\r\n\0]
Space [ \t\f\b]

DecimalDigits [0-9]+

Alpha [a-zA-Z]
RuleStart {Alpha}
Letter {RuleStart}|"$"

RuleName {RuleStart}({Letter}|-)*


RuleParam -rule
LineParam -line

LineNumber {DecimalDigits}
LineNumbers {DecimalDigits}{Space}+{DecimalDigits}

WaiveCommand waive

%x WAIVE_COMMAND
%x WAIVE_RULE
%x WAIVE_LINES

%%

{WaiveCommand} {
  yy_push_state(WAIVE_COMMAND);
  //std::cout << "command start [" << yytext << "]" << std::endl;
}

<WAIVE_COMMAND>{RuleParam} {
  yy_push_state(WAIVE_RULE);
  //std::cout << "rule start [" << yytext << "]" << std::endl;
}
<WAIVE_RULE>{RuleName} {
  yy_pop_state();
  std::cout << "Waiving " << yytext << " ";
}

<WAIVE_COMMAND>{LineParam} {
  yy_push_state(WAIVE_LINES);
}
<WAIVE_LINES>{LineNumber} {
  yy_pop_state();
  std::cout << "at line " << yytext << std::endl;
}
<WAIVE_LINES>{LineNumbers} {
  yy_pop_state();
  std::cout << "at lines " << yytext << std::endl;
}

<*>{Space} {
  ;
}
<*>{LineTerminator}     {
  yy_pop_state();
  std::cout << std::endl;
}


%%
