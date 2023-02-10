#include "lps_vdma.h"



static unsigned int context_init = 0;
vdma_handle vdma_context[XPAR_XAXIVDMA_NUM_INSTANCES];

/*****************************************************************************/
/**
 *
 * run_triple_frame_buffer API
 *
 * This API is the interface between application and other API. When application will call
 * this API with right argument, This API will call rest of the API to configure the read
 * and write path of VDMA,based on ID. After that it will start both the read and write path
 * of VDMA
 *
 * @return
 *		- XST_SUCCESS if example finishes successfully
 *		- XST_FAILURE if example fails.
 *
 * @Argument
 * 		- InstancePtr:		The handle to XAxiVdma data structure.
 * 		- DeviceId:			The device ID of current VDMA
 * 		- hsize:			The horizontal size of the frame. It will be in Pixels.
 * 							The actual size of frame will be calculated by multiplying this
 * 							with tdata width.
 * 		-vsize:				Vertical size of the frame.
 * 		-buf_base_addr:		The buffer address where frames will be written and read by VDMA
 * 		-number_frame_count:If application needs interrupt on frame processing, this variable
 * 							will tell after how many frames interrupt should come.
 * 		-enable_frm_cnt_intr: Enable frame count interrupt.
 *
 ******************************************************************************/

