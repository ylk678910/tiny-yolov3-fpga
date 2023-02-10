
`timescale 1 ns / 1 ps
`include "TinyYolov3.vh"


module TinyYolov3_v1_0_S00_AXIS #
       (
           // Users to add parameters here

           // User parameters ends
           // Do not modify the parameters beyond this line

           // AXI4Stream sink: Data Width
           parameter integer C_S_AXIS_TDATA_WIDTH	= 32,   //  不要改
           parameter integer CHAR_NUM_PER_FLOAT = 2,        //! 1 2 4
           parameter integer FLOAT_NUM_PER_RECV = 2         //! FLOAT_NUM_PER_RECV * CHAR_NUM_PER_FLOAT <= 4 && FLOAT_NUM_PER_RECV為2的次方(1 2 4)
       )
       (
`ifdef DEBUG
`endif
           S_AXIS_ACLK,
           S_AXIS_ARESETN,
           S_AXIS_TREADY,
           S_AXIS_TDATA,
           S_AXIS_TKEEP,
           S_AXIS_TLAST,
           S_AXIS_TVALID,
           core_ready_read,
           packet_finish,
           float_data_valid,
           float_data
       );

// function called clogb2 that returns an integer which has the
// value of the ceiling of the log base 2.
function integer clogb2;
    input integer bit_depth;
    begin
        for(clogb2=0; bit_depth>0; clogb2=clogb2+1)
            bit_depth = bit_depth >> 1;
    end
endfunction

//! Do not modify here
localparam integer CHAR_BIT             = 8;
localparam integer FLOAT_BIT	        = CHAR_BIT * CHAR_NUM_PER_FLOAT;
//!

// Users to add ports here
`ifdef DEBUG
`endif

input wire core_ready_read;
output wire packet_finish;
output wire [FLOAT_NUM_PER_RECV-1:0] float_data_valid;
output wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] float_data;
// User ports ends
// Do not modify the ports beyond this line

// AXI4Stream sink: Clock
input wire  S_AXIS_ACLK;
// AXI4Stream sink: Reset
input wire  S_AXIS_ARESETN;
// Ready to accept data in
output wire  S_AXIS_TREADY;
// Data in
input wire [C_S_AXIS_TDATA_WIDTH-1 : 0] S_AXIS_TDATA;
// Byte qualifier
input wire [(C_S_AXIS_TDATA_WIDTH/CHAR_BIT)-1 : 0] S_AXIS_TKEEP;
// Indicates boundary of last packet
input wire  S_AXIS_TLAST;
// Data is in valid
input wire  S_AXIS_TVALID;

// input stream data S_AXIS_TDATA
wire axis_tready;
// FIFO write enable
wire fifo_wren;
wire next_float_flg = core_ready_read;
reg next_data_flg;

reg packet_finish_reg;
assign packet_finish = packet_finish_reg;

`ifdef DEBUG
`endif
// I/O Connections assignments

assign axis_tready = next_data_flg & next_float_flg;
assign S_AXIS_TREADY = axis_tready;
// AXI Streaming Sink
//
// The example design sink is always ready to accept the S_AXIS_TDATA  until
// the FIFO is not filled with NUMBER_OF_INPUT_WORDS number of input words.


always@(posedge S_AXIS_ACLK) begin
    if(!S_AXIS_ARESETN) begin
        packet_finish_reg <= 1'b0;
    end
    else begin
        if (fifo_wren) begin
            if (S_AXIS_TLAST) begin
                // reads_done is asserted when NUMBER_OF_INPUT_WORDS numbers of streaming data
                // has been written to the FIFO which is also marked by S_AXIS_TLAST(kept for optional usage).
                packet_finish_reg <= 1'b1;
            end
        end
        else if(core_ready_read) begin
            packet_finish_reg <= 1'b0;
        end
    end
end

// FIFO write enable generation
assign fifo_wren = S_AXIS_TVALID && axis_tready;

wire [CHAR_BIT-1:0] char_tdata[0:C_S_AXIS_TDATA_WIDTH/CHAR_BIT-1];
genvar char_ptr;
generate
    for(char_ptr = 0; char_ptr<C_S_AXIS_TDATA_WIDTH/CHAR_BIT; char_ptr=char_ptr+1) begin: C_D
        assign char_tdata[C_S_AXIS_TDATA_WIDTH/CHAR_BIT-char_ptr-1] = S_AXIS_TDATA[C_S_AXIS_TDATA_WIDTH-char_ptr*CHAR_BIT-1 -: CHAR_BIT];
    end
endgenerate

reg [clogb2(C_S_AXIS_TDATA_WIDTH/CHAR_BIT-1)-1:0] data_char_ptr;
wire [clogb2(C_S_AXIS_TDATA_WIDTH/CHAR_BIT-1)-1:0] next_ptr = data_char_ptr+CHAR_NUM_PER_FLOAT*FLOAT_NUM_PER_RECV;
/*
reg data_valid_flg;
always @(posedge S_AXIS_ACLK) begin
    if(!S_AXIS_ARESETN) begin
        //data_char_ptr <= 4-CHAR_NUM_PER_FLOAT*FLOAT_NUM_PER_RECV;
        data_char_ptr <= 0;
    end
    //else if (next_float_flg && S_AXIS_TVALID) begin
    else if (next_float_flg && data_valid_flg) begin
        data_char_ptr <= data_char_ptr + CHAR_NUM_PER_FLOAT*FLOAT_NUM_PER_RECV;
    end
end
 */
