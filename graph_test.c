#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/Window.h>

void testFn(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        buffer[i] = sinf(x*x) / (x + 1);
    }
}

float upperBound(const float * samples, size_t size) {
    if (size == 0)
        return 0.f;
    float max = samples[0];
    for (int i = 1; i < size; i++) {
        if (samples[i] > max)
            max = samples[i];
    }
    return max;
}

float lowerBound(const float * samples, size_t size) {
    if (size == 0)
        return 0.f;
    float min = samples[0];
    for (int i = 1; i < size; i++) {
        if (samples[i] < min)
            min = samples[i];
    }
    return min;
}

float scale(float max) {
    float base10 = powf(floorf(log10f(max)) - 1, 10);
    base10 /= 10;
    return base10;
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

int main() {
    const size_t SIZE = 1000;
    float samples[SIZE];

    testFn(samples, SIZE);
    float upper = upperBound(samples, SIZE);
    float lower = lowerBound(samples, SIZE);
    upper += (upper - lower) / 10;
    lower += (lower - upper) / 10;

    printf("Upper: %f, Lower: %f\n", upper, lower);

    if (!sfShader_isAvailable()) {
        printf("Fatal: shader failed to initialize\n");
        return 1;
    }

    sfShader* graph_shader = sfShader_createFromFile(NULL, NULL, "graph_shader.fs");

    if (!graph_shader) {
        printf("Fatal: shader failed to initialize\n");
        return 1;
    }

    sfShader_setFloatUniform(graph_shader, "upperLimit", upper);
    sfShader_setFloatUniform(graph_shader, "lowerLimit", lower);
    sfShader_setFloatUniformArray(graph_shader, "data", samples, SIZE);

    sfVector2f canvas_size = { 800, 600 };
    //sfVector2f canvas_pos = { 100, 100};

    sfRenderTexture* render_texture = sfRenderTexture_create(800, 600, sfFalse);

    sfRectangleShape* canvas = sfRectangleShape_create();
    sfRectangleShape_setSize(canvas, canvas_size);
    sfRectangleShape_setFillColor(canvas, sfBlack);
    //sfRectangleShape_setPosition(canvas, canvas_pos);

    sfVector2f grad_size = { 25, 1 };
    sfRectangleShape* graduation = sfRectangleShape_create();
    sfRectangleShape_setSize(graduation, grad_size);
    sfRectangleShape_setFillColor(graduation, sfGreen);

    sfRenderStates states = { .blendMode = sfBlendAlpha, .transform = sfTransform_Identity, .texture = NULL, .shader = graph_shader };
    sfRenderTexture_clear(render_texture, sfBlack);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, NULL);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, &states);
    //draw grads
    // TODO: try to do this in the shader
    // bad bad bad
    float g = 0;
    float grad_space = scale(upper - lower);
    printf("Grad: %f\n", grad_space);
    sfVector2f grad_pos = { 25, 0 };
    if (upper < 0) {
        while (g > upper)
            g -= grad_space;
        while (g > lower) {
            grad_pos.y = map(g, lower, upper, 50, 550);
            sfRectangleShape_setPosition(graduation, grad_pos);
            sfRenderTexture_drawRectangleShape(render_texture, graduation, NULL);
            g -= grad_space;
        }
    } else if (lower > 0) {
        while (g < lower)
            g += grad_space;
        while (g < upper) {
            grad_pos.y = map(g, lower, upper, 50, 550);
            sfRectangleShape_setPosition(graduation, grad_pos);
            sfRenderTexture_drawRectangleShape(render_texture, graduation, NULL);
            g += grad_space;
        }
    } else {
        while (g < upper) {
            grad_pos.y = map(g, lower, upper, 50, 550);
            sfRectangleShape_setPosition(graduation, grad_pos);
            sfRenderTexture_drawRectangleShape(render_texture, graduation, NULL);
            g += grad_space;
        }
        g = 0;
        while (g > lower) {
            grad_pos.y = map(g, lower, upper, 50, 550);
            sfRectangleShape_setPosition(graduation, grad_pos);
            sfRenderTexture_drawRectangleShape(render_texture, graduation, NULL);
            g -= grad_space;
        }
    }
    sfRenderTexture_display(render_texture);

    sfImage* rendered = sfTexture_copyToImage(sfRenderTexture_getTexture(render_texture));
    sfImage_saveToFile(rendered, "render.png");

    sfImage_destroy(rendered);
    sfRectangleShape_destroy(canvas);
    sfRenderTexture_destroy(render_texture);
    sfShader_destroy(graph_shader);
    return 0;
}

