//Bump Mapping
#include "Angel.h"
#include "math.h"
#include "mat.h"
#include "TextureLoader.h"
#define N 512
using namespace std;

typedef Angel::vec4  point4;
typedef Angel::vec4  color4;
//------------------------------------------------------------------------------------------------------
// Projection transformation parameters
GLfloat  left = -1.0, right = 1.0;
GLfloat  bottom = -1.0, top = 1.0;
GLfloat  zNear = 0.5, zFar = 3.0;
//------------------------------—-----------------------------------------------------------------------
//variables used in  trackball function
float rotX, rotY, rotZ;
float difX, difY;
float X, Y, Z, dX, dY, dZ;
//--------------------------------------------------------------------------------------------------------
//boolean for projections
bool prF = false;
bool prO = false;
//------------------------------------------------------------------------------------------------------------------------------
int Offset = 0;
int num = 36;
point4 points[6];
vec2 tex_coord[6];
mat4 prLook, projection;
GLuint Projection;
//common draw type used for objects
GLenum DrawType = GL_TRIANGLES; 
//---------------------------------------------------------------------------------------------
vec4 colors[8] = {
    vec4( 0.0, 0.0, 0.0, 1.0 ),  
    vec4( 1.0, 0.0, 0.0, 1.0 ),  
    vec4( 1.0, 1.0, 0.0, 1.0 ),  
    vec4( 0.0, 1.0, 0.0, 1.0 ),  
    vec4( 0.0, 0.0, 1.0, 1.0 ),  
    vec4( 1.0, 0.0, 1.0, 1.0 ),  
    vec4( 1.0, 1.0, 1.0, 1.0 ),  
    vec4( 1.0, 1.0, 1.0, 1.0 )   
};
//---------------------------------------------------------------------------------
//shows appropriate offsets for buffer
struct Figure{
    int pointsOffset; 
    int textureOffset; 
    int numberOfPoints;
    int colorOffset;
    GLenum drawtype; 
};
//------------------------------------------------------------------------------------------------------
Figure Pyramid, Sphere, Cube;

vec4 pyramid_data[5] = {
    vec4( -1.0, -1.0,  1.0, 1.0 ),
    vec4(  1.0, -1.0,  1.0, 1.0 ),
    vec4(  1.0, -1.0, -1.0, 1.0 ),
    vec4( -1.0, -1.0, -1.0, 1.0 ),
    vec4(  0.0,  1.0,  0.0, 1.0 ),
};

vec4 cube_data[8] = {
    vec4( -1.0, -1.0,  1.0, 1.0 ),
    vec4( -1.0,  1.0,  1.0, 1.0 ),
    vec4(  1.0,  1.0,  1.0, 1.0 ),
    vec4(  1.0, -1.0,  1.0, 1.0 ),
    vec4( -1.0, -1.0, -1.0, 1.0 ),
    vec4( -1.0,  1.0, -1.0, 1.0 ),
    vec4(  1.0,  1.0, -1.0, 1.0 ),
    vec4(  1.0, -1.0, -1.0, 1.0 )
};
//arrays for normals 
vec4 cubelighting[36], pyramidlighting[18], spherelighting[23450]; 
vec4 cubePoints[36], pyramidpoints[18], spherepoints[23450];
vec2 cubetexcoord[36], pyramidtexcoord[18], spheretexcoord[23450]; 
vec4 cubecolors[36], pyramidcolors[18], spherecolors[23450];

