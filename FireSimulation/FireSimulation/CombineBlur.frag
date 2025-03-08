uniform sampler2D originalScene;
uniform sampler2D blurredBloom;

void main() {

    vec4 sceneColor = texture2D(originalScene, gl_TexCoord[0].xy);
    vec4 bloomColor = texture2D(blurredBloom, gl_TexCoord[0].xy);
    
    float bloomStrength = 1.15; // Increase bloom intensity
    gl_FragColor = sceneColor + bloomColor * bloomStrength;
}