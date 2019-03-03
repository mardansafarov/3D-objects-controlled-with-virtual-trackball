varying vec3 L;
varying vec3 V;
uniform sampler2D tex;
uniform sampler2D texNorm;
varying vec2 st;
uniform vec4 DiffuseProduct;
varying  vec4 color;



void main()
{
    
    vec4 N = texture2D(tex, st);
    vec4 NN =  normalize(2.0*N-1.0);
    vec4 M = texture2D(texNorm, st);
    vec4 MM = normalize(2.0*M-1.0);
    
    vec3 LL = normalize(L);
    
    vec4 obshi = mix(MM, NN, 0.5);
    float Kd = max(dot(obshi.xyz, LL), 0.0);
    
    
    
    gl_FragColor = Kd*DiffuseProduct; 
}