//generating pyramid
int index = 0;
// Make a triagle surface by vertex reference
void triangle(int a, int b, int c) {
    vec4 u = pyramid_data[b] - pyramid_data[a];
    vec4 v = pyramid_data[c] - pyramid_data[b];
    vec4 normal = normalize( cross(u, v) );
    pyramidlighting[index] = normal; pyramidpoints[index] = pyramid_data[a];pyramidtexcoord[index] = vec2(pyramid_data[a][0], pyramid_data[a][2]); index++;
    pyramidlighting[index] = normal; pyramidpoints[index] = pyramid_data[b];pyramidtexcoord[index] = vec2(pyramid_data[b][0], pyramid_data[b][2]); index++;
    pyramidlighting[index] = normal; pyramidpoints[index] = pyramid_data[c];pyramidtexcoord[index] = vec2(pyramid_data[c][0], pyramid_datar[c][2]); index++;
};
void pyramid() {
    triangle(0, 1, 4); 
    triangle(1, 2, 4); 
    triangle(2, 3, 4); 
    triangle(3, 0, 4); 
    triangle(0, 2, 1); 
    triangle(0, 3, 2); 

    numPyr = 18;
    Pyramid.drawtype = GL_TRIANGLES;
};

//generating cube
int i = 0;
void createCube(int a, int b, int c, int d){
    vec4 u = cube_data[b] - cube_data[a];
    vec4 v = cube_data[c] - cube_data[b];
    vec4 normal = normalize( cross(u, v) );
    cubelighting[i] = normal; cubePoints[i] = cube_data[a];cubetexcoord[i] = vec2(cube_data[a][0], cube_data[a][2]); i++;
    cubelighting[i] = normal; cubePoints[i] = cube_data[b];cubetexcoord[i] = vec2(cube_data[b][0], cube_data[b][2]); i++;
    cubelighting[i] = normal; cubePoints[i] = cube_data[c];cubetexcoord[i] = vec2(cube_data[c][0], cube_data[c][2]); i++;
    cubelighting[i] = normal; cubePoints[i] = cube_data[a];cubetexcoord[i] = vec2(cube_data[a][0], cube_data[a][2]); i++;
    cubelighting[i] = normal; cubePoints[i] = cube_data[c];cubetexcoord[i] = vec2(cube_data[c][0], cube_data[c][2]); i++;
    cubelighting[i] = normal; cubePoints[i] = cube_data[d];cubetexcoord[i] = vec2(cube_data[d][0], cube_data[d][2]); i++;
}
void cube()
{
    createCube( 1, 0, 3, 2 );
    createCube( 2, 3, 7, 6 );
    createCube( 3, 0, 4, 7 );
    createCube( 6, 5, 1, 2 );
    createCube( 4, 5, 6, 7 );
    createCube( 5, 4, 0, 1 );
    
    numCube = 36;
}


//generating sphere
void sphere(){
    const float DegreesToRadians = M_PI / 180.0; // M_PI = 3.14159...
    int k = 0;
    for(float phi = -180.0; phi <= 180.0; phi += 5.0)
    {
        float phir   = phi*DegreesToRadians;
        float phir20 = (phi + 5.0)*DegreesToRadians;
        for(float theta = -180.0; theta <= 180.0; theta += 5.0)
        {
            float thetar = theta*DegreesToRadians;
            spherepoints[k] = vec4(sin(thetar)*cos(phir),
                                   cos(thetar)*cos(phir), sin(phir), 1);
            //spheretexcoord[k] = vec2(sin(thetar)*cos(phir), sin(phir));
            k++;
            spherepoints[k] = vec4(sin(thetar)*cos(phir20),cos(thetar)*cos(phir20), sin(phir20), 1);
            //spheretexcoord[k] = vec2(sin(thetar)*cos(phir20), sin(phir20));
            k++;
        }
    }
    for (int i = 0; i < 23450; i++) {
        spherelighting[i] = spherepoints[i];//(cross(-spherepoints[i+2]+spherepoints[i], spherepoints[i+2]-spherepoints[i+1]));
        spheretexcoord[i] = vec2(spherepoints[i].x, spherepoints[i].z);
    }
    
    numSphere = 23450;
    
}
//--------------------------------------------------------------------------------------------------------------
GLuint programm;
int numLightSource = 0; //shows number of light sources
//arrays for each light
vec4 PositionsOfLights[5];
vec4 AmbientOfLights[5];
vec4 DiffuseOfLights[5];
vec4 SpecularOfLights[5];
float Shininess[5];
//------------------------------------------------------------------------------------------------------------------------------
float randomm(){
    return -1 + 6*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));  // returns random values between -1 and 5
}
//---------------------------------------------------------------------------------—------------------------------------------
typedef vec4 color4;
typedef vec4 point4;
//-------------------------------------------------------------------------------------------------------------
void addLighting() {  //adds new light source with random position
    
    point4 light_position( randomm(), randomm(), randomm(), 0.0 );//generate random position
    
    // Initialize shader lighting parameters
 vec4 light_position (randomm(), randomm(), randomm(), 1.0 );  //random light position between -1 and 5
    color4 light_ambient( 0.2, 0.5, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    color4 material_ambient( 1.0, 1.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.5, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.8, 1.0 );
    float  material_shininess = 200.0;
    
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    

    ambientLight[numLightSource] = ambient_product;
    diffuseLight[numLightSource] = diffuse_product;
    specularLight[numLightSource] = specular_product;
    shininess[numLightSource] = material_shininess;
    lightposition[numLightSource] = light_position;
    //number of lights increased
     numLightSource++;
    
    //send arrays to shader
 glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
                 numLightSource, *ambientLight );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
                 numLightSource, *diffuseLight );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
                 numLightSource, *specularLight );

    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                 numLightSource, *lightposition );

    glUniform1fv( glGetUniformLocation(program, "Shininess"),
                numLightSource, shininess );
    
    glUniform1i( glGetUniformLocation(programm, "numOfLights"),
                numLightSource );
}


