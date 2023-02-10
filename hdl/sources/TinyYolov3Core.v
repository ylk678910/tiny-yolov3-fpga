`timescale 1ns / 1ps
`include "TinyYolov3.vh"
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 2021/08/02 17:11:31
// Design Name:
// Module Name: TinyYolov3Core
// Project Name:
// Target Devices:
// Tool Versions:
// Description:
//
// Dependencies:
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
//////////////////////////////////////////////////////////////////////////////////


module TinyYolov3Core #(
           parameter integer CHAR_NUM_PER_FLOAT = 2, //! 1 2 4
           parameter integer FLOAT_NUM_PER_RECV = 2, //! FLOAT_NUM_PER_RECV * CHAR_NUM_PER_FLOAT <= 4 && FLOAT_NUM_PER_RECV為2的次方(1 2 4)
           parameter integer FLOAT_NUM_PER_SEND = FLOAT_NUM_PER_RECV,
           parameter integer C_NUM_OF_INTR = 3,

           parameter integer LAYER_NUM = 24,
           parameter integer LAYER_BIT_NUM = 5,
           parameter integer RECV_DATA_TYPE_NUM = 3,
           parameter integer SEND_DATA_TYPE_NUM = 2,
           parameter integer RECV_START_BIT_NUM = 10,
           parameter integer RECV_LINES_BIT_NUM = 3
       )
       (
`ifdef DEBUG
`endif
           clk,
           rst_n,
           //Receiver(S_00_AXIS)
           recv_float_data,
           recv_float_data_valid,
           recv_ready_read_flg,
           recv_packet_finish_flg,
           //Sender(M_00_AXIS)
           send_data,
           send_data_valid,
           send_core_ready_send_flg,
           send_packet_finish_flg,
           send_ready_send_flg,
           //Interrupt(S_AXI_INTR)
           intr_ack,
           intr_from_core,
           intr_Data,
           intr_RecvSendRequest,
           intr_SendSendRequest,
           intr_Ctrl
       );

//log_2 function
function integer clogb2 (input integer bit_depth);
    begin
        for(clogb2 = 0; bit_depth>0; clogb2 = clogb2+1)
            bit_depth = bit_depth >> 1;
    end
endfunction
//
//! Do not modify here
localparam integer CHAR_BIT             = 8;
localparam integer FLOAT_BIT	        = CHAR_BIT * CHAR_NUM_PER_FLOAT;
//!
input wire clk;
input wire rst_n;
//Receiver(S_00_AXIS)
input wire [FLOAT_NUM_PER_RECV-1:0] recv_float_data_valid;
input wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] recv_float_data;
input wire recv_packet_finish_flg;
output wire recv_ready_read_flg;
//Sender(M_00_AXIS)
output wire [FLOAT_BIT*FLOAT_NUM_PER_RECV-1:0] send_data;
output wire [FLOAT_NUM_PER_RECV-1 : 0] send_data_valid;
output wire send_core_ready_send_flg;
input wire send_packet_finish_flg;
input wire send_ready_send_flg;
//Interrupt(S_AXI_INTR)
input wire [C_NUM_OF_INTR-1 : 0] intr_ack;
input wire [C_NUM_OF_INTR-1 : 0] intr_from_core;
output wire [32-1 : 0] intr_Data;
output wire [32-1 : 0] intr_RecvSendRequest;
output wire [32-1 : 0] intr_SendSendRequest;
input wire [32-1 : 0] intr_Ctrl;

//Control
wire Ctrl_Start_flg = intr_Ctrl[0];

reg AllDone_flg;

//Receiver Data and Valid
genvar float_ptr;
generate
    for(float_ptr = 0; float_ptr<FLOAT_NUM_PER_RECV; float_ptr=float_ptr+1) begin: RECV
        wire valid;
        wire [FLOAT_BIT-1 : 0] data;
        assign valid = recv_float_data_valid[float_ptr];
        assign data = recv_float_data[(FLOAT_BIT*(FLOAT_NUM_PER_RECV - float_ptr))-1 -: FLOAT_BIT];
    end
endgenerate

//Sender Data and Valid
generate
    for(float_ptr = 0; float_ptr<FLOAT_NUM_PER_SEND; float_ptr=float_ptr+1) begin: SEND
        reg valid;
        reg [FLOAT_BIT-1 : 0] data;
        assign send_data_valid[float_ptr] = valid;
        assign send_data[FLOAT_BIT*float_ptr +: FLOAT_BIT] = data;
        //assign send_data[(FLOAT_BIT*(FLOAT_NUM_PER_SEND - float_ptr))-1 -: FLOAT_BIT] = data;
    end
endgenerate

/*===================================================================================================*/
// Localparam Define
// - 1.Generl
/*---------------------------------------------------------------------------------------------------*/
// - 2.Layer Control
// - - a.FSM State
// - - - a_1.State
localparam LCTRL_IDLE = 0,
           LCTRL_CONV_LAYER = 1; //!LCTRL_LAYER_TYPE_NUM, LCTRL_STATE_NUM記得改
