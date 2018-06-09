
// The object modelling tasks performed by this file are usually done 
// by reading a scene configuration file or through a help of graphics user interface!!!

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

typedef struct _material {
	glm::vec4 emission, ambient, diffuse, specular;
	GLfloat exponent;
} Material;

#define N_MAX_GEOM_COPIES 5
typedef struct _Object {
	char filename[512];

	GLenum front_face_mode; // clockwise or counter-clockwise
	int n_triangles;

	int n_fields; // 3 floats for vertex, 3 floats for normal, and 2 floats for texcoord
	GLfloat *vertices; // pointer to vertex array data
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax; // bounding box <- compute this yourself

	GLuint VBO, VAO; // Handles to vertex buffer object and vertex array object

	int n_geom_instances;
	glm::mat4 ModelMatrix[N_MAX_GEOM_COPIES];
	Material material[N_MAX_GEOM_COPIES];
} Object;

#define N_MAX_STATIC_OBJECTS		15
Object static_objects[N_MAX_STATIC_OBJECTS]; // allocage memory dynamically every time it is needed rather than using a static array
Object car_objects[6];
int n_static_objects = 0;

#define OBJ_BUILDING		0
#define OBJ_MINIBUILDING		1
#define OBJ_TABLE			2
#define OBJ_LIGHT			3
#define OBJ_TEAPOT1			4
#define OBJ_TEAPOT2			5
#define OBJ_NEW_CHAIR1		6
#define OBJ_NEW_CHAIR2		7
#define OBJ_FRAME			8
#define OBJ_NEW_PICTURE		9
#define OBJ_COW1			10
#define OBJ_COW2			11
#define OBJ_GODZILLA		12
#define OBJ_IRONMAN				13



GLfloat *vec_to_float(glm::vec4 vec) {
	GLfloat f4[4];
	f4[0] = vec.x;
	f4[1] = vec.y;
	f4[2] = vec.z;
	f4[3] = vec.w;

	return f4;
}

void set_material(Object *obj_ptr) {

	// assume ShaderProgram_PS is used
	glUniform4fv(loc_material.ambient_color, 1, vec_to_float(obj_ptr->material[0].ambient));
	glUniform4fv(loc_material.diffuse_color, 1, vec_to_float(obj_ptr->material[0].diffuse));
	glUniform4fv(loc_material.specular_color, 1, vec_to_float(obj_ptr->material[0].specular));
	glUniform1f(loc_material.specular_exponent, obj_ptr->material[0].exponent);
	glUniform4fv(loc_material.emissive_color, 1, vec_to_float(obj_ptr->material[0].emission));
}

///////////////////////car///////////////

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER;
glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()


float rotation_angle_car = 0.0f;


#define rad 1.7f
#define ww 1.0f

#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

GLuint path_VBO, path_VAO;
GLfloat *path_vertices;
int path_n_vertices;

int read_path_file(GLfloat **object, char *filename) {
	int i, n_vertices;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading path from the path file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the path file %s ...", filename);
		return -1;
	}

	fscanf(fp, "%d", &n_vertices);
	*object = (float *)malloc(n_vertices * 3 * sizeof(float));
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the path file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < n_vertices; i++) {
		fscanf(fp, "%f %f %f", flt_ptr, flt_ptr + 1, flt_ptr + 2);
		flt_ptr += 3;
	}
	fclose(fp);

	fprintf(stdout, "Read %d vertices successfully.\n\n", n_vertices);

	return n_vertices;
}

void prepare_path(void) { // Draw path.
	char path[] = "Data/path.txt";  //	return;
	path_n_vertices = read_path_file(&path_vertices,path);
	printf("%d %f\n", path_n_vertices, path_vertices[(path_n_vertices - 1)]);
	// Initialize vertex array object.
	glGenVertexArrays(1, &path_VAO);
	glBindVertexArray(path_VAO);
	glGenBuffers(1, &path_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, path_VBO);
	glBufferData(GL_ARRAY_BUFFER, path_n_vertices * 3 * sizeof(float), path_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_path(void) {
	glBindVertexArray(path_VAO);
	glUniform3f(loc_primitive_color, 1.000f, 1.000f, 0.000f); // color name: Yellow
	glDrawArrays(GL_LINE_STRIP, 0, path_n_vertices);
}

void free_path(void) {
	glDeleteVertexArrays(1, &path_VAO);
	glDeleteBuffers(1, &path_VBO);
}


#define N_GEOMETRY_OBJECTS 6
#define GEOM_OBJ_ID_CAR_BODY 0
#define GEOM_OBJ_ID_CAR_WHEEL 1
#define GEOM_OBJ_ID_CAR_NUT 2
#define GEOM_OBJ_ID_COW 3
#define GEOM_OBJ_ID_TEAPOT 4
#define GEOM_OBJ_ID_BOX 5

GLuint geom_obj_VBO[N_GEOMETRY_OBJECTS];
GLuint geom_obj_VAO[N_GEOMETRY_OBJECTS];

int geom_obj_n_triangles[N_GEOMETRY_OBJECTS];
GLfloat *geom_obj_vertices[N_GEOMETRY_OBJECTS];

// codes for the 'general' triangular-mesh object
typedef enum _GEOM_OBJ_TYPE { GEOM_OBJ_TYPE_V = 0, GEOM_OBJ_TYPE_VN, GEOM_OBJ_TYPE_VNT } GEOM_OBJ_TYPE;
// GEOM_OBJ_TYPE_V: (x, y, z)
// GEOM_OBJ_TYPE_VN: (x, y, z, nx, ny, nz)
// GEOM_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int GEOM_OBJ_ELEMENTS_PER_VERTEX[3] = { 3, 6, 8 };

int read_geometry_file(GLfloat **object, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}

	fscanf(fp, "%d", &n_triangles);
	*object = (float *)malloc(3 * n_triangles*GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float));
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; i++)
		fscanf(fp, "%f", flt_ptr++);
	fclose(fp);

	fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	return n_triangles;
}

