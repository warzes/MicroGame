#include "stdafx.h"
#include "Core.h"
#include "Platform.h"
#include "oRenderer.h"

extern glm::vec3 ClearColor;
extern float perspectiveFOV;
extern float perspectiveNear;
extern float perspectiveFar;
extern glm::mat4 projectionMatrix;
namespace
{
	unsigned currentVAO = 0;
	FrameBuffer* currentFrameBuffer = nullptr;
}

inline GLenum translate(PrimitiveDraw p)
{
	switch (p)
	{
	case PrimitiveDraw::Lines:     return GL_LINES;
	case PrimitiveDraw::Triangles: return GL_TRIANGLES;
	case PrimitiveDraw::Points:    return GL_POINTS;
	}
	return 0;
}

inline GLenum translate(VertexAttributeTypeRaw p)
{
	switch (p)
	{
	case VertexAttributeTypeRaw::Float:     return GL_FLOAT;
	}
	return 0;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_id > 0) Destroy();
	if (!vbo || attribs.empty()) return false;

	m_ibo = ibo;
	m_vbo = vbo;

	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);

	vbo->Bind();
	for (size_t i = 0; i < attribs.size(); i++)
	{
		const auto& att = attribs[i];
		glEnableVertexAttribArray(static_cast<GLuint>(i));
		glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
	}
	m_attribsCount = attribs.size();

	if (m_ibo) m_ibo->Bind();

	glBindVertexArray(currentVAO);
	return true;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs, const std::vector<VertexAttributeRaw>& instanceAttribs)
{
	if (!Create(vbo, ibo, attribs))
		return false;

	SetInstancedBuffer(instanceBuffer, instanceAttribs);
	return true;
}

bool VertexArrayBuffer::Create(const std::vector<VertexBuffer*>& vbo, IndexBuffer* ibo, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_id > 0) Destroy();
	if (vbo.size() != attribs.size()) return false;

	m_ibo = ibo;
	m_vbo = vbo[0];

	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);

	for (size_t i = 0; i < vbo.size(); i++)
	{
		const auto& att = attribs[i];
		glEnableVertexAttribArray(static_cast<GLuint>(i));
		vbo[i]->Bind();
		glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, att.stride, att.pointer);
	}
	if (m_ibo) m_ibo->Bind();

	glBindVertexArray(currentVAO);
	return true;
}

bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, ShaderProgram* shaders)
{
	if (!shaders || !shaders->IsValid()) return false;

	auto attribInfo = shaders->GetAttribInfo();
	if (attribInfo.empty()) return false;

	size_t stride = 0;
	size_t offset = 0;
	std::vector<VertexAttributeRaw> attribs(attribInfo.size());
	for (int i = 0; i < attribInfo.size(); i++)
	{
		if (attribInfo[i].location != i)
		{
			// TODO: сделать возможность указания локации атрибутов. сейчас подразумевается что первый атрибут - 0, второй - 1 и о порядку
			LogError("Shader attribute location: " + std::to_string(attribInfo[i].location) + " not support!");
			return false;
		}

		size_t sizeType = 0;

		attribs[i].normalized = false;
		switch (attribInfo[i].typeId)
		{
		case GL_FLOAT:
			attribs[i].size = 1;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC2:
			attribs[i].size = 2;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC3:
			attribs[i].size = 3;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;
		case GL_FLOAT_VEC4:
			attribs[i].size = 4;
			attribs[i].type = VertexAttributeTypeRaw::Float;
			sizeType = attribs[i].size * sizeof(float);
			break;

		default:
			LogError("Shader attribute type: " + attribInfo[i].typeName + " not support!");
			return false;
		}

		attribs[i].pointer = (void*)+offset;
		offset += sizeType;
	}
	for (int i = 0; i < attribs.size(); i++)
	{
		attribs[i].stride = offset;
	}

	return Create(vbo, ibo, attribs);
}

void VertexArrayBuffer::Destroy()
{
	if (m_id > 0 && currentVAO == m_id)
	{
		if (m_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		currentVAO = 0;
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_id);
	}
	m_id = 0;
	m_instanceBuffer = nullptr;
}

