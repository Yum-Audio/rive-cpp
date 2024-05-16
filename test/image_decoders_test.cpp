#include "rive_file_reader.hpp"
#include "rive_testing.hpp"
#include "rive/decoders/bitmap_decoder.hpp"

TEST_CASE("png file decodes correctly", "[image-decoder]")
{
    auto file = ReadFile("../../test/assets/placeholder.png");
    REQUIRE(file.size() == 1096);

    auto bitmap = Bitmap::decode(file.data(), file.size());

    REQUIRE(bitmap != nullptr);

    REQUIRE(bitmap->width() == 226);
    REQUIRE(bitmap->height() == 128);
}

TEST_CASE("jpeg file decodes correctly", "[image-decoder]")
{
    auto file = ReadFile("../../test/assets/open_source.jpg");
    REQUIRE(file.size() == 8880);

    auto bitmap = Bitmap::decode(file.data(), file.size());

    REQUIRE(bitmap != nullptr);

    REQUIRE(bitmap->width() == 350);
    REQUIRE(bitmap->height() == 200);
}
