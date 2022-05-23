#pragma once
#include <vector>
#include <glm/glm.hpp>

class SSAO
{
public:
	SSAO();
	~SSAO();

	void GenerateSSAOKernel(unsigned int kernelSize);

private:
	std::vector<glm::vec3> kernelSSAO;
};