void deleteLighting() {  //deletes added light source
    
    currentNumberOfLights--; 
    glUniform1i( glGetUniformLocation(program, "numOfLights"),
                numLightSource );
}
//-------------------------------------------------------------------------------------------

GLfloat normalsPyr[N][N][3];
GLfloat normalsCube[N][N][3];
GLfloat normalsSphere[N][N][3];
GLuint         program;
GLuint        texMapLocation;
GLfloat tangent[3] = {1.0, 0.0, 0.0};

//-----------------------------------------------------------------------------------------------------

float dataPyr[N+1][N+1];
float dataSphere[N+1][N+1];
float dataCube[N+1][N+1];
const int  TextureSize  = 512;
GLuint textures[10];
GLubyte mapPyramid[TextureSize][TextureSize][3];
GLubyte mapCube[TextureSize][TextureSize][3];
GLubyte mapSphere[TextureSize][TextureSize][3];

vec4 normal = point4(0.0, 1.0, 0.0, 0.0);
color4 light_diffuse = color4(1.0, 1.0, 1.0, 1.0);
color4 material_diffuse = color4( 0.91, 0.76, 0.65, 1.0);
point4  light_position = point4(0.0, 7.0, 0.0, 1.0);
vec4 eye =  vec4(2.0, 2.0, 2.0, 1.0);
vec4 at = vec4(0.5, 0.0, 0.5, 1.0);
vec4 up = vec4(0.0, 1.0, 0.0, 1.0);



GLuint loc, loc2, loc3;
GLuint buffers[2];

GLuint normal_loc;
GLuint diffuse_product_loc;
GLuint light_position_loc;
GLuint ctm_loc, projection_loc;
GLuint tangent_loc;


/* standard OpenGL initialization */

