#ifndef H_HDDBOOT_INC
#define H_HDDBOOT_INC

/**
 * @brief Executes the HDDLOAD module from a file
 * @param path path to the HDDLOAD IRX Module
 * @param ret integer pointer to obtain the Module return value. it should return 0 on successfull load
 * @return Module ID. if 0 or larger, success. if negative number, then MODLOAD refused to load the module (see kerr.h)
*/
int BootHDDLoadFile(char* path, int* ret);

/**
 * @brief Executes the HDDLOAD module from a buffer on EE
 * @param irx Pointer to the buffer holding HDDLOAD IRX Module
 * @param size Size of the HDDLOAD IRX Module buffer
 * @param ret integer pointer to obtain the Module return value. it should return 0 on successfull load
 * @return Module ID. if 0 or larger, success. if negative number, then MODLOAD refused to load the module (see kerr.h)
*/
int BootHDDLoadBuffer(void* irx, unsigned int size, int* ret);

/**
 * @brief Internal check for the HDDLOAD Status
 * @return 1 if module loaded successfully
*/
int GetHddSupportEnabledStatus(void);

/**
 * @brief Checks if the HDDLOAD Module thread has completed the DMA Transfer of the MBR.KELF from HDD to RAM
 * @note once it is done, check with the ::DetermineHDDLoadStatus function
*/
int GetHddUpdateStatus(void);

/**
 * @brief Returns the status of the HDDLOAD thread execution
 * @return 1 if module Completed it's task
*/
void DetermineHDDLoadStatus(void);

#endif