void prepare_geom_obj(int geom_obj_ID, char *filename, GEOM_OBJ_TYPE geom_obj_type) {
	int n_bytes_per_vertex;

	n_bytes_per_vertex = GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float);
	geom_obj_n_triangles[geom_obj_ID] = read_geometry_file(&geom_obj_vertices[geom_obj_ID], filename, geom_obj_type);

	// Initialize vertex array object.
	glGenVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glGenBuffers(1, &geom_obj_VBO[geom_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * geom_obj_n_triangles[geom_obj_ID] * n_bytes_per_vertex,
		geom_obj_vertices[geom_obj_ID], GL_STATIC_DRAW);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	if (geom_obj_type >= GEOM_OBJ_TYPE_VN) {
		glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	if (geom_obj_type >= GEOM_OBJ_TYPE_VNT) {
		glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(geom_obj_vertices[geom_obj_ID]);
}


void draw_geom_obj(int geom_obj_ID) {
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * geom_obj_n_triangles[geom_obj_ID]);
	glBindVertexArray(0);
}

void free_geom_obj(int geom_obj_ID) {
	glDeleteVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glDeleteBuffers(1, &geom_obj_VBO[geom_obj_ID]);
}

void draw_wheel_and_nut(int cam_idx) {
	// angle is used in Hierarchical_Car_Correct later
	int i;
	
	car_objects[GEOM_OBJ_ID_CAR_WHEEL].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_WHEEL].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_WHEEL].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_WHEEL].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_WHEEL].material[0].exponent = 128.0f*0.6;

	//glUniform3f(loc_primitive_color, 0.20f, 0.20f, 1.0f); // color name: Aquamarine
	set_material(&car_objects[GEOM_OBJ_ID_CAR_WHEEL]);
	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL);


	//glUniform3f(loc_primitive_color, 0.800f, 0.808f, 0.020f); // color name: DarkTurquoise
	//draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL); // draw wheel

	for (i = 0; i < 5; i++) {
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN*72.0f*i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_NUT;
		ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_NUT;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));

		if (shader_mode==GS) {
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
			glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
			glUniform3f(loc_primitive_color, 0.690f, 0.769f, 0.871f);
		}
		else {
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
			glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		}
		glUniform3f(loc_primitive_color, 0.690f, 0.769f, 0.871f); // color name: LightSteelBlue

		draw_geom_obj(GEOM_OBJ_ID_CAR_NUT); // draw i-th nut
	}
	//glUseProgram(0);
}

void draw_car_dummy(int cam_idx) {

	car_objects[GEOM_OBJ_ID_CAR_BODY].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_BODY].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_BODY].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_BODY].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	car_objects[GEOM_OBJ_ID_CAR_BODY].material[0].exponent = 128.0f*0.6;

	glUniform3f(loc_primitive_color, 0.20f, 0.20f, 1.0f); // color name: Aquamarine
	set_material(&car_objects[GEOM_OBJ_ID_CAR_BODY]);
	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY); // draw body




	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, -30 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 200 * rotation_angle_car*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));

	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_wheel_and_nut(cam_idx);  // draw wheel 0

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, 4.5f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 200 * rotation_angle_car*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_wheel_and_nut(cam_idx);  // draw wheel 1

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 30 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 80 * rotation_angle_car*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_wheel_and_nut(cam_idx);  // draw wheel 2

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, 80 * rotation_angle_car*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_WHEEL;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_wheel_and_nut(cam_idx);  // draw wheel 3
	//glUseProgram(0);
}


