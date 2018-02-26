#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common/common.h"
#include "network/nw.h"
#include "util/util.h"

int main(void)
{
	com_print();
	network_print();
	util_print();
	
	return 0;
}