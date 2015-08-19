#include "common/memory/memory.h"
#include "common/math/math_types.h"
#include "common/math/math.h"
#include "common/math/quat.h"
#include "common/math/vector2.h"
#include "common/math/matrix33.h"
#include "common/math/simd/simd.h"
#include "common/types.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/ecs/entitymanager.h"
#include "runtime/runtime.h"

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace cetech1;
using namespace rapidjson;



void init() {
    memory_globals::init();
    runtime::init();    
}

void run() {
    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. Modify it by DOM.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);

    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer < StringBuffer > writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;

    const cetech1::Vector2 v = cetech1::vector2::normalized({2.0f, 0.0f});

    std::cout << "len: " << cetech1::vector2::len(v) << ", " << cetech1::vector2::is_normalized(v) << std::endl;

    cetech1::Matrix33 m1 = cetech1::matrix33::IDENTITY;
    cetech1::Matrix33 m2 = cetech1::matrix33::IDENTITY;

    cetech1::Matrix33 m3 = m1 * m2;

    printf("sqrt: %f, ivn_sqrt: %f\n", cetech1::math::fast_sqrt(4), cetech1::math::fast_inv_sqrt(4));
}

void shutdown() {
    runtime::shutdown();
    memory_globals::shutdown();
}

int main(int argc, char** argv) {
    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
