#include "../src/FontResolve.h"
#include <cstdio>
#include <cstdlib>
using namespace AlterEgo::Font;

static int g_fail = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); ++g_fail; } } while (0)

int main() {
    // Default face -> empty id, IsDefault true.
    Config d; // {NexusDefault, "", 16}
    CHECK(Identifier(d).empty());
    CHECK(IsDefault(d));

    // Bundled id format + px rounding.
    Config b{Face::Bundled, "", 16.0f};
    CHECK(Identifier(b) == "AlterEgo_F_BUNDLED_16");
    CHECK(!IsDefault(b));
    Config b18{Face::Bundled, "", 18.0f};
    Config b1799{Face::Bundled, "", 17.99f};
    CHECK(Identifier(b18) == Identifier(b1799));      // 18.0 and 17.99 -> same id
    Config b19{Face::Bundled, "", 19.0f};
    CHECK(Identifier(b18) != Identifier(b19));         // 18 vs 19 differ

    // Custom id depends on path; same path+px -> same id, different path -> different id.
    Config ca{Face::Custom, "/x/a.ttf", 16.0f};
    Config ca2{Face::Custom, "/x/a.ttf", 16.0f};
    Config cb{Face::Custom, "/x/b.ttf", 16.0f};
    CHECK(Identifier(ca) == Identifier(ca2));
    CHECK(Identifier(ca) != Identifier(cb));
    CHECK(Identifier(ca).rfind("AlterEgo_F_C", 0) == 0);

    if (g_fail == 0) { printf("ALL PASS\n"); return 0; }
    printf("%d FAILURE(S)\n", g_fail); return 1;
}