void display_car(int cam_idx) {

	ModelMatrix_CAR_BODY = glm::translate(glm::mat4(1.0f),glm::vec3(40.0f, 87.0f, 5.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, -3*rotation_angle_car, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(14.0f, 0.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	

	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_CAR_BODY;
	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix_CAR_BODY;
 	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	draw_car_dummy(cam_idx);
	//glUseProgram(0);

}


void cleanup_car(void) {
	free_path();

	free_geom_obj(GEOM_OBJ_ID_CAR_BODY);
	free_geom_obj(GEOM_OBJ_ID_CAR_WHEEL);
	free_geom_obj(GEOM_OBJ_ID_CAR_NUT);
	free_geom_obj(GEOM_OBJ_ID_CAR_BODY);
	free_geom_obj(GEOM_OBJ_ID_COW);
	free_geom_obj(GEOM_OBJ_ID_TEAPOT);
	free_geom_obj(GEOM_OBJ_ID_BOX);
}





///////////////////////////////////////////////////////







GLfloat view_line[2][3] = { {0.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f} };


GLfloat view_color[2][3] = {
	{ 0 / 255.0f, 149 / 255.0f, 159 / 255.0f },{ 255 / 255.0f, 255 / 255.0f, 0 / 255.0f }
};
GLuint VBO_view, VAO_view;

void prepare_view() {
	GLsizeiptr buffer_size = sizeof(view_line);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_view);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_view);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(view_line), view_line);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_view);
	glBindVertexArray(VAO_view);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_view);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void draw_view(int color) {
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glBindVertexArray(VAO_view);

	glUniform3fv(loc_primitive_color, 1, view_color[color]);

	glLineWidth(3.0);
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.0);
	glBindVertexArray(0);
}

void display_view(int cam_idx) {
	glm::mat4 ModelMatrix_view;

	float lenxy = sqrt((-cam[MAIN_VIEW].naxis.x)*(-cam[MAIN_VIEW].naxis.x) + (-cam[MAIN_VIEW].naxis.y) *(-cam[MAIN_VIEW].naxis.y));
	float lenxz = sqrt((-cam[MAIN_VIEW].naxis.x)*(-cam[MAIN_VIEW].naxis.x) + (-cam[MAIN_VIEW].naxis.z) *(-cam[MAIN_VIEW].naxis.z));
	
	float rx = acos(dot(glm::vec3(-cam[MAIN_VIEW].naxis.x,cam[MAIN_VIEW].naxis.y,0.0f ), glm::vec3(1.0f, 0.0f, 0.0f)) / lenxy );
	float rz = acos(dot(glm::vec3(-cam[MAIN_VIEW].naxis.x, 0.0f, -cam[MAIN_VIEW].naxis.z), glm::vec3(1.0f,0.0f, 0.0f)) /lenxz);

	rx = (1.0f * (-cam[MAIN_VIEW].naxis.y) - 0.0f *(-cam[MAIN_VIEW].naxis.x));
	rz = (1.0f * (-cam[MAIN_VIEW].naxis.z) - 0.0f *(-cam[MAIN_VIEW].naxis.x)) < 0 ? rz : 360 * TO_RADIAN - rz;



	ModelMatrix_view = glm::translate(glm::mat4(1.0f), cam[MAIN_VIEW].pos);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rz, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rx, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view,cam[MAIN_VIEW].fov_y, cam[MAIN_VIEW].uaxis);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, cam[MAIN_VIEW].vaxis);
	ModelMatrix_view = glm::scale(ModelMatrix_view, glm::vec3(cam[MAIN_VIEW].far_clip, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_view;
	draw_view(1);

	ModelMatrix_view = glm::translate(glm::mat4(1.0f), cam[MAIN_VIEW].pos);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rz, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rx, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, -cam[MAIN_VIEW].uaxis);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, -cam[MAIN_VIEW].vaxis);
	ModelMatrix_view = glm::scale(ModelMatrix_view, glm::vec3(cam[MAIN_VIEW].far_clip, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_view;
	draw_view(1);


	ModelMatrix_view = glm::translate(glm::mat4(1.0f), cam[MAIN_VIEW].pos);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rz, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rx, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, -cam[MAIN_VIEW].uaxis);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, cam[MAIN_VIEW].vaxis);
	ModelMatrix_view = glm::scale(ModelMatrix_view, glm::vec3(cam[MAIN_VIEW].far_clip, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_view;
	draw_view(1);

	ModelMatrix_view = glm::translate(glm::mat4(1.0f), cam[MAIN_VIEW].pos);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rz, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, rx, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, cam[MAIN_VIEW].uaxis);
	ModelMatrix_view = glm::rotate(ModelMatrix_view, cam[MAIN_VIEW].fov_y, -cam[MAIN_VIEW].vaxis);
	ModelMatrix_view = glm::scale(ModelMatrix_view, glm::vec3(cam[MAIN_VIEW].far_clip, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_view;
	draw_view(1);


}

GLfloat cctv_body[1][3] = { {0.0, 0.0, 0.0} };
GLfloat main_color[1][3] = { { 100 / 255.0f, 255 / 255.0f, 100 / 255.0f } };
GLfloat cctv_color[1][3] = { { 0 / 255.0f, 149 / 255.0f, 159 / 255.0f } };
GLuint VBO_cctv, VAO_cctv;
GLsizeiptr buffer_size = sizeof(cctv_body);


void prepare_cctv() {
	GLsizeiptr buffer_size = sizeof(cctv_body);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cctv);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cctv);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cctv_body), cctv_body);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cctv);
	glBindVertexArray(VAO_cctv);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cctv);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename) {
	int n_triangles;
	FILE *fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open the object file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float *)malloc(n_triangles*bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Error: cannot allocate memory for the geometry file %s ...\n", filename);
		exit(EXIT_FAILURE);
	}
	fread(*object, bytes_per_primitive, n_triangles, fp); // assume the data file has no faults.
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void compute_AABB(Object *obj_ptr) {
	// Do it yourself.
}
	 
