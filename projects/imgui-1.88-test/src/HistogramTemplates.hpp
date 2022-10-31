
#include "Histogram.hpp"
#include "HistogramRenderObj.hpp"

// template instantiations
//
template class Histogram<int8_t>;
template class Histogram<int16_t>;
template class Histogram<int32_t>;
template class Histogram<int64_t>;

template class Histogram<uint8_t>;
template class Histogram<uint16_t>;
template class Histogram<uint32_t>;
template class Histogram<uint64_t>;

template class Histogram<float>;
template class Histogram<double>;

//
template class HistogramRenderObj<int8_t>;
template class HistogramRenderObj<int16_t>;
template class HistogramRenderObj<int32_t>;
template class HistogramRenderObj<int64_t>;

template class HistogramRenderObj<uint8_t>;
template class HistogramRenderObj<uint16_t>;
template class HistogramRenderObj<uint32_t>;
template class HistogramRenderObj<uint64_t>;

template class HistogramRenderObj<float>;
template class HistogramRenderObj<double>;

