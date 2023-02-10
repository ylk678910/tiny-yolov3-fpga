
`timescale 1 ns / 1 ps
`include "TinyYolov3.vh"

module TinyYolov3_v1_0_M00_AXIS #
       (
           // Users to add parameters here

           // User parameters ends
           // Do not modify the parameters beyond this line

           // Width of S_AXIS address bus. The slave accepts the read and write addresses of width C_M_AXIS_TDATA_WIDTH.
           parameter integer C_M_AXIS_TDATA_WIDTH	= 32,
           // Start count is the number of clock cycles the master will wait before initiating/issuing any transaction.
           parameter integer C_M_START_COUNT	= 32,
           parameter integer CHAR_NUM_PER_FLOAT = 2,        //! 1 2 4
           parameter integer FLOAT_NUM_PER_RECV = 2         //! FLOAT_NUM_PER_RECV * CHAR_NUM_PER_FLOAT <= 4 && FLOAT_NUM_PER_RECV為2的次方(1 2 4)
       )
       (
           M_AXIS_ACLK,
           M_AXIS_ARESETN,
           M_AXIS_TVALID,
           M_AXIS_TDATA,
           M_AXIS_TKEEP,
           M_AXIS_TLAST,
           M_AXIS_TREADY,
           send_data,
           send_data_valid,
           send_core_ready_send_flg,
           send_packet_finish_flg,
           send_ready_send_flg
       );
localparam integer CHAR_BIT = 8;
localparam integer FLOAT_BIT	          = CHAR_BIT * CHAR_NUM_PER_FLOAT;

// Users to add ports here
input wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] send_data;
input wire [FLOAT_NUM_PER_RECV-1 : 0] send_data_valid;
input wire send_core_ready_send_flg;
input wire send_packet_finish_flg;
output wire send_ready_send_flg;

reg [1:0] FIFO_in_ptr, FIFO_out_ptr;
wire FIFO_Ready;
reg LastDatampty_flg;
// User ports ends
// Do not modify the ports beyond this line

// Global ports
input wire  M_AXIS_ACLK;
//
input wire  M_AXIS_ARESETN;
// Master Stream Ports. TVALID indicates that the master is driving a valid transfer; A transfer takes place when both TVALID and TREADY are asserted.
output wire  M_AXIS_TVALID;
// TDATA is the primary payload that is used to provide the data that is passing across the interface from the master.
output wire [C_M_AXIS_TDATA_WIDTH-1 : 0] M_AXIS_TDATA;
// TSTRB is the byte qualifier that indicates whether the content of the associated byte of TDATA is processed as a data byte or a position byte.
//output wire [(C_M_AXIS_TDATA_WIDTH/8)-1 : 0] M_AXIS_TSTRB;
output wire [(C_M_AXIS_TDATA_WIDTH/8)-1 : 0] M_AXIS_TKEEP;
// TLAST indicates the boundary of a packet.
output wire  M_AXIS_TLAST;
// TREADY indicates that the slave can accept a transfer in the current cycle.
input wire  M_AXIS_TREADY;

// Total number of output data
localparam NUMBER_OF_OUTPUT_WORDS = 8;

// function called clogb2 that returns an integer which has the
// value of the ceiling of the log base 2.
function integer clogb2 (input integer bit_depth);
    begin
        for(clogb2=0; bit_depth>0; clogb2=clogb2+1)
            bit_depth = bit_depth >> 1;
    end
endfunction

// WAIT_COUNT_BITS is the width of the wait counter.
localparam integer WAIT_COUNT_BITS = clogb2(C_M_START_COUNT-1);

// Define the states of state machine
// The control state machine oversees the writing of input streaming data to the FIFO,
// and outputs the streaming data from the FIFO
localparam [1:0] IDLE = 2'b00,        // This is the initial/idle state

           INIT_COUNTER  = 2'b01, // This state initializes the counter, once
           // the counter reaches C_M_START_COUNT count,
           // the state machine changes state to SEND_STREAM
           SEND_STREAM   = 2'b10, // In this state the
           SEND_ALL   = 2'b11; // In this state the
// stream data is output through M_AXIS_TDATA
// State variable
reg [1:0] mst_exec_state;

// AXI Stream internal signals
//wait counter. The master waits for the user defined number of clock cycles before initiating a transfer.
reg [WAIT_COUNT_BITS-1 : 0] 	count;



// I/O Connections assignments



