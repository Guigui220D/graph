#version 130

uniform float upperLimit;
uniform float lowerLimit;
uniform float data[1000];

out vec4 color;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

float peak(float x, float width) {
    return width / abs(x);
}

void main() {
    vec2 coord = gl_FragCoord.xy;
    int index = int((coord.x / 800) * 1000);
    float height = map(data[index], lowerLimit, upperLimit, 0.0, 1.0);

    float i = peak((coord.y / 600) - height, 0.002);

    color = vec4(i, i, 0.0 / 800, 1.0);
}