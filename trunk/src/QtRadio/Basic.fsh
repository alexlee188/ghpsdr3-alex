uniform sampler2D spectrumTexture;
uniform float cy;

#define MAX_CL_WIDTH 2048
#define MAX_CL_HEIGHT 256

void main()
{

  float y_coord = gl_TexCoord[0].t + cy;
  if (y_coord > 1.0) y_coord -= 1.0;
  vec4 texel = texture2D(spectrumTexture, vec2(gl_TexCoord[0].s, y_coord));

  //vec4 texel = texture2D(spectrumTexture, gl_TexCoord[0].st);
  gl_FragColor = texel;
}
