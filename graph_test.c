#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/Window.h>

void testFn(float * buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (float)sin(((float)i) / 30) * i * i;
        //buffer[i] = 0.1;
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
    max *= (max > 0) ? 1.1f : 0.9f;
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
    min *= (min < 0) ? 1.1f : 0.9f;
    return min;
}

int main() {
    const size_t SIZE = 1000;
    float samples[SIZE];

    testFn(samples, SIZE);
    float upper = upperBound(samples, SIZE);
    float lower = lowerBound(samples, SIZE);

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

    sfRenderTexture* render_texture = sfRenderTexture_create(800, 600, sfFalse);
    sfRectangleShape* canvas = sfRectangleShape_create();
    sfRectangleShape_setSize(canvas, canvas_size);
    sfRectangleShape_setFillColor(canvas, sfWhite);

    sfRenderStates states = { .blendMode = sfBlendAlpha, .transform = sfTransform_Identity, .texture = NULL, .shader = graph_shader };
    sfRenderTexture_clear(render_texture, sfBlack);
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

