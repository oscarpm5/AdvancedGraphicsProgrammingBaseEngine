#include "SSAO.h"
#include<random>

void SSAO::GenerateSSAOKernel(unsigned int kernelSize)
{
	kernelSSAO.clear();

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	for (unsigned int i = 0; i < kernelSize; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator));

		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		kernelSSAO.push_back(sample);
	}
}
