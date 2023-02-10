
`timescale 1 ns / 1 ps

`include "TinyYolov3.vh"

module TinyYolov3_v1_0 #(parameter C_S00_AXIS_TDATA_WIDTH	 = 32,
                         parameter C_M00_AXIS_TDATA_WIDTH	 = 32,
                         parameter C_M00_AXIS_START_COUNT	 = 32,
                         parameter C_S_AXI_INTR_DATA_WIDTH	 = 32,
                         parameter C_S_AXI_INTR_ADDR_WIDTH	 = 5,
                         parameter C_NUM_OF_INTR	 = 3,
                         parameter C_INTR_SENSITIVITY	 = 32'hFFFFFFFF,
                         parameter C_INTR_ACTIVE_STATE	 = 32'hFFFFFFFF,
                         parameter C_IRQ_SENSITIVITY	 = 1,
                         parameter C_IRQ_ACTIVE_STATE	 = 1)
       (
`ifdef DEBUG
           //
`endif
           input wire s00_axis_aclk,
           input wire s00_axis_aresetn,
           output wire s00_axis_tready,
           input wire [C_S00_AXIS_TDATA_WIDTH-1 : 0] s00_axis_tdata,
           input wire [(C_S00_AXIS_TDATA_WIDTH/8)-1 : 0] s00_axis_tkeep,
           input wire s00_axis_tlast,
           input wire s00_axis_tvalid,
           input wire m00_axis_aclk,
           input wire m00_axis_aresetn,
           output wire m00_axis_tvalid,
           output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
           output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tkeep,
           output wire m00_axis_tlast,
           input wire m00_axis_tready,
           input wire s_axi_intr_aclk,
           input wire s_axi_intr_aresetn,
           input wire [C_S_AXI_INTR_ADDR_WIDTH-1 : 0] s_axi_intr_awaddr,
           input wire [2 : 0] s_axi_intr_awprot,
           input wire s_axi_intr_awvalid,
           output wire s_axi_intr_awready,
           input wire [C_S_AXI_INTR_DATA_WIDTH-1 : 0] s_axi_intr_wdata,
           input wire [(C_S_AXI_INTR_DATA_WIDTH/8)-1 : 0] s_axi_intr_wstrb,
           input wire s_axi_intr_wvalid,
           output wire s_axi_intr_wready,
           output wire [1 : 0] s_axi_intr_bresp,
           output wire s_axi_intr_bvalid,
           input wire s_axi_intr_bready,
           input wire [C_S_AXI_INTR_ADDR_WIDTH-1 : 0] s_axi_intr_araddr,
           input wire [2 : 0] s_axi_intr_arprot,
           input wire s_axi_intr_arvalid,
           output wire s_axi_intr_arready,
           output wire [C_S_AXI_INTR_DATA_WIDTH-1 : 0] s_axi_intr_rdata,
           output wire [1 : 0] s_axi_intr_rresp,
           output wire s_axi_intr_rvalid,
           input wire s_axi_intr_rready,
           output wire irq);

//log_2 function
function integer clogb2 (input integer bit_depth);
    begin
        for(clogb2 = 0; bit_depth>0; clogb2 = clogb2+1)
            bit_depth = bit_depth >> 1;
    end
endfunction
//
// Float Size Define
localparam integer CHAR_NUM_PER_FLOAT = 2;          //! 1 2 4
localparam integer FLOAT_NUM_PER_RECV = 2;          //! FLOAT_NUM_PER_RECV * CHAR_NUM_PER_FLOAT < = 4 && FLOAT_NUM_PER_RECV為2的次方(1 2 4)
localparam integer FLOAT_NUM_PER_SEND = FLOAT_NUM_PER_RECV;
//localparam integer LAYER_NUM        = 24;
localparam integer LAYER_NUM          = 3;          //!test
//! Do not modify
// Constant Define
localparam integer CHAR_BIT               = 8;
localparam integer FLOAT_BIT	          = CHAR_BIT * CHAR_NUM_PER_FLOAT;

localparam integer LAYER_BIT_NUM          = 5;
localparam integer RECV_DATA_TYPE_NUM     = 3;
localparam integer SEND_DATA_TYPE_NUM     = 2;

localparam integer RECV_START_BIT_NUM     = 10;
localparam integer RECV_LINES_BIT_NUM     = 3;
//!

