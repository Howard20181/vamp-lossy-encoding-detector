
#include "detect.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <iostream>

#include <cstdlib>
#include <cstring>
#include <cmath>

//#define VERBOSE 1

using namespace std;

t_3
convolve(const t_3 &in,
         const t_4 &weights,
         const t_1 &biases)
{
    // Weights tensor is indexed HWCK, i.e. height - width - number of
    // input channels (e.g. image depth) - number of output channels
    // (i.e. number of learned kernels)
    
    size_t kernel_height = weights.size();
    size_t kernel_width = weights[0].size();
    size_t nkernels = weights[0][0][0].size();
    
    size_t out_height = in.size();
    if (out_height < kernel_height - 1) {
        throw runtime_error("Input too small in convolve");
    }

    size_t out_width = in[0].size();
    if (out_width < kernel_width - 1) {
        throw runtime_error("Input too small in convolve");
    }

    size_t depth = in[0][0].size();
    if (depth != weights[0][0].size()) {
        cerr << "convolve: input depth is " << depth << " but we expected "
             << weights[0][0].size() << endl;
        throw runtime_error("Depth mismatch in convolve");
    }
        
    out_height -= kernel_height - 1;
    out_width -= kernel_width - 1;

#ifdef VERBOSE
    cerr << "convolve: " << nkernels << " kernels of size "
         << kernel_width << "x" << kernel_height << "; output size "
         << out_width << "x" << out_height << "; input depth " << depth << endl;
#endif

    auto out = t_3 (out_height, t_2 (out_width, t_1 (nkernels, 0.f)));

    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            for (size_t ky = 0; ky < kernel_height; ++ky) {
                for (size_t kx = 0; kx < kernel_width; ++kx) {
                    for (size_t c = 0; c < depth; ++c) {
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable)
#elif defined(_MSC_VER)
#pragma loop(ivdep)
#elif defined(__GNUC__)
#pragma GCC ivdep
#endif
                        for (size_t k = 0; k < nkernels; ++k) {
                            out[y][x][k] +=
                                weights[ky][kx][c][k] * in[y + ky][x + kx][c];
                        }
                    }
                }
            }
        }
    }

    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            for (size_t k = 0; k < nkernels; ++k) {
                out[y][x][k] += biases[k];
            }
        }
    }
    
    return out;
}

t_3
convolve_WH(const t_2 &in,
            const t_4 &weights,
            const t_1 &biases)
{
    // Equivalent to convolve, above, for a simple input with a single
    // channel in WH rather than HWC format

    size_t kernel_height = weights.size();
    size_t kernel_width = weights[0].size();
    size_t nkernels = weights[0][0][0].size();
    
    size_t out_width = in.size();
    if (out_width < kernel_width - 1) {
        throw runtime_error("Input too small in convolve");
    }

    size_t out_height = in[0].size();
    if (out_height < kernel_height - 1) {
        throw runtime_error("Input too small in convolve");
    }
        
    out_height -= kernel_height - 1;
    out_width -= kernel_width - 1;

#ifdef VERBOSE
    cerr << "convolve_WH: " << nkernels << " kernels of size "
         << kernel_width << "x" << kernel_height << "; output size "
         << out_width << "x" << out_height << "; input depth fixed to 1" << endl;
#endif

    auto out = t_3 (out_height, t_2 (out_width, t_1 (nkernels, 0.f)));

    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            for (size_t ky = 0; ky < kernel_height; ++ky) {
                for (size_t kx = 0; kx < kernel_width; ++kx) {
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable)
#elif defined(_MSC_VER)
#pragma loop(ivdep)
#elif defined(__GNUC__)
#pragma GCC ivdep
#endif
                    for (size_t k = 0; k < nkernels; ++k) {
                        out[y][x][k] +=
                            weights[ky][kx][0][k] * in[x + kx][y + ky];
                    }
                }
            }
        }
    }

    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            for (size_t k = 0; k < nkernels; ++k) {
                out[y][x][k] += biases[k];
            }
        }
    }
    
    return out;
}