vec4 product(vec4 a, vec4 b)
{
    return vec4(a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]);
}
static void init()
{
    
    pyramid();
    cube();
    sphere();
    
    int q = 0;
    
    std::string path = "pyramid.bmp";  //take pyramid map
    unsigned int hh = 512, ww = 512;
    unsigned char *coordObj;
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    
    
    
    q = 0;
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            mapPyramid[i][j][0]  = (coordObj[q]); q++;
            mapPyramid[i][j][1]  = (coordObj[q]); q++;
            mapPyramid[i][j][2]  = (coordObj[q]); q++;
            
        }
    }
    
    path = "sphere.bmp"; //take sphere map
    hh = 512, ww = 512;
    
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    
    
    
    q = 0;
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            mapSphere[i][j][0]  = (coordObj[q]); q++;
            mapSphere[i][j][1]  = (coordObj[q]); q++;
            mapSphere[i][j][2]  = (coordObj[q]); q++;
            pyramidcolors[i] = vec4(mapSphere[i][j][0], mapSphere[i][j][1], mapSphere[i][j][2], 1.0f)/10.0;
            spherecolors[i/20] = vec4(mapSphere[i][j][0], mapSphere[i][j][1], mapSphere[i][j][2], 1.0f)/10.0;
            cubecolors[i/10] = vec4(mapSphere[i][j][0], mapSphere[i][j][1], mapSphere[i][j][2], 1.0f)/10.0;
        }
    }
    path = "pyramid.bmp"; //should be cube but cube is not working
    hh = 512, ww = 512;
    
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    
    
    
    q = 0;
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            mapCube[i][j][0]  = (coordObj[q]); q++;
            mapCube[i][j][1]  = (coordObj[q]); q++;
            mapCube[i][j][2]  = (coordObj[q]); q++;
        }
    }

//--------------------------------------------------------------------------------------------
    
    //send to buffer
    
    glGenTextures( 6, textures );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, mapPyramid );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    
    // Create a vertex array object
    
    const float endColor[]     = {0.7, 0.7, 0.7, 1.0};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, endColor);
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGB, GL_FLOAT, normalsPyr);
    glEnable(GL_TEXTURE_2D);
    
    glUniform1i(glGetUniformLocation(program, "texNorm"), 1);
    
    
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, mapCube );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGB, GL_FLOAT, normalsCube);
    glEnable(GL_TEXTURE_2D);
    
    
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, textures[4] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, mapSphere );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textures[5]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGB, GL_FLOAT, normalsSphere);
    glEnable(GL_TEXTURE_2D);
    
    glEnable(GL_DEPTH_TEST);