// - - - a_2.Other
localparam LCTRL_LAYER_TYPE_NUM = 1,
           LCTRL_STATE_NUM = 2;
/*---------------------------------------------------------------------------------------------------*/
// - 2.Receiver
// - - a.FSM State
// - - - a_1.State
localparam RECV_IDLE = 0,
           RECV_SEND_REQUEST = 1,
           RECV_PACKET = 2,
           RECV_PACKET_OK = 3;
localparam RECV_STATE_NUM = 4;
// - - - - - - RECV_SEND_REQUEST
localparam DATA_TYPE_NEW_IMAGE = 0;
localparam DATA_TYPE_STORED_DATA = 1;
localparam DATA_TYPE_WEIGHT = 2;

// - - - - - - RECV_PACKET
//
// - - - - - - RECV_PACKET_OK
//
// - - - - - - RECV_PACKET_ERROR
//
/*---------------------------------------------------------------------------------------------------*/
// - 3.Sender
// - - a.FSM State
// - - - a_1.State
localparam SEND_IDLE = 0,
           SEND_SEND_REQUEST = 1,
           SEND_PACKET = 2,
           SEND_PACKET_OK = 3;
localparam SEND_STATE_NUM = 4;
// - - - - - - SEND_SEND_REQUEST
localparam DATA_TYPE_FINAL_OUTPUT = 0;
//localparam DATA_TYPE_STORED_DATA = 1; //!Defined earlier

