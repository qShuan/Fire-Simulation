uniform sampler2D texture;
uniform float resolution;

void main() {

    vec2 texOffset = vec2(1.0 / resolution, 0.0);
    vec4 sum = vec4(0.0);

    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 6.0) * 0.02;
    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 5.0) * 0.04;
    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 4.0) * 0.08;
    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 3.0) * 0.12;
    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 2.0) * 0.15;
    sum += texture2D(texture, gl_TexCoord[0].xy - texOffset * 1.0) * 0.18;
    sum += texture2D(texture, gl_TexCoord[0].xy) * 0.20; // Center pixel
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 1.0) * 0.18;
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 2.0) * 0.15;
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 3.0) * 0.12;
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 4.0) * 0.08;
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 5.0) * 0.04;
    sum += texture2D(texture, gl_TexCoord[0].xy + texOffset * 6.0) * 0.02;

    gl_FragColor = sum * 3.0;
}