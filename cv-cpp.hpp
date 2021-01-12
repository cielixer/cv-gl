#pragma once

#include "common.hpp"

namespace std {
	template <typename Val_type>
	inline auto begin(cv::Mat& mat) { return mat.begin<Val_type>(); }

	template <typename Val_type>
	inline auto cbegin(const cv::Mat& mat) { return mat.begin<Val_type>(); }

	template <typename Val_type>
	inline auto end(cv::Mat& mat) { return mat.end<Val_type>(); }

	template <typename Val_type>
	inline auto cend(cv::Mat& mat) { return mat.end<Val_type>(); }
}