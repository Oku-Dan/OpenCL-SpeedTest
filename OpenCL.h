void InitializeCL();
void GetMemoryCL(size_t source, size_t result, int frame_rows, int frame_cols);
void WriteSourceCL(unsigned char *source);
void ProcessCL();
void ReadResultCL(unsigned char *result);
void ReleaseMemoryCL();
void ShowAvailableDevicesCL();