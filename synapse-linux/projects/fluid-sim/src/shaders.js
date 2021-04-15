
const baseVertexShader = compileShader(gl.VERTEX_SHADER, `
precision highp float;

attribute vec2 aPosition;
varying vec2 vUv;
varying vec2 vL;
varying vec2 vR;
varying vec2 vT;
varying vec2 vB;
uniform vec2 texelSize;

void main () {
	vUv = aPosition * 0.5 + 0.5;
	vL = vUv - vec2(texelSize.x, 0.0);
	vR = vUv + vec2(texelSize.x, 0.0);
	vT = vUv + vec2(0.0, texelSize.y);
	vB = vUv - vec2(0.0, texelSize.y);
	gl_Position = vec4(aPosition, 0.0, 1.0);
}
`);

const copyShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;
precision mediump sampler2D;

varying highp vec2 vUv;
uniform sampler2D uTexture;

void main () {
	gl_FragColor = texture2D(uTexture, vUv);
}
`);

const clearShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;
precision mediump sampler2D;

varying highp vec2 vUv;
uniform sampler2D uTexture;
uniform float value;

void main () {
	gl_FragColor = value * texture2D(uTexture, vUv);
}
`);

const colorShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;

uniform vec4 color;

void main () {
	gl_FragColor = color;
}
`);

const displayShaderSource = `
precision highp float;
precision highp sampler2D;

varying vec2 vUv;
varying vec2 vL;
varying vec2 vR;
varying vec2 vT;
varying vec2 vB;
uniform sampler2D uTexture;
uniform sampler2D uBloom;
uniform sampler2D uSunrays;
uniform sampler2D uDithering;
uniform vec2 ditherScale;
uniform vec2 texelSize;

vec3 linearToGamma (vec3 color) {
	color = max(color, vec3(0));
	return max(1.055 * pow(color, vec3(0.416666667)) - 0.055, vec3(0));
}

void main () {
	vec3 c = texture2D(uTexture, vUv).rgb;

#ifdef SHADING
	vec3 lc = texture2D(uTexture, vL).rgb;
	vec3 rc = texture2D(uTexture, vR).rgb;
	vec3 tc = texture2D(uTexture, vT).rgb;
	vec3 bc = texture2D(uTexture, vB).rgb;

	float dx = length(rc) - length(lc);
	float dy = length(tc) - length(bc);

	vec3 n = normalize(vec3(dx, dy, length(texelSize)));
	vec3 l = vec3(0.0, 0.0, 1.0);

	float diffuse = clamp(dot(n, l) + 0.7, 0.7, 1.0);
	c *= diffuse;
#endif

#ifdef BLOOM
	vec3 bloom = texture2D(uBloom, vUv).rgb;
#endif

#ifdef SUNRAYS
	float sunrays = texture2D(uSunrays, vUv).r;
	c *= sunrays;
#ifdef BLOOM
	bloom *= sunrays;
#endif
#endif

#ifdef BLOOM
	float noise = texture2D(uDithering, vUv * ditherScale).r;
	noise = noise * 2.0 - 1.0;
	bloom += noise / 255.0;
	bloom = linearToGamma(bloom);
	c += bloom;
#endif

	float a = max(c.r, max(c.g, c.b));
	gl_FragColor = vec4(c, a);
}
`;
const splatShader = compileShader(gl.FRAGMENT_SHADER, `
precision highp float;
precision highp sampler2D;

varying vec2 vUv;
uniform sampler2D uTarget;
uniform float aspectRatio;
uniform vec3 color;
uniform vec2 point;
uniform float radius;

void main () {
	vec2 p = vUv - point.xy;
	p.x *= aspectRatio;
	vec3 splat = exp(-dot(p, p) / radius) * color;
	vec3 base = texture2D(uTarget, vUv).xyz;
	gl_FragColor = vec4(base + splat, 1.0);
}
`);

const advectionShader = compileShader(gl.FRAGMENT_SHADER, `
precision highp float;
precision highp sampler2D;