// Control state machine implementation
always @(posedge M_AXIS_ACLK) begin
    if (!M_AXIS_ARESETN)
        // Synchronous reset (active low)
    begin
        mst_exec_state <= IDLE;
        count    <= 0;
    end
    else
    case (mst_exec_state)
        IDLE:
            mst_exec_state  <= INIT_COUNTER;

        INIT_COUNTER:
            // The slave starts accepting tdata when
            // there tvalid is asserted to mark the
            // presence of valid streaming data
            if ( count == C_M_START_COUNT - 1 ) begin
                mst_exec_state  <= SEND_STREAM;
            end
            else begin
                count <= count + 1;
                mst_exec_state  <= INIT_COUNTER;
            end

        SEND_STREAM:
            // The example design streaming master functionality starts
            // when the master drives output tdata from the FIFO and the slave
            // has finished storing the S_AXIS_TDATA
            if(LastDatampty_flg) begin
                mst_exec_state <= IDLE;
            end
            else if (send_packet_finish_flg && FIFO_Ready) begin
                mst_exec_state <= SEND_ALL;
            end
            else begin
                mst_exec_state <= SEND_STREAM;
            end
        SEND_ALL:
            // The example design streaming master functionality starts
            // when the master drives output tdata from the FIFO and the slave
            // has finished storing the S_AXIS_TDATA
            if (LastDatampty_flg && M_AXIS_TREADY) begin
                mst_exec_state <= IDLE;
            end
            else begin
                mst_exec_state <= SEND_ALL;
            end
    endcase
end


wire [15:0] send_data_s [0:1];
assign send_data_s[0] = send_data[15:0];
assign send_data_s[1] = send_data[31:16];


wire [1:0] CoreValidDataNum = send_data_valid[0] + send_data_valid[1];
reg [16-1 : 0] Data_tmp [0:3];
reg [32-1 : 0] Send_tdata;
reg [1:0] Send_tvalid;
reg Send_tlast;

wire MasterReadyStore_flg = (send_ready_send_flg && send_core_ready_send_flg);
wire CoreWantSend_flg = MasterReadyStore_flg && (CoreValidDataNum);
reg FIFO_BufferFull_flg, FIFO_BufferEmpty_flg;
assign FIFO_Ready = !FIFO_BufferFull_flg;
assign send_ready_send_flg = !FIFO_BufferFull_flg && mst_exec_state==SEND_STREAM;
reg [1:0] ValidSendNumNow;
wire MasterReadySend_flg = ((mst_exec_state==SEND_STREAM && ValidSendNumNow >= 2) || mst_exec_state==SEND_ALL);

