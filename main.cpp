#include "cv-cpp.hpp"
#include "cv-gl.hpp"

void sample_simple_img()
{
	// 이미지 읽기 및 렌더링 (CPU)
	auto test = cv::imread("lena.jpg");
	cv::imshow("cv_window", test);

	// CPU 이미지를 GPU(cv::ogl::Texture2D)로 upload 해서
	// 앞서 생성한 OpenGL window ("gl_window")로 렌더링
	// CPU -> GPU : upload
	// GPU -> CPU : download
	auto gl_test = cv::ogl::Texture2D(test);
	cv::imshow("gl_window", gl_test);

	cv::waitKey(0);
}

void sample_video()
{
	cv::VideoCapture vid;
	vid.open(0);

	cv::Mat frame;
	cv::ogl::Texture2D gl_tex;
	while (cv::waitKey(1) != 27) {
		// 일반적인 opencv 방식(CPU)
		frame.release();
		vid >> frame;
		cv::imshow("cv-frame", frame);

		// cv::Mat(CPU) -> cv::ogl::Texture2D(GPU)로 복사
		gl_tex.copyFrom(frame);
		cv::imshow("gl_window", gl_tex);
	}
}

void sample_simple_3D(const cv::Size& win_sz)
{
	// 모델을 불러옴
	auto model_mesh = cvgl::loadModel("teapot.obj");

	// 셰이더를 불러오고 컴파일
	auto model_shader = cvgl::Shader("shader/simple.vs.glsl", "shader/simple.fs.glsl");

	// rendering 할 3D 객체는 depth testing을 해야 제대로 렌더링 됨
	// 해당 부분이 렌더링해야 하는 부분인지 아닌지 알아야 하므로 RGBA를 사용
	auto model_fbo = cvgl::FBO(win_sz, cvgl::FBO::cv_Tex2D_Format::RGBA, true);

	// 일반적으로 화면에 그냥 렌더링(default frame buffer)이 가능하지만
	// cv::namedWindow로 생성된 opencv의 opengl 윈도우는 그 기능을 지원X
	// 따라서 FBO(Frame Buffer Object)에 렌더링을 하고 렌더링된 텍스쳐를
	// cv::namedWindow로 렌더링하는 방식을 사용함

	cv::Mat background; // 카메라 입력 이미지
	cv::ogl::Texture2D back_texture; // OpenGL용 텍스쳐

	// 함성 이미지를 위한 VAO
	// vertex shader에 이미지 꼭지점들의 위치를 박아 넣어서
	// buffer를 통한 렌더링(VBO)은 하지 않아도 되지만
	// vao를 생성하고 활성화 시키긴 해야 함
	GLuint synthesis_vao;
	glGenVertexArrays(1, &synthesis_vao);

	// 3D 객체와 background를 합치는 shader
	auto synthesis_shader = cvgl::Shader("shader/synthesis.vs.glsl", "shader/synthesis.fs.glsl");

	// 합성 이미지를 그릴 타겟 FBO
	auto synthesis_fbo = cvgl::FBO(win_sz);

	// projection matrix
	auto projection_mat = cvgl::cvProj2glProj(480, 480, 320, 240, 320, 240);

	// view matrix
	// 원래는 opencv로 구성한 [R|T]를 cvgl::cvRT2glRT 함수로 변환해야 함
	auto view_mat = glm::lookAt(
		glm::vec3(0.f, 5.f, 8.f),
		glm::vec3(2.f, -4.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f)
	);

	// model_mat은 렌더링할 모델의 크기, 위치, 회전을 정의함
	// glm::translate, glm::rotate, glm::scale을 사용해야 함
	// 아래는 teapot이 너무 커서... 반으로 스케일을 줄임
	// 참고로 teapot.obj를 소스코드랑 같은 위치에 두고, VS 탐색기에 추가시켜 놓으면
	// c/c++ 코드가 컴파일된 바이너리인 *.obj랑 확장자가 겹쳐 컴파일 할때
	// 3D 모델로 안읽고 컴파일된 바이너리인 obj로 읽어서 에러가 남!
	auto model_mat = glm::mat4(1.f);
	model_mat = glm::scale(model_mat, glm::vec3(0.5f));

	cv::VideoCapture vid;
	vid.open(0);
	while (cv::waitKey(1) != 27) {
		// 카메라 입력 영상 (cv::Mat)
		background.release(); vid >> background;

		// cv::Mat -> cv::ogl::Texture2D 복사
		back_texture.release(); back_texture.copyFrom(background);
		
		// Default FBO 색상 버퍼 초기화
		// glClear(GL_COLOR_BUFFER_BIT);

		model_fbo.bind(); // 모델을 그릴 FBO 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 3D 오브젝트일 경우 depth에 따른 렌더링을 해야 하므로
		// Depth(깊이) 정보 테스팅을 활성화 시켜야 함
		// 다만 사용하고 있는 fbo를 생성할 때 깊이 정보를 확성화 해야 함
		// cvgl::FBO(..)의 4번째 파라미터 use_depth = true
		glEnable(GL_DEPTH_TEST);

		auto prog_id = model_shader.bind(); // 모델을 그릴 shader를 bind
		{
			glUniformMatrix4fv(
				glGetUniformLocation(prog_id, "proj_mat"),
				1, GL_FALSE, glm::value_ptr(projection_mat)
			);

			glUniformMatrix4fv(
				glGetUniformLocation(prog_id, "view_mat"),
				1, GL_FALSE, glm::value_ptr(view_mat)
			);

			glUniformMatrix4fv(
				glGetUniformLocation(prog_id, "model_mat"),
				1, GL_FALSE, glm::value_ptr(model_mat)
			);

			render(model_mesh);
		}
		model_shader.unbind();

		model_fbo.unbind();

		// 만약 디버깅 용도로 fbo가 렌더링한 이미지를 opengl 렌더링이 아닌
		// opencv로 확인해 보고 싶다면, 아래와 같이 cv::Mat 만들어서 복사 해넣어야 함
		// 참고로 좌표계 시스템이 달라서 cv::flip으로 상하를 뒤집어야 함
		// 또한 rgb로 렌더링했으니 bgr로 바꾸는 과정이 필요할 수 있음
		if (false) {
			cv::Mat test;
			model_fbo.getTargetTexture().copyTo(test);
			cv::flip(test, test, 0);
			cv::imshow("test", test);
		}
		
		synthesis_fbo.bind();
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(synthesis_vao);
		prog_id = synthesis_shader.bind();
		{
			// 셰이더에 텍스쳐를 여러개 전송해야 할 땐 glActiveTexture로
			// 텍스쳐 마다 번호를 지정해줌
			glActiveTexture(GL_TEXTURE0); // 0번 활성화
			back_texture.bind(); // 활성화된 0번에 바인딩
			glUniform1i(
				glGetUniformLocation(prog_id, "back_tex"),
				0 // 0번을 전달
			);

			glActiveTexture(GL_TEXTURE1); // 1번 활성화
			model_fbo.getTargetTexture().bind(); // 활성화된 1번에 바인딩
			glUniform1i(
				glGetUniformLocation(prog_id, "model_tex"),
				1 // 1번을 전달
			);

			glUniform1i( // 상하 반전 활성화
				glGetUniformLocation(prog_id, "flip_obj_y"),
				1
			);

			glUniform1i( // RGB 반전 활성화
				glGetUniformLocation(prog_id, "flip_obj_rgb"),
				1
			);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // 그림
		}
		synthesis_fbo.unbind();

		// 마찬가지로 디버깅 용도
		if (true) {
			cv::Mat render_result;
			synthesis_fbo.getTargetTexture().copyTo(render_result);
			cv::imshow("render_result", render_result);

			// 현재 확인된 에러...
			/*
			* 복잡한 렌더링 과정을 거치고 cv::imshow("gl_window", ...)로
			* 생성된 opengl window에 렌더링을 하게 되면 이상한 에러가 발생하면서
			* 렌더링이 되지 않음...
			* 
			* Texture를 여러개 바꾸면서 opengl에 바인딩된 텍스쳐와 opencv가 알고있는
			* binding 정보가 다르거나...
			* 
			* opengl로 렌더링해야 할 opengl context가 제대로 switching되지 않거나
			* 
			* fbo가 바뀔 때 제대로 안바뀐다던지...
			* 
			* GLFW3같이 opengl로 렌더링하는 창을 따로 띄운다면 해결되는 문제임
			*/
		}
	}
}

int main()
{
	// OpenGL 창을 만들고
	cv::namedWindow("gl_window", cv::WINDOW_OPENGL | cv::WINDOW_AUTOSIZE);
	// OpenGL 초기화 진행
	cvgl::initGL();

	// sample_simple_img();
	// sample_video();
	sample_simple_3D({ 640, 480 });
}