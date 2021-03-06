#include <stdlib.h>
#include <GL/glew.h>
#ifdef __APPLE__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif // __APPLE__
#include <math.h>
#include <stdio.h>
#include "util.h"


static const GLfloat g_vertex_buffer_data[] = {
	-1.0f , -1.0f, 0.0f, 1.0f,
	1.0f , -1.0f, 0.0f, 1.0f,
	-1.0f , 1.0f, 0.0f, 1.0f,
	1.0f , 1.0f, 0.0f, 1.0f
};

static const GLushort g_element_buffer_buffer_data[] = { 0, 1, 2, 3 };

static struct{
	GLuint vertex_buffer, element_buffer;
	GLuint textures[2];

	/* Campos para os objetos do shader */

	GLuint vertex_shader, fragment_shader, program;

	struct{
		GLint timer;
		GLint texture[2];
	} uniforms;
	struct{
		GLint position;
	} attributes;

	GLfloat timer;

} g_resources;

static GLuint make_buffer(
	GLenum target,
	const void *buffer_data,
	GLsizei buffer_size
){
	GLuint buffer;
	glGenBuffers( 1 , &buffer );
	glBindBuffer( target , buffer );
	glBufferData( target , buffer_size, buffer_data, GL_STATIC_DRAW );
	return buffer;
}

static GLuint make_texture(const char *filename){
	GLuint texture;
	int width, height;
	void *pixels = read_tga(filename, &width, &height);

	if(!pixels) return 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(
		GL_TEXTURE_2D, 0, /* target e o nível de detalhe */
		GL_RGB8,			/* Formato interno */
		width, height, 0,	/* largura, altura e borda */
		GL_BGR, GL_UNSIGNED_BYTE,	/*formato externo e o tipo */
		pixels				/* os pixels */
	);
	free(pixels);
	return texture;
}

static void update_timer(void){
	int milliseconds = glutGet( GLUT_ELAPSED_TIME );
	g_resources.timer = (float)  milliseconds * 0.001f;
	glutPostRedisplay();
}

static void show_info_log(
	GLuint object,
	PFNGLGETSHADERIVPROC glGet__iv,
	PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
	){

	GLint log_length;
	char *log;

	glGet__iv( object , GL_INFO_LOG_LENGTH , &log_length );
	log = malloc( log_length );
	glGet__InfoLog( object , log_length , NULL , log );
	fprintf( stderr, "%s" , log );
	free( log );
}

static GLuint make_shader(GLenum type, const char *filename){
	GLint length;
	GLchar *source = file_contents( filename , &length );
	GLuint shader;
	GLint shader_ok;
	
	if( ! source ) 
		return 0;

	shader = glCreateShader( type );
	glShaderSource( shader , 1 , ( const GLchar** ) &source , &length );
	free( source );
	glCompileShader( shader );

	glGetShaderiv( shader , GL_COMPILE_STATUS , &shader_ok );

	if( ! shader_ok ){
		fprintf( stderr , "Failed to compile %s:\n", filename );
		show_info_log( shader , glGetShaderiv , glGetShaderInfoLog );
		glDeleteShader( shader );
		return 0;
	}

	return shader;
}

static GLuint make_program( GLuint vertex_shader , GLuint fragment_shader ){
	GLint program_ok;
	GLuint program = glCreateProgram();
	glAttachShader( program , vertex_shader );
	glAttachShader( program , fragment_shader );
	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS , &program_ok );
	if( ! program_ok ){
		fprintf( stderr , "Failed to link shader program:\n" );
		show_info_log( program, glGetProgramiv , glGetProgramInfoLog );
		glDeleteProgram( program );
		return 0;
	}
	return program;
}

static int make_resources( const char *vertex_shader_file ){

	/* Feitura do buffer */
	g_resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
	);
	g_resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_buffer_data,
		sizeof(g_element_buffer_buffer_data)
	);

	/* Feitura das Textures */
	g_resources.textures[0] = make_texture("hello1.tga");
	g_resources.textures[1] = make_texture("hello2.tga");
	
	if(g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
		return 0;

 	/* Feitura dos Shaders */
	g_resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		vertex_shader_file
	);
	if( g_resources.vertex_shader == 0 )
		return 0;

	g_resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		"hello-gl.f.glsl"
	);
	if( g_resources.fragment_shader == 0 )
		return 0;

	g_resources.program = make_program(
		g_resources.vertex_shader,
		g_resources.fragment_shader
	);
	if( g_resources.program == 0 )
		return 0;

	g_resources.uniforms.timer
		= glGetUniformLocation( g_resources.program , "timer" );
	g_resources.uniforms.texture[0]
		= glGetUniformLocation( g_resources.program , "textures[0]" );
	g_resources.uniforms.texture[1]
		= glGetUniformLocation( g_resources.program , "textures[1]" );

	g_resources.attributes.position
		= glGetAttribLocation( g_resources.program , "position" );

	return 1;
}

static void render(void){

	glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	glUseProgram( g_resources.program );

	glUniform1f( g_resources.uniforms.timer , g_resources.timer );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D , g_resources.textures[0] );
	glUniform1i( g_resources.uniforms.texture[0] , 0 );

	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D , g_resources.textures[1] );
	glUniform1i( g_resources.uniforms.texture[1] , 1 );

	glBindBuffer( GL_ARRAY_BUFFER , g_resources.vertex_buffer );
	glVertexAttribPointer(
		g_resources.attributes.position,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof( GLfloat ) * 4,
		(void *) 0
	);
	glEnableVertexAttribArray( g_resources.attributes.position );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , g_resources.element_buffer );
	glDrawElements(
		GL_TRIANGLE_STRIP,
		4,
		GL_UNSIGNED_SHORT,
		(void *) 0
	);

	glDisableVertexAttribArray( g_resources.attributes.position );

	glutSwapBuffers();
}

int main( int argc , char **argv ){

    glutInit( &argc , argv );

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );

    glutInitWindowSize(400,300);
    glutCreateWindow("Hello World");
    glutDisplayFunc(&render);
    glutIdleFunc(&update_timer);

    glewInit();

    if(!GLEW_VERSION_2_0){
        fprintf(stderr, "OpenGL 2.0 not available\n");
        return 1;
    }
    if(!make_resources( argc >= 2 ? argv[1] : "hello-gl.v.glsl" )){
        fprintf( stderr , "Failed to load resources\n" );
        return 1;
    }

    glutMainLoop();
    return 0;
}
