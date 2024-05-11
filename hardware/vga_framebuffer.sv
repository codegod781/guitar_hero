/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module vga_framebuffer (
    input logic clk,
    input logic reset,
    input logic [31:0] writedata,  // Format: {9 unused bits, 17-bit pixel number, 6-bit pixel data}
    input logic write,
    input chipselect,
    input logic [1:0] address,  // Unused, because a 17-bit address is weird

    output logic [7:0] VGA_R,
    VGA_G,
    VGA_B,
    output logic       VGA_CLK,
    VGA_HS,
    VGA_VS,
    VGA_BLANK_n,
    output logic       VGA_SYNC_n
);

  logic [10:0] hcount;
  logic [ 9:0] vcount;
  logic [ 8:0] pixel_y;
  logic [ 9:0] pixel_x;
  logic [16:0] read_addr, write_addr;

  logic [7:0] background_r, background_g, background_b;
  logic [5:0] write_data, pixel_data;
  logic write_mem;

  assign pixel_y   = vcount[8:0];
  assign pixel_x   = hcount[10:1] - 10'd244;  // Offset by 1 b/c we need a clock cycle to read
  assign read_addr = {pixel_y, pixel_x[7:0]};

  vga_counters counters (
      .clk50(clk),
      .*
  );


  vga_mem mem (
      .clk(clk),
      .ra(read_addr),
      .wa(write_addr),
      .write(write_mem),
      .wd(write_data),
      .rd(pixel_data)
  );

  always_ff @(posedge clk)
    if (reset) begin
      background_r <= 8'h0;
      background_g <= 8'h0;
      background_b <= 8'h80;
      write_addr <= 17'h0;
      write_data <= 8'h0;
      write_mem <= 1'd0;
    end else begin
      write_mem <= 1'd0;
      if (chipselect && write) begin
        //write_addr <= 17'd5;
        write_addr <= writedata[22:6];  // Extracting 17-bit pixel number from writedata
        write_data <= writedata[5:0];  // Extracting 8-bit pixel data from writedata
        write_mem  <= 1'd1;
      end
    end


  always_comb begin
    {VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, 8'h0};
    if (VGA_BLANK_n) begin
      if (hcount[10:1] >= 10'd245 && hcount[10:1] < 10'd395 && vcount[9:0] < 10'd480) begin
        case (pixel_data)
	  6'd0: {VGA_R, VGA_G, VGA_B} = 24'h000000; // Black
	  6'd1: {VGA_R, VGA_G, VGA_B} = 24'hffffff; // White
	  6'd2: {VGA_R, VGA_G, VGA_B} = 24'hff0000; // Red
	  6'd3: {VGA_R, VGA_G, VGA_B} = 24'h00ff00; // Green
	  6'd4: {VGA_R, VGA_G, VGA_B} = 24'h0000ff; // Blue
	  6'd5: {VGA_R, VGA_G, VGA_B} = 24'h14d345; // Light_green
	  6'd6: {VGA_R, VGA_G, VGA_B} = 24'h11a132; // Middle_green
	  6'd7: {VGA_R, VGA_G, VGA_B} = 24'h10a237; // Dark_green
	  6'd8: {VGA_R, VGA_G, VGA_B} = 24'hd3362f; // Light_red
	  6'd9: {VGA_R, VGA_G, VGA_B} = 24'h9a2a26; // Middle_red
	  6'd10: {VGA_R, VGA_G, VGA_B} = 24'h9b2929; // Dark_red
	  6'd11: {VGA_R, VGA_G, VGA_B} = 24'hfef335; // Light_yellow
	  6'd12: {VGA_R, VGA_G, VGA_B} = 24'hc5bd1a; // Middle_yellow
	  6'd13: {VGA_R, VGA_G, VGA_B} = 24'hcfbd3d; // Dark_yellow
	  6'd14: {VGA_R, VGA_G, VGA_B} = 24'h5375e0; // Light_blue
	  6'd15: {VGA_R, VGA_G, VGA_B} = 24'h3b59af; // Middle_blue
	  6'd16: {VGA_R, VGA_G, VGA_B} = 24'h4059ab; // Dark_blue
	  6'd17: {VGA_R, VGA_G, VGA_B} = 24'hda562b; // Light_orange
	  6'd18: {VGA_R, VGA_G, VGA_B} = 24'h8b3518; // Middle_orange
	  6'd19: {VGA_R, VGA_G, VGA_B} = 24'h8f3719; // Dark_orange
	  default: {VGA_R, VGA_G, VGA_B} = 24'hffffff; // Default to white
	endcase
      end else begin
        {VGA_R, VGA_G, VGA_B} = {background_r, background_g, background_b};
      end
    end
  end

endmodule

module vga_mem (
    input logic clk,
    input logic [16:0] ra, wa,
    input logic write,
    input logic [5:0] wd,
    output logic [5:0] rd
);

  logic [5:0] data[122775:0];  // 480 rows * 256 offset / row, plus up to 150 col offset 
  always_ff @(posedge clk) begin
    if (write) data[wa] <= wd;
    rd <= data[ra];
  end
endmodule

module vga_counters (
    input  logic        clk50,
    reset,
    output logic [10:0] hcount,  // hcount[10:1] is pixel column
    output logic [ 9:0] vcount,  // vcount[9:0] is pixel row
    output logic        VGA_CLK,
    VGA_HS,
    VGA_VS,
    VGA_BLANK_n,
    VGA_SYNC_n
);

  /*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
  // Parameters for hcount
  parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600

  // Parameters for vcount
  parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

  logic endOfLine;

  always_ff @(posedge clk50 or posedge reset)
    if (reset) hcount <= 0;
    else if (endOfLine) hcount <= 0;
    else hcount <= hcount + 11'd1;

  assign endOfLine = hcount == HTOTAL - 1;

  logic endOfField;

  always_ff @(posedge clk50 or posedge reset)
    if (reset) vcount <= 0;
    else if (endOfLine)
      if (endOfField) vcount <= 0;
      else vcount <= vcount + 10'd1;

  assign endOfField = vcount == VTOTAL - 1;

  // Horizontal sync: from 0x520 to 0x5DF (0x57F)
  // 101 0010 0000 to 101 1101 1111
  assign VGA_HS = !((hcount[10:8] == 3'b101) & !(hcount[7:5] == 3'b111));
  assign VGA_VS = !(vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

  assign VGA_SYNC_n = 1'b0;  // For putting sync on the green signal; unused

  // Horizontal active: 0 to 1279     Vertical active: 0 to 479
  // 101 0000 0000  1280	       01 1110 0000  480
  // 110 0011 1111  1599	       10 0000 1100  524
  assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
			!( vcount[9] | (vcount[8:5] == 4'b1111) );

  /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
  assign VGA_CLK = hcount[0];  // 25 MHz clock: rising edge sensitive

endmodule
