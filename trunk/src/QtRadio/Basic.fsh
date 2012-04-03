uniform sampler2D spectrumTexture;
uniform int cy;

#define MAX_CL_WIDTH 2048
#define MAX_CL_HEIGHT 256

void main()
{
  int level = gl_TexCoord[0].t * (float)MAX_CL_HEIGHT + cy;
  if (level >= MAX_CL_HEIGHT) level -= MAX_CL_HEIGHT;
  float y_coord = (float)level/ (float)MAX_CL_HEIGHT;
  vec4 texel = texture2D(spectrumTexture, vec2(gl_TexCoord[0].s, y_coord);
  gl_FragColor = texel;
}
