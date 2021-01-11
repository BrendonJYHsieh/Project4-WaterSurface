#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D screenTexture;

#define HueLevCount 6
#define SatLevCount 7
#define ValLevCount 4
float[HueLevCount] HueLevels = float[] (0.0,80.0,160.0,240.0,320.0,360.0);
float[SatLevCount] SatLevels = float[] (0.0,0.15,0.3,0.45,0.6,0.8,1.0);
float[ValLevCount] ValLevels = float[] (0.0,0.3,0.6,1.0);

vec3 RGBtoHSV( float r, float g, float b) {
   float minv, maxv, delta;
   vec3 res;

   minv = min(min(r, g), b);
   maxv = max(max(r, g), b);
   res.z = maxv;            // v
   
   delta = maxv - minv;

   if( maxv != 0.0 )
      res.y = delta / maxv;      // s
   else {
      // r = g = b = 0      // s = 0, v is undefined
      res.y = 0.0;
      res.x = -1.0;
      return res;
   }

   if( r == maxv )
      res.x = ( g - b ) / delta;      // between yellow & magenta
   else if( g == maxv )
      res.x = 2.0 + ( b - r ) / delta;   // between cyan & yellow
   else
      res.x = 4.0 + ( r - g ) / delta;   // between magenta & cyan

   res.x = res.x * 60.0;            // degrees
   if( res.x < 0.0 )
      res.x = res.x + 360.0;
      
   return res;
}

vec3 HSVtoRGB(float h, float s, float v ) {
   int i;
   float f, p, q, t;
   vec3 res;

   if( s == 0.0 ) {
      // achromatic (grey)
      res.x = v;
      res.y = v;
      res.z = v;
      return res;
   }

   h /= 60.0;         // sector 0 to 5
   i = int(floor( h ));
   f = h - float(i);         // factorial part of h
   p = v * ( 1.0 - s );
   q = v * ( 1.0 - s * f );
   t = v * ( 1.0 - s * ( 1.0 - f ) );

   switch( i ) {
      case 0:
         res.x = v;
         res.y = t;
         res.z = p;
         break;
      case 1:
         res.x = q;
         res.y = v;
         res.z = p;
         break;
      case 2:
         res.x = p;
         res.y = v;
         res.z = t;
         break;
      case 3:
         res.x = p;
         res.y = q;
         res.z = v;
         break;
      case 4:
         res.x = t;
         res.y = p;
         res.z = v;
         break;
      default:      // case 5:
         res.x = v;
         res.y = p;
         res.z = q;
         break;
   }
   
   return res;
}

float nearestLevel(float col, int mode) {
   int levCount;
   if (mode==0) levCount = HueLevCount;
   if (mode==1) levCount = SatLevCount;
   if (mode==2) levCount = ValLevCount;
   
   for (int i =0; i<levCount-1; i++ ) {
     if (mode==0) {
        if (col >= HueLevels[i] && col <= HueLevels[i+1]) {
          return HueLevels[i+1];
        }
     }
     if (mode==1) {
        if (col >= SatLevels[i] && col <= SatLevels[i+1]) {
          return SatLevels[i+1];
        }
     }
     if (mode==2) {
        if (col >= ValLevels[i] && col <= ValLevels[i+1]) {
          return ValLevels[i+1];
        }
     }
   }
}

// averaged pixel intensity from 3 color channels
float avg_intensity(vec4 pix) {
 return (pix.r + pix.g + pix.b)/3.;
}

vec4 get_pixel(vec2 coords, float dx, float dy) {
 return texture2D(screenTexture,coords + vec2(dx, dy));
}

// returns pixel color
float IsEdge(in vec2 coords){
  float dxtex = 1.0 /float(textureSize(screenTexture,0)) ;
  float dytex = 1.0 /float(textureSize(screenTexture,0));
  float pix[9];
  int k = -1;
  float delta;

  // read neighboring pixel intensities
  for (int i=-1; i<2; i++) {
   for(int j=-1; j<2; j++) {
    k++;
    pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,
                                          float(j)*dytex));
   }
  }

  // average color differences around neighboring pixels
  delta = (abs(pix[1]-pix[7])+
          abs(pix[5]-pix[3]) +
          abs(pix[0]-pix[8])+
          abs(pix[2]-pix[6])
           )/4.;

  return clamp(5.5*delta,0.0,1.0);
}



uniform float vx_offset=0.55;
uniform float rt_w; 
uniform float rt_h; 
uniform float t;
uniform float pixel_w = 5; // 15.0
uniform float pixel_h = 6; 
uniform bool pixel_enable;
uniform bool offset_enable;
uniform bool other_enable;

void main()
{
        if(pixel_enable){
            vec2 uv = TexCoords.xy;
            float dx = pixel_w*(1.0/rt_w);
            float dy = pixel_h*(1.0/rt_h);
            vec2 coord = vec2(dx*floor(uv.x/dx),dy*floor(uv.y/dy));
            vec3 tc = texture2D(screenTexture, coord).rgb;
            FragColor = vec4(tc, 1.0);
        }
        else if(offset_enable){
            vec4 colorOrg = texture2D( screenTexture, TexCoords );
            vec3 vHSV =  RGBtoHSV(colorOrg.r,colorOrg.g,colorOrg.b);
            vHSV.x = nearestLevel(vHSV.x, 0);
            vHSV.y = nearestLevel(vHSV.y, 1);
            vHSV.z = nearestLevel(vHSV.z, 2);
            float edg = IsEdge(TexCoords);
            vec3 vRGB = (edg >= 0.3)? vec3(0.0,0.0,0.0):HSVtoRGB(vHSV.x,vHSV.y,vHSV.z);
            FragColor = vec4(vRGB , 1.0);
        }
        else if(other_enable){
                vec2 resolution = {590,590};
                const vec4 luminance_vector = vec4(0.3, 0.59, 0.11, 0.0);
                vec2 uv = (gl_FragCoord.xy / resolution)-vec2(1.0);
	            vec2 n = 1.0/resolution.xy;
	            vec4 CC = texture2D(screenTexture, uv);
	            vec4 RD = texture2D(screenTexture, uv + vec2( n.x, -n.y));
	            vec4 RC = texture2D(screenTexture, uv + vec2( n.x,  0.0));
	            vec4 RU = texture2D(screenTexture, uv + n);
	            vec4 LD = texture2D(screenTexture, uv - n);
	            vec4 LC = texture2D(screenTexture, uv - vec2( n.x,  0.0));
	            vec4 LU = texture2D(screenTexture, uv - vec2( n.x, -n.y));
	            vec4 CD = texture2D(screenTexture, uv - vec2( 0.0,  n.y));
	            vec4 CU = texture2D(screenTexture, uv + vec2( 0.0,  n.y));

	            FragColor = vec4(2.0*abs(length(
		            vec2(
			            -abs(dot(luminance_vector, RD - LD))
			            +4.0*abs(dot(luminance_vector, RC - LC))
			            -abs(dot(luminance_vector, RU - LU)),
			            -abs(dot(luminance_vector, LD - LU))
			            +4.0*abs(dot(luminance_vector, CD - CU))
			            -abs(dot(luminance_vector, RD - RU))
		            )
	            )-0.5));
        }
        else{
            vec3 col = texture(screenTexture, TexCoords).rgb;
            FragColor = vec4(col, 1.0);
        }
}   

//Reference http://coding-experiments.blogspot.com/2011/01/toon-pixel-shader.html , https://gist.github.com/jcayzac/1192583