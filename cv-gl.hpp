#pragma once

#include "common.hpp"

namespace cvgl {
	inline auto initGL() {
		if (glewInit() != GLEW_OK) {
			std::cerr << "[GLEW] Initialization failed!" << std::endl;
			std::cerr << "\t needs GL context before initGL()" << std::endl;
			std::cerr << "\t\t ex) cv::namedWindow(.., cv::WINDOW_OPENGL)" << std::endl;
			std::terminate();
		}
	}

	inline auto compileShader(std::string_view path, int type)
	{
		auto shaderID = glCreateShader(type);
		auto full_data = read_all_from_file(path);
		auto raw_data = full_data.c_str();

		std::cout << "Compileing Shader : " << path << std::endl;
		glShaderSource(shaderID, 1, &raw_data, nullptr);
		glCompileShader(shaderID);

		GLint result = GL_FALSE; int logLength;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

		if (result == GL_FALSE) {
			char* error_log = new char[logLength];
			glGetShaderInfoLog(shaderID, logLength, nullptr, error_log);
			std::cerr << "[Shader] ERROR! : Failed to compile shader" << std::endl;
			std::cerr << error_log << std::endl;
			delete[] error_log;
			assert(false);
		}

		return shaderID;
	}
	inline auto loadShader(std::initializer_list<GLuint> shader_list)
	{
		auto program = glCreateProgram();

		for (const auto& shader_id : shader_list)
		{
			glAttachShader(program, shader_id);
		}

		glLinkProgram(program);
		GLint result, logLength;

		glGetProgramiv(program, GL_LINK_STATUS, &result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		if (result == GL_FALSE) {
			auto error_log = new char[logLength];
			glGetProgramInfoLog(program, logLength, nullptr, error_log);
			std::cerr << "[Shader] ERROR! : Failed to link shaders" << std::endl;
			std::cerr << error_log << std::endl;
			delete[] error_log;
			std::terminate();
		}

		for (const auto& shader : shader_list) {
			glDetachShader(program, shader);
			glDeleteShader(shader);
		}

		return program;
	}

	class Shader {
	public:
		inline Shader(std::string_view vs_path, std::string_view fs_path) {
			this->m_program = loadShader({
					compileShader(vs_path, GL_VERTEX_SHADER),
					compileShader(fs_path, GL_FRAGMENT_SHADER)
				}
			);
		}
		virtual ~Shader() {
			glDeleteProgram(m_program);
		}

		inline auto bind() {
			glUseProgram(m_program);
		}

		inline auto unbind() {
			glUseProgram(0);
		}

	private:
		GLuint m_program;
	};

	class FBO {
	public:
		using cv_Tex2D_Format = cv::ogl::Texture2D::Format;
		inline FBO(const cv::Size& sz, cv_Tex2D_Format format = cv_Tex2D_Format::RGB, bool use_depth = false) : m_size(sz) {
			glGenFramebuffers(1, &m_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

			m_target = cv::ogl::Texture2D(sz, format);
			m_target.bind();
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_target.texId(), 0);

			if (use_depth == true) {
				m_depth = cv::ogl::Texture2D(sz, cv_Tex2D_Format::DEPTH_COMPONENT);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth.texId(), 0);
			}

			GLenum draw_buffer[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, draw_buffer);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Framebuffer error!" << std::endl;
				std::terminate();
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		virtual ~FBO() {
			glDeleteFramebuffers(1, &m_fbo);
		}

#pragma region deleted
		FBO(const FBO&) = delete;
		FBO(FBO&&) = delete;

		FBO& operator=(const FBO&) = delete;
		FBO& operator=(FBO&&) = delete;
#pragma endregion

	public:
		inline void bind() {
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			glViewport(0, 0, m_size.width, m_size.height);
			m_target.bind();
		}

		inline void unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		inline auto& getTargetTexture() { return m_target; }
		inline auto& getDepthTexture() { return m_depth; }

	private:

	private:
		GLuint m_fbo;
		cv::Size m_size;

		cv::ogl::Texture2D m_target;
		cv::ogl::Texture2D m_depth;
	};

	struct Model {
		GLuint vao;
		GLuint vertex_vbo;
		GLuint normal_vbo;		bool has_normal = false;
		GLuint texcoord_vbo;	bool has_texcoord = false;

		size_t sz;
	};

	cvgl::Model loadModel(std::string_view model_path);

	glm::mat4 cvProj2glProj(float fx, float fy, float cx, float cy, float w, float h);

	glm::mat4 cvRT2glRT(const cv::Mat& cvRT);
}