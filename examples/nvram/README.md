# User NVRAM

User NVRAM is persistent storage used by the application to preserve data across device resets.

The NVRAM should be declared once and only once to access it. It should be declared like `NVRAM_MEM uint8_t NvramMem[NVRAM_MEM_SIZE];` where the variable name `NvramMem` can be changed by the application.
The total storage size available is defined by `NVRAM_MEM_SIZE`. When the NVRAM is declared, the maximum user application size is reduced by `NVRAM_MEM_SIZE`. The application build will fail if the application size is too big.

Data can only be written to the NVRAM by calling `NvramWrite()`. Contents in the NVRAM can't be overwritten without calling `NvramClear()`. After calling `NvramClear()`, the NVRAM will be filled with 0xFF.

The example application demonstrates how to declare, read from, write to and clear the NVRAM. 