//-------------------------------------------------------------------------------------------------
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    
    //load data for every objects to buffer 
    glBufferData( GL_ARRAY_BUFFER, sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(pyramidpoints) + sizeof(pyramidtexcoord) + sizeof(cubecolors) + sizeof(spherecolors) + sizeof(pyramidcolors),
                 NULL, GL_STATIC_DRAW );
    
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cubePoints), cubePoints);
    Cube.pointsOffset = 0;
    glBufferSubData( GL_ARRAY_BUFFER,sizeof(cubePoints),
                    sizeof(cubetexcoord), cubetexcoord );
    Cube.textureOffset = sizeof(cubePoints);
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cubePoints) + sizeof(cubetexcoord),
                    sizeof(cubecolors), cubecolors );
    Cube.colorOffset = sizeof(cubePoints) + sizeof(cubetexcoord);

    glBufferSubData( GL_ARRAY_BUFFER,sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors),
                    sizeof(spherepoints), spherepoints );
    Sphere.pointsOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors);
    
    glBufferSubData( GL_ARRAY_BUFFER,sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints),
                    sizeof(spheretexcoord), spheretexcoord );
    Sphere.textureOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints);
    
    glBufferSubData( GL_ARRAY_BUFFER,sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord),
                    sizeof(spherecolors), spherecolors );
    Sphere.colorOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord);
    
    glBufferSubData( GL_ARRAY_BUFFER,sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors),
                    sizeof(pyramidpoints), pyramidpoints );
    Pyramid.pointsOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors);
    
    
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors) + sizeof(pyramidpoints), sizeof(pyramidtexcoord), pyramidtexcoord);
    Pyramid.textureOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors) + sizeof(pyramidpoints);
    
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors) + sizeof(pyramidpoints) + sizeof(pyramidtexcoord), sizeof(pyramidcolors), pyramidcolors);
    Pyramid.colorOffset = sizeof(cubePoints) + sizeof(cubetexcoord) + sizeof(cubecolors) + sizeof(spherepoints) + sizeof(spheretexcoord) + sizeof(spherecolors) + sizeof(pyramidpoints) + sizeof(pyramidtexcoord);
    // set up vertex arrays
    
    loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    
    
    loc2 = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( loc2 );
    // Initialize shader lighting parameters
    glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    glVertexAttribPointer( loc2, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(cubePoints)));
    
    loc3 = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( loc3 );
    glVertexAttribPointer( loc3, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof cubePoints + sizeof cubetexcoord) );
                          
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader21.glsl", "fshader21.glsl" );
 	glUseProgram( program );
    
    // Retrieve transformation uniform variable locations
    
    tangent_loc = glGetUniformLocation(program, "objTangent");
    glUniform3fv(tangent_loc, 1, tangent);
    
    normal_loc = glGetUniformLocation(program, "Normal");
    glUniform4fv(normal_loc, 1, normal);
    
    vec4 diffuse_product = product(light_diffuse, material_diffuse);
    diffuse_product_loc = glGetUniformLocation(program, "DiffuseProduct");
    glUniform4fv(diffuse_product_loc, 1, diffuse_product);
    
    light_position_loc = glGetUniformLocation(program, "LightPosition");
    glUniform4fv(light_position_loc, 1, light_position);
    
    ctm_loc = glGetUniformLocation(program, "ModelView");
    
    prLook = LookAt(eye, at , up);
    glUniformMatrix4fv(ctm_loc, 1, GL_TRUE, prLook);
    
    mat4 nm;
    GLfloat tR;
    tR = prLook[0][0]*prLook[1][1]*prLook[2][2]+prLook[0][1]*prLook[1][2]*prLook[2][1]
    -prLook[2][0]*prLook[1][1]*prLook[0][2]-prLook[1][0]*prLook[0][1]*prLook[2][2]-prLook[0][0]*prLook[1][2]*prLook[2][1];
    nm[0][0] = (prLook[1][1]*prLook[2][2]-prLook[1][2]*prLook[2][1])/tR;
    nm[0][1] = -(prLook[0][1]*prLook[2][2]-prLook[0][2]*prLook[2][1])/tR;
    nm[0][2] = (prLook[0][1]*prLook[2][0]-prLook[2][1]*prLook[2][2])/tR;
    nm[1][0] = -(prLook[0][1]*prLook[2][2]-prLook[0][2]*prLook[2][1])/tR;
    nm[1][1] = (prLook[0][0]*prLook[2][2]-prLook[0][2]*prLook[2][0])/tR;
    nm[1][2] = -(prLook[0][0]*prLook[2][1]-prLook[2][0]*prLook[0][1])/tR;
    nm[2][0] = (prLook[0][1]*prLook[1][2]-prLook[1][1]*prLook[0][2])/tR;
    nm[2][1] = -(prLook[0][0]*prLook[1][2]-prLook[0][2]*prLook[1][0])/tR;
    nm[2][2] = (prLook[0][0]*prLook[1][1]-prLook[1][0]*prLook[0][1])/tR;
    
    GLuint nm_loc;
    nm_loc = glGetUniformLocation(program, "NormalMatrix");
    
    glUniformMatrix4fv(nm_loc, 1, GL_TRUE, nm);
    
    projection_loc = glGetUniformLocation(program, "Projection");
    projection = Ortho(-0.75,0.75,-0.75,0.75,-5.5,5.5) * Scale(0.4, 0.4, 0.4);
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);
    
    texMapLocation = glGetUniformLocation(program, "texMap");
    
}

/* set up uniform parameter */


static void display()
{
    
    glUniform1i(texMapLocation, 0);
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glDrawArrays(DrawType, 0, num);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, numPyr);
    
    glutSwapBuffers();
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

void changeOffset(int pointOffset, int textureOffset, int colorOffset){
    glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(pointOffset) );
    glVertexAttribPointer( loc2, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(textureOffset));
    glVertexAttribPointer( loc3, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(colorOffset) );
}

