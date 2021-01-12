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

void sample_simple_3D()
{
	// 모델을 불러옴
	auto model = cvgl::loadModel("teapot.obj");

	// 셰이더를 불러오고 컴파일
	auto model_shader = cvgl::loadShader({
			cvgl::compileShader("simple.vs.glsl", GL_VERTEX_SHADER), 
			cvgl::compileShader("simple.fs.glsl", GL_FRAGMENT_SHADER)
		}
	);

	// 일반적으로 화면에 그냥 렌더링(default frame buffer)이 가능하지만
	// cv::namedWindow로 생성된 opencv의 opengl 윈도우는 그 기능을 지원X
	// 따라서 FBO(Frame Buffer Object)에 렌더링을 하고 렌더링된 텍스쳐를
	// cv::namedWindow로 렌더링하는 방식을 사용함

	cv::Mat background; // 카메라 입력 이미지

	// 배경을 렌더링할 셰이더
	auto back_shader = cvgl::loadShader(
		{
			cvgl::compileShader("background.vs.glsl", GL_VERTEX_SHADER),
			cvgl::compileShader("background.fs.glsl", GL_FRAGMENT_SHADER)
		}
	);
}

int main()
{
	// OpenGL 창을 만들고
	cv::namedWindow("gl_window", cv::WINDOW_OPENGL | cv::WINDOW_AUTOSIZE);
	// OpenGL 초기화 진행
	cvgl::initGL();

	// sample_simple_img();
	// sample_video();
	sample_simple_3D();
}