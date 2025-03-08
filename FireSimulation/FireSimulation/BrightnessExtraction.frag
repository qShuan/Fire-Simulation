uniform sampler2D texture;

void main() {

    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture2D(texture, uv);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); // Luminance
    
    float threshold = 0.01;
    
    if (brightness > threshold)
        gl_FragColor = color;
    else
        gl_FragColor = vec4(0.0);
}