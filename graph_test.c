#include <stdio.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/System.h>
#include <SFML/Window.h>

void testFn(float * buffer, size_t size) {
    for (int i = 0; i < size; i++) {
        float x = (float)i / 1000.f;
        x *= 10;
        //buffer[i] = cosf(x * x) * x * x;
        buffer[i] = (i / 50) % 3;
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
    float b10 = pow(10, floorf(log10f(span)));
    if (span / b10 <= 3)
        b10 /= 2;
    return b10;
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

    sfClock* benchclock = sfClock_create();

    if (!sfShader_isAvailable()) {
        printf("Fatal: shader failed to initialize\n");
        return 1;
    }

    sfShader* graph_shader = sfShader_createFromFile(NULL, NULL, "graph_shader.fs");

    if (!graph_shader) {
        printf("Fatal: shader failed to initialize\n");
        return 1;
    }

    sfVector2f canvas_size = { 800, 600 };

    sfShader_setFloatUniform(graph_shader, "upperLimit", upper);
    sfShader_setFloatUniform(graph_shader, "lowerLimit", lower);
    sfShader_setFloatUniform(graph_shader, "gradPeriod", grad);
    sfShader_setFloatUniformArray(graph_shader, "data", samples, SIZE);
    sfShader_setVec2Uniform(graph_shader, "resolution", canvas_size);

    printf("Shader loading took %fs\n", sfTime_asSeconds(sfClock_restart(benchclock)));

    sfRenderTexture* render_texture = sfRenderTexture_create(800, 600, sfFalse);

    sfRectangleShape* canvas = sfRectangleShape_create();
    sfRectangleShape_setSize(canvas, canvas_size);
    sfRectangleShape_setFillColor(canvas, sfBlack);
    //sfRectangleShape_setPosition(canvas, canvas_pos);

    sfFont* grad_font = sfFont_createFromFile("open-sans.regular.ttf");
    sfText* grad_text = sfText_create();
    sfText_setFont(grad_text, grad_font);
    sfText_setCharacterSize(grad_text, 12);
    char buffer[100];

    sfRenderStates states = { .blendMode = sfBlendAlpha, .transform = sfTransform_Identity, .texture = NULL, .shader = graph_shader };
    
    sfClock_restart(benchclock);
    
    sfRenderTexture_clear(render_texture, sfBlack);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, NULL);
    sfRenderTexture_drawRectangleShape(render_texture, canvas, &states);
    
    float moving_grad = ceilf(lower / grad) * grad;
    do {
        sprintf(buffer, "%.2e", moving_grad);
        sfText_setString(grad_text, buffer);
        float actual_height = (1 - (moving_grad - lower) / (upper - lower)) * 500 + 50;
        sfText_setPosition(grad_text, (sfVector2f){ 0, actual_height });
        sfRenderTexture_drawText(render_texture, grad_text, NULL);
        moving_grad += grad; 
    } while (moving_grad < upper);
    
    sfRenderTexture_display(render_texture);

    printf("Render took %fs\n", sfTime_asSeconds(sfClock_getElapsedTime(benchclock)));

    sfImage* rendered = sfTexture_copyToImage(sfRenderTexture_getTexture(render_texture));

    printf("Render + copy to ram took %fs\n", sfTime_asSeconds(sfClock_restart(benchclock)));

    sfImage_saveToFile(rendered, "render.png");


    sfImage_destroy(rendered);
    sfText_destroy(grad_text);
    sfFont_destroy(grad_font);
    sfRectangleShape_destroy(canvas);
    sfRenderTexture_destroy(render_texture);
    sfShader_destroy(graph_shader);
    return 0;
}