// - - - - - - SEND_PACKET
//
// - - - - - - SEND_PACKET_OK
//
// - - - - - - SEND_PACKET_ERROR
//
/*===================================================================================================*/
// reg/wire Define
// - 1.Generl
reg [clogb2(LAYER_NUM-1)-1 : 0] LayerNow;
wire [clogb2(LAYER_NUM-1)-1 : 0] LayerNext = LayerNow + 1;
/*---------------------------------------------------------------------------------------------------*/
// - 2.Layer Control
// - - a.FSM State
reg [clogb2(LCTRL_STATE_NUM-1)-1:0] LCtrl_CurrState;
reg [clogb2(LCTRL_STATE_NUM-1)-1:0] LCtrl_NextState;
// - - b.Flag
reg [clogb2(LCTRL_LAYER_TYPE_NUM)-1 : 0] LCtrl_Layer_Type; //Layer Type Now
// - - - b_1.Next State Flag
reg LCtrl_NextLayer_flg;
reg LCtrl_DoneLayer_flg;
reg LCtrl_FirstClkOfLayer_flg;
wire LCtrl_Busy_flg = (LCtrl_CurrState != LCTRL_IDLE);
wire LCtrl_LastLayerNow_flg = (LayerNow == LAYER_NUM - 1);
// - - - b_2.LCTRL_CONV Flag
reg LCtrl_CONV_Layer_DBL_flg;
// - - c.Request Setting
// - - - c_1.Receive Data Setting
reg [clogb2(RECV_DATA_TYPE_NUM-1)-1 : 0] LCtrl_Recv_ReqDataType;
reg [RECV_START_BIT_NUM-1 : 0] LCtrl_Recv_ReqStartLocation;
reg [RECV_LINES_BIT_NUM-1 : 0] LCtrl_Recv_ReqLineNum;
reg [LAYER_BIT_NUM-1 : 0] LCtrl_Recv_ReqLayer;
// - - - c_2.Send Data Setting
reg [clogb2(SEND_DATA_TYPE_NUM-1)-1 : 0] LCtrl_Send_ReqDataType;
reg [LAYER_BIT_NUM-1 : 0] LCtrl_Send_ReqLayer;
/*---------------------------------------------------------------------------------------------------*/
// - 3.Receiver
// - - a.FSM State
reg [clogb2(RECV_STATE_NUM-1)-1:0] Recv_CurrState;
reg [clogb2(RECV_STATE_NUM-1)-1:0] Recv_NextState;
// - - b.State
// - - - RECV_SEND_REQUEST(定點數bit數 = FLOAT_BIT)
reg [clogb2(RECV_DATA_TYPE_NUM-1)-1 : 0] RecvSendRequest_DataType;   //Receiver Send Request Data Type
reg [RECV_START_BIT_NUM-1 : 0] RecvSendRequest_StartLocation;        //Receiver Send Request Data Start Location
reg [RECV_LINES_BIT_NUM-1 : 0] RecvSendRequest_LineNum;              //Receiver Send Request Number of Line
reg [LAYER_BIT_NUM-1 : 0] RecvSendRequest_Layer;                     //Receiver Send Request Layer
// - - - RECV_PACKET
wire RecvPacket_GotAll_flg;
//assign RecvPacket_GotAll_flg = Recv_PacketFinish_flg;
assign RecvPacket_GotAll_flg = recv_packet_finish_flg;
reg RecvPacket_ReadyRead_flg;                                               //Ready to receive next data
assign recv_ready_read_flg = RecvPacket_ReadyRead_flg && (Recv_CurrState==RECV_PACKET);
// - - - RECV_PACKET_OK
// - - c.Flag
// - - - c_1.Next State Flag
reg RecvSendRequest_flg;
wire Recv_Ack = intr_ack[`INTR_RECV];
wire Recv_PacketFinish_flg = recv_packet_finish_flg;
// - - d.Receiver Interrupt
reg RecvSendRequest_Intr;
assign intr_from_core[`INTR_RECV] = RecvSendRequest_Intr;
/*---------------------------------------------------------------------------------------------------*/
// - 4.Sender
// - - a.FSM State
reg [clogb2(SEND_STATE_NUM-1)-1:0] Send_CurrState;
reg [clogb2(SEND_STATE_NUM-1)-1:0] Send_NextState;
// - - b.State
// - - - SEND_SEND_REQUEST(定點數bit數 = FLOAT_BIT)
reg [clogb2(SEND_DATA_TYPE_NUM-1)-1 : 0] SendSendRequest_DataType;   //Sender Send Request Data Type
reg [LAYER_BIT_NUM-1 : 0] SendSendRequest_Layer;                     //Sender Send Request Layer
// - - - SEND_PACKET
reg SendPacket_ReadySend_flg;                                               //Ready to send next data
assign send_core_ready_send_flg = SendPacket_ReadySend_flg && (Send_CurrState==SEND_PACKET);
wire SendPacket_SenderReady_flg = send_ready_send_flg;                       //Sender Ready to Send Next Data
// - - - SEND_PACKET_OK
// - - c.Flag
// - - - c_1.Next State Flag
reg SendSendRequest_flg;
wire Send_Ack = intr_ack[`INTR_SEND];
reg Send_PacketFinish_flg;
assign send_packet_finish_flg = Send_PacketFinish_flg;
// - - d.Sender Interrupt
reg SendSendRequest_Intr;
assign intr_from_core[`INTR_SEND] = SendSendRequest_Intr;
/*---------------------------------------------------------------------------------------------------*/
// - 5.Other
assign intr_Data            = {{32- 0{1'b0}}};
assign intr_RecvSendRequest = {{32-20{1'b0}}, RecvSendRequest_DataType, RecvSendRequest_StartLocation, RecvSendRequest_LineNum, RecvSendRequest_Layer};
assign intr_SendSendRequest = {{32- 6{1'b0}}, SendSendRequest_DataType, SendSendRequest_Layer};

//!test
reg Test_RecvReadyRead_flg;
reg Test_Counter;
reg Test_Send_RecvRequest;
reg Test_Send_SendRequest;
reg [clogb2(5-1)-1:0]Test_Times;
reg [clogb2(416-1)-1:0] Test_Line;
reg [clogb2(1024-1)-1:0] Test_Filter;
/*===================================================================================================*/
// Receiver FMS Start
// - 1.FMS Current State Control
always @(posedge clk) begin
    if(!rst_n) begin
        Recv_CurrState <= RECV_IDLE;
    end
    else begin
        Recv_CurrState <= Recv_NextState;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 2.FMS Next State Control
always @(*) begin
    case (Recv_CurrState)
        RECV_IDLE: begin
            if(RecvSendRequest_flg && !AllDone_flg) begin
                Recv_NextState = RECV_SEND_REQUEST;
            end
            else begin
                Recv_NextState = RECV_IDLE;
            end
        end
        RECV_SEND_REQUEST: begin
            if(Recv_Ack) begin
                Recv_NextState = RECV_PACKET;
            end
            else begin
                Recv_NextState = RECV_SEND_REQUEST;
            end
        end
        RECV_PACKET: begin
            if(RecvPacket_GotAll_flg && RecvPacket_ReadyRead_flg) begin              //Receiver收到的封包結束訊號
                Recv_NextState = RECV_PACKET_OK;
            end
            else begin
                Recv_NextState = RECV_PACKET;
            end
        end
        RECV_PACKET_OK: begin
            Recv_NextState = RECV_IDLE;
        end
    endcase
end
/*---------------------------------------------------------------------------------------------------*/
// - 3.Receiver Request Control
always @(posedge clk) begin
    if(!rst_n) begin
        RecvSendRequest_DataType        <= DATA_TYPE_NEW_IMAGE;
        RecvSendRequest_StartLocation   <= {RECV_START_BIT_NUM{1'b0}};
        RecvSendRequest_LineNum         <= {RECV_LINES_BIT_NUM{1'b0}};
        RecvSendRequest_Layer           <= {LAYER_BIT_NUM{1'b0}};
    end
    else if(Recv_CurrState == RECV_SEND_REQUEST) begin
        RecvSendRequest_DataType        <= LCtrl_Recv_ReqDataType;
        RecvSendRequest_StartLocation   <= LCtrl_Recv_ReqStartLocation;
        RecvSendRequest_LineNum         <= LCtrl_Recv_ReqLineNum;
        RecvSendRequest_Layer           <= LCtrl_Recv_ReqLayer;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 4.Receiver Interrupt Control
always @(posedge clk) begin
    if(!rst_n) begin
        RecvSendRequest_Intr <= `FALSE;
    end
    else if(Recv_CurrState == RECV_SEND_REQUEST) begin
        //
        if(!Recv_Ack) begin
            RecvSendRequest_Intr <= `TRUE;
        end
        else begin
            RecvSendRequest_Intr <= `FALSE;
        end
    end
    else begin
        RecvSendRequest_Intr <= `FALSE;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 5.Receiver Number of Data Control
/*===================================================================================================*/
// Sender FMS Start
// - 1.FMS Current State Control
always @(posedge clk) begin
    if(!rst_n) begin
        Send_CurrState <= SEND_IDLE;
    end
    else begin
        Send_CurrState <= Send_NextState;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 2.FMS Next State Control
always @(*) begin
    case (Send_CurrState)
        SEND_IDLE: begin
            if(SendSendRequest_flg && !AllDone_flg) begin
                Send_NextState = SEND_SEND_REQUEST;
            end
            else begin
                Send_NextState = SEND_IDLE;
            end
        end
        SEND_SEND_REQUEST: begin
            if(Send_Ack) begin
                Send_NextState = SEND_PACKET;
            end
            else begin
                Send_NextState = SEND_SEND_REQUEST;
            end
        end
        SEND_PACKET: begin
            if(Send_PacketFinish_flg && SendPacket_SenderReady_flg) begin   //Sender收到的封包結束訊號
                Send_NextState = SEND_PACKET_OK;
            end
            else begin
                Send_NextState = SEND_PACKET;
            end
        end
        SEND_PACKET_OK: begin
            Send_NextState = SEND_IDLE;
        end
    endcase
end
/*---------------------------------------------------------------------------------------------------*/
// - 3.Sender Request Control
always @(posedge clk) begin
    if(!rst_n) begin
        SendSendRequest_DataType        <= DATA_TYPE_STORED_DATA;
        SendSendRequest_Layer           <= {LAYER_BIT_NUM{1'b0}};
    end
    else if(Send_CurrState == SEND_SEND_REQUEST) begin
        SendSendRequest_DataType        <= LCtrl_Send_ReqDataType;
        SendSendRequest_Layer           <= LCtrl_Send_ReqLayer;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 4.Sender Interrupt Control
always @(posedge clk) begin
    if(!rst_n) begin
        SendSendRequest_Intr <= `FALSE;
    end
    else if(Send_CurrState == SEND_SEND_REQUEST) begin
        //
        if(!Send_Ack) begin
            SendSendRequest_Intr <= `TRUE;
        end
        else begin
            SendSendRequest_Intr <= `FALSE;
        end
    end
    else begin
        SendSendRequest_Intr <= `FALSE;
    end
end
/*===================================================================================================*/
// Layer Control FMS Start
// - 1.FMS Current State Control
always @(posedge clk) begin
    if(!rst_n) begin
        LCtrl_CurrState <= LCTRL_IDLE;
    end
    else begin
        LCtrl_CurrState <= LCtrl_NextState;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 2.FMS Next State Control
always @(*) begin
    if(LCtrl_NextLayer_flg) begin
        if(!LCtrl_LastLayerNow_flg) begin
            LCtrl_NextState = LCtrl_Layer_Type;
        end
        else begin
            LCtrl_NextState = LCTRL_IDLE;
        end
    end
    else begin
        LCtrl_NextState = LCtrl_CurrState;
    end
end
/*---------------------------------------------------------------------------------------------------*/
// - 3.Layer Control
always @(posedge clk) begin
    if(!rst_n) begin
        LCtrl_NextLayer_flg <= `FALSE;
    end
    else begin
        if(!LCtrl_NextLayer_flg) begin
            if(!LCtrl_Busy_flg && Ctrl_Start_flg && !AllDone_flg) begin           //在IDLE，同時PS給出開始訊號後
                LCtrl_NextLayer_flg = `TRUE;
            end
            else if(LCtrl_DoneLayer_flg) begin
                LCtrl_NextLayer_flg <= `TRUE;
            end
            else begin
                LCtrl_NextLayer_flg <= `FALSE;
            end
        end
        else begin
            LCtrl_NextLayer_flg <= `FALSE;
        end
    end
end

always @(posedge clk) begin
    if(!rst_n) begin
        LCtrl_FirstClkOfLayer_flg <= `FALSE;
    end
    else begin
        LCtrl_FirstClkOfLayer_flg <= LCtrl_NextLayer_flg;
    end
end

always @(posedge clk) begin
    if(!rst_n) begin
        LayerNow <= 0;
    end
    else begin
        if(LCtrl_Busy_flg && LCtrl_NextLayer_flg) begin
            if(LCtrl_LastLayerNow_flg)
                LayerNow <= 0;
            else
                LayerNow <= LayerNow + 1;
        end
    end
end

always @(posedge clk) begin
    if(!rst_n) begin
        LCtrl_DoneLayer_flg <= `FALSE;
    end
    else begin
        if(LCtrl_Busy_flg && !LCtrl_NextLayer_flg) begin
            //!test
            case (LayerNow)
                0: begin
                    if(Recv_CurrState == RECV_PACKET_OK && Test_Times==4 && Test_Counter==1) begin          //!test
                        LCtrl_DoneLayer_flg <= `TRUE;
                    end
                    else begin
                        LCtrl_DoneLayer_flg <= `FALSE;
                    end
                end
                1: begin
                    if(Send_CurrState == SEND_PACKET_OK) begin          //!test
                        LCtrl_DoneLayer_flg <= `TRUE;
                    end
                    else begin
                        LCtrl_DoneLayer_flg <= `FALSE;
                    end
                end
                2: begin
                    if(Recv_CurrState == RECV_PACKET_OK && Test_Times==2 && Test_Counter==1) begin          //!test
                        LCtrl_DoneLayer_flg <= `TRUE;
                    end
                    else begin
                        LCtrl_DoneLayer_flg <= `FALSE;
                    end
                end
                default begin
                end
            endcase
            //!test
        end
        else begin
            LCtrl_DoneLayer_flg <= `FALSE;
        end
    end
end
/*===================================================================================================*/
// Layer Configuration
// - Layer Type
always @(*) begin
    case (LayerNow)
        0: begin                        //Layer 0 Configuration
            LCtrl_Layer_Type = LCTRL_CONV_LAYER;
            LCtrl_CONV_Layer_DBL_flg = `TRUE;
        end
        1: begin                        //Layer 1 Configuration //!test
            LCtrl_Layer_Type = LCTRL_CONV_LAYER;
            LCtrl_CONV_Layer_DBL_flg = `FALSE;
        end
        2: begin                        //Layer 1 Configuration //!test
            LCtrl_Layer_Type = LCTRL_CONV_LAYER;
            LCtrl_CONV_Layer_DBL_flg = `FALSE;
        end
        default: begin
            LCtrl_Layer_Type = LCTRL_IDLE; //Error Occur
            LCtrl_CONV_Layer_DBL_flg = `FALSE;
        end
    endcase
end
/*---------------------------------------------------------------------------------------------------*/
// - Layer Receiver Configuration
// - - 1.Request Control
always @(posedge clk) begin
    if(!rst_n) begin
        RecvSendRequest_flg <= `FALSE;
    end
    else begin
        //!test
        if(!LCtrl_DoneLayer_flg && LCtrl_Busy_flg) begin //!test 測試code:該層一開始時候給出一個request
            case (LayerNow)
                0: begin                        //Layer 0 Configuration
                    if(Test_Send_RecvRequest) begin
                        RecvSendRequest_flg <= `TRUE;
                    end
                    else begin
                        RecvSendRequest_flg <= `FALSE;
                    end
                end
                1: begin                        //Layer 1 Configuration
                    if(Test_Send_RecvRequest) begin
                        RecvSendRequest_flg <= `TRUE;
                    end
                    else begin
                        RecvSendRequest_flg <= `FALSE;
                    end
                end
                2: begin                        //Layer 2 Configuration
                    if(Test_Send_RecvRequest) begin
                        RecvSendRequest_flg <= `TRUE;
                    end
                    else begin
                        RecvSendRequest_flg <= `FALSE;
                    end
                end
                default: begin
                    RecvSendRequest_flg <= `FALSE;
                end
            endcase
        end
        else begin
            RecvSendRequest_flg <= `FALSE;
        end
        //!test
    end
end

always @(*) begin
    //rst_n==1   LCtrl_DoneLayer_flg==0  LayerNow==0
    //!test
    if(!LCtrl_DoneLayer_flg && LCtrl_Busy_flg) begin
        case (LayerNow)
            0: begin                        //Layer 0 Configuration
                RecvPacket_ReadyRead_flg = Test_RecvReadyRead_flg;
            end
            1: begin                        //Layer 1 Configuration
                RecvPacket_ReadyRead_flg = Test_RecvReadyRead_flg;
            end
            2: begin                        //Layer 2 Configuration
                RecvPacket_ReadyRead_flg = Test_RecvReadyRead_flg;
            end
            default: begin
                RecvPacket_ReadyRead_flg = `FALSE;
            end
        endcase
    end
    else begin
        RecvPacket_ReadyRead_flg = `FALSE;
    end
    //!test
end
// - - 2.Request Setting
always @(*) begin
    case (LayerNow)
        0: begin                        //Layer 0 Configuration //!test
            case (Test_Counter)
                0: begin
                    LCtrl_Recv_ReqDataType      = DATA_TYPE_NEW_IMAGE;
                    LCtrl_Recv_ReqStartLocation = Test_Line;
                    LCtrl_Recv_ReqLineNum       = 3;
                    LCtrl_Recv_ReqLayer         = 0;                    //DATA_TYPE_NEW_IMAGE no need
                end
                1: begin
                    LCtrl_Recv_ReqDataType      = DATA_TYPE_WEIGHT;
                    LCtrl_Recv_ReqStartLocation = Test_Filter;        //!一定要是偶數(含0)
                    LCtrl_Recv_ReqLineNum       = 0;                    //DATA_TYPE_WEIGHT no need
                    LCtrl_Recv_ReqLayer         = LayerNow;
                end
            endcase
        end
        1: begin                        //Layer 1 Configuration //!test
            LCtrl_Recv_ReqDataType      = DATA_TYPE_STORED_DATA;
            LCtrl_Recv_ReqStartLocation = 1;
            LCtrl_Recv_ReqLineNum       = 7;
            LCtrl_Recv_ReqLayer         = 0;
        end
        2: begin                        //Layer 2 Configuration //!test
            case (Test_Counter)
                0: begin
                    LCtrl_Recv_ReqDataType      = DATA_TYPE_STORED_DATA;
                    LCtrl_Recv_ReqStartLocation = Test_Line;
                    LCtrl_Recv_ReqLineNum       = 2;
                    LCtrl_Recv_ReqLayer         = 1;
                end
                1: begin
                    LCtrl_Recv_ReqDataType      = DATA_TYPE_WEIGHT;
                    LCtrl_Recv_ReqStartLocation = Test_Filter;        //!一定要是偶數(含0)
                    LCtrl_Recv_ReqLineNum       = 0;                    //DATA_TYPE_WEIGHT no need
                    LCtrl_Recv_ReqLayer         = 15;
                end
            endcase
        end
        default: begin
            LCtrl_Recv_ReqDataType      = DATA_TYPE_NEW_IMAGE;
            LCtrl_Recv_ReqStartLocation = {RECV_START_BIT_NUM{1'b0}};
            LCtrl_Recv_ReqLineNum       = {RECV_LINES_BIT_NUM{1'b0}};
            LCtrl_Recv_ReqLayer         = {LAYER_BIT_NUM{1'b0}};
        end
    endcase
end
/*---------------------------------------------------------------------------------------------------*/
// - Layer Sender Configuration
// - - 1.Request Control
always @(posedge clk) begin
    if(!rst_n) begin
        SendSendRequest_flg <= `FALSE;
    end
    else begin
        //!test
        if(!LCtrl_DoneLayer_flg && LCtrl_Busy_flg) begin
            case (LayerNow)
                0: begin                        //Layer 0 Configuration
                    if(Test_Send_SendRequest) begin
                        SendSendRequest_flg <= `TRUE;
                    end
                    else begin
                        SendSendRequest_flg <= `FALSE;
                    end
                end
                1: begin                        //Layer 1 Configuration
                    if(Test_Send_SendRequest) begin
                        SendSendRequest_flg <= `TRUE;
                    end
                    else begin
                        SendSendRequest_flg <= `FALSE;
                    end
                end
                2: begin                        //Layer 2 Configuration
                    if(Test_Send_SendRequest) begin
                        SendSendRequest_flg <= `TRUE;
                    end
                    else begin
                        SendSendRequest_flg <= `FALSE;
                    end
                end
                default: begin
                    SendSendRequest_flg <= `FALSE;
                end
            endcase
        end
        else begin
            SendSendRequest_flg <= `FALSE;
        end
        //!test
    end
end

always @(posedge clk) begin
    if(!rst_n) begin
        SendPacket_ReadySend_flg <= `FALSE;
    end
    else begin
        //!test
        if(!LCtrl_DoneLayer_flg && LCtrl_Busy_flg) begin
            case (LayerNow)
                0: begin                        //Layer 0 Configuration
                    if(Test_Counter==0) begin
                        SendPacket_ReadySend_flg <= `TRUE;
                    end
                    else if(SendPacket_SenderReady_flg) begin   //確定sender接收到所有送出的數值後
                        SendPacket_ReadySend_flg <= `FALSE;
                    end
                end
                1: begin                        //Layer 1 Configuration
                    SendPacket_ReadySend_flg <= `TRUE;
                end
                2: begin                        //Layer 2 Configuration
                    SendPacket_ReadySend_flg <= `TRUE;
                end
                default: begin
                    SendPacket_ReadySend_flg <= `FALSE;
                end
            endcase
        end
        else begin
            SendPacket_ReadySend_flg <= SendPacket_ReadySend_flg;
        end
        //!test
    end
end

// - - 2.Request Setting
always @(*) begin
    case (LayerNow)
        0: begin                        //Layer 0 Configuration //!test
            LCtrl_Send_ReqDataType      = DATA_TYPE_STORED_DATA;
            LCtrl_Send_ReqLayer         = LayerNow;
        end
        1: begin                        //Layer 1 Configuration //!test
            LCtrl_Send_ReqDataType      = DATA_TYPE_STORED_DATA;
            LCtrl_Send_ReqLayer         = LayerNow;
        end
        2: begin                        //Layer 1 Configuration //!test
            LCtrl_Send_ReqDataType      = DATA_TYPE_STORED_DATA;
            LCtrl_Send_ReqLayer         = LayerNow;
        end
        default: begin
            LCtrl_Send_ReqDataType      = DATA_TYPE_STORED_DATA;
            LCtrl_Send_ReqLayer         = {LAYER_BIT_NUM{1'b0}};
        end
    endcase
end
// - - 3.Send Data
always @(*) begin
    case (LayerNow)
        0: begin                        //Layer 0 Configuration
            SEND[0].valid   <= RECV[0].valid;    //!test 測試code:送出所有收到的data
            SEND[0].data    <= RECV[0].data;
            SEND[1].valid   <= RECV[1].valid;
            SEND[1].data    <= RECV[1].data;
        end
        1: begin                        //Layer 1 Configuration
            SEND[0].valid   <= RECV[0].valid;    //!test 測試code:送出所有收到的data
            SEND[0].data    <= RECV[0].data;
            SEND[1].valid   <= RECV[1].valid;
            SEND[1].data    <= RECV[1].data;
        end
        2: begin                        //Layer 1 Configuration
            SEND[0].valid   <= RECV[0].valid;    //!test 測試code:送出所有收到的data
            SEND[0].data    <= RECV[0].data;
            SEND[1].valid   <= RECV[1].valid;
            SEND[1].data    <= RECV[1].data;
        end
        default: begin
            SEND[0].valid   <= `FALSE;
            SEND[0].data    <= 0;
            SEND[1].valid   <= `FALSE;
            SEND[1].data    <= 0;
        end
    endcase
end
/*---------------------------------------------------------------------------------------------------*/
// - Layer Function
//!test
always @(posedge clk) begin
    if(!rst_n) begin
        Send_PacketFinish_flg = `FALSE;
    end
    else begin
        case (LayerNow)
            0: begin
                if(Test_Times==4 && Test_Counter==0 && RecvPacket_GotAll_flg) begin
                    Send_PacketFinish_flg <= `TRUE;
                end
                else if(SendPacket_SenderReady_flg) begin
                    Send_PacketFinish_flg <= `FALSE;
                end
            end
            1: begin
                if(Recv_PacketFinish_flg) begin
                    Send_PacketFinish_flg <= `TRUE;
                end
                else if(SendPacket_SenderReady_flg) begin
                    Send_PacketFinish_flg <= `FALSE;
                end
            end
            2: begin
                if(Test_Times==2 && Test_Counter==1 && RecvPacket_GotAll_flg) begin
                    Send_PacketFinish_flg <= `TRUE;
                end
                else if(SendPacket_SenderReady_flg) begin
                    Send_PacketFinish_flg <= `FALSE;
                end
            end
            default begin
                Send_PacketFinish_flg = `FALSE;
            end
        endcase
    end
end
always @(*) begin
    if(LCtrl_Busy_flg) begin
        if(LayerNow==0 && Test_Counter==1) begin
            Test_RecvReadyRead_flg = `TRUE;
        end
        else if(SendPacket_SenderReady_flg && SendPacket_ReadySend_flg) begin
            Test_RecvReadyRead_flg = `TRUE;
        end
        else begin
            Test_RecvReadyRead_flg = `FALSE;
        end
    end
    else begin
        Test_RecvReadyRead_flg = `FALSE;
    end
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Times <= 0;
    end
    else begin
        if(LCtrl_DoneLayer_flg) begin
            Test_Times <= 0;
        end
        else if(Recv_CurrState==RECV_PACKET_OK) begin
            case (LayerNow)
                0: begin
                    if(Test_Counter==1)
                        Test_Times <= Test_Times + 1;
                end
                1: begin
                    Test_Times <= 0;
                end
                2: begin
                    if(Test_Counter==1)
                        Test_Times <= Test_Times + 1;
                end
                default begin
                    Test_Times <= 0;
                end
            endcase
        end
    end
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Counter <= 0;
    end
    else begin
        case (LayerNow)
            0: begin
                if(Recv_CurrState==RECV_PACKET_OK) begin //Receiver的資料已經送給sender，且sender接收完畢
                    Test_Counter <= Test_Counter + 1;
                end
            end
            1: begin
                Test_Counter <= 0;
            end
            2: begin
                if(Recv_CurrState==RECV_PACKET_OK) begin
                    Test_Counter <= Test_Counter + 1;
                end
            end
            default begin
                Test_Counter <= 0;
            end
        endcase
    end
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Send_RecvRequest <= `FALSE;
    end
    else begin
        if(Recv_CurrState==RECV_IDLE && LCtrl_Busy_flg && !AllDone_flg) begin
            Test_Send_RecvRequest <= `TRUE;
        end
        else begin
            Test_Send_RecvRequest <= `FALSE;
        end
    end
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Send_SendRequest <= `FALSE;
    end
    else begin
        if(LCtrl_FirstClkOfLayer_flg) begin
            Test_Send_SendRequest <= `TRUE;
        end
        else begin
            Test_Send_SendRequest <= `FALSE;
        end
    end
end

reg [clogb2(416-1)-1:0] Test_Line_reg;
always @(*) begin
    case (LayerNow)
        0: begin
            case (Test_Counter)
                0: begin
                    if(Test_Times==0) begin
                        Test_Line = 3;
                    end
                    else begin
                        Test_Line = Test_Line_reg;
                    end
                end
                default begin
                    Test_Line = Test_Line_reg;
                end
            endcase
        end
        1: begin
            Test_Line = 1;
        end
        2: begin
            case (Test_Counter)
                0: begin
                    if(Test_Times==0) begin
                        Test_Line = 2;
                    end
                    else begin
                        Test_Line = Test_Line_reg;
                    end
                end
                default begin
                    Test_Line = Test_Line_reg;
                end
            endcase
        end
        default begin
            Test_Line = Test_Line_reg;
        end
    endcase
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Line_reg <= 0;
    end
    else begin
        if(Recv_CurrState==RECV_PACKET_OK) begin
            case (LayerNow)
                0: begin
                    case (Test_Counter)
                        0: begin
                            Test_Line_reg <= Test_Line + 3;
                        end
                        default begin
                        end
                    endcase
                end
                1: begin
                end
                2: begin
                    case (Test_Counter)
                        0: begin
                            Test_Line_reg <= Test_Line + 2;
                        end
                        default begin
                        end
                    endcase
                end
                default begin
                end
            endcase
        end
        else begin
            Test_Line_reg = Test_Line_reg;
        end
    end
end
reg [clogb2(1024-1)-1:0] Test_Filter_reg;
always @(*) begin
    case (LayerNow)
        0: begin
            case (Test_Counter)
                1: begin
                    if(Test_Times==0) begin
                        Test_Filter = 2;
                    end
                    else begin
                        Test_Filter = Test_Filter_reg;
                    end
                end
                default begin
                    Test_Filter = Test_Filter_reg;
                end
            endcase
        end
        1: begin
            Test_Filter = 0;
        end
        2: begin
            case (Test_Counter)
                1: begin
                    if( Test_Times==0) begin
                        Test_Filter = 250;
                    end
                    else begin
                        Test_Filter = Test_Filter_reg;
                    end
                end
                default begin
                    Test_Filter = Test_Filter_reg;
                end
            endcase
        end
        default begin
            Test_Filter = Test_Filter_reg;
        end
    endcase
end
always @(posedge clk) begin
    if(!rst_n) begin
        Test_Filter_reg <= 0;
    end
    else begin
        if(Recv_CurrState==RECV_PACKET_OK) begin
            case (LayerNow)
                0: begin
                    case (Test_Counter)
                        1: begin
                            Test_Filter_reg <= Test_Filter + 2;
                        end
                        default begin
                        end
                    endcase
                end
                1: begin
                end
                2: begin
                    case (Test_Counter)
                        1: begin
                            Test_Filter_reg <= Test_Filter + 2;
                        end
                        default begin
                        end
                    endcase
                end
                default begin
                end
            endcase
        end
        else begin
            Test_Filter_reg = Test_Filter_reg;
        end
    end
end

//!test

/*===================================================================================================*/
// Final Control
reg AllDone_intr;
assign intr_from_core[`INTR_DONE] = AllDone_intr;
wire Done_Ack = intr_ack[`INTR_RECV];

always @(*) begin
    AllDone_intr = LCtrl_LastLayerNow_flg && LCtrl_NextLayer_flg;//!test 現在是測試code 這個flag正確用法:全部都執行完，和最終boundingbox傳出的intr一起給
end

always @(posedge clk) begin
    if(!rst_n) begin
        AllDone_flg <= `FALSE;
    end
    else begin
        if(Ctrl_Start_flg || Done_Ack) begin
            AllDone_flg <= `FALSE;
        end
        if(AllDone_intr) begin
            AllDone_flg <= `TRUE;
        end
    end
end


endmodule