void VertexArrayBuffer::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttributeRaw>& attribs)
{
	if (m_instanceBuffer == instanceBuffer) return;

	m_instanceBuffer = instanceBuffer;

	while (m_instancedAttribsCount > m_attribsCount)
	{
		glDisableVertexAttribArray(m_instancedAttribsCount);
		m_instancedAttribsCount--;
	}
	m_instancedAttribsCount = m_attribsCount;

#if 0
	// TODO: сейчас это под матрицы, надо как-то сделать чтобы под другие типы тоже
	{
		glBindVertexArray(m_id);
		instanceBuffer->Bind();
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
		m_instancedAttribsCount++;
		glEnableVertexAttribArray(m_instancedAttribsCount);
		glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
		glVertexAttribDivisor(m_instancedAttribsCount, 1);
	}
#else
	{
		glBindVertexArray(m_id);
		instanceBuffer->Bind();

		for (size_t i = 0; i < attribs.size(); i++)
		{
			m_instancedAttribsCount = m_instancedAttribsCount + i;
			const auto& att = attribs[i];

			if (att.type == VertexAttributeTypeRaw::Matrix4)
			{
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)0);
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
				m_instancedAttribsCount++;
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
			}
			else
			{
				// TODO: проверить
				glEnableVertexAttribArray(m_instancedAttribsCount);
				glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
				glVertexAttribDivisor(m_instancedAttribsCount, 1);
			}
		}
	}
#endif

	glBindVertexArray(currentVAO);
}

void VertexArrayBuffer::Draw(PrimitiveDraw primitive, uint32_t instanceCount)
{
	if (instanceCount == 0) return;

	if (currentVAO != m_id)
	{
		currentVAO = m_id;
		glBindVertexArray(m_id);
		m_vbo->Bind();
		if (m_ibo) m_ibo->Bind();
	}

	if (m_instanceBuffer)
	{
		if (instanceCount == 1 || instanceCount > m_instanceBuffer->GetVertexCount())
			instanceCount = m_instanceBuffer->GetVertexCount();
	}

	if (m_ibo)
	{
		const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

		if (instanceCount > 1)
			glDrawElementsInstanced(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr, instanceCount);
		else
			glDrawElements(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr);

	}
	else
	{
		if (instanceCount > 1)
			glDrawArraysInstanced(translate(primitive), 0, m_vbo->GetVertexCount(), instanceCount);
		else
			glDrawArrays(translate(primitive), 0, m_vbo->GetVertexCount());
	}
}

void VertexArrayBuffer::DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex)
{
	if (!m_ibo) return;

	if (currentVAO != m_id)
	{
		currentVAO = m_id;
		glBindVertexArray(m_id);
		m_vbo->Bind();
		if (m_ibo) m_ibo->Bind();
	}

	const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	glDrawElementsBaseVertex(translate(primitive), indexCount, indexSizeType, (void*)(m_ibo->GetIndexSize() * baseIndex), baseVertex);
}

void VertexArrayBuffer::UnBind()
{
	currentVAO = 0;
	glBindVertexArray(0);
}

bool FrameBuffer::Create(int width, int height)
{
	if (width < 1 || height < 1) return false;
	m_width = width;
	m_height = height;
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	glGenTextures(1, &m_texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
#if 1
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#else
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, nullptr);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // TODO: GL_LINEAR 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Texture2D::UnBind(); // TODO:

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texColorBuffer, 0);

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	if (!checkFramebuffer())
	{
		LogError("Framebuffer is not complete!");
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	currentFrameBuffer = nullptr;
	const float aspect = (float)width / (float)height;
	const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / aspect) * 2.0f;
	m_projectionMatrix = glm::perspective(FOVY, aspect, perspectiveNear, perspectiveFar);

	return true;
}

void FrameBuffer::Destroy()
{
	if (currentFrameBuffer == this) MainFrameBufferBind();

	glDeleteTextures(1, &m_texColorBuffer);
	glDeleteRenderbuffers(1, &m_rbo);
	glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::Bind(const glm::vec3& color)
{
	if (currentFrameBuffer != this)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		glViewport(0, 0, m_width, m_height);
		currentFrameBuffer = this;
	}
	glClearColor(color.x, color.y, color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::MainFrameBufferBind()
{
	if (currentFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GetFrameBufferWidth(), GetFrameBufferHeight());
	glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	currentFrameBuffer = nullptr;
}

void FrameBuffer::BindTextureBuffer()
{
	Texture2D::UnBind(); // TODO:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
}

bool FrameBuffer::checkFramebuffer()
{
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		return true;

	std::string strStatus = "";
	switch (status)
	{
	case GL_FRAMEBUFFER_UNDEFINED: strStatus = "GL_FRAMEBUFFER_UNDEFINED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
	case GL_FRAMEBUFFER_UNSUPPORTED: strStatus = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: strStatus = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
	default: strStatus = "UNKNOWN"; break;
	}
	LogError("OpenGL Error = " + strStatus);

	return false;
}

const glm::mat4& GetCurrentProjectionMatrix()
{
	if (currentFrameBuffer)
		return currentFrameBuffer->GetProjectionMatrix();
	else
		return projectionMatrix;
}