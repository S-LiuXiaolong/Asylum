// void Tessellate()
// void Tessellate(int count, int currentcurve)
// {
// 	OpenGLMesh* surface = nullptr;
// 	OpenGLEffect* tessellatesurfacemy = nullptr;
// 
// 	OpenGLVertexElement decl2[] = {
// 		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
// 		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
// 		{ 0xff, 0, 0, 0, 0 }
// 	};
// 
// 	// create surface
// 	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
// 		MYERROR("Could not create surface");
// 		return ;
// 	}
// 
// 	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
// 		MYERROR("Could not load compute shader");
// 		return ;
// 	}
// 
// 	CurveData& current = curves[currentcurve];
// 
// 	// update surface cvs
// 	Math::Vector4* surfacecvs = new Math::Vector4[NumControlVertices * NumControlVertices];
// 	GLuint index;
// 
// 	for (GLuint i = 0; i < NumControlVertices; ++i) {
// 		for (GLuint j = 0; j < NumControlVertices; ++j) {
// 			index = i * NumControlVertices + j;
// 
// 			surfacecvs[index][0] = current.controlpoints[i][0];
// 			surfacecvs[index][2] = current.controlpoints[j][0];
// 			surfacecvs[index][1] = (current.controlpoints[i][1] + current.controlpoints[j][1]) * 0.5f;
// 		}
// 	}
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());
// 
// // 	if (current.degree > 1) {
// // 		tessellatesurfacemy->SetInt("numVerticesU", numsegments + 1);
// // 		tessellatesurfacemy->SetInt("numVerticesV", numsegments + 1);
// // 	} else {
// // 		tessellatesurfacemy->SetInt("numVerticesU", NumControlVertices);
// // 		tessellatesurfacemy->SetInt("numVerticesV", NumControlVertices);
// // 	}
// 
// 	if (current.degree > 1) {
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// 	} else {
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
// 	}
// 
// 
// 	tessellatesurfacemy->SetInt("numControlPointsU", NumControlVertices);
// 	tessellatesurfacemy->SetInt("numControlPointsV", NumControlVertices);
// 	tessellatesurfacemy->SetInt("degreeU", current.degree);
// 	tessellatesurfacemy->SetInt("degreeV", current.degree);
// 	tessellatesurfacemy->SetFloatArray("knotsU", current.knots, NumControlVertices + current.degree + 1);
// 	tessellatesurfacemy->SetFloatArray("knotsV", current.knots, NumControlVertices + current.degree + 1);
// 	tessellatesurfacemy->SetFloatArray("weightsU", current.weights, NumControlVertices);
// 	tessellatesurfacemy->SetFloatArray("weightsV", current.weights, NumControlVertices);
// 	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], NumControlVertices * NumControlVertices);
// 
// 	tessellatesurfacemy->Begin();
// 	{
// 		glDispatchCompute(1, 1, 1);
// 	}
// 	tessellatesurfacemy->End();
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
// 
// 	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT|GL_ELEMENT_ARRAY_BARRIER_BIT);
// 
// 	int numSegmentsU = numSegBetweenKnotsU * (NumControlVertices - current.degree);
// 	int numSegmentsV = numSegBetweenKnotsV * (NumControlVertices - current.degree);
// 	if (current.degree > 1) {
// 		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// 	} else {
// //		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
// 	}
// 
// 	delete[] surfacecvs;
// 
// 	surfacegroup.push_back(surface);
// }

// void Tessellate(int count, int currentcurve)
// {
// 	auto& faceindex = chan[0];
// 
// 	OpenGLMesh* surface = nullptr;
// 	OpenGLEffect* tessellatesurfacemy = nullptr;
// 
// 	OpenGLVertexElement decl2[] = {
// 		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
// 		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
// 		{ 0xff, 0, 0, 0, 0 }
// 	};
// 
// 	// create surface
// 	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
// 		MYERROR("Could not create surface");
// 		return;
// 	}
// 
// 	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
// 		MYERROR("Could not load compute shader");
// 		return;
// 	}
// 
// 	// CurveData& current = curves[currentcurve];
// 
// 	// update surface cvs
// 	Math::Vector4* surfacecvs = new Math::Vector4[6 * 5];
// 	GLuint index;
// 
// 	for (GLuint i = 0; i < 6; ++i) {
// 		for (GLuint j = 0; j < 5; ++j) {
// 			index = i * 5 + j;
// 
// 			surfacecvs[index][0] = mesh_cp_vertices[faceindex[i][j] - 1].x;
// 			surfacecvs[index][2] = mesh_cp_vertices[faceindex[i][j] - 1].z;
// 			surfacecvs[index][1] = mesh_cp_vertices[faceindex[i][j] - 1].y;
// 		}
// 	}
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());
// 
// 	// 	if (current.degree > 1) {
// 	// 		tessellatesurfacemy->SetInt("numVerticesU", numsegments + 1);
// 	// 		tessellatesurfacemy->SetInt("numVerticesV", numsegments + 1);
// 	// 	} else {
// 	// 		tessellatesurfacemy->SetInt("numVerticesU", NumControlVertices);
// 	// 		tessellatesurfacemy->SetInt("numVerticesV", NumControlVertices);
// 	// 	}
// 
// // 	if (current.degree > 1) {
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// // 	}
// // 	else {
// // 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// // 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
// // 	}
// 
// 	tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// 	tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// 
// 
// 	tessellatesurfacemy->SetInt("numControlPointsU", 6);
// 	tessellatesurfacemy->SetInt("numControlPointsV", 5);
// 	tessellatesurfacemy->SetInt("degreeU", 2);
// 	tessellatesurfacemy->SetInt("degreeV", 2);
// 	tessellatesurfacemy->SetFloatArray("knotsU", &knotz[0], 6 + 2 + 1);
// 	tessellatesurfacemy->SetFloatArray("knotsV", &knoty[0], 5 + 2 + 1);
// 	tessellatesurfacemy->SetFloatArray("weightsU", weightz, 6);
// 	tessellatesurfacemy->SetFloatArray("weightsV", weighty, 5);
// 	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], 6 * 5);
// 
// 	tessellatesurfacemy->Begin();
// 	{
// 		glDispatchCompute(1, 1, 1);
// 	}
// 	tessellatesurfacemy->End();
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
// 
// 	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);
// 
// 	int numSegmentsU = numSegBetweenKnotsU * (6 - 2);
// 	int numSegmentsV = numSegBetweenKnotsV * (5 - 2);
// // 	if (current.degree > 1) {
// // 		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// // 	}
// // 	else {
// // 		//		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
// // 	}
// 	surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// 
// 	delete[] surfacecvs;
// 
// 	surfacegroup.push_back(surface);
// }