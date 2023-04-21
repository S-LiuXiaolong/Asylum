#include <iostream>
#include <sstream>
#include <fstream>
#include <io.h>
#include <queue>

std::vector<bool> IntervalMerging(std::vector<float> onelineRho)
{
	// FIXME: elementRhoThreshold is 0.5 now
	float elementRhoThreshold = 0.5f;

	int nel = onelineRho.size();
	std::vector<bool> isBoundOneLine(nel);
	for (int indexLeft = 0; indexLeft < onelineRho.size();)
	{
		int indexRight = indexLeft;
		if (onelineRho[indexLeft] < elementRhoThreshold)
		{
			indexLeft++;
			continue;
		}
		else
		{
			while (onelineRho[indexRight] >= elementRhoThreshold)
			{
				indexRight++;
				if (indexRight == onelineRho.size())
					break;
			}
			isBoundOneLine[indexLeft] = true;
			isBoundOneLine[indexRight - 1] = true;

			indexLeft = indexRight;
		}
	}
	return isBoundOneLine;
}

int main(int argc, char *argv[])
{

	float elementRhoThreshold = 0.5f;

	std::vector<float> onelineRhoTowardY = {0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};
	int nely = onelineRhoTowardY.size();
	auto isBoundOneLine = IntervalMerging(onelineRhoTowardY);
	// int indexLeft = 0, indexRight = 0;


}
