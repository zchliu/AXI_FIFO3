#include <stdio.h>
#include "AXI_FIFO.h"

int main()
{
	trans_pkt A;
	trans_pkt B;

	hls::stream< trans_pkt > A_hls;
	hls::stream< trans_pkt > B_hls;

	ap_uint<32> A_arr[10];
	ap_uint<32> B_arr[10];

    A.data = 1;
    A.last = 0;
    A.keep = 0xf;
    A.strb = 0xf;

    A_hls.write(A);

  	example(A_hls, B_hls);

  	B = B_hls.read();

	if(B.data != 6){
	  printf("ERROR: HW and SW results mismatch\n");
	  return 1;
	}

	printf("Success: HW and SW results match\n");
	return 0;
}
