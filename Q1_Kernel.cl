__constant int edge_kernel[9] = {
     1,  0, -1,
     1,  0, -1,
     1,  0, -1
};

__kernel void convolve(__global float* input, __global float* output, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= 1 && x < width - 1 && y >= 1 && y < height - 1) {
        // Load 3 pixels from left and 3 pixels from right
        float4 left = (float4)(
            input[(y - 1) * width + (x - 1)],
            input[y * width + (x - 1)],
            input[(y + 1) * width + (x - 1)],
            0.0f // padding, unused
        );

        float4 right = (float4)(
            input[(y - 1) * width + (x + 1)],
            input[y * width + (x + 1)],
            input[(y + 1) * width + (x + 1)],
            0.0f // padding, unused
        );

        float4 diff = left - right;
        float sum = dot(diff, (float4)(1.0f, 1.0f, 1.0f, 0.0f)); // Weighted sum
        output[y * width + x] = sum;
    }
}
