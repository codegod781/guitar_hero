module note_reader(
    input logic clk,
    input logic reset,
    input chipselect,
    input logic [2:0] address,
    input logic [3:0] KEY,
    input logic [5:0] GPIO_1,
    
    output [7:0] LEDR,
    output logic [7:0] readdata,
    input logic read,
    output logic waitrequest
);

assign LEDR = GPIO_1; // Assign GPIO_1 directly to LEDR output
//assign LEDR[6] = KEY[0]

// Combinational logic to assign readdata based on KEY inputs
always_comb begin
    if (read && chipselect) begin
        readdata = 8'b00000000; // Initialize readdata to all zeros
        // Loop through each key
        for (int i = 0; i < 6; i = i + 1) begin
            // If the corresponding key is pressed, set the corresponding bit in readdata
            if (GPIO_1[i]) begin
                readdata = readdata | (1 << i);
            end
        end
		if (KEY[0]) begin
                readdata = readdata | (1 << 6);
        end
    end
    else begin
        readdata = 8'b00000000; // Clear readdata if read or chipselect is low
    end
end

endmodule

