
module  state_machine(
    input   wire        clk,
    output  wire        z1, z2,
    input   wire        x1, x2, x3
);
    reg[1:0] ps; // represents current state
    reg[1:0] ns; // represents next state
    
    localparam[1:0]
        s0 = 0, s1 = 1, s2 = 2;
    
    assign z1 = s1 || s2 || (s0 && x2);
    assign z2 = (s0 && !x2);
    assign z3 = s0 && x2 && x1;
    
    always @(clk) begin
        if(!clk) ps <= ns;
        else 
        case(ps)
            s0: ns <= 
                x2  
                ?   x1
                    ?   s2
                    :   x3 
                        ? s2
                        : s1
                : s1;
                
            s1: ns <=
                x2
                ? s2
                : s1;
                
            s2: ns <=
                x1
                ? s0
                : s1;
            default:
                ns <= s0;
        endcase
    end
endmodule