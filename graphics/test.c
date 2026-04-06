#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 1000
#define HEIGHT 1000

//gcc test.c -Iinclude -Llib -lglfw3 -lglew32 -lopengl32 -lgdi32 -o mandelbrot

const char* vertex_shader_src =
"#version 330 core\n"
"layout (location = 0) in vec2 pos;\n"
"out vec2 fragPos;\n"
"void main(){\n"
"    fragPos = pos;\n"
"    gl_Position = vec4(pos,0.0,1.0);\n"
"}";

const char* fragment_shader_src =
"#version 330 core\n"
"in vec2 fragPos;\n"
"out vec4 color;\n"
"#define PI 3.14159265359\n"
"#define ANTIALIASCONST 3000\n"
"uniform float coordX; uniform float coordY; uniform float scroll;\n"
"vec2 cmul(vec2 a, vec2 b) {\n"
"    return vec2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);\n"
"}\n"
"vec2 pow(vec2 a,int n) {\n"
"   vec2 r = a;\n"
"   for(int i=0;i<n-1;i++) {\n"
"       r=cmul(r,a);\n"
"   }\n"
"   return r;\n"
"}\n"
"vec2 inv(vec2 a) {\n"
"   if(dot(a,a)==0) return vec2(0.0,0.0);"
"   return (1/dot(a,a))*vec2(a.x,-a.y);\n"
"}\n"
"vec2 cln(vec2 a) {\n"
"    return vec2(0.5*log(dot(a,a)), atan(a.y, a.x));\n"
"}\n"
"vec2 cexp(vec2 a) {\n"
"   float s = exp(a.x);"
"   return vec2(s*cos(a.y),s*sin(a.y));"
"}\n"
"vec2 cpow(vec2 a, vec2 b) {\n"
"   if (length(a) == 0.0) return vec2(0.0);\n" 
"   vec2 logA = cln(a);\n"
"   vec2 blogA = cmul(b, logA);\n"
"   return cexp(blogA);\n"
"}\n"
"vec2 ccos(vec2 a) {\n"
"   return vec2(cos(a.x)*cosh(a.y),-sin(a.x)*sinh(a.y));\n"
"}\n"
"vec2 csin(vec2 a) {\n"
"   return vec2(sin(a.x)*cosh(a.y),cos(a.x)*sinh(a.y));\n"
"}\n"
"void main(){\n"
"   vec2 zoomCentre = vec2(coordX,coordY);\n"
"   color = vec4(0.0,0.0,0.0,0.0);"
"   for(int x=-1;x<=1;x+=2) {\n"
"   for(int y=-1;y<=1;y+=2) {\n"
"   vec2 z0 = fragPos/scroll + zoomCentre;\n"
"   z0 = z0 + vec2(x,y)/(scroll*ANTIALIASCONST);\n"
"   int i=0; int maxloop = 200; int r2 = 200; vec2 z=z0;\n"
"   while(dot(z,z)<=r2 && i<=maxloop) {\n"
"      z=cpow(z,z)+z0;\n"
"      i++;\n"
"   }\n"
"   float smoothp=0.0;\n"
"   float pow=2;\n"
"   if(i<maxloop) {\n"
"       float nu = log(log(float(i)*log2(0.5*log(dot(z,z)))/log(pow)));\n"
"       nu = nu/log(log(float(maxloop)*log(0.5*log(r2))/log(pow))); // one more log for both to colour zpowz \n"
"       vec3 colorconst = 0.5+0.5*cos(2*PI*(nu*vec3(1.5,1.5,1.5)+vec3(0.1,0.2,0.3)));\n"
"       color += vec4(colorconst,1.0);\n"
"   } else {\n"
"       color += vec4(0.0,0.0,0.0,1.0);\n"
"   }\n"  
"   }\n"
"   }\n"
"   color=color/4;\n"
"}";

double scroll=1;float coordX=0;float coordY=0;
double scrollsensitivity =0.1;
float movesensitivity=0.06;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(abs(scroll)<0.0001) {
        scroll+=scrollsensitivity*yoffset;
        return;
    }
    scroll+= scrollsensitivity*abs(scroll)*yoffset;
}

void key_callback(GLFWwindow* window,int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        unsigned char* data = (unsigned char*)malloc(WIDTH * HEIGHT * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_flip_vertically_on_write(1);
        stbi_write_png("screenshot.png", WIDTH, HEIGHT, 4, data, WIDTH * 4);

        free(data);
    }
    if(action==GLFW_REPEAT || action==GLFW_PRESS) {
        switch(key) {
            case(GLFW_KEY_LEFT):
                coordX-=movesensitivity/scroll;
                break;
            case(GLFW_KEY_RIGHT):
                coordX+=movesensitivity/scroll;
                break;
            case(GLFW_KEY_UP):
                coordY+=movesensitivity/scroll;
                break;
            case(GLFW_KEY_DOWN):
                coordY-=movesensitivity/scroll;
                break;
        }
    }
}

GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,NULL);
    glCompileShader(s);
    return s;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES,4);
    GLFWwindow* window =glfwCreateWindow(WIDTH,HEIGHT,"polybrot",NULL,NULL);
    glfwMakeContextCurrent(window);
    glEnable(GL_MULTISAMPLE);  
    glewInit();
    float quad[] = {
        -1,-1,
         1,-1,
         1, 1,
        -1, 1
    };
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    unsigned int indices[] = {0,1,2, 2,3,0};

    GLuint vao,vbo,ebo;

    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    GLuint vs = compile_shader(GL_VERTEX_SHADER,vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER,fragment_shader_src);

    GLuint program = glCreateProgram();
    glAttachShader(program,vs);
    glAttachShader(program,fs);
    glLinkProgram(program);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        
        GLint cx = glGetUniformLocation(program, "coordX");
        GLint cy = glGetUniformLocation(program, "coordY");
        GLint scrl = glGetUniformLocation(program, "scroll");
        glUniform1f(cx, coordX);
        glUniform1f(cy, coordY);
        glUniform1f(scrl,(float)scroll);

        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
