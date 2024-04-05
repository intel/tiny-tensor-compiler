using namespace tinytc::ir;

auto pb = program_builder{};
try {
    pb.create("gemm", [&](function_builder &fb) {
        auto A = fb.argument(memref_type(scalar_type::f32, {16, 4, dynamic}), "A");
        auto B = fb.argument(memref_type(scalar_type::f32, {4, 8}), "B");
        auto C = fb.argument(memref_type(scalar_type::f32, {16, 8, dynamic}), "C");
        fb.work_group_size(16, 1);
        fb.body([&](region_builder &bb) {
            auto gid = bb.create_group_id();
            auto a = bb.create_subview(A, {slice(0, ir::dynamic), slice(0, ir::dynamic), gid});
            auto c = bb.create_subview(C, {slice(0, ir::dynamic), slice(0, ir::dynamic), gid});
            bb.create_gemm(transpose::N, transpose::N, value(1.0, scalar_type::f32), a, B,
                           value(0.0, scalar_type::f32), c);
        });
    });
} catch (compilation_error const &e) {
    std::cerr << e.loc() << ": " << e.what() << std::endl;
}

