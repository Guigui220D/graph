#version 130

uniform float upperLimit;
uniform float lowerLimit;
uniform float gradPeriod;
uniform float data[1000];

out vec4 color;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

float peak(float x, float width) {
    return (abs(x) < width) ? 1 : 0;
}

void main() {
  vec2 coord = gl_FragCoord.xy;
  if (coord.x >= 50 && coord.x <= 750 && coord.y >= 50 && coord.y <= 550) {

    int indexa = int(map(coord.x + 0, 50, 750, 0, 997));
    int indexb = int(map(coord.x + 1, 50, 750, 0, 997));
    int indexc = int(map(coord.x + 2, 50, 750, 0, 997));

    float avg1 = 0;
    for (int i = indexa; i < indexb; i++) {
      avg1 += data[i];
    }
    avg1 /= indexb - indexa;

    float avg2 = 0;
    for (int i = indexb; i < indexc; i++) {
      avg2 += data[i];
    }
    avg2 /= indexc - indexb;

    float height1 = map(avg1, lowerLimit, upperLimit, 50, 550);
    float height2 = map(avg2, lowerLimit, upperLimit, 50, 550);

    float height = (height1 + height2) / 2;
    float width = max(abs(height1 - height2) / 2, 1);

    float line_col = peak(coord.y - height, width);
    float dot_col = peak(coord.y - height1, 2);
    float graduation = 0;
    if (indexb % 50 < indexa % 50)
      graduation = 0.1;
    if (mod(map(coord.y, 50, 550, lowerLimit, upperLimit), gradPeriod) < gradPeriod / 50)
      graduation = 0.1;

    color = vec4(line_col + graduation, line_col - dot_col + graduation, graduation, 1);
  } else {
    color = vec4(vec3(0.2), 1);

    if (coord.x >= 25 && coord.x < 50) {
      if (mod(map(coord.y, 50, 550, lowerLimit, upperLimit), gradPeriod) < gradPeriod / 50)
        color = vec4(0, 1, 0, 1);
    }
  }
}