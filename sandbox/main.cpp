#include <iostream>

#include <sycl/sycl.hpp>
#include <tinytc/tinytc.hpp>
#include <tinytc/tinytc_sycl.hpp>

const static auto q = sycl::queue{};

template <typename T> class matrix {
  public:
    using value_type = T;
    constexpr static T poison = T(-1);

    matrix(std::int64_t rows, std::int64_t cols, T initial_value = poison)
        : rows_{rows}, cols_{cols}, alloc_(q), data_(rows * cols, initial_value, alloc_) {}

    inline auto rows() const -> std::int64_t { return rows_; }
    inline auto cols() const -> std::int64_t { return cols_; }
    inline auto bytes() const -> std::size_t { return rows_ * cols_ * sizeof(T); }

    inline auto data() -> T * { return data_.data(); }
    inline auto data() const -> T const * { return data_.data(); }

    inline auto operator()(std::int64_t i, std::int64_t j) -> T & { return data_[i + j * rows_]; }
    inline auto operator()(std::int64_t i, std::int64_t j) const -> T const & {
        return data_[i + j * rows_];
    }

    void print(std::ostream &os) const {
        for (std::int64_t i = 0; i < rows_; ++i) {
            for (std::int64_t j = 0; j < cols_; ++j) {
                os << operator()(i, j) << " ";
            }
            os << "\n";
        }
    }

  private:
    std::int64_t rows_, cols_;
    sycl::usm_allocator<T, sycl::usm::alloc::shared> alloc_;
    std::vector<T, decltype(alloc_)> data_;
};

template<class T>
std::ostream &operator<<(std::ostream &os, const matrix<T> &mat) {
    mat.print(os);
    return os;
}

int main(int argc, char *argv[]) {

    auto ctx = tinytc::create_compiler_context();
    tinytc::set_error_reporter(ctx.get(), [](char const *what, const tinytc_location_t *, void *) {
        std::cerr << what << std::endl;
    });
    try {
        // Initialize tensors
        matrix<float> A(32, 32);
        // kernel

        const std::string code = R"TinyTL(
func @store_block2d(%A: memref<f32x32x32> {alignment=128})
    attributes{subgroup_size=16,work_group_size=[16,1]} {
    parallel {
        %0 = constant 42.0 : coopmatrix<f32x8x8,matrix_acc>
        %1 = constant 23 : index
        %2 = constant 13 : index
        cooperative_matrix_store %0, %A[%1,%2]
    }
})TinyTL";

        // JIT compile program
        auto q = sycl::queue{};
        auto program = tinytc::parse_string(code, ctx.get());
        auto bundle = tinytc::create_kernel_bundle(q.get_context(), q.get_device(), program.get());
        auto kernel = tinytc::create_kernel(bundle, "store_block2d");
        // range number of workgroup like cuda z,y,x order, here only one block
        // I fix here then number of block using z,y,x but the size of the block is defined in the
        // kernel workgroup size can not be defined at runtime in tinytc right now this function do
        // the transpose between the block/block size of Carsten to the "Sycl" range
        auto exe_range = tinytc::get_execution_range(kernel, sycl::range<3u>{1, 1, 1});
        q.submit([&](sycl::handler &h) {
             h.set_args(A.data());
             h.parallel_for(exe_range, kernel);
         }).wait();

        std::cout << A << std::endl;
    } catch (tinytc::status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::to_string(st)
                  << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}