//-----------------------------------------------------------------------------------------------------
//KEYBOARD
//do following command when some button pressed
static void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'C':
            glUniform1i(glGetUniformLocation(program, "tex"), 2);
            glUniform1i(glGetUniformLocation(program, "texNorm"), 3);
            changeOffset(Cube.pointsOffset, Cube.textureOffset, Cube.colorOffset);
            num = numCube;
            DrawType = GL_TRIANGLES;
            break;
        case 'P':
            glUniform1i(glGetUniformLocation(program, "tex"), 0);
            glUniform1i(glGetUniformLocation(program, "texNorm"), 1);
            changeOffset(Pyramid.pointsOffset, Pyramid.textureOffset, Pyramid.colorOffset);
            num = numPyr;
            DrawType = GL_TRIANGLES;
            break;
        case 'S':
            glUniform1i(glGetUniformLocation(program, "tex"), 4);
            glUniform1i(glGetUniformLocation(program, "texNorm"), 5);
            changeOffset(Sphere.pointsOffset, Sphere.textureOffset, Sphere.colorOffset);
            num = numSphere;
            DrawType = GL_TRIANGLE_STRIP;
            break;
        case 'O':
            projection = Ortho(-1.0, 1.0, -1.0, 1.0, -2, 2);
            glUniformMatrix4fv(Projection, 1, GL_TRUE, projection); //send to shader
            break;
        case 'F':
            projection = Perspective( 45.0, 1, 0.5, 3.0 )*Translate(0, 0, -1);
            glUniformMatrix4fv(Projection, 1, GL_TRUE, projection); //send to shader
            break;
        case 'L':
            addLighting();
            break;
        case 'l':
            deleteLighting();
        case 'Q':
            exit(EXIT_SUCCESS);
            break;
    }
}
//---------------------------------------------------------------------------------------------------------------
//idle callback
void idle()
{
    int t;
    t = glutGet(GLUT_ELAPSED_TIME);
    light_position[0] = 5.5*sin(0.001*t);
    light_position[2] = 5.5*cos(0.001*t);
    glUniform4fv(light_position_loc, 1, light_position);
    
    glutPostRedisplay();
}
//-------------------------------------------------------------------------------------------------------------------------
    //TRACKBALL

    void trackball(int x, int y)
    {
            X = 2.0*x/w -1;        //normalization of first and last X and Y points
            Y = 2.0*(h-y)/h - 1;
            dX = 2.0*difX/w -1;
            dY = 2.0*(h-difY)/h - 1;

        rotY = X - dX;  // difference for X and Y between first and last points (with their normalized versions and shown by rotX and rotY)
        rotX = Y - dY;

        float Dis = sqrt(X*X + Y*Y);
        float dis = sqrt(dX*dX + dY*dY);

        if(Dis < 1)
                Z = sqrt(1 - Dis*Dis);  //this gives last Z coordinate and if Z takes value out of radius assigns it to 0
            else Z = 0;
            if(dis < 1)
                dZ = sqrt(1 - dis*dis); //this gives last Z coordinate and if Z takes value out of radius assigns it to 0
            else dZ = 0;

        rotZ = Z - dZ;  //difference between first and last Z coordinates

        angleX += rotX/sqrt(rotX*rotX + rotY*rotY + rotZ*rotZ); //by adding rotX, rotY, and rotZ (normalized) to angleX,Y,Z we generate object
        angleY += rotY/sqrt(rotX*rotX + rotY*rotY + rotZ*rotZ); // to stay in the point where it is rotated in the next rotation
        angleZ += rotZ/sqrt(rotX*rotX + rotY*rotY + rotZ*rotZ); //for preventing objects to return to their first positions

        difX = x;
        difY = y;

            glutPostRedisplay();
    }

//----------------------------------------------------------------------------------------------------------------


