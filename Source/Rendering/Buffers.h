#ifndef BUFFERS_H
#define BUFFERS_H

unsigned int CreateFrameBuffer();
unsigned int CreateTextureAttachment(int width, int height);
unsigned int CreateDepthTextureAttachment(int width, int height);
unsigned int CreateRenderBufferAttachment(int width, int height);

#endif 