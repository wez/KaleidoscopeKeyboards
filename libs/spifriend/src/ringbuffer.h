/*
Copyright 2016-2017 Wez Furlong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
namespace wezkeeb {
// A simple ringbuffer holding Size elements of type T
template <typename T, uint8_t Size>
class RingBuffer {
 protected:
  T buf_[Size];
  uint8_t head_{0}, tail_{0};
 public:
  inline uint8_t nextPosition(uint8_t position) {
    return (position + 1) % Size;
  }

  inline uint8_t prevPosition(uint8_t position) {
    if (position == 0) {
      return Size - 1;
    }
    return position - 1;
  }

  inline bool enqueue(const T &item) {
    static_assert(Size > 1, "RingBuffer size must be > 1");
    uint8_t next = nextPosition(head_);
    if (next == tail_) {
      // Full
      return false;
    }

    buf_[head_] = item;
    head_ = next;
    return true;
  }

  inline bool get(T &dest, bool commit = true) {
    auto tail = tail_;
    if (tail == head_) {
      // No more data
      return false;
    }

    dest = buf_[tail];
    tail = nextPosition(tail);

    if (commit) {
      tail_ = tail;
    }
    return true;
  }

  // Commit the result of a prior peek.
  // Ignores the value!
  inline bool commit() {
    auto tail = tail_;
    if (tail == head_) {
      // No more data
      return false;
    }

    tail_ = nextPosition(tail);
    return true;
  }

  inline bool empty() const { return head_ == tail_; }

  inline uint8_t size() const {
    int diff = head_ - tail_;
    if (diff >= 0) {
      return diff;
    }
    return Size + diff;
  }

  inline T& front() {
    return buf_[tail_];
  }

  inline bool peek(T &item) {
    return get(item, false);
  }
};
}
