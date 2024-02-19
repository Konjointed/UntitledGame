#ifndef BUFFERS_H
#define BUFFERS_H

// If bindOnCreate is false use BindFramebuffer()
unsigned int CreateFrameBuffer(bool bindOnCreate = false);
unsigned int CreateColorTextureAttachment(int width, int height);
unsigned int CreateDepthTextureAttachment(int width, int height);
unsigned int CreateDepthStencilTextureAttachment(int width, int height);
unsigned int CreateRenderBufferAttachment(int width, int height);
unsigned int CreateDepthTextureArray(int width, int height, int layers);
void BindFramebuffer(unsigned int fbo);
void BindDefaultFramebuffer();
void ValidateBuffer(unsigned int fbo);

#endif 