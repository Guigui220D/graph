const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const lib = b.addStaticLibrary("graph", "src/main.zig");
    lib.linkLibC();
    lib.addLibPath("glfw-3.3.7/lib-mingw-w64");
    lib.linkSystemLibrary("glfw3");
    lib.linkSystemLibrary("opengl32");
    if (@import("builtin").os.tag == .windows)
        lib.linkSystemLibrary("gdi32");
    lib.addIncludeDir("glfw-3.3.7/include");
    lib.addIncludeDir("glad/include");
    lib.addCSourceFile("glad/src/glad.c", &[_][]const u8{});
    lib.setBuildMode(mode);
    lib.install();

    const main_tests = b.addTest("src/main.zig");
    main_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&main_tests.step);

    const ctest = b.addExecutable("cexe", null);
    ctest.linkLibC();
    ctest.addCSourceFile("src/test.c", &[_][]const u8{});
    ctest.addLibPath("zig-out/lib");
    ctest.addLibPath("glfw-3.3.7/lib-mingw-w64");
    ctest.linkSystemLibrary("graph");
    ctest.linkSystemLibrary("glfw3");
    ctest.linkSystemLibrary("opengl32");
    if (@import("builtin").os.tag == .windows)
        ctest.linkSystemLibrary("gdi32");
    ctest.setBuildMode(mode);
    //ctest.install();
    ctest.step.dependOn(&lib.install_step.?.step);

    const run_step = b.step("run", "Run the c test program");
    run_step.dependOn(&ctest.run().step);
}