void prepare_geom_of_static_object(Object *obj_ptr) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = obj_ptr->n_fields * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	obj_ptr->n_triangles = read_geometry(&(obj_ptr->vertices), n_bytes_per_triangle, obj_ptr->filename);

	// Initialize vertex buffer object.
	glGenBuffers(1, &(obj_ptr->VBO));

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glBufferData(GL_ARRAY_BUFFER, (obj_ptr->n_triangles)*n_bytes_per_triangle, obj_ptr->vertices, GL_STATIC_DRAW);

	compute_AABB(obj_ptr);
	free(obj_ptr->vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &(obj_ptr->VAO));
	glBindVertexArray(obj_ptr->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void define_static_objects(void) {
	
	//ironman
	strcpy(static_objects[OBJ_IRONMAN].filename, "Data/IronMan.geom");
	static_objects[OBJ_IRONMAN].n_fields = 8;

	static_objects[OBJ_IRONMAN].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_IRONMAN]));

	static_objects[OBJ_IRONMAN].n_geom_instances = 1;

	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::mat4(1.0f);

	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::translate(static_objects[OBJ_IRONMAN].ModelMatrix[0], glm::vec3(150.0f, 30.0f, 40.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[0], 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
//	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::rotate(static_objects[OBJ_IRONMAN].ModelMatrix[0], -110 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_IRONMAN].ModelMatrix[0] = glm::scale(static_objects[OBJ_IRONMAN].ModelMatrix[0], glm::vec3(10.0f, 10.0f, 10.0f));

	static_objects[OBJ_IRONMAN].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].diffuse = glm::vec4(0.999f, 0.30f, 0.0f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_IRONMAN].material[0].exponent = 128.0f*0.1f;



	// godzilla
	strcpy(static_objects[OBJ_GODZILLA].filename, "Data/Godzilla.geom");
	static_objects[OBJ_GODZILLA].n_fields = 8;

	static_objects[OBJ_GODZILLA].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_GODZILLA]));

	static_objects[OBJ_GODZILLA].n_geom_instances = 1;

	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::mat4(1.0f);

	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::translate(static_objects[OBJ_GODZILLA].ModelMatrix[0], glm::vec3(215.0f, 145.0f, 0.0f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::rotate(static_objects[OBJ_GODZILLA].ModelMatrix[0],90*TO_RADIAN ,glm::vec3(0.0f, 1.0f, 0.0f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::rotate(static_objects[OBJ_GODZILLA].ModelMatrix[0], -110 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	static_objects[OBJ_GODZILLA].ModelMatrix[0] = glm::scale(static_objects[OBJ_GODZILLA].ModelMatrix[0], glm::vec3(0.1f, 0.1f, 0.1f));

	static_objects[OBJ_GODZILLA].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].diffuse = glm::vec4(0.14f, 0.98f, 0.32f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_GODZILLA].material[0].exponent = 128.0f*0.1f;


	// building
	strcpy(static_objects[OBJ_BUILDING].filename, "Data/Building1_vnt.geom");
	static_objects[OBJ_BUILDING].n_fields = 8;

	static_objects[OBJ_BUILDING].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_BUILDING]));

	static_objects[OBJ_BUILDING].n_geom_instances = 1;

    static_objects[OBJ_BUILDING].ModelMatrix[0] = glm::mat4(1.0f);
	
	static_objects[OBJ_BUILDING].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_BUILDING].material[0].exponent = 128.0f*0.1f;

	// building
	strcpy(static_objects[OBJ_MINIBUILDING].filename, "Data/Building1_vnt.geom");
	static_objects[OBJ_MINIBUILDING].n_fields = 8;

	static_objects[OBJ_MINIBUILDING].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_MINIBUILDING]));

	static_objects[OBJ_MINIBUILDING].n_geom_instances = 1;

	static_objects[OBJ_MINIBUILDING].ModelMatrix[0] = glm::mat4(1.0f);

	static_objects[OBJ_MINIBUILDING].ModelMatrix[0] = glm::translate(static_objects[OBJ_MINIBUILDING].ModelMatrix[0], glm::vec3(18.0f, 30.0f, 0.0f));
	static_objects[OBJ_MINIBUILDING].ModelMatrix[0] = glm::scale(static_objects[OBJ_MINIBUILDING].ModelMatrix[0], glm::vec3(0.15f, 0.15f, 0.15f));

	static_objects[OBJ_MINIBUILDING].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_MINIBUILDING].material[0].ambient = glm::vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
	static_objects[OBJ_MINIBUILDING].material[0].diffuse = glm::vec4(0.54f, 0.89f, 0.63f, 1.0f);
	static_objects[OBJ_MINIBUILDING].material[0].specular = glm::vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
	static_objects[OBJ_MINIBUILDING].material[0].exponent = 128.0f*0.1f;

	// table
	strcpy(static_objects[OBJ_TABLE].filename, "Data/Table_vn.geom");
	static_objects[OBJ_TABLE].n_fields = 6;

	static_objects[OBJ_TABLE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TABLE]));

	static_objects[OBJ_TABLE].n_geom_instances = 2;

	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(157.0f, 76.5f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[0] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[0], 
		glm::vec3(0.5f, 0.5f, 0.5f));

	static_objects[OBJ_TABLE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[0].ambient = glm::vec4(0.1f, 0.3f, 0.1f, 1.0f);
	static_objects[OBJ_TABLE].material[0].diffuse = glm::vec4(0.4f, 0.6f, 0.3f, 1.0f);
	static_objects[OBJ_TABLE].material[0].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[0].exponent = 15.0f;

	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(198.0f, 120.0f, 0.0f));
	static_objects[OBJ_TABLE].ModelMatrix[1] = glm::scale(static_objects[OBJ_TABLE].ModelMatrix[1],
		glm::vec3(0.8f, 0.6f, 0.6f));

	static_objects[OBJ_TABLE].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TABLE].material[1].ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	static_objects[OBJ_TABLE].material[1].diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	static_objects[OBJ_TABLE].material[1].specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	static_objects[OBJ_TABLE].material[1].exponent = 128.0f*0.078125f;

	// Light
	strcpy(static_objects[OBJ_LIGHT].filename, "Data/Light_vn.geom");
	static_objects[OBJ_LIGHT].n_fields = 6;

	static_objects[OBJ_LIGHT].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(static_objects + OBJ_LIGHT);

	static_objects[OBJ_LIGHT].n_geom_instances = 5;

	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 100.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[0] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[0].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::translate(glm::mat4(1.0f), glm::vec3(80.0f, 47.5f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[1] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[1],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[1].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[1].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 130.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[2] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[2],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[2].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[2].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, 60.0f, 49.0f));
	static_objects[OBJ_LIGHT].ModelMatrix[3] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[3],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[3].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[3].exponent = 128.0f*0.4f;

	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::translate(glm::mat4(1.0f), glm::vec3(210.0f, 112.5f, 49.0));
	static_objects[OBJ_LIGHT].ModelMatrix[4] = glm::rotate(static_objects[OBJ_LIGHT].ModelMatrix[4],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_LIGHT].material[4].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
	static_objects[OBJ_LIGHT].material[4].exponent = 128.0f*0.4f;

	// teapot1
	strcpy(static_objects[OBJ_TEAPOT1].filename, "Data/Teapotn_vn.geom");
	static_objects[OBJ_TEAPOT1].n_fields = 6;

	static_objects[OBJ_TEAPOT1].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TEAPOT1]));

	static_objects[OBJ_TEAPOT1].n_geom_instances = 1;

	static_objects[OBJ_TEAPOT1].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(193.0f, 120.0f, 11.0f));
	static_objects[OBJ_TEAPOT1].ModelMatrix[0] = glm::scale(static_objects[OBJ_TEAPOT1].ModelMatrix[0],
		glm::vec3(2.0f, 2.0f, 2.0f));

	static_objects[OBJ_TEAPOT1].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT1].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT1].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT1].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT1].material[0].exponent = 128.0f*0.6;

	// teapot2
	strcpy(static_objects[OBJ_TEAPOT2].filename, "Data/Teapotn_vn.geom");
	static_objects[OBJ_TEAPOT2].n_fields = 6;

	static_objects[OBJ_TEAPOT2].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_TEAPOT2]));

	static_objects[OBJ_TEAPOT2].n_geom_instances = 1;

	static_objects[OBJ_TEAPOT2].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 140.0f, 1.0f));
	static_objects[OBJ_TEAPOT2].ModelMatrix[0] = glm::scale(static_objects[OBJ_TEAPOT2].ModelMatrix[0],
		glm::vec3(3.0f, 3.0f, 3.0f));
	static_objects[OBJ_TEAPOT2].ModelMatrix[0] = glm::rotate(static_objects[OBJ_TEAPOT2].ModelMatrix[0], 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_TEAPOT2].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_TEAPOT2].material[0].ambient = glm::vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
	static_objects[OBJ_TEAPOT2].material[0].diffuse = glm::vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
	static_objects[OBJ_TEAPOT2].material[0].specular = glm::vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
	static_objects[OBJ_TEAPOT2].material[0].exponent = 128.0f*0.6;

	// new_chair1
	strcpy(static_objects[OBJ_NEW_CHAIR1].filename, "Data/new_chair_vnt.geom");
	static_objects[OBJ_NEW_CHAIR1].n_fields = 8;

	static_objects[OBJ_NEW_CHAIR1].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_CHAIR1]));

	static_objects[OBJ_NEW_CHAIR1].n_geom_instances = 1;

	static_objects[OBJ_NEW_CHAIR1].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 104.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR1].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_CHAIR1].ModelMatrix[0],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR1].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR1].ModelMatrix[0],
		180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR1].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR1].material[0].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR1].material[0].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR1].material[0].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR1].material[0].exponent = 128.0f*0.078125f;

	// new_chair2
	strcpy(static_objects[OBJ_NEW_CHAIR2].filename, "Data/new_chair_vnt.geom");
	static_objects[OBJ_NEW_CHAIR2].n_fields = 8;

	static_objects[OBJ_NEW_CHAIR2].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_CHAIR2]));

	static_objects[OBJ_NEW_CHAIR2].n_geom_instances = 1;

	static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 90.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0], 90*TO_RADIAN,glm::vec3(1.0f,0.0f, 0.0f));
	static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0],
		glm::vec3(0.8f, 0.8f, 0.8f));
	static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_CHAIR2].ModelMatrix[0],
		180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	static_objects[OBJ_NEW_CHAIR2].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR2].material[0].ambient = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_CHAIR2].material[0].diffuse = glm::vec4(0.5f, 0.5f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_CHAIR2].material[0].specular = glm::vec4(0.7f, 0.7f, 0.04f, 1.0f);
	static_objects[OBJ_NEW_CHAIR2].material[0].exponent = 128.0f*0.078125f;


	// frame
	strcpy(static_objects[OBJ_FRAME].filename, "Data/Frame_vn.geom");
	static_objects[OBJ_FRAME].n_fields = 6;

	static_objects[OBJ_FRAME].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_FRAME]));

	static_objects[OBJ_FRAME].n_geom_instances = 1;

	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(188.0f, 116.0f, 30.0f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::scale(static_objects[OBJ_FRAME].ModelMatrix[0],
		glm::vec3(0.6f, 0.6f, 0.6f));
	static_objects[OBJ_FRAME].ModelMatrix[0] = glm::rotate(static_objects[OBJ_FRAME].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_FRAME].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_FRAME].material[0].ambient = glm::vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
	static_objects[OBJ_FRAME].material[0].diffuse = glm::vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
	static_objects[OBJ_FRAME].material[0].specular = glm::vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
	static_objects[OBJ_FRAME].material[0].exponent = 128.0f*0.1f;


	// new_picture
	strcpy(static_objects[OBJ_NEW_PICTURE].filename, "Data/new_picture_vnt.geom");
	static_objects[OBJ_NEW_PICTURE].n_fields = 8;

	static_objects[OBJ_NEW_PICTURE].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_NEW_PICTURE]));

	static_objects[OBJ_NEW_PICTURE].n_geom_instances = 1;

	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(189.5f, 116.0f, 30.0f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::scale(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		glm::vec3(13.5f*0.6f, 13.5f*0.6f, 13.5f*0.6f));
	static_objects[OBJ_NEW_PICTURE].ModelMatrix[0] = glm::rotate(static_objects[OBJ_NEW_PICTURE].ModelMatrix[0],
		 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	static_objects[OBJ_NEW_PICTURE].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
	static_objects[OBJ_NEW_PICTURE].material[0].exponent = 128.0f*0.6f;

	// new_picture
	strcpy(static_objects[OBJ_COW1].filename, "Data/cow_vn.geom");
	static_objects[OBJ_COW1].n_fields = 6;

	static_objects[OBJ_COW1].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_COW1]));

	static_objects[OBJ_COW1].n_geom_instances = 1;

	static_objects[OBJ_COW1].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(215.0f, 100.0f, 9.5f));
	static_objects[OBJ_COW1].ModelMatrix[0] = glm::scale(static_objects[OBJ_COW1].ModelMatrix[0],
		glm::vec3(30.0f, 30.0f, 30.0f));
	static_objects[OBJ_COW1].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW1].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	static_objects[OBJ_COW1].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW1].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
 
	static_objects[OBJ_COW1].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW1].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW1].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW1].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW1].material[0].exponent = 0.21794872f*0.6f;

	// new_picture
	strcpy(static_objects[OBJ_COW2].filename, "Data/cow_vn.geom");
	static_objects[OBJ_COW2].n_fields = 6;

	static_objects[OBJ_COW2].front_face_mode = GL_CCW;
	prepare_geom_of_static_object(&(static_objects[OBJ_COW2]));

	static_objects[OBJ_COW2].n_geom_instances = 1;

	static_objects[OBJ_COW2].ModelMatrix[0] = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 140.0f, 11.0f));
	static_objects[OBJ_COW2].ModelMatrix[0] = glm::scale(static_objects[OBJ_COW2].ModelMatrix[0],
		glm::vec3(50.0f, 50.0f, 50.0f));
	static_objects[OBJ_COW2].ModelMatrix[0] = glm::rotate(static_objects[OBJ_COW2].ModelMatrix[0],
		90.0f*TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	static_objects[OBJ_COW2].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	static_objects[OBJ_COW2].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
	static_objects[OBJ_COW2].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
	static_objects[OBJ_COW2].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
	static_objects[OBJ_COW2].material[0].exponent = 0.21794872f*0.6f;

	n_static_objects = 11;
}




