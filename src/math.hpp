#pragma once

#include<vector>
#include<iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Math {

	class vec {
	public:
		virtual float* toPtr() = 0;
	protected:
		std::vector<float> data;
	};

	class vec3 : public vec {
	public:
		vec3() {
			data.resize(3, 0.0f);
		}
		vec3(float x) {
			data.resize(3, x);
		}
		vec3(float x, float y, float z) {
			data.resize(3);
			data[0] = x; data[1] = y; data[2] = z;
		}
		vec3(const vec3& v) {
			data.assign(v.data.begin(), v.data.end());
		}

		float x() { return data[0]; }
		float y() { return data[1]; }
		float z() { return data[2]; }

		float* toPtr() {
			return data.data();
		}
		
		glm::vec3 toGlmVec() const {
			glm::vec3 res(data[0], data[1], data[2]);
			return res;
		}
		
		float squareNorm() const {
			return data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
		}

		float norm() const { 
			return sqrt(squareNorm()); 
		}

		void normalize() {
			float invNorm = 1.0f / norm();
			data[0] *= invNorm;
			data[1] *= invNorm;
			data[2] *= invNorm;
		}

		vec3 normalized() const {
			vec3 res(data[0], data[1], data[2]);
			res.normalize();
			return res;
		}

		friend std::ostream& operator<<(std::ostream& os, const vec3& v) {
			os << "(" << v.data[0] << " " << v.data[1] << ' ' << v.data[2] << ')';
			return os;
		};
	};
}


inline std::vector<float> toVec3f(float x, float y, float z)
{
	std::vector<float> res(3);
	res[0] = x;
	res[1] = y;
	res[2] = z;
	return res;
}

inline std::vector<float> toVec3f(const glm::vec3& vec)
{
	std::vector<float> res(3);
	res[0] = vec.x;
	res[1] = vec.y;
	res[2] = vec.z;
	return res;
}

inline glm::vec3 toGlmVec3(const std::vector<float>& vec)
{
	assert(vec.size() == 3);
	glm::vec3 res(vec[0], vec[1], vec[2]);
	return res;
}


