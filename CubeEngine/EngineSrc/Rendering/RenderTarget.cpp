#include "RenderTarget.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
namespace tzw {

RenderTarget::RenderTarget()
{
	m_fbo = 0;
	m_depthTex = 0;
	m_colorTexs = nullptr;
}

void RenderTarget::init(integer_u width, integer_u height, integer_u numOfOutputs, bool isUseDepth)
{
	m_width = width;
	m_height = height;
	m_numOfOutputs = numOfOutputs;
	m_isUseDepth = isUseDepth;
	assert(m_numOfOutputs);
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	m_colorTexs = static_cast<integer_u *>(malloc(sizeof (integer_u) * m_numOfOutputs));

	//color buffers
	if(m_numOfOutputs>0)
	{
		glGenTextures(m_numOfOutputs, m_colorTexs);
		for (unsigned int i = 0 ; i < m_numOfOutputs ; i++) {
			glBindTexture(GL_TEXTURE_2D, m_colorTexs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorTexs[i], 0);
		}
	}

	// depth buffer
	if(isUseDepth)
	{
		glGenTextures(1, &m_depthTex);

		glBindTexture(GL_TEXTURE_2D, m_depthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);
	}

	//match the draw buffers;
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0,
							 GL_COLOR_ATTACHMENT1,
							 GL_COLOR_ATTACHMENT2,
							 GL_COLOR_ATTACHMENT3 };

	glDrawBuffers(m_numOfOutputs, DrawBuffers);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void RenderTarget::bindForWriting()
{
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void RenderTarget::bindForReadingGBuffer()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
	for (unsigned int i = 0 ; i < m_numOfOutputs; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_colorTexs[i]);
	}
	if(m_isUseDepth)
	{
		glActiveTexture(GL_TEXTURE0 + m_numOfOutputs);
		glBindTexture(GL_TEXTURE_2D, m_depthTex);
	}

}

void RenderTarget::bindForReading()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
}

void RenderTarget::bindDepth(int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, m_depthTex);
}

void RenderTarget::setReadBuffer(integer_u index)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
}

} // namespace tzw
