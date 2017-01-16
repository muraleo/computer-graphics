uniform float uVertexScale;
uniform float uTime;
uniform float uAspRad;

attribute vec2 aPosition;
attribute vec3 aColor;
attribute vec2 aTexCoord0, aTexCoord1;

varying vec3 vColor;
varying vec2 vTexCoord0, vTexCoord1;

void main() {
  if(  uAspRad==1.0 )       //change
  {
        gl_Position = vec4(aPosition.x * uVertexScale, aPosition.y, 0,1);
  }
  else 
  {
        if(  uAspRad > 1.0 )
        {
            gl_Position = vec4(aPosition.x * uVertexScale / uAspRad, aPosition.y, 0,1);
        }
        else
        {
            gl_Position = vec4(aPosition.x * uVertexScale, aPosition.y * uAspRad, 0,1);
        }
  }

  vColor = aColor;
  vTexCoord0 = aTexCoord0;
  vTexCoord1 = aTexCoord1;
}
