#include "cv-gl.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

cvgl::Model cvgl::loadModel(std::string_view model_path)
{
	cvgl::Model model;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;

	auto ret = tinyobj::LoadObj(
		&attrib, &shapes, &materials, &warn, &err, model_path.data()
	);

	if (!warn.empty()) {
		std::cerr << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (ret == false) {
		std::cerr << "failed loading OBJ : " << model_path << std::endl;
		std::terminate();
	}

	for (auto s = 0; s < shapes.size(); ++s) {
		auto index_offset = 0;

		for (auto f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			auto fv = shapes[s].mesh.num_face_vertices[f];

			for (auto v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				vertices.push_back({ vx, vy, vz });

				if (attrib.normals.size() != 0) {
					tinyobj::real_t	 nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t	 ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t	 nz = attrib.normals[3 * idx.normal_index + 2];

					normals.push_back({ nx, ny, nz });
				}
				if (attrib.texcoords.size() != 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

					texcoords.push_back({ tx, ty });
				}
			}
			index_offset += fv;
		}
	}

	glGenVertexArrays(1, &model.vao);
	glGenBuffers(1, &model.vertex_vbo);
	if (normals.size() != 0) {
		glGenBuffers(1, &model.normal_vbo);
		model.has_normal = true;
	}

	if (texcoords.size() != 0) {
		glGenBuffers(1, &model.texcoord_vbo);
		model.has_texcoord = true;
	}

	glBindVertexArray(model.vao);

	glBindBuffer(GL_ARRAY_BUFFER, model.vertex_vbo);
	glBufferData(
		GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
		vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	if (normals.size() != 0) {
		glBindBuffer(GL_ARRAY_BUFFER, model.normal_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
			normals.data(), GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	else if (texcoords.size() != 0) {
		glBindBuffer(GL_ARRAY_BUFFER, model.texcoord_vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2),
			vertices.data(), GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	model.sz = vertices.size();

	if (texcoords.size() != 0) glDisableVertexAttribArray(2);
	if (texcoords.size() != 0) glDisableVertexAttribArray(1);

	glDisableVertexAttribArray(0);

	glBindVertexArray(0);

	return model;
}

glm::mat4 cvgl::cvProj2glProj(float fx, float fy, float cx, float cy, float w, float h) {
	constexpr auto far = 100.f;
	constexpr auto near = 0.01f;
	glm::mat4 gl_proj{
		2.f * fx / w,	0.f,	1.f - (2.f * cx) / w,			0.f,
		0.f,	 2.f * fy / h,	-1.f + (2.f * cy) / h,			0.f,
		0.f,			0.f,  (-far - near) / (far - near), -2.f * far * near / (far - near),
		0.f,			0.f,			-1.f,				0.f
	};

	return glm::transpose(gl_proj);
}

glm::mat4 cvgl::cvRT2glRT(const cv::Mat& cvRT)
{
	static constexpr glm::mat4 convertCV2GL{
		1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	glm::mat4 glRT{
		cvRT.at<float>(0, 0), cvRT.at<float>(1, 0), cvRT.at<float>(2, 0), cvRT.at<float>(3, 0),
		cvRT.at<float>(0, 1), cvRT.at<float>(1, 1), cvRT.at<float>(2, 1), cvRT.at<float>(3, 1),
		cvRT.at<float>(0, 2), cvRT.at<float>(1, 2), cvRT.at<float>(2, 2), cvRT.at<float>(3, 2),
		cvRT.at<float>(0, 3), cvRT.at<float>(1, 3), cvRT.at<float>(2, 3), cvRT.at<float>(3, 3)
	};

	return convertCV2GL * glRT;
}