#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <string>
#include <utility>
#include <algorithm>

#define CHECK_CL(err, msg) if (err != CL_SUCCESS) { std::cerr << msg << " Error: " << err << "\n"; exit(1); }

void generateImage(std::vector<float>& image, int width, int height) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(0.0f, 255.0f);
    for (int i = 0; i < width * height; ++i)
        image[i] = dist(gen);
}

bool readPGM(const std::string& filename, std::vector<float>& image, int& width, int& height) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;

    std::string magic;
    in >> magic;
    if (magic != "P5") return false;

    in >> width >> height;
    int maxVal;
    in >> maxVal;
    in.get(); // consume newline

    image.resize(width * height);
    for (int i = 0; i < width * height; ++i) {
        unsigned char byte;
        in.read(reinterpret_cast<char*>(&byte), 1);
        image[i] = static_cast<float>(byte);
    }

    return true;
}

void writePGM(const std::vector<float>& image, int width, int height, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out << "P5\n" << width << " " << height << "\n255\n";
    for (float val : image) {
        unsigned char byte = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, val)));
        out.write(reinterpret_cast<char*>(&byte), 1);
    }
    out.close();
}

std::string loadKernel(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open kernel file: " << filename << "\n";
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main() {
    int width = 1024, height = 1024;
    std::vector<float> input, output;

    // ------------------ Device Listing ------------------
    cl_platform_id selectedPlatform = nullptr;
    cl_device_id selectedDevice = nullptr;
    std::vector<std::pair<cl_platform_id, cl_device_id>> deviceList;

    cl_uint numPlatforms;
    CHECK_CL(clGetPlatformIDs(0, nullptr, &numPlatforms), "Getting Platforms");
    std::vector<cl_platform_id> platforms(numPlatforms);
    CHECK_CL(clGetPlatformIDs(numPlatforms, platforms.data(), nullptr), "Getting Platform IDs");

    int index = 0;
    for (auto platform : platforms) {
        cl_uint numDevices = 0;
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
        if (numDevices == 0) continue;

        std::vector<cl_device_id> devices(numDevices);
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr);

        for (auto device : devices) {
            char name[128];
            cl_device_type type;
            clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, nullptr);
            clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, nullptr);

            std::string typeStr;
            if (type & CL_DEVICE_TYPE_CPU) typeStr = "CPU";
            else if (type & CL_DEVICE_TYPE_GPU) typeStr = "GPU";
            else if (type & CL_DEVICE_TYPE_ACCELERATOR) typeStr = "Accelerator";
            else typeStr = "Other";

            std::cout << "[" << index << "] Platform: " << name << " (" << typeStr << ")\n";
            deviceList.push_back({ platform, device });
            ++index;
        }
    }

    if (deviceList.empty()) {
        std::cerr << "No OpenCL devices found.\n";
        return 1;
    }

    // ------------------ Device Selection ------------------
    int choice = 0;
    std::cout << "Select device [0 - " << deviceList.size() - 1 << "]: ";
    std::cin >> choice;
    if (choice < 0 || choice >= deviceList.size()) {
        std::cerr << "Invalid choice.\n";
        return 1;
    }

    selectedPlatform = deviceList[choice].first;
    selectedDevice = deviceList[choice].second;

    // ------------------ Image Input Choice ------------------
    int inputMode = 0;
    std::cout << "Select image input method:\n[0] Generate Random Image\n[1] Load from PGM file (input.pgm)\nChoice: ";
    std::cin >> inputMode;

    if (inputMode == 1) {
        if (!readPGM("input.pgm", input, width, height)) {
            std::cerr << "Failed to read 'input.pgm'.\n";
            return 1;
        }
    } else {
        input.resize(width * height);
        generateImage(input, width, height);
    }

    output.resize(width * height, 0);

    // ------------------ OpenCL Setup ------------------
    cl_int err;
    cl_context context = clCreateContext(nullptr, 1, &selectedDevice, nullptr, nullptr, &err);
    CHECK_CL(err, "Creating Context");

    cl_command_queue queue = clCreateCommandQueue(context, selectedDevice, 0, &err);
    CHECK_CL(err, "Creating Command Queue");

    std::string kernelSrc = loadKernel("Q1_Kernel.cl");
    const char* src = kernelSrc.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &src, nullptr, &err);
    CHECK_CL(err, "Creating Program");

    err = clBuildProgram(program, 1, &selectedDevice, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        char buffer[2048];
        clGetProgramBuildInfo(program, selectedDevice, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, nullptr);
        std::cerr << "Build Error:\n" << buffer << "\n";
        return 1;
    }

    cl_kernel kernel = clCreateKernel(program, "convolve", &err);
    CHECK_CL(err, "Creating Kernel");

    cl_mem inputBuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * input.size(), input.data(), &err);
    cl_mem outputBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * output.size(), nullptr, &err);
    CHECK_CL(err, "Creating Buffers");

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuf);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuf);
    err |= clSetKernelArg(kernel, 2, sizeof(int), &width);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &height);
    CHECK_CL(err, "Setting Kernel Args");

    size_t globalSize[2] = { (size_t)width, (size_t)height };

    auto start = std::chrono::high_resolution_clock::now();
    err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalSize, nullptr, 0, nullptr, nullptr);
    CHECK_CL(err, "Running Kernel");
    clFinish(queue);
    auto end = std::chrono::high_resolution_clock::now();

    clEnqueueReadBuffer(queue, outputBuf, CL_TRUE, 0, sizeof(float) * output.size(), output.data(), 0, nullptr, nullptr);

    double time = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "OpenCL convolution time: " << time << " ms\n";

    std::string outName = "output_opencl_device_" + std::to_string(choice) + ".pgm";
    writePGM(output, width, height, outName);
    std::cout << "Wrote image: " << outName << "\n";

    clReleaseMemObject(inputBuf);
    clReleaseMemObject(outputBuf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
