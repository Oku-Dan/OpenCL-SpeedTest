#include <CL/cl.h>
#include <stdio.h>
#include <cstdlib>

#define checkError(openclFunction)     \
    if (cl_int err = (openclFunction)) \
    {                                  \
        printf("error : %d\n", err);   \
    }

#define checkArgError(clSetKernelArg)\
	switch ((clSetKernelArg)){\
		case CL_SUCCESS : break;\
		case CL_INVALID_KERNEL : printf("CL_INVALID_KERNEL\n");break;\
		case CL_INVALID_ARG_INDEX : printf("CL_INVALID_ARG_INDEX\n");break;\
		case CL_INVALID_ARG_VALUE : printf("CL_INVALID_ARG_VALUE\n");break;\
		case CL_INVALID_MEM_OBJECT : printf("CL_INVALID_MEM_OBJECT\n");break;\
		case CL_INVALID_ARG_SIZE : printf("CL_INVALID_ARG_SIZE\n");break;\
		case CL_OUT_OF_RESOURCES : printf("CL_OUT_OF_RESOURCES\n");break;\
		case CL_OUT_OF_HOST_MEMORY : printf("CL_OUT_OF_HOST_MEMORY\n");break;\
		default : break;\
	}\

cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel kernel;
cl_mem memory1, memory2;
size_t source_size, result_size;
int rows, cols;
size_t processSize[2];
cl_int2 size;

cl_program compileProgram(char *fileName){
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        printf("%s load failed\n", fileName);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    const int filesize = ftell(fp);

    fseek(fp, 0, 0);
    char *sourceString = (char *)malloc(filesize);
    size_t sourceSize = fread(sourceString, sizeof(char), filesize, fp);
    fclose(fp);

    cl_program res = clCreateProgramWithSource(context, 1, (const char **)&sourceString, (const size_t *)&sourceSize, NULL);
    cl_int err = clBuildProgram(res, 1, &device, NULL, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        size_t logSize;
        clGetProgramBuildInfo(res, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char *buildLog = (char *)malloc((logSize + 1));
        clGetProgramBuildInfo(res, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
        printf("%s", buildLog);
        free(buildLog);
    }
    free(sourceString);
    return res;
}

void InitializeCL()
{
    cl_uint platformNumber = 0;
    cl_platform_id platformIds[8];
    checkError(clGetPlatformIDs(8, platformIds, &platformNumber));
    platform = platformIds[0];

    cl_uint deviceNumber = 0;
    cl_device_id deviceIds[8];
    checkError(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 8, deviceIds, &deviceNumber));
    device = deviceIds[0];

    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueueWithProperties(context, device, 0, NULL);

    program = compileProgram((char*)"kernel.cl");
    kernel = clCreateKernel(program, "process", NULL);
    return;
}

void GetMemoryCL(size_t source, size_t result,int frame_rows,int frame_cols) {
	rows = frame_rows;
	cols = frame_cols;
	source_size = source;
	result_size = result;
    memory1 = clCreateBuffer(context, CL_MEM_READ_WRITE, source_size, NULL, NULL);
    memory2 = clCreateBuffer(context, CL_MEM_READ_WRITE, result_size, NULL, NULL);
	size = { (cl_int)cols,(cl_int)rows };
	processSize[0] = (size_t)cols;
	processSize[1] = (size_t)rows;
}

void WriteSourceCL(unsigned char *source) {
    clEnqueueWriteBuffer(queue, memory1, CL_TRUE, 0, source_size, source, 0, NULL, NULL);
}

void ProcessCL()
{
	clSetKernelArg(kernel, 0, sizeof(memory1), &memory1);
    clSetKernelArg(kernel, 1, sizeof(memory2), &memory2);
	clSetKernelArg(kernel, 2, sizeof(size), &size);

    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, processSize, NULL, 0, NULL, NULL);

    return;
}

void ReadResultCL(unsigned char *result) {
    clEnqueueReadBuffer(queue, memory2, CL_TRUE, 0, result_size, result, 0, NULL, NULL);
}

void ReleaseMemoryCL() {
    clReleaseMemObject(memory1);
    clReleaseMemObject(memory2);
}

void ShowAvailableDevicesCL(){

    cl_uint platformNumber = 0;
    cl_platform_id platformIds[8];
    clGetPlatformIDs(8, platformIds, &platformNumber);

    char string[256];
    cl_device_type type;
    cl_uint value;
    size_t sizes[3];
    cl_ulong ulvalue;
    for (int i = 0; i < platformNumber; i++)
    {
        printf("platform idx : %d\n", i);
        cl_platform_id platform = platformIds[i];
        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 256, string, NULL);
        printf("platform vendor : %s\n", string);
        clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, string, NULL);
        printf("platform name : %s\n", string);
        clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, string, NULL);
        printf("platform version : %s\n", string);

        // デバイス取得
        cl_uint deviceNumber = 0;
        cl_device_id deviceIds[8];
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 8, deviceIds, &deviceNumber);
        for (int j = 0; j < deviceNumber; j++)
        {
            printf("    device idx : %d\n", j);
            cl_device_id device = deviceIds[j];
            clGetDeviceInfo(device, CL_DEVICE_NAME, 256, string, NULL);
            printf("    device name : %s\n", string);
            clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
            if (type == CL_DEVICE_TYPE_CPU)
                printf("    device type : CPU\n");
            if (type == CL_DEVICE_TYPE_GPU)
                printf("    device type : GPU\n");
            if (type == CL_DEVICE_TYPE_ACCELERATOR)
                printf("    device type : ACCELERATOR\n");
            clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &value, NULL);
            printf("    device max compute units : %d\n", value);
            clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, sizes, NULL);
            printf("    device max work item sizes : [%d][%d][%d]\n", sizes[0], sizes[1], sizes[2]);
            clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_uint), &value, NULL);
            printf("    device max work group size : %d\n", value);
            clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulvalue, NULL);
            printf("    device max mem alloc size : %d\n", ulvalue);
            clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &ulvalue, NULL);
            printf("    device max constant buffer size : %d\n", ulvalue);
        }
    }
}

