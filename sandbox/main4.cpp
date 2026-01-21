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
        // Initialize tensors
        matrix<float> A(36, 32, 1);
        matrix<float> B(36, 32, 0);

        const std::string code = R"TinyTL(
func @tilling(%A: memref<f32x36x32> {alignment=128},
              %B: memref<f32x36x32> {alignment=128})        
     attributes{subgroup_size=16,work_group_size=[16,16]} {
    ; alias
    $mat_t = coopmatrix<f32x16x16,matrix_acc>
    %c0 = constant 0 : index
    %c1 = constant 36 : index
    %c11 = constant 32 : index
    %c2 = constant 2.0 : $mat_t 
    %c4 = constant 3.0 : $mat_t 

 ; def: shape w = 16, h = 16, size = h * w
 ; i,j are the beginning of every tile
 ; I understand ti and tj are the reminders  
 ; c0 and c1, c11 must be the total size of the matrix 
 ; 16,16 I think is the shape of the tilling
 ; t_i = min(tile_shape_i , to_i - var_i) (reminder)
 ; foreach_tile (var_i, ...) = (from_i, ...),(to_i, ...)

;  %toto = constant 4 : index
; here I check the reminder
    foreach_tile (%i,%j)=(%c0,%c0),(%c1,%c11) as (%ti,%tj)<=(16,16) {
        %c3 = constant 16 : index
; I just check the reminder on the number of row here 4 <16
        %is_remainder = less_than %ti, %c3 : bool
; variant with the exact value of the reminder
;       %is_remainder = equal %ti, %toto : bool
        if %is_remainder {
            %tile = cooperative_matrix_load %A[%i,%j] : $mat_t
            %tile_final = add %tile, %c4 : $mat_t
; I suppose it know the dimension of b and the position of the Tile so it can check if there is "overwrite" beyond the last row
            cooperative_matrix_store.rows_checked %tile_final, %B[%i,%j]
        } else {
; normal tile there is nothing to check
             %tile = cooperative_matrix_load.rows_checked %A[%i,%j] : $mat_t
             %tile_final = add %tile, %c2 : $mat_t
             cooperative_matrix_store %tile_final, %B[%i,%j]
        }
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
        const auto global_range = sycl::range<3u>{1,3,3};

        q.submit([&](sycl::handler &h) {
             h.set_args(A.data(),B.data());
             h.parallel_for(tinytc::get_global_size(global_range, sycl::range<3u>(1,16,16)), kernel);
         }).wait();

        std::cout << A << std::endl;
        std::cout << B << std::endl;
    } catch (tinytc::status const &st) {
        std::cerr << "Error (" << static_cast<int>(st) << "): " << tinytc::to_string(st)
                  << std::endl;
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}
