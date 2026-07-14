#include <catch2/catch_test_macros.hpp>
#include <amd64/ir/ir.hpp>
#include <amd64/ir/fmt.hpp>

using namespace amd64::ir;

TEST_CASE("fmt_inst renders iconst correctly", "[ir][format]") {
    function_t fn;
    fn.create_block({});
    fn.iconst(42);

    std::string output;
    fmt_inst(fn.blocks[0].instructions[0], output);

    REQUIRE(output == "\t%0 = iconst 42\n");
}

TEST_CASE("fmt_inst renders iadd correctly", "[ir][format]") {
    function_t fn;
    fn.create_block({});
    auto v1 = fn.iconst(10);
    auto v2 = fn.iconst(20);
    fn.iadd(v1, v2);

    std::string output;
    fmt_inst(fn.blocks[0].instructions[2], output);

    REQUIRE(output == "\t%2 = iadd %0, %1\n");
}

TEST_CASE("fmt_inst renders icmp correctly", "[ir][format]") {
    function_t fn;
    fn.create_block({});
    auto v1 = fn.iconst(1);
    auto v2 = fn.iconst(0);
    fn.icmp(set_cc_kind::eq, v1, v2);

    std::string output;
    fmt_inst(fn.blocks[0].instructions[2], output);

    REQUIRE(output == "\t%2 = icmp.eq %0, %1\n");
}

TEST_CASE("fmt_block renders header and parameters", "[ir][format]") {
    function_t fn;
    fn.create_block({type_t::i64, type_t::boolean});

    std::string output;
    fmt_block(fn.blocks[0], 0, fn.param_indices[0], output);

    REQUIRE(output.find("bb0(i64 %0, bool %1):") != std::string::npos);
}

TEST_CASE("to_string renders a complete function", "[ir][format]") {
    module_t mod;

    auto& fn = mod.create_function("test_fn", {type_t::i64}, type_t::i64);
    auto val = fn.iconst(100);
    fn.ret(val);

    std::string output = to_string(fn);
    std::string expected_ret = std::format("\tret %{}", val);

    REQUIRE(output.find("fn @test_fn(i64 %0) -> i64 {") != std::string::npos);
    REQUIRE(output.find(expected_ret) != std::string::npos);
}

TEST_CASE("fmt_inst renders call native correctly", "[ir][format]") {
    function_t fn;
    fn.create_block({});

    int dummy = 0;

    call_target_t target{.target = static_cast<const void*>(&dummy)};
    fn.call({}, target);

    std::string output;
    fmt_inst(fn.blocks[0].instructions[0], output);

    REQUIRE(output.find("call native@") != std::string::npos);
}