// Drop-in pieces for your existing testbench. Rename signals to match YOUR core.
// 1) Load the program into memory before reset is released:
//    initial $readmemh("prog.hex", dut.imem.mem);   // adjust path/array name
//
// 2) Watch data-memory writes for the UART and the mailbox:
always @(posedge clk) begin
  if (dut_dmem_we) begin                       // your data-memory write-enable
    case (dut_dmem_addr)                       // your data-memory address
      32'h1000_0004: $write("%c", dut_dmem_wdata[7:0]);   // UART char
      32'h1000_0000: begin                                // mailbox
        if (dut_dmem_wdata == 1) $display("*** ALL TESTS PASSED ***");
        else                     $display("*** TESTS FAILED ***");
        $finish;
      end
    endcase
  end
end