int run_triple_frame_buffer(XAxiVdma* InstancePtr, int DeviceId, int hsize,
		int vsize, int buf_base_addr, int number_frame_count,
		int enable_frm_cnt_intr) {
	int Status, i;
	XAxiVdma_Config *Config;
	XAxiVdma_FrameCounter FrameCfgPtr;
	/* This is one time initialization of state machine context. In first call it will be done
	 * for all VDMA instances in the system
	 */
	if (context_init == 0) {
		for (i = 0; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) {
			vdma_context[i].InstancePtr = NULL;
			vdma_context[i].device_id = -1;
			vdma_context[i].hsize = 0;
			vdma_context[i].vsize = 0;
			vdma_context[i].init_done = 0;
			vdma_context[i].buffer_address = 0;
			vdma_context[i].enable_frm_cnt_intr = 0;
			vdma_context[i].number_of_frame_count = 0;
		}
		context_init = 1;
	}

	/* The below initialization will happen for each VDMA. The API argument will be stored in
	 * internal data structure
	 */
	if (vdma_context[DeviceId].init_done == 0) {
		vdma_context[DeviceId].InstancePtr = InstancePtr;
		vdma_context[DeviceId].device_id = DeviceId;
		vdma_context[DeviceId].vsize = vsize;

		vdma_context[DeviceId].buffer_address = buf_base_addr;
		vdma_context[DeviceId].enable_frm_cnt_intr = enable_frm_cnt_intr;
		vdma_context[DeviceId].number_of_frame_count = number_frame_count;

		/* The information of the XAxiVdma_Config comes from hardware build.
		 * The user IP should pass this information to the AXI DMA core.
		 */
		Config = XAxiVdma_LookupConfig(DeviceId);
		if (!Config) {
			xil_printf("No video DMA found for ID %d\r\n", DeviceId);
			return XST_FAILURE;
		}
		/* Initialize DMA engine */
		Status = XAxiVdma_CfgInitialize(vdma_context[DeviceId].InstancePtr,
				Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {

			xil_printf("Configuration Initialization failed %d\r\n", Status);

			return XST_FAILURE;
		}
		vdma_context[DeviceId].hsize = hsize;
		vdma_context[DeviceId].stride = hsize;
		//vdma_context[DeviceId].stride = hsize * (Config->Mm2SStreamWidth >> 3);
		xil_printf("hsize: %d, Config->Mm2SStreamWidth: %d\r\n", hsize,
				Config->Mm2SStreamWidth >> 3);
		vdma_context[DeviceId].init_done = 1;
	}

	xil_printf("Start to WriteSetup\r\n");
	/* Setup the write channel
	 */

/*	Status = WriteSetup(&vdma_context[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel setup failed %d\r\n", Status);
		if (Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");

		return XST_FAILURE;
	}
*/

	/*
	 * Setup your video IP that reads from the memory
	 */

	xil_printf("Start to ReadSetup\r\n");
	/* Setup the read channel
	 */
	Status = ReadSetup(&vdma_context[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel setup failed %d\r\n", Status);
		if (Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");

		return XST_FAILURE;
	}
	/* The the frame counter interrupt is enable, setting VDMA for same */
	if (vdma_context[DeviceId].enable_frm_cnt_intr) {
		FrameCfgPtr.ReadDelayTimerCount = 1;
		FrameCfgPtr.ReadFrameCount = number_frame_count;
		FrameCfgPtr.WriteDelayTimerCount = 1;
		FrameCfgPtr.WriteFrameCount = number_frame_count;

		XAxiVdma_SetFrameCounter(vdma_context[DeviceId].InstancePtr,
				&FrameCfgPtr);
	}

	xil_printf("Start to StartTransfer\r\n");
	/* Start the DMA engine to transfer
	 */
	Status = StartTransfer(vdma_context[DeviceId].InstancePtr);
	if (Status != XST_SUCCESS) {
		if (Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}

#if DEBUG_MODE
	xil_printf(
			"Code is in Debug mode, Make sure that buffer addresses are at valid memory \r\n");
	xil_printf(
			"In triple mode, there has to be six consecutive buffers for Debug mode \r\n");
	{
		u32 pixels, j;
		u32 *dst, *src;
		u32 total_pixel = vdma_context[DeviceId].hsize
				* vdma_context[DeviceId].vsize;

		src = (u32 *) vdma_context[DeviceId].buffer_address;
		dst = src + total_pixel * XPAR_AXI_VDMA_0_NUM_FSTORES;

		for (j = 0; j < XPAR_AXI_VDMA_0_NUM_FSTORES; j++) {
			for (pixels = 0; pixels < total_pixel; pixels++) {
				xil_printf("VDMA transfer failed: SRC=0x%x, DST=0x%x\r\n",
						src[pixels], dst[pixels]);
			}
			xil_printf("Next Frame Start\r\n\r\n");
			src = src + total_pixel;
			dst = dst + total_pixel;
		}
	}
	xil_printf("VDMA transfer is happening and checked for 3 frames \r\n");
#endif

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 *
 * This function sets up the read channel
 *
 * @param	vdma_context is the context pointer to the VDMA engine.
 *
 * @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
 *
 * @note		None.
 *
 ******************************************************************************/
int ReadSetup(vdma_handle *vdma_context) {
	int Index;
	u32 Addr;
	int Status;

	vdma_context->ReadCfg.VertSizeInput = vdma_context->vsize;
	vdma_context->ReadCfg.HoriSizeInput = vdma_context->hsize;

	vdma_context->ReadCfg.Stride = vdma_context->stride;
	vdma_context->ReadCfg.FrameDelay = 0; /* This example does not test frame delay */

	vdma_context->ReadCfg.EnableCircularBuf = 1;
	vdma_context->ReadCfg.EnableSync = 0; /* No Gen-Lock */

	vdma_context->ReadCfg.PointNum = 0; /* No Gen-Lock */
	vdma_context->ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

	vdma_context->ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
	/* Configure the VDMA is per fixed configuration, THis configuration is being used by majority
	 * of customer. Expert users can play around with this if they have different configurations */

	Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_READ,
			&vdma_context->ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = vdma_context->buffer_address;
	for (Index = 0; Index < XPAR_AXI_VDMA_0_NUM_FSTORES; Index++) {
		vdma_context->ReadCfg.FrameStoreStartAddr[Index] = Addr;
	}



	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr, XAXIVDMA_READ,
			vdma_context->ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 *
 * This function sets up the write channel
 *
 * @param	dma_context is the context pointer to the VDMA engine..
 *
 * @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
 *
 * @note		None.
 *
 ******************************************************************************/
int WriteSetup(vdma_handle *vdma_context) {
	int Index;
	u32 Addr, Addr_tmp;
	int Status;

	vdma_context->WriteCfg.VertSizeInput = vdma_context->vsize;
	vdma_context->WriteCfg.HoriSizeInput = vdma_context->hsize;

	vdma_context->WriteCfg.Stride = vdma_context->stride;
	vdma_context->WriteCfg.FrameDelay = 0; /* This example does not test frame delay */

	vdma_context->WriteCfg.EnableCircularBuf = 1;
	vdma_context->WriteCfg.EnableSync = 0; /* No Gen-Lock */

	vdma_context->WriteCfg.PointNum = 0; /* No Gen-Lock */
	vdma_context->WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

	vdma_context->WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
	/* Configure the VDMA is per fixed configuration, THis configuration is being used by majority
	 * of customer. Expert users can play around with this if they have different configurations */

	Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_WRITE,
			&vdma_context->WriteCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * Use physical addresses
	 */
	Addr = vdma_context->buffer_address
			+ XPAR_AXI_VDMA_0_NUM_FSTORES * vdma_context->stride
					* vdma_context->vsize;
	Addr_tmp = Addr;

	for (Index = 0; Index < XPAR_AXI_VDMA_0_NUM_FSTORES; Index++) {
		vdma_context->WriteCfg.FrameStoreStartAddr[Index] = Addr;

		xil_printf("Write Buffer %d address: 0x%x \r\n", Index, Addr);

		Addr += (vdma_context->stride * vdma_context->vsize);
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 */
	Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr,
			XAXIVDMA_WRITE, vdma_context->WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Clear data buffer
	 */
	memset((void *) Addr_tmp, 10,
			vdma_context->WriteCfg.Stride * vdma_context->WriteCfg.VertSizeInput
					* XPAR_AXI_VDMA_0_NUM_FSTORES);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function starts the DMA transfers. Since the DMA engine is operating
 * in circular buffer mode, video frames will be transferred continuously.
 *
 * @param	InstancePtr points to the DMA engine instance
 *
 * @return	XST_SUCCESS if both read and write start successfully
 *		XST_FAILURE if one or both directions cannot be started
 *
 * @note		None.
 *
 ******************************************************************************/
int StartTransfer(XAxiVdma *InstancePtr) {
	int Status;
	/* Start the write channel of VDMA */
/*	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_WRITE);
	if (Status != XST_SUCCESS) {
		xil_printf("Start Write transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}
*/
	/* Start the Read channel of VDMA */
	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
	if (Status != XST_SUCCESS) {
		xil_printf("Start read transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
