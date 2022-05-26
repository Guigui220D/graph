#version 130

uniform float upperLimit;
uniform float lowerLimit;
uniform float gradPeriod;
uniform float data[1000];
uniform vec2 resolution;

out vec4 color;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

float peak(float x, float width) {
    return (abs(x) < width) ? 1 : 0;
}

void main() {
  vec2 coord = gl_FragCoord.xy;
  float xmax = resolution.x - 50;
  float ymax = resolution.y - 50;
  float xmin = 50;
  float ymin = 50;
  if (coord.x >= xmin && coord.x <= xmax && coord.y >= ymin && coord.y <= ymax) {

    int indexa = int(map(coord.x + 0, xmin, xmax, 0, 998));
    int indexb = int(map(coord.x + 1, xmin, xmax, 0, 998));

    float avg1 = 0;
    for (int i = indexa; i < indexb; i++) {
      avg1 += data[i];
    }
    avg1 /= indexb - indexa;

    float height1 = map(data[indexa], lowerLimit, upperLimit, ymin, ymax);
    float height2 = map(data[indexb], lowerLimit, upperLimit, ymin, ymax);

    float height = (height1 + height2) / 2;
    float width = max(abs(height1 - height2) / 2, 1);

    float line_col = peak(coord.y - height, width);
    float dot_col = peak(coord.y - height1, 1);
    float graduation = 0;
    if (indexb % 50 < indexa % 50)
      graduation = 0.1;
    // TODO: here gradPeriod / 100 is not ok to have 1px wide graduation
    if (mod(map(coord.y, ymin, ymax, lowerLimit, upperLimit), gradPeriod) < gradPeriod / 100)
      graduation = 0.1;

    color = vec4(line_col + graduation, line_col - dot_col + graduation, graduation, 1);
  } else {
    color = vec4(vec3(0.2), 1);

    if (coord.x >= 25 && coord.x < 50) {
      // TODO: same as above
      if (mod(map(coord.y, ymin, ymax, lowerLimit, upperLimit), gradPeriod) < gradPeriod / 100)
        color = vec4(0, 1, 0, 1);
    }
  }
}