//Receiver
wire recv_packet_finish_flg;
wire recv_ready_read_flg;
wire [FLOAT_NUM_PER_RECV-1:0] recv_float_data_valid;
wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] recv_float_data;
//Sender
wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] send_data;
wire [FLOAT_NUM_PER_RECV-1 : 0] send_data_valid;
wire send_core_ready_send_flg;
wire send_packet_finish_flg;
wire send_ready_send_flg;
//Interrup
wire [C_NUM_OF_INTR-1 : 0] intr_from_core;
wire [C_NUM_OF_INTR-1 : 0] intr_ack;
wire [32-1 : 0] intr_Data;
wire [32-1 : 0] intr_RecvSendRequest;
wire [32-1 : 0] intr_SendSendRequest;
wire [32-1 : 0] intr_Ctrl;

// Instantiation of Axi Bus Interface S00_AXIS
TinyYolov3_v1_0_S00_AXIS # (
                             .C_S_AXIS_TDATA_WIDTH(C_S00_AXIS_TDATA_WIDTH),
                             .CHAR_NUM_PER_FLOAT(CHAR_NUM_PER_FLOAT),
                             .FLOAT_NUM_PER_RECV(FLOAT_NUM_PER_RECV)
                         ) TinyYolov3_v1_0_S00_AXIS_inst(
`ifdef DEBUG
    `endif
                             .S_AXIS_ACLK(s00_axis_aclk),
                             .S_AXIS_ARESETN(s00_axis_aresetn),
                             .S_AXIS_TREADY(s00_axis_tready),
                             .S_AXIS_TDATA(s00_axis_tdata),
                             .S_AXIS_TLAST(s00_axis_tlast),
                             .S_AXIS_TVALID(s00_axis_tvalid),
                             .S_AXIS_TKEEP(s00_axis_tkeep),
                             .core_ready_read(recv_ready_read_flg),
                             .packet_finish(recv_packet_finish_flg),
                             .float_data_valid(recv_float_data_valid),
                             .float_data(recv_float_data)
                         );

// Instantiation of Axi Bus Interface M00_AXIS
TinyYolov3_v1_0_M00_AXIS # (
                             .C_M_AXIS_TDATA_WIDTH(C_M00_AXIS_TDATA_WIDTH),
                             .C_M_START_COUNT(C_M00_AXIS_START_COUNT),
                             .CHAR_NUM_PER_FLOAT(CHAR_NUM_PER_FLOAT),
                             .FLOAT_NUM_PER_RECV(FLOAT_NUM_PER_RECV)
                         ) TinyYolov3_v1_0_M00_AXIS_inst (
                             .M_AXIS_ACLK(m00_axis_aclk),
                             .M_AXIS_ARESETN(m00_axis_aresetn),
                             .M_AXIS_TVALID(m00_axis_tvalid),
                             .M_AXIS_TDATA(m00_axis_tdata),
                             .M_AXIS_TKEEP(m00_axis_tkeep),
                             .M_AXIS_TLAST(m00_axis_tlast),
                             .M_AXIS_TREADY(m00_axis_tready),
                             .send_data(send_data),
                             .send_data_valid(send_data_valid),
                             .send_core_ready_send_flg(send_core_ready_send_flg),
                             .send_packet_finish_flg(send_packet_finish_flg),
                             .send_ready_send_flg(send_ready_send_flg)
                         );

