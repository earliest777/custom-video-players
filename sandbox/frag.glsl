#version 400

in vec4 gl_FragCoord;
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D Y;
uniform sampler2D U;
uniform sampler2D V;

//This is mostly stolen from Wikipedia
//https://en.wikipedia.org/wiki/YUV#Y%E2%80%B2UV444_to_RGB888_conversion
void main(){
    vec2 flippedTextCord = vec2(texCoord.x, 1-texCoord.y);
    int y = int(texture(Y, flippedTextCord).x * 255);
    int u = int(texture(U, flippedTextCord).x * 255);
    int v = int(texture(V, flippedTextCord).x * 255);
    int c = y - 16;
    int d = u - 128;
    int e = v - 128;
    float r = clamp((298*c + 409*e + 128) >> 8, 0, 255) / 255.0;
    float g = clamp((298*c + 100*d - 208*e + 128) >> 8, 0, 255) / 255.0;
    float b = clamp((298*c + 516*d + 128) >> 8, 0, 255) / 255.0;
    FragColor = vec4(r, g, b, 1.0);
    //FragColor = vec4(u, u, u, 1.0);
}