#ifndef TACHYON_BASE_BUFFER_VECTOR_BUFFER_H_
#define TACHYON_BASE_BUFFER_VECTOR_BUFFER_H_

#include <vector>
#include <utility>

#include "tachyon/base/buffer/buffer.h"

namespace tachyon::base {

class TACHYON_EXPORT VectorBuffer : public Buffer {
 public:
  VectorBuffer() = default;
  VectorBuffer(const VectorBuffer& other) = delete;
  VectorBuffer& operator=(const VectorBuffer& other) = delete;
  VectorBuffer(VectorBuffer&& other)
      : Buffer(std::move(other)),
        owned_buffer_(std::move(other.owned_buffer_)) {}
  VectorBuffer& operator=(VectorBuffer&& other) {
    owned_buffer_ = std::move(other.owned_buffer_);
    return *this;
  }
  ~VectorBuffer() override = default;

  bool Grow(size_t size) override {
    owned_buffer_.resize(size);
    buffer_ = owned_buffer_.data();
    buffer_len_ = owned_buffer_.size();
    return true;
  }

 protected:
  std::vector<char> owned_buffer_;
};

}  // namespace tachyon::base

#endif  // TACHYON_BASE_BUFFER_VECTOR_BUFFER_H_
