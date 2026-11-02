#include "mock_cmsis.h"
#include <uhal/cache.h>
uint32_t mock_icache;
int main(void) { uhal_icache_enable(); return mock_icache == 1u ? 0 : 1; }
