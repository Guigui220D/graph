#version 130

uniform float upperLimit;
uniform float lowerLimit;
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
      float nextpx = coord.x + 1;
      int index = int(map(coord.x, 50, 750, 0, 997));
      int index_nextpx = int(map(coord.x + 1, 50, 750, 0, 997));
      int index_nextnextpx = int(map(coord.x + 2, 50, 750, 0, 997));
      float avg = 0;
      for (int i = index; i < index_nextpx; i++) {
        avg += data[i];
      }
      avg /= index_nextpx - index;
      float avg2 = 0;
      for (int i = index_nextpx; i < index_nextnextpx; i++) {
        avg2 += data[i];
      }
      avg2 /= index_nextnextpx - index_nextpx;

      float heighta = map(avg, lowerLimit, upperLimit, 0, 600);
      float heightb = map(avg2, lowerLimit, upperLimit, 0, 600);

      float height = (heighta + heightb) / 2;
      float width = max(abs(heighta - heightb) / 2, 1);

      float i = peak(map(coord.y, 0, 600, -50, 650) - height, width);

      color = vec4(i, i, 0.0 / 800, i);
    } else {
      color = vec4(vec3(0.2), 1.0);
    }
}