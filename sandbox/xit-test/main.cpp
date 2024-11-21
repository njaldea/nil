#include "userland_utils.hpp"

#include <filesystem>

// TODO: move to command line arguments or env var?
const auto source_path = std::filesystem::path(__FILE__).parent_path();
XIT_PATH_MAIN_UI_DIRECTORY(source_path);
XIT_PATH_UI_DIRECTORY(source_path);
XIT_PATH_TEST_DIRECTORY(source_path);
XIT_PATH_SERVER_DIRECTORY(source_path / "node_modules/@nil-/xit");

// -  Frame == Panel
//     -  Each Frame represents one single data
// -  FRAME MACRO signature
//     -  1ST argument is the FRAME ID
//     -  2ND argument is the path to the UI file
//     -  3RD argument of FRAME MACROS dictates the type it will hold
//         -  for input frames:
//             -  can either be a instance of an object (ex Ranges(3, 2, 1))
//             -  or a callable that can provide the data (ex from_file)
//         -  for output frames:
//             -  just a type
// -  `value()` signature
//     -  only value id
//           -  the whole data owned by the frame will be bound to the value id
//     -  value id with getter/setter, accessor, or pointer to member
//           -  portion of the data owned by the frame will be bound the the specified value id
// - Test
//     -  see Sample below to define what the inputs and outputs of a test
//     -  Input Frames are accessible from the test via `xit_inputs`
//     -  Output Frames are accessible from the test via `xit_outputs`
//     -  destructuring `xit_inputs` and `xit_outputs` are the way to access each frame
//     -  You can also use `get<N>(xit_inputs)` or `get<N>(xit_inputs)`
// -  To be able to use strings(frame id) in defining the inputs/outputs, the definition
// (like Sample) should have visibility of the FRAME MACRO usage.
// -  Use XIT_PATH MACROS to modify where to access the files
//     -  XIT_PATH_TEST_DIRECTORY    -- from_file and XIT_TEST files
//     -  XIT_PATH_MAIN_UI_DIRECTORY -- main ui file
//     -  XIT_PATH_UI_DIRECTORY      -- ui files

XIT_FRAME_MAIN("gui/Main.svelte", nlohmann::json);
// also accepts callable types (nlohamnn::json is a class/struct)
// [](const std::vector<std::string>& v) { return nlohmann::json(v); }

XIT_FRAME_TAGGED_INPUT(
    "input_frame",
    "gui/InputFrame.svelte",
    from_file("files", "input_frame.json", &as_json)
)
    .value("value");

// XIT_FRAME_UNIQUE_INPUT("slider_frame", "gui/Slider.svelte", Ranges(3, 2, 1))
XIT_FRAME_UNIQUE_INPUT(
    "slider_frame",
    "gui/Slider.svelte",
    from_file("files", "slider_frame", &as_range)
)
    .value("value-1", &Ranges::v1)
    .value("value-2", &Ranges::v2)
    .value("value-3", &Ranges::v3);

XIT_FRAME_OUTPUT("view_frame", "gui/ViewFrame.svelte", nlohmann::json)
    .value("value-x", from_json_ptr("/x"))
    .value("value-y", from_json_ptr("/y"));

using Sample = nil::xit::gtest::Test<
    nil::xit::gtest::Input<"input_frame", "slider_frame">,
    nil::xit::gtest::Output<"view_frame">>;

XIT_TEST(Sample, Demo, "files")
{
    const auto& [input_data, ranges] = xit_inputs;
    //           ┃           ┃         ┗━━━ from Input<"input_frame", "slider_frame">
    //           ┃           ┗━━━ type == Ranges
    //           ┗━━━ type == nlohmann::json

    auto& [view] = xit_outputs;
    //     ┃       ┗━━━ from Output<"view_frame">
    //     ┗━━━ type == nlohmann::json

    view = input_data;
    view["y"][0] = std::int64_t(input_data["y"][0]) * ranges.v1;
    view["y"][1] = std::int64_t(input_data["y"][1]) * ranges.v2;
    view["y"][2] = std::int64_t(input_data["y"][2]) * ranges.v3;
}

// TODO:
//  -   another frame to represent the "page" containing input/output frame list
//  -   headless mode
//  -   builders are in xit_gtest...
//  -   how about runtime configured tests (scripting languages)
