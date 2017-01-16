uniform float uVertexScale;
uniform float uTime;
uniform sampler2D uTexUnit0, uTexUnit1;

varying vec2 vTexCoord0, vTexCoord1;
varying vec3 vColor;

void main(void) {
  vec4 color = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor0 = texture2D(uTexUnit0, vTexCoord0);
  vec4 texColor1 = texture2D(uTexUnit1, vTexCoord1);

  float lerper = clamp(.5 *uVertexScale, 0., 1.);    // clamp return min( max(x, mivVal), maxVal)
  float lerper2 = clamp(.5 * uVertexScale + 1.0, 0.0, 1.0);
  
  // ======================================
  // TODO: use sin and uTime to interpolate
  //       between the two images
  // ======================================


  //gl_FragColor = sin(uTime/10000.0) * texColor0 + ( 1.0 - sin( uTime/10000.0 ) ) * texColor1;
  //gl_FragColor = (-1.0) * sin(uTime/10000.0) * texColor0;  // for test
  if(sin(uTime/1000.0)>=0.0)       //change
  {
      gl_FragColor =sin(uTime/1000.0) * texColor0;
  }
  else
  {
      gl_FragColor =(-1.0) * sin(uTime/1000.0) * texColor1; 
  }
}
