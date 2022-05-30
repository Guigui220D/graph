const std = @import("std");

const c = @cImport({
    @cInclude("glad/glad.h");
    @cInclude("GLFW/glfw3.h");
});

const InogGraph = struct {
    thread: std.Thread,
    stop: bool,
    data: []const f32,
};

export fn inog_init_resources() c_int {
    std.log.debug("Initializing glfw", .{});
    if (c.glfwInit() == 0) {
        std.log.err("Could not init glfw context", .{});
        return -1;
    }

    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MAJOR, 3);
    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MINOR, 3);
    c.glfwWindowHint(c.GLFW_OPENGL_PROFILE, c.GLFW_OPENGL_CORE_PROFILE);

    return 0;
}

export fn inog_deinit_resources() void {
    c.glfwTerminate();
}

export fn inog_make_context(data: [*]const f32, size: usize) ?*InogGraph {
    return make_context(std.heap.c_allocator, data[0..size]) catch |e| blk: {
        std.log.err("Could not create context: {}", .{e});
        break :blk null;
    };
}

fn make_context(allocator: std.mem.Allocator, data: []const f32) !*InogGraph {
    std.log.debug("Allocating context", .{});
    var ret = try allocator.create(InogGraph);
    errdefer allocator.destroy(ret);
    ret.data = try allocator.dupe(f32, data);
    errdefer allocator.free(ret.data);
    ret.stop = false;
    std.log.debug("Spawning thread", .{});
    ret.thread = try std.Thread.spawn(.{}, window_thread_function, .{ ret });
    return ret;
}

fn window_thread_function(context: *InogGraph) void {
    std.log.debug("Creating window", .{});

    // TODO get error codes out
    var window = c.glfwCreateWindow(800, 600, "Graph window", null, null) orelse {
        std.log.err("Could not create window", .{});
        return;
    };
    c.glfwMakeContextCurrent(window);

    std.log.debug("Loading glad", .{});
    if (c.gladLoadGLLoader(@ptrCast(c.GLADloadproc, c.glfwGetProcAddress)) == 0) {
        std.log.err("Could not load GLAD", .{});
        return;
    }

    std.log.debug("Initializing OpenGL", .{});

    c.glViewport(0, 0, 800, 600);

    std.log.debug("Entering window loop", .{});
    while (c.glfwWindowShouldClose(window) == 0) {
        if (@atomicLoad(bool, &context.stop, .Unordered))
            break;

        c.glClearColor(0.2, 0.3, 0.3, 1.0);
        c.glClear(c.GL_COLOR_BUFFER_BIT);

        c.glfwSwapBuffers(window);
        c.glfwPollEvents();
    }
    std.log.debug("Window closed", .{});
}

export fn inog_destroy_context(context: ?*InogGraph) void {
    if (context) |cont| {
        @atomicStore(bool, &cont.stop, true, .Unordered);
        cont.thread.join();
        std.heap.c_allocator.free(cont.data);
        std.heap.c_allocator.destroy(cont);
    } else std.log.warn("No context to destroy (null pointer)", .{});
}