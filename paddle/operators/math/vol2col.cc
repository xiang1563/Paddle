/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/operators/math/vol2col.h"

namespace paddle {
namespace operators {
namespace math {

/*
 * vol = [input_channels, input_depth, input_height, input_width]
 * col =
 *   [input_channels, filter_depth, filter_height, filter_width,
 *                    output_depth, output_height, output_width]
 */
template <class T>
class Vol2ColFunctor<platform::CPUPlace, T> {
 public:
  void operator()(const platform::DeviceContext& context,
                  const framework::Tensor& vol, framework::Tensor& col,
                  int stride_depth, int stride_height, int stride_width,
                  int padding_depth, int padding_height,
                  int padding_width) const {
    PADDLE_ENFORCE(vol.dims().size() == 4);
    PADDLE_ENFORCE(col.dims().size() == 7);

    int input_channels = vol.dims()[0];
    int input_depth = vol.dims()[1];
    int input_height = vol.dims()[2];
    int input_width = vol.dims()[3];
    int filter_depth = col.dims()[1];
    int filter_height = col.dims()[2];
    int filter_width = col.dims()[3];
    int output_depth = col.dims()[4];
    int output_height = col.dims()[5];
    int output_width = col.dims()[6];
    int channels_col =
        input_channels * filter_depth * filter_height * filter_width;

    const T* vol_data = vol.data<T>();
    T* col_data = col.data<T>();

    for (int c = 0; c < channels_col; ++c) {
      int w_offset = c % filter_width;
      int h_offset = (c / filter_width) % filter_height;
      int d_offset = (c / filter_width / filter_height) % filter_depth;
      int c_in = c / filter_width / filter_height / filter_depth;
      for (int d = 0; d < output_depth; ++d) {
        int d_pad = d * stride_depth - padding_depth + d_offset;
        for (int h = 0; h < output_height; ++h) {
          int h_pad = h * stride_height - padding_height + h_offset;
          for (int w = 0; w < output_width; ++w) {
            int w_pad = w * stride_width - padding_width + w_offset;

            int col_idx =
                ((c * output_depth + d) * output_height + h) * output_width + w;
            if (h_pad < 0 || h_pad >= input_height || w_pad < 0 ||
                w_pad >= input_width || d_pad < 0 || d_pad >= input_depth) {
              col_data[col_idx] = static_cast<T>(0);
            } else {
              int vol_idx =
                  ((c_in * input_depth + d_pad) * input_height + h_pad) *
                      input_width +
                  w_pad;
              col_data[col_idx] = vol_data[vol_idx];
            }
          }
        }
      }
    }
  }
};

/*
 * vol = [input_channels,input_depth, input_height, input_width]
 * col =
 *   [input_channels, filter_depth, filter_height, filter_width,
 *                    output_depth, output_height, output_width]
 */
template <class T>
class Col2VolFunctor<platform::CPUPlace, T> {
 public:
  void operator()(const platform::DeviceContext& context,
                  framework::Tensor& vol, const framework::Tensor& col,
                  int stride_depth, int stride_height, int stride_width,
                  int padding_depth, int padding_height,
                  int padding_width) const {
    PADDLE_ENFORCE(vol.dims().size() == 4);
    PADDLE_ENFORCE(col.dims().size() == 7);

    int input_channels = vol.dims()[0];
    int input_depth = vol.dims()[1];
    int input_height = vol.dims()[2];
    int input_width = vol.dims()[3];
    int filter_depth = col.dims()[1];
    int filter_height = col.dims()[2];
    int filter_width = col.dims()[3];
    int output_depth = col.dims()[4];
    int output_height = col.dims()[5];
    int output_width = col.dims()[6];
    int channels_col =
        input_channels * filter_depth * filter_height * filter_width;

    T* vol_data = vol.data<T>();
    const T* col_data = col.data<T>();

    for (int c = 0; c < channels_col; ++c) {
      int w_offset = c % filter_width;
      int h_offset = (c / filter_width) % filter_height;
      int d_offset = (c / filter_width / filter_height) % filter_depth;
      int cIm = c / filter_width / filter_height / filter_depth;
      for (int d = 0; d < output_depth; ++d) {
        int d_pad = d * stride_depth - padding_depth + d_offset;
        for (int h = 0; h < output_height; ++h) {
          int h_pad = h * stride_height - padding_height + h_offset;
          for (int w = 0; w < output_width; ++w) {
            int w_pad = w * stride_width - padding_width + w_offset;

            if (h_pad >= 0 && h_pad < input_height && w_pad >= 0 &&
                w_pad < input_width && d_pad >= 0 && d_pad < input_depth) {
              int vol_idx =
                  ((cIm * input_depth + d_pad) * input_height + h_pad) *
                      input_width +
                  w_pad;
              int col_idx =
                  ((c * output_depth + d) * output_height + h) * output_width +
                  w;
              vol_data[vol_idx] += col_data[col_idx];
            }
          }
        }
      }
    }
  }
};

template class Vol2ColFunctor<platform::CPUPlace, float>;
template class Vol2ColFunctor<platform::CPUPlace, double>;
template class Col2VolFunctor<platform::CPUPlace, float>;
template class Col2VolFunctor<platform::CPUPlace, double>;

}  // namespace math
}  // namespace operators
}  // namespace paddle