void draw_static_object(Object *obj_ptr, int instance_ID, int cam_idx) {
	glFrontFace(obj_ptr->front_face_mode);

	set_material(obj_ptr);//material qhsownrl!

	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * obj_ptr->ModelMatrix[instance_ID];
	ModelViewProjectionMatrix = ProjectionMatrix[cam_idx] * ModelViewMatrix[cam_idx];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	glUniform3f(loc_primitive_color, obj_ptr->material[instance_ID].diffuse.r,
		obj_ptr->material[instance_ID].diffuse.g, obj_ptr->material[instance_ID].diffuse.b);

	glBindVertexArray(obj_ptr->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3*obj_ptr->n_triangles);
	glBindVertexArray(0);
}

GLuint VBO_axes, VAO_axes;
GLfloat vertices_axes[6][3] = {
	{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } };

void define_axes(void) {  
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_axes), &vertices_axes[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

#define WC_AXIS_LENGTH		60.0f
void draw_axes(int cam_idx) {
	ModelViewMatrix[cam_idx] = glm::scale(ViewMatrix[cam_idx], glm::vec3(WC_AXIS_LENGTH, WC_AXIS_LENGTH, WC_AXIS_LENGTH));
	ModelViewProjectionMatrix = ProjectionMatrix[cam_idx] * ModelViewMatrix[cam_idx];
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(VAO_axes);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}
void draw_cctv() {
	glPointSize(10.0f);
	glEnable(GL_POINT_SMOOTH);
	
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glBindVertexArray(VAO_cctv);
	glUniform3fv(loc_primitive_color, 1, cctv_color[0]);
	glDrawArrays(GL_POINTS, 0, 1);

	glBindVertexArray(0);
}
void display_cctv(int cam_idx) {
	glm::mat4 ModelMatrix_cctv;
	ModelMatrix_cctv = glm::translate(glm::mat4(1.0f), glm::vec3(cam[CCTV1_VIEW].pos));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_cctv;
	
	draw_cctv();


	ModelMatrix_cctv = glm::translate(glm::mat4(1.0f), glm::vec3(cam[CCTV2_VIEW].pos));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_cctv;

	draw_cctv();

	ModelMatrix_cctv = glm::translate(glm::mat4(1.0f), glm::vec3(cam[CCTV3_VIEW].pos));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_cctv;

	draw_cctv();

	ModelMatrix_cctv = glm::translate(glm::mat4(1.0f), glm::vec3(cam[DYNAMIC_CCTV_VIEW].pos));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_cctv;

	draw_cctv();
}
void draw_main_cam(int cam_idx) {
	glm::mat4 ModelMatrix_main;
	ModelMatrix_main = glm::translate(glm::mat4(1.0f), glm::vec3(cam[MAIN_VIEW].pos));
	ModelViewProjectionMatrix = ViewProjectionMatrix[cam_idx] * ModelMatrix_main;

	glPointSize(10.0f);
	glEnable(GL_POINT_SMOOTH);

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glBindVertexArray(VAO_cctv);
	glUniform3fv(loc_primitive_color, 1, main_color[0]);
	glDrawArrays(GL_POINTS, 0, 1);

	glBindVertexArray(0);
}


#define N_TIGER_FRAMES 12
Object tiger[N_TIGER_FRAMES];
struct {
	int cur_frame = 0;
	float rotation_angle = 0.0f;
} tiger_data;

void define_animated_tiger(void) {
	for (int i = 0 ; i < N_TIGER_FRAMES ; i++) {
		sprintf(tiger[i].filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);

		tiger[i].n_fields = 8;
		tiger[i].front_face_mode = GL_CW;
		prepare_geom_of_static_object(&(tiger[i]));

		tiger[i].n_geom_instances = 1;

		tiger[i].ModelMatrix[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

		tiger[i].material[0].emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		tiger[i].material[0].ambient = glm::vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
		tiger[i].material[0].diffuse = glm::vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
		tiger[i].material[0].specular = glm::vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
		tiger[i].material[0].exponent = 128.0f*0.21794872f;

	}
}



int end_time = 1000;
float tiger_time = 0;
glm::vec3 curr_pos;
glm::vec3 tiger_route_pos[] = { glm::vec3(80.0f, 40.0f, 0.0f),glm::vec3(80.0f, 90.0f, 0.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(200.0f,90.0f,0.0f),
	glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(200.0f, 60.0f,0.0f),glm::vec3(-1.0f, -1.0f, -1.0f),glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(200.0f, 90.0f, 0.0f)
	,glm::vec3(-1.0f, -1.0f, -1.0f),glm::vec3(90.0f, 90.0f, 0.0f),glm::vec3(-1.0f, -1.0f, -1.0f),glm::vec3(80.0f, 40.0f, 0.0f) };
int tiger_dir = 0;
int turn_time = 0;

void draw_animated_tiger(int cam_idx) {

	if (tiger_dir == 0) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3(0.0f, tiger_time, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], (90.0f)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 1) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3(10.0f, 0.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(-10.0f, 0.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], (90)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 2){
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3( tiger_time-turn_time, 0.0f, 0.0f));
	}
	else if (tiger_dir == 3) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir]);
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(0.0f, 10.0f, 0));
	}
	else if (tiger_dir == 4) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] - glm::vec3(0.0f,tiger_time-turn_time, 0));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 5) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir]);
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(10.0f, 0.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_dir == 6) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir]+glm::vec3(0.0f,10.0f,0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(0.0f, -10.0f, 0));
	}
	else if (tiger_dir == 7) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3(0.0f, tiger_time - turn_time, 0));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 8) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir]);
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(10.0f, 0.0f, 0));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 9){
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] - glm::vec3( tiger_time - turn_time, 0.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 10) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3(0.0f, -10.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(0.0f, 10.0f, 0));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], 180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	}
	else if (tiger_dir == 11) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] - glm::vec3(0.0f, tiger_time - turn_time, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (tiger_dir == 12) {
		ModelMatrix[cam_idx] = glm::translate(glm::mat4(1), tiger_route_pos[tiger_dir] + glm::vec3(-1.0f, 0.0f, 0.0f));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -3 * (tiger_time - turn_time)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix[cam_idx] = glm::translate(ModelMatrix[cam_idx], glm::vec3(1.0f, 0.0f, 0));
		ModelMatrix[cam_idx] = glm::rotate(ModelMatrix[cam_idx], -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * ModelMatrix[cam_idx];


	ModelViewMatrix[cam_idx] = glm::rotate(ModelViewMatrix[cam_idx], 90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix[cam_idx] *= tiger[tiger_data.cur_frame].ModelMatrix[0];

	
	curr_pos.x = ModelMatrix[cam_idx][3][0]; 
	curr_pos.y = ModelMatrix[cam_idx][3][1]; 
	curr_pos.z = ModelMatrix[cam_idx][3][2];

	//change direction
	switch (tiger_dir) {
	case 0:
		if (curr_pos == tiger_route_pos[tiger_dir + 1] - glm::vec3(0.0f, 1.0f, 0.0f)) {
			tiger_dir++;
			turn_time = tiger_time;
		}
		break;
	case 3:
	case 1:
		if (-3 * (tiger_time - turn_time) <= -90) {
			tiger_dir++;
			tiger_route_pos[tiger_dir] = curr_pos;
			turn_time = tiger_time;
		}
		break;
	case 2:
		if (curr_pos.x >= tiger_route_pos[tiger_dir + 1].x) {
			tiger_dir++;
			turn_time = tiger_time;
			tiger_route_pos[tiger_dir].x = curr_pos.x;
		}
		break;
	case 4:
	case 11:
		if (curr_pos.y <= tiger_route_pos[tiger_dir + 1].y) {
			tiger_dir++;
			turn_time = tiger_time;
			tiger_route_pos[tiger_dir].y = curr_pos.y;
		}
		break;
	case 5:
		if (-3 * (tiger_time - turn_time) <= -270) {
			tiger_dir++;
			tiger_route_pos[tiger_dir] = curr_pos;
			turn_time = tiger_time;
		}
		break;
	case 6:
	case 8:
	case 10:
		if (3 * (tiger_time - turn_time) >= 90) {
			tiger_dir++;
			tiger_route_pos[tiger_dir] = curr_pos;
			turn_time = tiger_time;
		}
		break;
	case 7:
		if (curr_pos.y >= tiger_route_pos[tiger_dir + 1].y) {
			tiger_dir++;
			turn_time = tiger_time;
			tiger_route_pos[tiger_dir].y = curr_pos.y;
		}
		break;
	case 9:
		if (curr_pos.x <= tiger_route_pos[tiger_dir + 1].x) {
			tiger_dir++;
			turn_time = tiger_time;
			tiger_route_pos[tiger_dir] = curr_pos;
		}
		break;
	case 12:
		if (-3 * (tiger_time - turn_time) <= -180) {
			tiger_dir=0;
			turn_time = tiger_time;
			end_time = tiger_time;
		}
	}

	//set tiger
	glUniform4fv(loc_material.ambient_color, 1, vec_to_float(tiger[tiger_data.cur_frame].material[0].ambient));
	glUniform4fv(loc_material.diffuse_color, 1, vec_to_float(tiger[tiger_data.cur_frame].material[0].diffuse));
	glUniform4fv(loc_material.specular_color, 1, vec_to_float(tiger[tiger_data.cur_frame].material[0].specular));
	glUniform1f(loc_material.specular_exponent, tiger[tiger_data.cur_frame].material[0].exponent);
	glUniform4fv(loc_material.emissive_color, 1, vec_to_float(tiger[tiger_data.cur_frame].material[0].ambient));


	
	
	ModelViewProjectionMatrix = ProjectionMatrix[cam_idx] * ModelViewMatrix[cam_idx];
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix[cam_idx]));
	if (shader_mode == GS) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	else {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_PS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_PS, 1, GL_FALSE, &ModelViewMatrix[cam_idx][0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_PS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	}
	glUniform3f(loc_primitive_color, tiger[tiger_data.cur_frame].material[0].diffuse.r,
		tiger[tiger_data.cur_frame].material[0].diffuse.g, tiger[tiger_data.cur_frame].material[0].diffuse.b);

	glBindVertexArray(tiger[tiger_data.cur_frame].VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tiger[tiger_data.cur_frame].n_triangles);
	glBindVertexArray(0);

	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes(cam_idx);


	//glUseProgram(0);
}

void cleanup_OpenGL_stuffs(void) {
	for (int i = 0; i < n_static_objects; i++) {
		glDeleteVertexArrays(1, &(static_objects[i].VAO));
		glDeleteBuffers(1, &(static_objects[i].VBO));
	}

	for (int i = 0; i < N_TIGER_FRAMES; i++) {
		glDeleteVertexArrays(1, &(tiger[i].VAO));
		glDeleteBuffers(1, &(tiger[i].VBO));
	}

	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);
}