// Instantiation of Axi Bus Interface S_AXI_INTR
TinyYolov3_v1_0_S_AXI_INTR # (
                               .C_S_AXI_DATA_WIDTH(C_S_AXI_INTR_DATA_WIDTH),
                               .C_S_AXI_ADDR_WIDTH(C_S_AXI_INTR_ADDR_WIDTH),
                               .C_NUM_OF_INTR(C_NUM_OF_INTR),
                               .C_INTR_SENSITIVITY(C_INTR_SENSITIVITY),
                               .C_INTR_ACTIVE_STATE(C_INTR_ACTIVE_STATE),
                               .C_IRQ_SENSITIVITY(C_IRQ_SENSITIVITY),
                               .C_IRQ_ACTIVE_STATE(C_IRQ_ACTIVE_STATE),

                               .LAYER_BIT_NUM(LAYER_BIT_NUM),
                               .RECV_DATA_TYPE_NUM(RECV_DATA_TYPE_NUM),
                               .SEND_DATA_TYPE_NUM(SEND_DATA_TYPE_NUM),
                               .RECV_START_BIT_NUM(RECV_START_BIT_NUM),
                               .RECV_LINES_BIT_NUM(RECV_LINES_BIT_NUM)
                           ) TinyYolov3_v1_0_S_AXI_INTR_inst (
                               .S_AXI_ACLK(s_axi_intr_aclk),
                               .S_AXI_ARESETN(s_axi_intr_aresetn),
                               .S_AXI_AWADDR(s_axi_intr_awaddr),
                               .S_AXI_AWPROT(s_axi_intr_awprot),
                               .S_AXI_AWVALID(s_axi_intr_awvalid),
                               .S_AXI_AWREADY(s_axi_intr_awready),
                               .S_AXI_WDATA(s_axi_intr_wdata),
                               .S_AXI_WSTRB(s_axi_intr_wstrb),
                               .S_AXI_WVALID(s_axi_intr_wvalid),
                               .S_AXI_WREADY(s_axi_intr_wready),
                               .S_AXI_BRESP(s_axi_intr_bresp),
                               .S_AXI_BVALID(s_axi_intr_bvalid),
                               .S_AXI_BREADY(s_axi_intr_bready),
                               .S_AXI_ARADDR(s_axi_intr_araddr),
                               .S_AXI_ARPROT(s_axi_intr_arprot),
                               .S_AXI_ARVALID(s_axi_intr_arvalid),
                               .S_AXI_ARREADY(s_axi_intr_arready),
                               .S_AXI_RDATA(s_axi_intr_rdata),
                               .S_AXI_RRESP(s_axi_intr_rresp),
                               .S_AXI_RVALID(s_axi_intr_rvalid),
                               .S_AXI_RREADY(s_axi_intr_rready),
                               .irq(irq),

                               .intr_ack(intr_ack),
                               .intr_from_core(intr_from_core),
                               .intr_Data(intr_Data),
                               .intr_RecvSendRequest(intr_RecvSendRequest),
                               .intr_SendSendRequest(intr_SendSendRequest),
                               .intr_Ctrl(intr_Ctrl)
                           );

// Add user logic here
TinyYolov3Core  # (
                    .CHAR_NUM_PER_FLOAT(CHAR_NUM_PER_FLOAT),
                    .FLOAT_NUM_PER_RECV(FLOAT_NUM_PER_RECV),
                    .FLOAT_NUM_PER_SEND(FLOAT_NUM_PER_SEND),
                    .C_NUM_OF_INTR(C_NUM_OF_INTR),

                    .LAYER_NUM(LAYER_NUM),
                    .LAYER_BIT_NUM(LAYER_BIT_NUM),
                    .RECV_DATA_TYPE_NUM(RECV_DATA_TYPE_NUM),
                    .SEND_DATA_TYPE_NUM(SEND_DATA_TYPE_NUM),
                    .RECV_START_BIT_NUM(RECV_START_BIT_NUM),
                    .RECV_LINES_BIT_NUM(RECV_LINES_BIT_NUM)
                )  TinyYolov3Core_inst (
`ifdef DEBUG
    `endif
                    .clk(s00_axis_aclk),
                    .rst_n(s00_axis_aresetn),
                    //Receiver(S_00_AXIS)
                    .recv_float_data(recv_float_data),
                    .recv_float_data_valid(recv_float_data_valid),
                    .recv_ready_read_flg(recv_ready_read_flg),
                    .recv_packet_finish_flg(recv_packet_finish_flg),
                    //Sender(M_00_AXIS)
                    .send_data(send_data),
                    .send_data_valid(send_data_valid),
                    .send_core_ready_send_flg(send_core_ready_send_flg),
                    .send_packet_finish_flg(send_packet_finish_flg),
                    .send_ready_send_flg(send_ready_send_flg),
                    //Interrupt(S_AXI_INTR)
                    .intr_ack(intr_ack),
                    .intr_from_core(intr_from_core),
                    .intr_Data(intr_Data),
                    .intr_RecvSendRequest(intr_RecvSendRequest),
                    .intr_SendSendRequest(intr_SendSendRequest),
                    .intr_Ctrl(intr_Ctrl)
                );
// User logic ends

endmodule