t_3
maxPool(const t_3 &in,
        size_t pool_y,
        size_t pool_x)
{
    size_t out_height = in.size() / pool_y;
    if (out_height < 1) {
        throw runtime_error("Input too small in maxPool");
    }

    size_t out_width = in[0].size() / pool_x;
    if (out_width < 1) {
        throw runtime_error("Input too small in maxPool");
    }
    
    size_t depth = in[0][0].size();
    
#ifdef VERBOSE
    cerr << "maxPool: input size " << in[0].size() << "x" << in.size()
         << "; pool size " << pool_x << "x" << pool_y << "; output size "
         << out_width << "x" << out_height << " and depth " << depth << endl;
#endif
    
    auto out = t_3 (out_height, t_2 (out_width, t_1 (depth, -INFINITY)));

    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            for (size_t i = 0; i < pool_y; ++i) {
                for (size_t j = 0; j < pool_x; ++j) {
                    for (size_t c = 0; c < depth; ++c) {
                        float value = in[y * pool_y + i][x * pool_x + j][c];
                        out[y][x][c] = max(out[y][x][c], value);
                    }
                }
            }
        }
    }

    return out;
}

t_3
zeroPad(const t_3 &in,
        size_t pad_y,
        size_t pad_x)
{
    size_t in_height = in.size();
    if (in_height == 0) {
        throw runtime_error("Input too small in zeroPad");
    }

    size_t in_width = in[0].size();
    if (in_width == 0) {
        throw runtime_error("Input too small in zeroPad");
    }
    
    size_t depth = in[0][0].size();
    
#ifdef VERBOSE
    cerr << "zeroPad: input size " << in_width << "x" << in_height
         << "; padding " << pad_x << "," << pad_y << "; output size "
         << in_width + 2 * pad_x << "x" << in_height + 2 * pad_y
         << " and depth " << depth << endl;
#endif
    
    auto out =
        t_3 (in_height + 2 * pad_y,
             t_2 (in_width + 2 * pad_x, t_1 (depth, 0.f)));

    for (size_t y = 0; y < in_height; ++y) {
        for (size_t x = 0; x < in_width; ++x) {
            for (size_t c = 0; c < depth; ++c) {
                out[y + pad_y][x + pad_x][c] = in[y][x][c];
            }
        }
    }

    return out;
}

t_2
zeroPad_WH(const t_2 &in,
           size_t pad_y,
           size_t pad_x)
{
    // Again a simplified version of zeroPad. Output is also WH format
    
    size_t in_width = in.size();
    if (in_width == 0) {
        throw runtime_error("Input too small in zeroPad");
    }

    size_t in_height = in[0].size();
    if (in_height == 0) {
        throw runtime_error("Input too small in zeroPad");
    }
    
#ifdef VERBOSE
    cerr << "zeroPad: input size " << in_width << "x" << in_height
         << "; padding " << pad_x << "," << pad_y << "; output size "
         << in_width + 2 * pad_x << "x" << in_height + 2 * pad_y
         << " and depth fixed to 1" << endl;
#endif
    
    auto out =
        t_2 (in_width + 2 * pad_x,
             t_1 (in_height + 2 * pad_y, 0.f));

    for (size_t y = 0; y < in_height; ++y) {
        for (size_t x = 0; x < in_width; ++x) {
            out[x + pad_x][y + pad_y] = in[x][y];
        }
    }

    return out;
}

t_1
flatten(const t_3 &in)
{
    size_t height = in.size();
    if (height < 1) {
        throw runtime_error("Input too small in flatten");
    }

    size_t width = in[0].size();
    if (width < 1) {
        throw runtime_error("Input too small in flatten");
    }
    
    size_t depth = in[0][0].size();

#ifdef VERBOSE
    cerr << "flatten: input size " << in[0].size() << "x" << in.size()
         << " and depth " << depth << ", output length "
         << width * height * depth << endl;
#endif
    
    t_1 out(width * height * depth, 0.f);

    size_t i = 0;
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            for (size_t c = 0; c < depth; ++c) {
                out[i++] = in[y][x][c];
            }
        }
    }

    return out;
}

