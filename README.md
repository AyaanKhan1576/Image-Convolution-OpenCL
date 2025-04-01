---
# 🧠 SIMD-Optimized Edge Detection using OpenCL and C++

This project implements a high-performance 2D convolution on grayscale images using both a scalar C++ baseline and a parallelized OpenCL version. The goal is to compare performance between scalar CPU execution and SIMD-accelerated GPU/CPU execution using OpenCL.

It supports input from real grayscale images (`.pgm`) or generates synthetic images. The OpenCL kernel performs vectorized operations using `float4` to exploit data-level parallelism.

---

## 📌 Features

- ✅ Vertical edge detection using a 3×3 kernel
- ✅ Scalar C++ baseline for performance comparison
- ✅ OpenCL version with SIMD-style vectorization (`float4`)
- ✅ Supports both CPU and GPU OpenCL devices
- ✅ PGM image input/output (compatible with GIMP, IrfanView, OpenCV)
- ✅ Benchmark-ready with execution time measurement

---

## 🔍 Convolution Kernel

The project uses a vertical edge detection kernel:

```
 1  0  -1  
 1  0  -1  
 1  0  -1
```

This kernel detects vertical changes in intensity by computing the difference between left and right columns of pixel values.

---

## 📂 File Structure

| File               | Description                                      |
|--------------------|--------------------------------------------------|
| `Q1_Scalar.cpp`    | Scalar baseline version using C++ only           |
| `Q1_OpenCL.cpp`    | OpenCL-enabled version with SIMD optimizations   |
| `main_Kernel.cl`   | OpenCL kernel code (`float4` vectorized)         |

---

## ⚙️ Build Instructions

### 🛠️ Prerequisites

- **MSYS2** or another GCC-compatible terminal
- **NVIDIA CUDA Toolkit** (for OpenCL headers/libraries)
- An OpenCL-capable **GPU or CPU**

### 🧱 Scalar Version

```bash
g++ -o Q1_Scalar Q1_Scalar.cpp
```

### ⚡ OpenCL Version

```bash
g++ Q1_OpenCL.cpp -o Q1_OpenCL ^
  "-IC:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\include" ^
  "-LC:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\lib\x64" ^
  -lOpenCL
```

> ⚠️ Replace the paths above if your CUDA Toolkit is installed in a different location.

---

## ▶️ How to Run

### 🧮 Scalar Version

```bash
./Q1_Scalar
```

- Prompts you to load `input.pgm` or generate a synthetic image.
- Applies scalar convolution.
- Saves result to `output_scalar.pgm`.

---

### ⚙️ OpenCL Version

```bash
./Q1_OpenCL
```

- Prompts to:
  1. Select OpenCL device (CPU or GPU)
  2. Choose image input (load or generate)
- Runs OpenCL kernel
- Saves result as `output_opencl_device_X.pgm`

---

## 📊 Performance Results

| Image Size | Input Type | Scalar Time | OpenCL (CPU) | OpenCL (GPU) | Speedup (GPU vs Scalar) |
|------------|------------|-------------|---------------|---------------|--------------------------|
| 512×512    | Generated  | 10.02 ms    | 1.0091 ms     | 0.9997 ms     | **10.02×**               |
| 1024×1024  | Generated  | 37.47 ms    | 2.0015 ms     | 2.0125 ms     | **18.62×**               |
| 512×512    | PGM File   | 10.99 ms    | 0.9999 ms     | 1.0141 ms     | **10.84×**               |
| 1024×1024  | PGM File   | 40.52 ms    | 2.0024 ms     | 2.0001 ms     | **20.26×**               |

> Times will vary based on hardware.

---

## 💡 SIMD Optimizations in OpenCL

- Used `float4` to group 3 vertically adjacent pixels from each side (left/right)
- Vector subtraction and dot product for fast sum computation:
  ```c
  float4 diff = left - right;
  float sum = dot(diff, (float4)(1.0f, 1.0f, 1.0f, 0.0f));
  ```
- Work-items are distributed per-pixel
- Global memory is used efficiently; local memory can be added for further optimization

---

## 🚧 Challenges Faced

- GPU overhead for small images led to slower performance vs CPU
- Manual device selection was implemented for flexibility
- Memory alignment was required for vectorized `float4` access

---

## 📸 Output Visualization

- Open output `.pgm` files using:
  - GIMP
  - IrfanView
  - OpenCV (Python/C++)
  - VSCode with PGM viewer plugin

---

## 🧠 Author

**Ayaan Khan**  
*Computer Science Undergraduate | FAST NUCES*  
[GitHub](https://github.com/AyaanKhan1576)

---

## 🪪 License

This project is licensed under the **MIT License**.
```

---
