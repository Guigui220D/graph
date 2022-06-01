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
    return makeContext(std.heap.c_allocator, data[0..size]) catch |e| blk: {
        std.log.err("Could not create context: {}", .{e});
        break :blk null;
    };
}

fn makeContext(allocator: std.mem.Allocator, data: []const f32) !*InogGraph {
    std.log.debug("Allocating context", .{});
    var ret = try allocator.create(InogGraph);
    errdefer allocator.destroy(ret);
    ret.data = try allocator.dupe(f32, data);
    errdefer allocator.free(ret.data);
    ret.stop = false;
    std.log.debug("Spawning thread", .{});
    ret.thread = try std.Thread.spawn(.{}, windowThreadFunction, .{ ret });
    return ret;
}

fn windowThreadFunction(context: *InogGraph) void {
    if (context.data.len == 0) {
        // TODO: do something else
        std.log.err("No data to render", .{});
        return;
    }

    var success: c_int = undefined;
    var log_buf: [512]u8 = undefined;

    var max = std.mem.max(f32, context.data);
    var min = std.mem.min(f32, context.data);
    adjustBounds(&min, &max);
    const grad = calcGradPeriod(max - min);
    std.log.debug("Upper bound: {}, lower bound: {}, grad period: {}", .{max, min, grad});

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

    var vao: c.GLuint = undefined;
    c.glGenVertexArrays(1, &vao);
    c.glBindVertexArray(vao);

    var vbo: c.GLuint = undefined;
    c.glGenBuffers(1, &vbo);
    c.glBindBuffer(c.GL_ARRAY_BUFFER, vbo);
    c.glBufferData(c.GL_ARRAY_BUFFER, quad_vertices.len, &quad_vertices, c.GL_STATIC_DRAW);

    var ebo: c.GLuint = undefined;
    c.glGenBuffers(1, &ebo);
    c.glBindBuffer(c.GL_ELEMENT_ARRAY_BUFFER, ebo);
    c.glBufferData(c.GL_ELEMENT_ARRAY_BUFFER, quad_indices.len, &quad_indices, c.GL_STATIC_DRAW);

    var vertex_shader = c.glCreateShader(c.GL_VERTEX_SHADER);
    c.glShaderSource(vertex_shader, 1, @ptrCast([*]const[*]const u8, &vertex_shader_source), null);
    c.glCompileShader(vertex_shader);
    c.glGetShaderiv(vertex_shader, c.GL_COMPILE_STATUS, &success);
    if (success == 0) {
        // TODO: log message
        c.glGetShaderInfoLog(vertex_shader, 512, null, &log_buf);
        std.log.err("Failed to compile vertex shader:\n{s}", .{log_buf});
        return;
    }
    defer c.glDeleteShader(vertex_shader);

    var fragment_shader = c.glCreateShader(c.GL_FRAGMENT_SHADER);
    c.glShaderSource(fragment_shader, 1, @ptrCast([*]const[*]const u8, &fragment_shader_source), null);
    c.glCompileShader(fragment_shader);
    c.glGetShaderiv(fragment_shader, c.GL_COMPILE_STATUS, &success);
    if (success == 0) {
        c.glGetShaderInfoLog(fragment_shader, 512, null, &log_buf);
        std.log.err("Failed to compile fragment shader:\n{s}", .{log_buf});
        return;
    }
    defer c.glDeleteShader(fragment_shader);

    var shader_program = c.glCreateProgram();
    c.glAttachShader(shader_program, vertex_shader);
    c.glAttachShader(shader_program, fragment_shader);
    c.glLinkProgram(shader_program);
    c.glGetProgramiv(shader_program, c.GL_LINK_STATUS, &success);
    if (success == 0) {
        c.glGetProgramInfoLog(shader_program, 512, null, &log_buf);
        std.log.err("Failed to link shader program:\n{s}", .{log_buf});
        return;
    }
    defer c.glDeleteShader(fragment_shader);

    c.glUseProgram(shader_program);

    c.glUniform2f(c.glGetUniformLocation(shader_program, "resolution"), 800, 600);
    c.glUniform1f(c.glGetUniformLocation(shader_program, "upperLimit"), max);
    c.glUniform1f(c.glGetUniformLocation(shader_program, "lowerLimit"), min);
    c.glUniform1f(c.glGetUniformLocation(shader_program, "gradPeriod"), grad);
    //TODO: modify shader to tolerate more than 1000 samples
    c.glUniform1fv(c.glGetUniformLocation(shader_program, "data"), @intCast(c_int, context.data.len), context.data.ptr);

    // Specify the layout of the vertex data
    var pos_attrib = @intCast(c_uint, c.glGetAttribLocation(shader_program, "position"));
    c.glEnableVertexAttribArray(pos_attrib);
    c.glVertexAttribPointer(pos_attrib, 4, c.GL_FLOAT, c.GL_FALSE, 2 * @sizeOf(c.GLfloat), null);

    std.log.debug("Entering window loop", .{});
    while (c.glfwWindowShouldClose(window) == 0) {
        if (@atomicLoad(bool, &context.stop, .Unordered))
            break;

        c.glClearColor(0.5, 0.0, 0.5, 1.0);
        c.glClear(c.GL_COLOR_BUFFER_BIT);

        c.glDrawElements(c.GL_TRIANGLES, 6, c.GL_UNSIGNED_INT, null);

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

fn adjustBounds(lower: *f32, upper: *f32) void {
    const diff = upper.* - lower.*;
    upper.* += diff / 10;
    lower.* -= diff / 10;
}

fn calcGradPeriod(span: f32) f32 {
    // TODO
    return span / 100;
}

const quad_vertices = [_]f32{
    -1, 1,
    1, 1,
    1, -1,
    -1, -1,
};

const quad_indices = [_]u32{
    0, 1, 2,
    2, 3, 0,
};

const vertex_shader_source = 
\\#version 330 core
\\layout (location = 0) in vec3 position;
\\void main()
\\{
\\   gl_Position = vec4(position.x, position.y, position.z, 1.0);
\\}
;

// TODO: load that on program startup
const fragment_shader_source = @embedFile("graph_shader.fs");