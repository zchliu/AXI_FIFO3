#include "AXI_FIFO.h"

void example(unsigned int N, hls::stream< trans_pkt > &A, hls::stream< trans_pkt > &B)
{
#pragma HLS INTERFACE mode=s_axilite port=N
#pragma HLS TOP name=example
#pragma HLS INTERFACE mode=s_axilite port=return
#pragma HLS INTERFACE mode=axis port=A
#pragma HLS INTERFACE mode=axis port=B
	trans_pkt tmp1;
	trans_pkt tmp2;
	for (int i=0;i<N;i++){
		A.read(tmp1);
		tmp2.data = tmp1.data += 5;
		if (i==N-1)
			tmp2.last = 1;
		else
			tmp2.last = 0;
		tmp2.strb = 0xf;
		tmp2.keep = 0xf;
		B.write(tmp2);
	}
}
