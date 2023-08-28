/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xaxidma.h"
#include "xexample.h"

#define N 20

// AXI DMA Instance
XAxiDma AxiDma;

int init_DMA()
{
	int status;
    // 初始化DMA
    XAxiDma_Config *CfgPtr;

	CfgPtr = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_DEVICE_ID);
	if(!CfgPtr){
		print("Error looking for AXI DMA config\n\r");
		return XST_FAILURE;
	}
	status = XAxiDma_CfgInitialize(&AxiDma,CfgPtr);
	if(status != XST_SUCCESS){
		print("Error initializing DMA\n\r");
		return XST_FAILURE;
	}
	//check for scatter gather mode
	if(XAxiDma_HasSg(&AxiDma)){
		print("Error DMA configured in SG mode\n\r");
		return XST_FAILURE;
	}
	/* Disable interrupts, we use polling mode */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	// Reset DMA
	XAxiDma_Reset(&AxiDma);
	while (!XAxiDma_ResetIsDone(&AxiDma)) {}
	return XST_SUCCESS;
}

int Run_HW_Accelerator(unsigned int A[N], unsigned int B[N], int size)
{
	//transfer A to the Vivado HLS block
	int status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)A, size, XAXIDMA_DMA_TO_DEVICE);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Wait for transfer to be done */
	while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) ;

	//get results from the Vivado HLS block
	status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)B, size, XAXIDMA_DEVICE_TO_DMA);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Wait for transfer to be done */
	while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) ;

	//poll the DMA engine to verify transfers are complete
	while ((XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) || (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE))) ;
//	while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) ;

	return 0;
}

int main()
{
    int status;
    unsigned int A[N];
    unsigned int B[N];
    int size=N*sizeof(unsigned int);

    status=init_DMA();
    // DMA init
    if(status != XST_SUCCESS){
    	print("\rError: DMA init failed\n");
    	return XST_FAILURE;
    }
	print("\nDMA Init done\n");

	// data
	init_platform();
	for (unsigned int i=0;i<N;i++){
		A[i] = i;
		B[i] = 0;
	}

	// HW init
	XExample x_exam;
	XExample_Config * exam_ptr;

	printf("Look up the device configuration.\n");
	exam_ptr = XExample_LookupConfig(XPAR_EXAMPLE_0_DEVICE_ID);
	if(!exam_ptr){
		printf("Error, lookup failed!");
		return XST_FAILURE;
	}

	printf("Initialize the Device\n");
	status = XExample_CfgInitialize(&x_exam, exam_ptr);
	if(status!=XST_SUCCESS)
	{
		printf("Error, can't initialize accelerator.\n");
		return XST_FAILURE;
	}

	XExample_Set_N(&x_exam, N);

	XExample_Start(&x_exam);              // start

	//flush the cache
	Xil_DCacheFlushRange((UINTPTR)A,size);
	Xil_DCacheFlushRange((UINTPTR)B,size);
	print("\rCache cleared\n\r");

	// HW compute
	status=Run_HW_Accelerator(A,B,size);

	int err = 0;
	for (unsigned int i=0;i<N;i++){
		if (B[i] != A[i]+5)
			err ++;
	}

	if (err == 0)
		printf("Test Pass!");
	else
		printf("Test Fail!");

    cleanup_platform();
    return 0;
}






