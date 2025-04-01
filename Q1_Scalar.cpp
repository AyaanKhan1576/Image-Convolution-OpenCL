// // #include <iostream>
// // #include <vector>
// // #include <chrono>
// // #include <fstream>
// // #include <algorithm>

// // // Generate a 2D gradient image
// // void generate_gradient_image(std::vector<std::vector<float>>& image, int M) {
// //     image.resize(M, std::vector<float>(M));
// //     for (int i = 0; i < M; ++i)
// //         for (int j = 0; j < M; ++j)
// //             image[i][j] = static_cast<float>(i - j);
// // }

// // // Save the image as a PGM (grayscale) file
// // void save_pgm(const std::vector<std::vector<float>>& img, const std::string& filename) {
// //     int M = img.size();
// //     std::ofstream out(filename);
// //     out << "P2\n" << M << " " << M << "\n255\n";
// //     for (const auto& row : img) {
// //         for (float pix : row) {
// //             int val = std::min(255, std::max(0, static_cast<int>(pix + 128)));
// //             out << val << " ";
// //         }
// //         out << "\n";
// //     }
// // }

// // int main() {
// //     const int M = 1024;
// //     std::vector<std::vector<float>> image, output(M, std::vector<float>(M, 0.0f));
// //     generate_gradient_image(image, M);

// //     float kernel[3][3] = {
// //         { 1, 0, -1 },
// //         { 1, 0, -1 },
// //         { 1, 0, -1 }
// //     };

// //     auto start = std::chrono::high_resolution_clock::now();

// //     for (int i = 1; i < M - 1; ++i) {
// //         for (int j = 1; j < M - 1; ++j) {
// //             float sum = 0.0f;
// //             for (int ki = -1; ki <= 1; ++ki)
// //                 for (int kj = -1; kj <= 1; ++kj)
// //                     sum += image[i + ki][j + kj] * kernel[ki + 1][kj + 1];
// //             output[i][j] = sum;
// //         }
// //     }

// //     auto end = std::chrono::high_resolution_clock::now();

// //     std::cout << "Scalar Time: "
// //               << std::chrono::duration<double>(end - start).count()
// //               << " seconds\n";

// //     save_pgm(output, "output_scalar.pgm");
// //     return 0;
// // }

// #include <iostream>
// #include <vector>
// #include <fstream>
// #include <chrono>
// #include <random>

// const int KERNEL_SIZE = 3;
// const int EDGE_KERNEL[KERNEL_SIZE][KERNEL_SIZE] = {
//     {1, 0, -1},
//     {1, 0, -1},
//     {1, 0, -1}
// };

// void generateImage(std::vector<float> &image, int width, int height) {
//     std::mt19937 gen(42);
//     std::uniform_real_distribution<float> dist(0.0f, 255.0f);
//     for (int i = 0; i < width * height; ++i)
//         image[i] = dist(gen);
// }

// void writePGM(const std::vector<float> &image, int width, int height, const std::string &filename) {
//     std::ofstream out(filename, std::ios::binary);
//     out << "P5\n" << width << " " << height << "\n255\n";
//     for (float val : image) {
//         unsigned char byte = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, val)));
//         out.write(reinterpret_cast<char*>(&byte), 1);
//     }
//     out.close();
// }

// void convolve(const std::vector<float> &input, std::vector<float> &output, int width, int height) {
//     for (int y = 1; y < height - 1; ++y) {
//         for (int x = 1; x < width - 1; ++x) {
//             float sum = 0.0f;
//             for (int ky = 0; ky < KERNEL_SIZE; ++ky) {
//                 for (int kx = 0; kx < KERNEL_SIZE; ++kx) {
//                     int px = x + kx - 1;
//                     int py = y + ky - 1;
//                     sum += input[py * width + px] * EDGE_KERNEL[ky][kx];
//                 }
//             }
//             output[y * width + x] = sum;
//         }
//     }
// }

// int main() {
//     int width = 512, height = 512;
//     std::vector<float> input(width * height);
//     std::vector<float> output(width * height, 0);

//     generateImage(input, width, height);

//     auto start = std::chrono::high_resolution_clock::now();
//     convolve(input, output, width, height);
//     auto end = std::chrono::high_resolution_clock::now();

//     double time = std::chrono::duration<double, std::milli>(end - start).count();
//     std::cout << "Scalar convolution time: " << time << " ms\n";

//     writePGM(output, width, height, "output_scalar.pgm");

//     return 0;
// }

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>

const int KERNEL_SIZE = 3;
const int EDGE_KERNEL[KERNEL_SIZE][KERNEL_SIZE] = {
    {1, 0, -1},
    {1, 0, -1},
    {1, 0, -1}
};

void generateImage(std::vector<float> &image, int width, int height) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(0.0f, 255.0f);
    for (int i = 0; i < width * height; ++i)
        image[i] = dist(gen);
}

bool readPGM(const std::string &filename, std::vector<float> &image, int &width, int &height) {
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

void writePGM(const std::vector<float> &image, int width, int height, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    out << "P5\n" << width << " " << height << "\n255\n";
    for (float val : image) {
        unsigned char byte = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, val)));
        out.write(reinterpret_cast<char*>(&byte), 1);
    }
    out.close();
}

void convolve(const std::vector<float> &input, std::vector<float> &output, int width, int height) {
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            float sum = 0.0f;
            for (int ky = 0; ky < KERNEL_SIZE; ++ky) {
                for (int kx = 0; kx < KERNEL_SIZE; ++kx) {
                    int px = x + kx - 1;
                    int py = y + ky - 1;
                    sum += input[py * width + px] * EDGE_KERNEL[ky][kx];
                }
            }
            output[y * width + x] = sum;
        }
    }
}

int main() {
    int width = 512, height = 512;
    std::vector<float> input, output;

    // User choice for input
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

    output.resize(width * height, 0.0f);

    auto start = std::chrono::high_resolution_clock::now();
    convolve(input, output, width, height);
    auto end = std::chrono::high_resolution_clock::now();

    double time = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "Scalar convolution time: " << time << " ms\n";

    writePGM(output, width, height, "output_scalar.pgm");
    std::cout << "Wrote image: output_scalar.pgm\n";

    return 0;
}

