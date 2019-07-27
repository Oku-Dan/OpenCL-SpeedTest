__kernel void process(__global unsigned char* source, __global unsigned char* result, int2 size)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

	int val = source[3 * (size.x * y + x)] + source[3 * (size.x * y + x) + 1] + source[3 * (size.x * y + x) + 2];
	result[size.x * y + x] = val / 3;
}