// FIFO Implementation
genvar tmp_ptr;
genvar float_ptr;
generate
    if(CHAR_NUM_PER_FLOAT == 3) begin //!沒好
        /*
        for(tmp_ptr = 0; tmp_ptr<CHAR_NUM_PER_FLOAT-(C_S_AXIS_TDATA_WIDTH/CHAR_BIT-CHAR_NUM_PER_FLOAT); tmp_ptr=tmp_ptr+1) begin: TMP_GEN
            reg [CHAR_BIT-1:0] char_tmp; //接收到資料多餘的暫存
            always @(posedge S_AXIS_ACLK) begin
                if(!S_AXIS_ARESETN) begin
                    char_tmp <= {CHAR_BIT{1'b0}};
                end
                else if (fifo_wren) begin
                    char_tmp <= C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+tmp_ptr].char_tdata;
                end
            end
        end
        wire [2:0] cmp0 = {1'b0, data_char_ptr};
        wire need_tmp_flg = (cmp0+CHAR_NUM_PER_FLOAT > 3'd4);
        always @(*) begin
            if(need_tmp_flg) begin
                char_data[0] = () ? C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+0].char_tdata : TMP_GEN[data_char_ptr+CHAR_NUM_PER_FLOAT+0].char_tmp;
                char_data[1] = C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+1].char_tdata;
                char_data[2] = C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+2].char_tdata;
            end
            else begin
                char_data[0] = C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+0].char_tdata;
                char_data[1] = C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+1].char_tdata;
                char_data[2] = C_D[data_char_ptr+CHAR_NUM_PER_FLOAT+2].char_tdata;
            end
            case (data_char_ptr)
                0: begin

                end
                default: begin

                end
            endcase
        end
        */
    end
    else begin
        always @(posedge S_AXIS_ACLK) begin
            if(!S_AXIS_ARESETN) begin
                next_data_flg <= 1;
            end
            else if (fifo_wren && next_ptr != 0) begin
                next_data_flg <= 0;
            end
            else if(next_ptr==0 && next_float_flg) begin
                next_data_flg <= 1;
            end
        end
        /*
        always @(posedge S_AXIS_ACLK) begin
            if(!S_AXIS_ARESETN) begin
                data_valid_flg <= 0;
            end
            else if (fifo_wren) begin
                data_valid_flg <= 1;
            end
            else if(next_ptr==0 && next_float_flg && !S_AXIS_TVALID) begin
                data_valid_flg <= 0;
            end
        end
        */
        // FIFO Implementation
        reg  [FLOAT_BIT-1:0] float_data_reg [0:FLOAT_NUM_PER_RECV-1];
        reg  float_data_valid_reg [0:FLOAT_NUM_PER_RECV-1];
        // Streaming input data is stored in FIFO
        for(float_ptr = 0; float_ptr<FLOAT_NUM_PER_RECV; float_ptr=float_ptr+1) begin: FLOAT_GEN
            integer char_ptr_i;
            always @( posedge S_AXIS_ACLK ) begin
                if(!S_AXIS_ARESETN) begin
                    float_data_valid_reg[float_ptr] <= 1'b0;
                    float_data_reg[float_ptr] <= 0;
                end
                if (next_float_flg) begin
                    //if(data_valid_flg|(fifo_wren && FLOAT_NUM_PER_RECV*CHAR_NUM_PER_FLOAT==4)) begin
                    if(fifo_wren) begin
                        for(char_ptr_i = 0; char_ptr_i<CHAR_NUM_PER_FLOAT; char_ptr_i=char_ptr_i+1) begin: CHAR_GEN
                            //float_data_reg[float_ptr][CHAR_BIT*char_ptr_i +: CHAR_BIT] <= char_tdata[data_char_ptr + (float_ptr*CHAR_NUM_PER_FLOAT) + char_ptr_i ];
                            float_data_reg[float_ptr][CHAR_BIT*char_ptr_i +: CHAR_BIT] <= char_tdata[(float_ptr*CHAR_NUM_PER_FLOAT) + char_ptr_i ];
                        end
                        //float_data_valid_reg[float_ptr] <= &S_AXIS_TKEEP[CHAR_NUM_PER_FLOAT*float_ptr+data_char_ptr +: CHAR_NUM_PER_FLOAT];
                        float_data_valid_reg[float_ptr] <= &S_AXIS_TKEEP[CHAR_NUM_PER_FLOAT*float_ptr +: CHAR_NUM_PER_FLOAT];
                    end
                    else begin
                        float_data_valid_reg[float_ptr] <= 0;
                    end
                end
            end

            assign float_data[(FLOAT_BIT*float_ptr) +: FLOAT_BIT] = float_data_reg[FLOAT_NUM_PER_RECV-float_ptr-1];
            assign float_data_valid[float_ptr] = float_data_valid_reg[float_ptr];
        end
    end
endgenerate

// Add user logic here
//assign float_data_0_0[(C_S_AXIS_TDATA_WIDTH/4)-1:0] = FIFO_GEN[0].float_data_reg[0][(C_S_AXIS_TDATA_WIDTH/4)-1:0];
// User logic ends

endmodule
