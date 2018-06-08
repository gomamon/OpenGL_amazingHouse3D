#version 400

struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;
	vec4 light_attenuation_factors; // compute this effect only if .w != 0.0f
	vec3 spot_direction;
	float spot_exponent;
	float spot_cutoff_angle;
	bool light_on;
};

struct MATERIAL {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 emissive_color;
	float specular_exponent;
};

uniform vec4 u_global_ambient_color;
#define NUMBER_OF_LIGHTS_SUPPORTED 4
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];
uniform MATERIAL u_material;

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
layout (location = 0) out vec4 final_color;

vec4 lighting_equation(in vec3 P_EC, in vec3 N_EC) {
	vec4 color_sum;
	float local_scale_factor, tmp_float; 
	vec3 L_EC;

	color_sum = u_material.emissive_color + u_global_ambient_color * u_material.ambient_color;
 
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue; //i번째 광원이 꺼져있으면 pass

		local_scale_factor = one_f;
		if (u_light[i].position.w != zero_f) { // point light source    point(0)와 direction(1)을 w를 보고 결정
			L_EC = u_light[i].position.xyz - P_EC.xyz;  //P_EC -> a.lignt[i].point.xyz xyz벡터

			if (u_light[i].light_attenuation_factors.w  != zero_f) {    //light attenuation 효과를 줄지말지 결정
				vec4 tmp_vec4;
				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);
				tmp_vec4.y = sqrt(tmp_vec4.z);
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f/dot(tmp_vec4, u_light[i].light_attenuation_factors); //atti = (1.0f, d, d^2,0.0f) dot (k0i, k1i, k2i, *)
			}

			L_EC = normalize(L_EC);

			if (u_light[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				float spot_cutoff_angle = clamp(u_light[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light[i].spot_direction);   //spot direction 정규화

				tmp_float = dot(-L_EC, spot_dir);   //cos(alpha)
				if (tmp_float >= cos(radians(spot_cutoff_angle))) {
					tmp_float = pow(tmp_float, u_light[i].spot_exponent);
				}   //숙제에서 한거ㅓ
				else //거리가 무한대이기 떄문에
					tmp_float = zero_f;
				local_scale_factor *= tmp_float;    //(1.0)
			}
		}
		else {  // directional light source
			L_EC = normalize(u_light[i].position.xyz);
		}	

		if (local_scale_factor > zero_f) {				
			vec4 local_color_sum = u_light[i].ambient_color * u_material.ambient_color;

			tmp_float = dot(N_EC, L_EC);
			if (tmp_float > zero_f) {
				local_color_sum += u_light[i].diffuse_color*u_material.diffuse_color*tmp_float;
			
				vec3 H_EC = normalize(L_EC - normalize(P_EC));  //하프웨이 벡터   -P는 p->눈
				tmp_float = dot(N_EC, H_EC); 
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].specular_color
										   *u_material.specular_color*pow(tmp_float, u_material.specular_exponent);     //specualry reflextion
				}
			}
			color_sum += local_scale_factor*local_color_sum;
		}
	}
 	return color_sum;
}

void main(void) {   
	// final_color = vec4(gl_FragCoord.x/800.0f, gl_FragCoord.y/800.0f, 0.0f, 1.0f); // what is this?
    // final_color = vec4(0.0f,  0.0f, 1.0 - gl_FragCoord.z/1.0f, 1.0f); // what is this?

   final_color = lighting_equation(v_position_EC, normalize(v_normal_EC)); // for normal rendering
}
