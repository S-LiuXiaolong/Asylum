#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>
#include <sstream>
#include <fstream>
#include <io.h>
#include <queue>

#include "gl4ext.h"

#define DEGREE 2

struct NURBSLayerData
{
	std::vector<std::vector<std::vector<uint32_t>>> cptsIndex;
	std::vector<std::vector<std::vector<float>>> weights;
	std::vector<float> knotU, knotV, knotW;

	std::vector<std::vector<std::vector<float>>> LayerRho;
};

NURBSLayerData mesh_layers[3];

void Tessellate();

std::vector<Math::Vector3> mesh_cp_vertices;
std::vector<float> mesh_cp_weights;
uint32_t nelx, nely, nelz;
uint32_t numCptx, numCpty, numCptz;
std::vector<float> knotx, knoty, knotz;
std::vector<std::vector<std::vector<uint32_t>>> chan;
std::vector<std::vector<std::vector<std::vector<float>>>> meshes_rho;

// TODO: Maybe put these functions into another utility file?
void read_float(std::string strFile, std::vector<float>& buffer);
void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer);

int GetFileNum(const std::string& inPath)
{
	int fileNum = 0;

	std::vector<std::string> pathVec;
	std::queue<std::string> q;
	q.push(inPath);

	while (!q.empty())
	{
		std::string item = q.front(); q.pop();

		std::string path = item + "\\*";
		struct _finddata_t fileinfo;
		auto handle = _findfirst(path.c_str(), &fileinfo);
		if (handle == -1) continue;

		while (!_findnext(handle, &fileinfo))
		{
			if (fileinfo.attrib & _A_SUBDIR)
			{
				if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)continue;
				q.push(item + "\\" + fileinfo.name);
			}
			else
			{
				fileNum++;
				pathVec.push_back(item + "\\" + fileinfo.name);
			}
		}
		_findclose(handle);
	}

	return fileNum;
}


void build_mesh()
{
	// Get all coords of points.
	std::vector<float> buffer_cpts;
	read_float("../../../Asset/matlab_small/controlPts.bin", buffer_cpts);

	mesh_cp_vertices.resize(buffer_cpts.size() / 3);

	for (int i = 0; i < buffer_cpts.size() / 3; i++)
	{
		mesh_cp_vertices[i] = { buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2] };
	}

	// Get the nels(but what is nel?) and numControlPts. numCpts = nel + DEGREE.
	std::vector<uint32_t> buffer_nels;
	read_uint32t("../../../Asset/matlab_small/nels.bin", buffer_nels);
	nelx = buffer_nels[0]; nely = buffer_nels[1]; nelz = buffer_nels[2];
	numCptx = buffer_nels[0] + DEGREE; numCpty = buffer_nels[1] + DEGREE; numCptz = buffer_nels[2] + DEGREE;

	// Get all weights(same size as the controlPts) from binary file.
	read_float("../../../Asset/matlab_small/weights.bin", mesh_cp_weights);

	// Get xyz knots from binary file.
	std::vector<float> buffer_knots;
	read_float("../../../Asset/matlab_small/knots.bin", buffer_knots);
	int rowLength = buffer_knots.size() / 3;
	
	auto knotxBegin = buffer_knots.begin(); auto knotxEnd = buffer_knots.begin() + numCptx + 2 + 1;
	auto knotyBegin = buffer_knots.begin() + rowLength; auto knotyEnd = buffer_knots.begin() + rowLength + numCpty + 2 + 1;
	auto knotzBegin = buffer_knots.begin() + rowLength * 2; auto knotzEnd = buffer_knots.begin() + rowLength * 2 + numCptz + 2 + 1;
	knotx.assign(knotxBegin, knotxEnd);
	knoty.assign(knotyBegin, knotyEnd);
	knotz.assign(knotzBegin, knotzEnd);

	// Get chan(but what is chan?) from binary file.
	std::vector<uint32_t> buffer_chan;
	read_uint32t("../../../Asset/matlab_small/chan.bin", buffer_chan);
	for (int i = 0; i < numCptx; i++)
	{
		std::vector<std::vector<uint32_t>> face;
		for (int j = 0; j < numCptz; j++)
		{
			std::vector<uint32_t> line;
			for (int k = 0; k < numCpty; k++)
			{
				line.push_back(buffer_chan[i * numCptz * numCpty + j * numCpty + k]);
			}
			face.push_back(line);
		}
		chan.push_back(face);
	}

	int numRhoFiles = GetFileNum("../../../Asset/matlab_small/rho");
	meshes_rho.resize(numRhoFiles);
	for(int s = 0; s < numRhoFiles; s++)
	{
		std::vector<float> buffer_rho;
		std::string path = "../../../Asset/matlab_small/rho/rho_" + std::to_string(s + 1) + ".bin";
		read_float(path, buffer_rho);
		for (int i = 0; i < nelx; i++)
		{
			std::vector<std::vector<float>> face;
			for (int j = 0; j < nelz; j++)
			{
				std::vector<float> line;
				for (int k = 0; k < nely; k++)
				{
					line.push_back(buffer_rho[i * nelz * nely + j * nely + k]);
				}
				face.push_back(line);
			}
			meshes_rho[s].push_back(face);
		}

		std::vector<std::vector<std::vector<bool>>> isBound(nelx, std::vector<std::vector<bool>>(nelz, std::vector<bool>(nely, 0)));
		auto& mesh_rho = meshes_rho[s];

		for (int i = 0; i < nelx; i++)
		{
			for (int j = 0; j < nelz; j++)
			{
				std::vector<float> onelineRhoTowardY;
				for (int k = 0; k < nely; k++)
				{
					onelineRhoTowardY.push_back(mesh_rho[i][j][k]);
				}

			}
		}

		for (int i = 0; i < nelx; i++)
		{
			for (int j = 0; j < nely; j++)
			{
				std::vector<float> onelineRhoTowardZ;
				for (int k = 0; k < nelz; k++)
				{
					onelineRhoTowardZ.push_back(mesh_rho[i][k][j]);
				}
				
			}
		}

		for (int i = 0; i < nelz; i++)
		{
			for (int j = 0; j < nely; j++)
			{
				std::vector<float> onelineRhoTowardX;
				for (int k = 0; k < nelx; k++)
				{
					onelineRhoTowardX.push_back(mesh_rho[k][i][j]);
				}

			}
		}


	}

}

void read_float(std::string strFile, std::vector<float>& buffer)
{
	float temp;
	std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
		return;
	}

	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

	while (infile.read((char*)&temp, sizeof(float)))
	{
		int readedBytes = infile.gcount();
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}
}

void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer)
{
	uint32_t temp;
	std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
		return;
	}

	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

	while (infile.read((char*)&temp, sizeof(uint32_t)))
	{
		int readedBytes = infile.gcount();
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}
}

int main(int argc, char* argv[])
{
	build_mesh();
}
