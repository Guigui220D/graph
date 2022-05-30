zig build
zig cc test.c -L zig-out/lib -L glfw-3.3.7/lib-mingw-w64 -lc -lgraph -lglfw3 -lopengl32 -lgdi32