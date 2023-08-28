#ifndef AXI_FIFO_H_
#define AXI_FIFO_H_

#include "ap_axi_sdata.h"
#include "hls_stream.h"

typedef ap_axiu<32, 0, 0, 0> trans_pkt;

void example(unsigned int N, hls::stream< trans_pkt > &A, hls::stream< trans_pkt > &B);

#endif
