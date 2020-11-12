//module m;
//  assign a=b&c&d&e&f&g;
//endmodule

module m;
  // Some instructions can only be executed in M-Mode
  assign illegal_umode = (priv_mode_i != PRIV_LVL_M) &
      // MRET must be in M-Mode. TW means trap WFI to M-Mode.
      (mret_insn | (csr_mstatus_tw_i & wfi_insn));
endmodule

module m;
  // Some instructions can only be executed in M-Mode
  assign illegal_umode = (priv_mode_i != PRIV_LVL_M) &
      // MRET must be in M-Mode.
      (mret_insn | (csr_mstatus_tw_i & wfi_insn));
endmodule


module always_if;
  always @(posedge clk)
    if (expr)
      z <= y;
endmodule

module m;
assign co = (aaaaaaaaaaaaaaaaa&bbbbbbbbbbbbbbbbbbbbbbbb)|
//a
(aaaaaaaaaaaaaaaaaa&cccccccccccccccccccciiiiiiiiiiiiiiiiiiii)|//b
(bbbbbbbbbbbbbbbbbbb&cccccccccccccccccciiiiiiiiiiiiiiiiiiii);
endmodule

module m;
//  //assign aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaillegal_umode =
//  //    (priv_mode_i & PRIV_LVL_M) &
//  //    (csr_mstatus_tw_i & wfi_insn);
//
//  //assign co = (a&b)|(a&ci)|(b&ci);
//
//  //assign m = (a != b) & (c | (d & e));
//  //assign m = (a != b) && (c == d);
//  //assign mm =
//  //((a != b) && (c == d))
//  //||
//  //((e != f) && (g == h))
//  //;
//  //assign n = (a && (b*c));
//  //assign n = (b*c);
//  //assign n = (b && c && d && e);
  assign n = (((((b) && c) && d) && e) && f);
  assign o = (f && (e && (d && (c && (b))))) || a;
endmodule
