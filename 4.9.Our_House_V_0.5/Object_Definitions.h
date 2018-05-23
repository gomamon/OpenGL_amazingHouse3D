
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

#define N_MAX_STATIC_OBJECTS		13
Object static_objects[N_MAX_STATIC_OBJECTS]; // allocage memory dynamically every time it is needed rather than using a static array
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
	glBufferData(GL_ARRAY_BUFFER, obj_ptr->n_triangles*n_bytes_per_triangle, obj_ptr->vertices, GL_STATIC_DRAW);

	compute_AABB(obj_ptr);
	free(obj_ptr->vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &(obj_ptr->VAO));
	glBindVertexArray(obj_ptr->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, obj_ptr->VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void define_static_objects(void) {
	
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

	ModelViewMatrix[cam_idx] = ViewMatrix[cam_idx] * obj_ptr->ModelMatrix[instance_ID];
	ModelViewProjectionMatrix = ProjectionMatrix[cam_idx] * ModelViewMatrix[cam_idx];
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glUniform3f(loc_primitive_color, obj_ptr->material[instance_ID].diffuse.r,
		obj_ptr->material[instance_ID].diffuse.g, obj_ptr->material[instance_ID].diffuse.b);

	glBindVertexArray(obj_ptr->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * obj_ptr->n_triangles);
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

	
	ModelViewProjectionMatrix = ProjectionMatrix[cam_idx] * ModelViewMatrix[cam_idx];



	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	
	glUniform3f(loc_primitive_color, tiger[tiger_data.cur_frame].material[0].diffuse.r,
		tiger[tiger_data.cur_frame].material[0].diffuse.g, tiger[tiger_data.cur_frame].material[0].diffuse.b);

	glBindVertexArray(tiger[tiger_data.cur_frame].VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tiger[tiger_data.cur_frame].n_triangles);
	glBindVertexArray(0);

	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes(cam_idx);



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