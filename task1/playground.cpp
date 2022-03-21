// Включаем стандартные заголовки
#include <stdio.h>
#include <stdlib.h>

// Включаем GLEW. Всегда включайте его ДО gl.h и glfw.h
#include <GL/glew.h>

// 
#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
using namespace glm;



int main()
{
	// Initialise GLFW
    // Инициализируем GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "Ошибка при инициализации GLFWn" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // version OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


    // Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Playground", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
    
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

    
    // Включим режим отслеживания нажатия клавиш, для проверки ниже
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    
    //
    // Create and compile our GLSL program from the shaders
    GLuint programIDs[2];
    
    programIDs[0] = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader1.fragmentshader" );
    programIDs[1] = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader2.fragmentshader" );

    // Get a handle for our buffers
    GLuint vertexPosition_modelspaceIDs[2];
    vertexPosition_modelspaceIDs[0] = glGetAttribLocation(programIDs[0], "vertexPosition_modelspace");
    vertexPosition_modelspaceIDs[1] = glGetAttribLocation(programIDs[1], "vertexPosition_modelspace");
    
    // Get a handle for our "MVP" uniform
    GLuint MatrixIDs[2];
    MatrixIDs[0] = glGetUniformLocation(programIDs[0], "MVP");
    MatrixIDs[1] = glGetUniformLocation(programIDs[1], "MVP");
    
    // Projection matrix : 45∞ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    mat4 Projection = perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
    
    // Model matrix : an identity matrix (model will be at the origin)
    mat4 Model = mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    
    
    
    // add coordinates 1st triangle
    static const GLfloat g_vertex_buffer_data1[] = {
        -0.5f, 0.5f, 0.0f,
         0.5f, 0.5f, 0.0f,
         0.0f,  -0.5f, 0.0f,
    };
    
    // add coordinates 2nd triangle
    static const GLfloat g_vertex_buffer_data2[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };

    
    
    // Create vertex buffers
    GLuint vertexbuffers[2]; // id buffers
    glGenBuffers(2, vertexbuffers); // gen buffers and write id in variable
    
    // buffer 1
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[0]); // bind buffer as current
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data1), g_vertex_buffer_data1, GL_STATIC_DRAW);
    
    // buffer 2
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[1]); // bind buffer as current
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data2), g_vertex_buffer_data2, GL_STATIC_DRAW);
    
    
    // enable blending for alpha chanel
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    GLfloat x = 1.;
    GLfloat y = 0.;
    GLfloat z = 1.;
    GLfloat r = 1.;
    do{
        r = 2. + sin(glfwGetTime());
        x = cos(glfwGetTime()) * r;
        y = sin(glfwGetTime()) * r;
        z = sin(glfwGetTime()) * r;
        
        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );
        
        // Camera matrix (for moving)
        mat4 View = glm::lookAt(
                                    vec3(x,y,z), // Camera is at (4,3,3), in World Space
                                    vec3(0,0,0), // and looks at the origin
                                    vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        
        mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
        
		// Draw triangles
        for (int j = 0; j < 2; ++j) {
            // Use our shader
            glUseProgram(programIDs[j]);
            
            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDs[j], 1, GL_FALSE, &MVP[0][0]);


            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(vertexPosition_modelspaceIDs[j]);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[j]);
            glVertexAttribPointer(
                vertexPosition_modelspaceIDs[j], // The attribute we want to configure
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

            // Draw the triangle
            glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

            glDisableVertexAttribArray(vertexPosition_modelspaceIDs[j]);
        }
        
        
        // Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

