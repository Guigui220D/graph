#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/Window.h>

void testFn(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        //buffer[i] = sinf(x*x) / (x + 1);
        buffer[i] = (x > 5) ? 3 : 0;
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

float gradPeriod(float span) {
    return pow(10, floorf(log10f(span)));
}

int main() {
    const size_t SIZE = 1000;
    float samples[SIZE];

    testFn(samples, SIZE);
    float upper = upperBound(samples, SIZE);
    float lower = lowerBound(samples, SIZE);
    upper += (upper - lower) / 10;
    lower += (lower - upper) / 10;
    float grad = gradPeriod(upper - lower);

    printf("Upper: %f, Lower: %f\n", upper, lower);
    printf("Grad: %f\n", grad);

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
    sfShader_setFloatUniform(graph_shader, "gradPeriod", grad);
    sfShader_setFloatUniformArray(graph_shader, "data", samples, SIZE);

    sfVector2f canvas_size = { 800, 600 };
    //sfVector2f canvas_pos = { 100, 100};

    sfRenderTexture* render_texture = sfRenderTexture_create(800, 600, sfFalse);

    sfRectangleShape* canvas = sfRectangleShape_create();
    sfRectangleShape_setSize(canvas, canvas_size);
    sfRectangleShape_setFillColor(canvas, sfBlack);
    //sfRectangleShape_setPosition(canvas, canvas_pos);

    sfRenderStates states = { .blendMode = sfBlendAlpha, .transform = sfTransform_Identity, .texture = NULL, .shader = graph_shader };
    sfRenderTexture_clear(render_texture, sfBlack);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, NULL);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, &states);
    sfRenderTexture_display(render_texture);

    sfImage* rendered = sfTexture_copyToImage(sfRenderTexture_getTexture(render_texture));
    sfImage_saveToFile(rendered, "render.png");

    sfImage_destroy(rendered);
    sfRectangleShape_destroy(canvas);
    sfRenderTexture_destroy(render_texture);
    sfShader_destroy(graph_shader);
    return 0;
}