always @(*) begin
    case (mst_exec_state)
        SEND_STREAM: begin
            LastDatampty_flg = FIFO_BufferEmpty_flg && send_packet_finish_flg;
        end
        SEND_ALL: begin
            LastDatampty_flg = (ValidSendNumNow == 1 || FIFO_in_ptr == FIFO_out_ptr + 2'd2);
        end
        default begin
            LastDatampty_flg = `FALSE;
        end
    endcase
end

always @(*) begin
    case (mst_exec_state)
        SEND_STREAM: begin
            LastDatampty_flg = FIFO_BufferEmpty_flg && send_packet_finish_flg;
        end
        SEND_ALL: begin
            LastDatampty_flg = (ValidSendNumNow == 1 || FIFO_in_ptr == FIFO_out_ptr + 2'd2);
        end
        default begin
            LastDatampty_flg = `FALSE;
        end
    endcase
end

assign M_AXIS_TVALID = |Send_tvalid;
assign M_AXIS_TDATA  = Send_tdata;
assign M_AXIS_TLAST  = Send_tlast;
assign M_AXIS_TKEEP  = {{2{Send_tvalid[1]}}, {2{Send_tvalid[0]}}};

always @(posedge M_AXIS_ACLK) begin
    if(!M_AXIS_ARESETN) begin
        Send_tdata  <= 0;
        Send_tvalid <= 0;
        Send_tlast  <= 0;
    end
    else begin
        if(M_AXIS_TREADY) begin
            if(MasterReadySend_flg) begin
                Send_tdata  <= {Data_tmp[FIFO_out_ptr+2'd1], Data_tmp[FIFO_out_ptr]};
            end
        end

        if(M_AXIS_TREADY) begin
            if(LastDatampty_flg) begin
                Send_tlast  <= 1;
            end
            else begin
                Send_tlast  <= 0;
            end
        end
        else begin
        end

        if(M_AXIS_TREADY) begin
            case (mst_exec_state)
                SEND_STREAM: begin
                    if(ValidSendNumNow >= 2) begin
                        Send_tvalid <= 2'b11;
                    end
                    else begin
                        Send_tvalid <= 2'b00;
                    end
                end
                SEND_ALL: begin
                    if(ValidSendNumNow == 1) begin
                        Send_tvalid <= 2'b01;
                    end
                    else if(ValidSendNumNow == 0) begin
                        Send_tvalid <= 2'b00;
                    end
                    else begin
                        Send_tvalid <= 2'b11;
                    end
                end
                default begin
                    Send_tvalid <= 2'b00;
                end
            endcase
        end
        else begin
        end
    end
end

wire [2:0] FIFO_out_ptr_real = (FIFO_in_ptr > FIFO_out_ptr) ? {1'b1, FIFO_out_ptr} : {1'b0, FIFO_out_ptr};
wire [2:0] FIFO_in_ptr_next = {1'b0, FIFO_in_ptr} + {1'b0, CoreValidDataNum};
wire [2:0] FIFO_Size_next = {1'b0, ValidSendNumNow} + {1'b0, CoreValidDataNum};
always @(*) begin
    /*
    if(FIFO_in_ptr == FIFO_out_ptr) begin
        FIFO_BufferFull_flg = `FALSE;
    end
    else begin
        if(FIFO_in_ptr_next >= FIFO_out_ptr_real) begin
            FIFO_BufferFull_flg = `TRUE;
        end
        else begin
            FIFO_BufferFull_flg = `FALSE;
        end
    end
    */
    /*
            if(mst_exec_state==SEND_STREAM) begin
                if(M_AXIS_TREADY) begin
                    if(ValidSendNumNow == 1) begin
                        if(FIFO_Size_next - 3'd1 > 3'd3) begin
                            FIFO_BufferFull_flg = `TRUE;
                        end
                        else begin
                            FIFO_BufferFull_flg = `FALSE;
                        end
                    end
                    else begin
                        if(FIFO_Size_next - 3'd2 > 3'd3) begin
                            FIFO_BufferFull_flg = `TRUE;
                        end
                        else begin
                            FIFO_BufferFull_flg = `FALSE;
                        end
                    end
                end
                else begin
                    if(FIFO_Size_next > 3'd3) begin
                        FIFO_BufferFull_flg = `TRUE;
                    end
                    else begin
                        FIFO_BufferFull_flg = `FALSE;
                    end
                end
            end
            else begin
                FIFO_BufferFull_flg = `FALSE;
            end
    */

    if(M_AXIS_TREADY && MasterReadySend_flg) begin
        if(ValidSendNumNow == 1) begin
            if(FIFO_Size_next - 3'd1 > 3'd3) begin
                FIFO_BufferFull_flg = `TRUE;
            end
            else begin
                FIFO_BufferFull_flg = `FALSE;
            end

            if(FIFO_Size_next - 3'd1 == 3'd0) begin
                FIFO_BufferEmpty_flg = `TRUE;
            end
            else begin
                FIFO_BufferEmpty_flg = `FALSE;
            end
        end
        else begin
            if(FIFO_Size_next - 3'd2 > 3'd3) begin
                FIFO_BufferFull_flg = `TRUE;
            end
            else begin
                FIFO_BufferFull_flg = `FALSE;
            end

            if(FIFO_Size_next - 3'd2 == 3'd0) begin
                FIFO_BufferEmpty_flg = `TRUE;
            end
            else begin
                FIFO_BufferEmpty_flg = `FALSE;
            end
        end
    end
    else begin
        if(FIFO_Size_next > 3'd3) begin
            FIFO_BufferFull_flg = `TRUE;
        end
        else begin
            FIFO_BufferFull_flg = `FALSE;
        end

        if(FIFO_Size_next == 3'd0) begin
            FIFO_BufferEmpty_flg = `TRUE;
        end
        else begin
            FIFO_BufferEmpty_flg = `FALSE;
        end
    end

end
always @(*) begin
    if(FIFO_in_ptr < FIFO_out_ptr) begin
        ValidSendNumNow = FIFO_in_ptr - FIFO_out_ptr;
    end
    else if(FIFO_in_ptr > FIFO_out_ptr) begin
        ValidSendNumNow = FIFO_in_ptr - FIFO_out_ptr;
    end
    else begin
        ValidSendNumNow = FIFO_in_ptr - FIFO_out_ptr;
    end
end

always @(posedge M_AXIS_ACLK) begin
    if(!M_AXIS_ARESETN) begin
        FIFO_in_ptr <= 0;
    end
    else begin
        if(MasterReadyStore_flg) begin
            FIFO_in_ptr <= FIFO_in_ptr + CoreValidDataNum;
        end
    end
end
always @(posedge M_AXIS_ACLK) begin
    if(!M_AXIS_ARESETN) begin
        FIFO_out_ptr <= 0;
    end
    else begin
        if(M_AXIS_TREADY) begin
            if(MasterReadySend_flg) begin
                if(ValidSendNumNow == 1) begin
                    FIFO_out_ptr <= FIFO_out_ptr + 1;
                end
                else begin
                    FIFO_out_ptr <= FIFO_out_ptr + 2;
                end
            end
        end
    end
end

always @(posedge M_AXIS_ACLK) begin
    if(!M_AXIS_ARESETN) begin
        Data_tmp[0] <= 0;
        Data_tmp[1] <= 0;
        Data_tmp[2] <= 0;
        Data_tmp[3] <= 0;
    end
    else begin
        if(MasterReadyStore_flg) begin
            case (CoreValidDataNum)
                0: begin
                end
                1: begin
                    Data_tmp[FIFO_in_ptr]   <= send_data_s[send_data_valid[1]];
                end
                2: begin
                    Data_tmp[FIFO_in_ptr]   <= send_data_s[0];
                    Data_tmp[FIFO_in_ptr+2'd1] <= send_data_s[1];
                end
                default begin
                end
            endcase
        end
    end
end

endmodule
