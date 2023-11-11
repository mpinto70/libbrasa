#include <brasa/buffer/Copy.h>

#include <gtest/gtest.h>

#include <numeric>
#include <random>
#include <source_location>
#include <string>

namespace brasa::buffer::impl {
namespace {

auto create_random(size_t n, char b, char e) {
    static std::mt19937 eng(std::random_device{}());
    std::uniform_int_distribution<char> dist(b, e);
    std::string out(n, ' ');
    std::generate(out.begin(), out.end(), [&dist]() { return dist(eng); });
    return out;
}

using copy_func_t = void (*)(void*, const void*, size_t);

class CopyTest : public ::testing::TestWithParam<std::tuple<copy_func_t, size_t, size_t>> {
protected:
    CopyTest()
          : copy_func_(std::get<0>(GetParam())),
            pad_size_(std::get<1>(GetParam())),
            buffer_size_(std::get<2>(GetParam())),
            pad_(create_random(pad_size_, '0', '9')),
            src_(create_random(buffer_size_, 'a', 'z')),
            dest0_(create_random(buffer_size_, 'A', 'Z')) {}

    std::string memory() const noexcept { return pad_ + dest0_ + pad_; }

    void expect_eq_pad(
          const std::string& dest,
          const std::source_location& loc = std::source_location::current()) const;

    const copy_func_t copy_func_;
    const size_t pad_size_;
    const size_t buffer_size_;
    const std::string pad_;
    const std::string src_;
    const std::string dest0_;
};

void CopyTest::expect_eq_pad(const std::string& dest, const std::source_location& loc) const {
    SCOPED_TRACE("FROM " + std::string(loc.file_name()) + ":" + std::to_string(loc.line()));
    EXPECT_EQ(dest.substr(0, pad_.size()), pad_);
    EXPECT_EQ(dest.substr(dest.size() - pad_.size()), pad_);
}

TEST_P(CopyTest, copy_full_buffer) {
    auto dest = memory();
    auto expected = dest;

    copy_func_(dest.data() + pad_size_, src_.data(), buffer_size_);
    EXPECT_NE(dest, expected);
    std::copy(src_.begin(), src_.end(), expected.begin() + pad_size_);
    EXPECT_EQ(dest, expected);
    expect_eq_pad(dest);
}

TEST_P(CopyTest, copy_unaligned_src_begin) {
    const auto dest0 = memory();
    for (size_t offset = 1; offset < 8; ++offset) {
        auto dest = dest0;
        auto expected = dest0;
        const auto size = buffer_size_ - offset;
        SCOPED_TRACE("src = " + src_ + ", dest = " + dest + ", offset = " + std::to_string(offset));

        copy_func_(dest.data() + pad_size_, src_.data() + offset, size);
        EXPECT_NE(dest, expected);
        std::copy(src_.begin() + offset, src_.end(), expected.begin() + pad_size_);
        EXPECT_EQ(dest, expected);
        expect_eq_pad(dest);
    }
}

TEST_P(CopyTest, copy_unaligned_dest_begin) {
    const auto dest0 = memory();
    for (size_t offset = 1; offset < 8; ++offset) {
        auto dest = dest0;
        auto expected = dest0;
        const auto size = buffer_size_ - offset;
        SCOPED_TRACE("src = " + src_ + ", dest = " + dest + ", offset = " + std::to_string(offset));

        copy_func_(dest.data() + pad_size_ + offset, src_.data(), buffer_size_ - offset);
        EXPECT_NE(dest, expected);
        std::copy(src_.begin(), src_.begin() + size, expected.begin() + pad_size_ + offset);
        EXPECT_EQ(dest, expected);
        expect_eq_pad(dest);
    }
}

TEST_P(CopyTest, copy_unaligned_both_begin) {
    const auto dest0 = memory();
    for (size_t offset_dest = 1; offset_dest < 8; ++offset_dest) {
        for (size_t offset_src = 1; offset_src < 8; ++offset_src) {
            auto dest = dest0;
            auto expected = dest0;
            SCOPED_TRACE(
                  "src = " + src_ + ", offset = " + std::to_string(offset_src) + ", dest = " + dest
                  + ", offset = " + std::to_string(offset_dest));
            const size_t size = buffer_size_ - std::max(offset_dest, offset_src);

            copy_func_(dest.data() + pad_size_ + offset_dest, src_.data() + offset_src, size);
            EXPECT_NE(dest, expected);
            std::copy(
                  src_.begin() + offset_src,
                  src_.begin() + offset_src + size,
                  expected.begin() + pad_size_ + offset_dest);
            EXPECT_EQ(dest, expected);
            expect_eq_pad(dest);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
      Correctness,
      CopyTest,
      testing::Combine(
            testing::Values(copy_by_byte, copy_by_qword),
            testing::Values(16, 32),  // pad size
            testing::Values(64, 96)), // buffer size
      [](const auto& info) {
          std::string func = "Not processed fix it";
          if (std::get<0>(info.param) == copy_by_byte) {
              func = "copy_by_byte_";
          } else if (std::get<0>(info.param) == copy_by_qword) {
              func = "copy_by_qword_";
          }
          return func + "PadSize_" + std::to_string(std::get<1>(info.param)) + "_BufferSize_"
                 + std::to_string(std::get<2>(info.param));
      });
} // namespace
} // namespace brasa::buffer::impl
