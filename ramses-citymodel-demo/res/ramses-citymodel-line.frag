#version 100

precision highp float;

varying vec3 v_distance;

void main(void)
{
    float dy = abs(v_distance.y);
    float dy2 = max(dy - v_distance.z, 0.0);

    vec2 d = vec2(v_distance.x, dy2);
    float a2 =clamp(1.0 - length(d), 0.0, 1.0);
    float a3 = 1.0 - a2 * a2;

    gl_FragColor = vec4(0.0, 0.0, 0.0, a3);
}