int main(int argc, char** argv)
{
    
    
    int i,j, k;
    float x;
    
    std::string path;
    unsigned int hh = 512, ww = 512;
    unsigned char *coordObj;
    path = "pyramid_normal_map.bmp";
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    int q = 0;
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            dataPyr[i][j]  = (coordObj[q]); q++;
            dataPyr[i][j]  += (coordObj[q]); q++;
            dataPyr[i][j]  += (coordObj[q]); q++;
            dataPyr[i][j] /= 1.0*(256*3);
            if(dataPyr[i][j]>0.5) dataPyr[i][j] = 1.0;
            else dataPyr[i][j] = 0;
        }
    }
    
    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        normalsPyr[i][j][0] = dataPyr[i][j]-dataPyr[i+1][j];
        normalsPyr[i][j][2] = dataPyr[i][j]-dataPyr[i][j+1];
        normalsPyr[i][j][1]= 1.0;
    }
    
    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        x = 0.0;
        for(k=0;k<3;k++) d+=normalsPyr[i][j][k]*normalsPyr[i][j][k];
        x=sqrt(x);
        for(k=0;k<3;k++) normalsPyr[i][j][k]= 0.5*normalsPyr[i][j][k]/x+0.5;
    }
    
    path = "sphere_normal_map.bmp";
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    q = 0;
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            dataSphere[i][j]  = (coordObj[q]); q++;
            dataSphere[i][j]  += (coordObj[q]); q++;
            dataSphere[i][j]  += (coordObj[q]); q++;
            dataSphere[i][j] /= 1.0*(256*3);
            if(dataSphere[i][j]>0.5) dataSphere[i][j] = 1.0;
            else dataSphere[i][j] = 0;
            
        }
    }
  for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        normalsSphere[i][j][0] = dataSphere[i][j]-dataSphere[i+1][j];
        normalsSphere[i][j][2] = dataSphere[i][j]-dataSphere[i][j+1];
        normalsSphere[i][j][1]= 1.0;
    }
    
    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        x = 0.0;
        for(k=0;k<3;k++) x+=normalsSphere[i][j][k]*normalsSphere[i][j][k];
        x=sqrt(x);
        for(k=0;k<3;k++) normalsSphere[i][j][k]= 0.5*normalsSphere[i][j][k]/x+0.5;
    }
    
    path = "pyramid_normal_map.bmp"; //should be cube but cube not working
    coordObj = loadBMPRaw(path.c_str(), hh, ww);
    q = 0;
    
    // Create a checkerboard pattern
    
    for ( int i = 0; i < 512; i++ ) {
        for ( int j = 0; j < 512; j++ ) {
            dataCube[i][j]  = (coordObj[q]); q++;
            dataCube[i][j]  += (coordObj[q]); q++;
            dataCube[i][j]  += (coordObj[q]); q++;
            dataCube[i][j] /= 1.0*(256*3);
            if(dataCube[i][j]>0.5) dataCube[i][j] = 1.0;
            else dataCube[i][j] = 0;
            
            
        }
    }
       
    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        normalsCube[i][j][0] = dataCube[i][j]-dataCube[i+1][j];
        normalsCube[i][j][2] = dataCube[i][j]-dataCube[i][j+1];
        normalsCube[i][j][1]= 1.0;
    }
    
    
    for(i=0;i<N;i++) for(j=0;j<N;j++)
    {
        x = 0.0;
        for(k=0;k<3;k++) x+=normalsCube[i][j][k]*normalsCube[i][j][k];
        x=sqrt(x);
        for(k=0;k<3;k++) normalsCube[i][j][k]= 0.5*normalsCube[i][j][k]/x+0.5;
    }
     
    glutInit(&argc, argv);
 	glutInitDisplayMode( GLUT_RGBA);
 	w=512; h=512;
 	glutInitWindowSize( w, h );
 	glutInitContextVersion( 3, 2 );
 	glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow("ASSIGNMENT 4");
 	glutDisplayFunc( display );  //display callback
 	glutMotionFunc(trackball); //mouse call
 	glutKeyboardFunc( keyboard );  // calls keyboard funcion
 	glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;

}