t_1
dense(const t_1 &in,
      const t_2 &weights,
      const t_1 &biases)
{
    size_t in_size = in.size();
    if (in_size != weights.size() || in_size == 0) {
        cerr << "dense: in_size = " << in_size
             << " but we expected " << weights.size() << endl;
        throw runtime_error("Input size mismatch in dense");
    }

    size_t out_size = weights[0].size();
    if (out_size != biases.size() || out_size == 0) {
        cerr << "dense: out_size = " << out_size
             << " but we expected " << biases.size() << endl;
        throw runtime_error("Output size mismatch in dense");
    }
    
#ifdef VERBOSE
    cerr << "dense: input length " << in_size << ", output length "
         << out_size << endl;
#endif

    t_1 out(out_size, 0.f);

    for (size_t i = 0; i < in_size; ++i) {
        for (size_t j = 0; j < out_size; ++j) {
            out[j] += weights[i][j] * in[i];
        }
    }

    for (size_t j = 0; j < out_size; ++j) {
        out[j] += biases[j];
    }

    return out;
}

// "activation" is an overloaded function name, as our representation
// of tensors awkwardly means we can't straightforwardly operate on
// both rank-1 and rank-3 tensors in the same function. This version
// is for rank-3 tensors as returned from convolution layers
//
t_3
activation(const t_3 &in,
           string type)
{
    auto out(in);

    if (type == "relu") {
        for (size_t i = 0; i < out.size(); ++i) {
            for (size_t j = 0; j < out[i].size(); ++j) {
                for (size_t k = 0; k < out[i][j].size(); ++k) {
                    if (out[i][j][k] < 0.f) {
                        out[i][j][k] = 0.f;
                    }
                }
            }
        }
    } else {
        throw runtime_error("Unknown activation function '" + type + "'");
    }
        
    return out;
}

// "activation" is an overloaded function name, as our representation
// of tensors awkwardly means we can't straightforwardly operate on
// both rank-1 and rank-3 tensors in the same function. This version
// is for rank-1 tensors as returned from dense layers
//
t_1
activation(const t_1 &in,
           string type)
{
    auto out(in);
    size_t sz = out.size();

    if (type == "relu") {
        for (size_t i = 0; i < sz; ++i) {
            if (out[i] < 0.f) {
                out[i] = 0.f;
            }
        }
    } else if (type == "softmax") {
        float sum = 0.f;
        for (size_t i = 0; i < sz; ++i) {
            out[i] = exp(out[i]);
            sum += out[i];
        }
        if (sum != 0.f) {
            for (size_t i = 0; i < sz; ++i) {
                out[i] /= sum;
            }
        }
    } else {
        throw runtime_error("Unknown activation function '" + type + "'");
    }

    return out;
}

t_1
classify(const t_2 &imageWH)
{
    t_3 t3;
    t_2 t2;

    t2 = zeroPad_WH(imageWH, 1, 1);
    t3 = convolve_WH(t2, weights_firstConv, biases_firstConv);
    t3 = activation(t3, "relu");
    t3 = maxPool(t3, 2, 2);

    t3 = zeroPad(t3, 1, 1);
    t3 = convolve(t3, weights_secondConv, biases_secondConv);
    t3 = activation(t3, "relu");
    t3 = maxPool(t3, 2, 2);

    t3 = zeroPad(t3, 1, 1);
    t3 = convolve(t3, weights_thirdConv, biases_thirdConv);
    t3 = activation(t3, "relu");
    t3 = maxPool(t3, 2, 2);

    t3 = zeroPad(t3, 1, 1);
    t3 = convolve(t3, weights_fourthConv, biases_fourthConv);
    t3 = activation(t3, "relu");
    t3 = maxPool(t3, 2, 2);

    t_1 flat = flatten(t3);
    
    flat = dense(flat, weights_firstDense, biases_firstDense);
    flat = activation(flat, "relu");

    flat = dense(flat, weights_labeller, biases_labeller);
    flat = activation(flat, "softmax");

    return flat;
}
