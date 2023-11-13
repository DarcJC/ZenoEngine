#pragma once

#pragma warning(disable:4146)

#if defined(ZENO_LOAD_PATH)
#include "roads/thirdparty/zpp_bits.h"
#else
#include "zpp_bits.h"
#endif

namespace roads::service {
    using namespace zpp::bits::literals;

    int32_t TestFunc(int32_t i);

    using rpc = zpp::bits::rpc<
        zpp::bits::bind<TestFunc, "TestFunc"_sha256_int>
    >;
}
