uniform sampler2D texture;
uniform float pixelSize;
uniform vec2 resolution;

void main(){

	vec2 uv = gl_TexCoord[0].xy;

	vec2 screenPos = uv * resolution;

	screenPos = floor(screenPos / pixelSize) * pixelSize; // Lock and map back to screen coordinates

	uv = screenPos / resolution;

	vec4 color = texture2D(texture, uv);

	gl_FragColor = color;
}