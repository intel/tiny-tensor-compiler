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


    auto toto = const_tinytc_core_info_t();


    try {
	// original size
	const int64_t size = 64;
        // Initialize tensors
        matrix<float> A(size, size, 1);
        matrix<float> B(size, size, 0);

        const std::string code = R"TinyTL(
func @tilling(%A: memref<f32x?x?,strided<1,?>> {alignment=128},
              %B: memref<f32x?x?,strided<1,?>> {alignment=128})        
     attributes{subgroup_size=16,work_group_size=[16,16]} {
    ; alias
    $mat_t = coopmatrix<f32x16x16,matrix_acc>
    ; creates tiles matrices of 16 x 16
    %c0 = constant 0 : index
    %c1 = constant 1.0 : $mat_t 
    %c16 = constant 16 : index 

    parallel {
    %sv = subview %A[%c0:%c16,%c0:%c16] : memref<f32x?x?,strided<1,?>>
    %tile = cooperative_matrix_load.both_checked %sv[%c0,%c0] : $mat_t
    %tile_final = add %tile, %c1 : $mat_t
    cooperative_matrix_store %tile_final, %B[%c0,%c0]
    }
})TinyTL";

        // JIT compile program
        auto q = sycl::queue{};
        auto program = tinytc::parse_string(code, ctx.get());
        auto bundle = tinytc::create_kernel_bundle(q.get_context(), q.get_device(), program.get());
        auto kernel = tinytc::create_kernel(bundle, "tilling");
        // range number of workgroup like cuda z,y,x order, here only one block
        // I fix here then number of block using z,y,x but the size of the block is defined in the
        // kernel workgroup size can not be defined at runtime in tinytc right now this function do
        // the transpose between the block/block size of Carsten to the "Sycl" range
	// 9 workgroup each work group has size 16 x 16 
	const int64_t tile_size = 16;
	const std::size_t gr_size = 1 + (A.rows() - 1) / tile_size; 
        const auto global_range = sycl::range<3u>{1, gr_size, gr_size  };

        std::cout << "gr_size: " << gr_size << std::endl;

        q.submit([&](sycl::handler &h) {
             h.set_args(A.data(),B.data());
             h.parallel_for(tinytc::get_global_size(global_range, sycl::range<3u>(1,16,16)), kernel);
         }).wait();

        std::cout << B << std::endl;
    } catch (tinytc::status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::to_string(st)
                  << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}