varying vec2 vUv;
uniform sampler2D uVelocity;
uniform sampler2D uSource;
uniform vec2 texelSize;
uniform vec2 dyeTexelSize;
uniform float dt;
uniform float dissipation;

vec4 bilerp (sampler2D sam, vec2 uv, vec2 tsize) {
	vec2 st = uv / tsize - 0.5;

	vec2 iuv = floor(st);
	vec2 fuv = fract(st);

	vec4 a = texture2D(sam, (iuv + vec2(0.5, 0.5)) * tsize);
	vec4 b = texture2D(sam, (iuv + vec2(1.5, 0.5)) * tsize);
	vec4 c = texture2D(sam, (iuv + vec2(0.5, 1.5)) * tsize);
	vec4 d = texture2D(sam, (iuv + vec2(1.5, 1.5)) * tsize);

	return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
}

void main () {
#ifdef MANUAL_FILTERING
	vec2 coord = vUv - dt * bilerp(uVelocity, vUv, texelSize).xy * texelSize;
	vec4 result = bilerp(uSource, coord, dyeTexelSize);
#else
	vec2 coord = vUv - dt * texture2D(uVelocity, vUv).xy * texelSize;
	vec4 result = texture2D(uSource, coord);
#endif
	float decay = 1.0 + dissipation * dt;
	gl_FragColor = result / decay;
}`,
ext.supportLinearFiltering ? null : ['MANUAL_FILTERING']
);

const divergenceShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;
precision mediump sampler2D;

varying highp vec2 vUv;
varying highp vec2 vL;
varying highp vec2 vR;
varying highp vec2 vT;
varying highp vec2 vB;
uniform sampler2D uVelocity;

void main () {
	float L = texture2D(uVelocity, vL).x;
	float R = texture2D(uVelocity, vR).x;
	float T = texture2D(uVelocity, vT).y;
	float B = texture2D(uVelocity, vB).y;

	vec2 C = texture2D(uVelocity, vUv).xy;
	if (vL.x < 0.0) { L = -C.x; }
	if (vR.x > 1.0) { R = -C.x; }
	if (vT.y > 1.0) { T = -C.y; }
	if (vB.y < 0.0) { B = -C.y; }

	float div = 0.5 * (R - L + T - B);
	gl_FragColor = vec4(div, 0.0, 0.0, 1.0);
}
`);

const pressureShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;
precision mediump sampler2D;

varying highp vec2 vUv;
varying highp vec2 vL;
varying highp vec2 vR;
varying highp vec2 vT;
varying highp vec2 vB;
uniform sampler2D uPressure;
uniform sampler2D uDivergence;

void main () {
	float L = texture2D(uPressure, vL).x;
	float R = texture2D(uPressure, vR).x;
	float T = texture2D(uPressure, vT).x;
	float B = texture2D(uPressure, vB).x;
	float C = texture2D(uPressure, vUv).x;
	float divergence = texture2D(uDivergence, vUv).x;
	float pressure = (L + R + B + T - divergence) * 0.25;
	gl_FragColor = vec4(pressure, 0.0, 0.0, 1.0);
}
`);

const gradientSubtractShader = compileShader(gl.FRAGMENT_SHADER, `
precision mediump float;
precision mediump sampler2D;

varying highp vec2 vUv;
varying highp vec2 vL;
varying highp vec2 vR;
varying highp vec2 vT;
varying highp vec2 vB;
uniform sampler2D uPressure;
uniform sampler2D uVelocity;

void main () {
	float L = texture2D(uPressure, vL).x;
	float R = texture2D(uPressure, vR).x;
	float T = texture2D(uPressure, vT).x;
	float B = texture2D(uPressure, vB).x;
	vec2 velocity = texture2D(uVelocity, vUv).xy;
	velocity.xy -= vec2(R - L, T - B);
	gl_FragColor = vec4(velocity, 0.0, 1.0);
}
`);
