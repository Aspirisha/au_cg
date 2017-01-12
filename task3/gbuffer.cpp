/*
        Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <cstring>
#include <iostream>

#include "gbuffer.h"
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

GBuffer::GBuffer()
{
    g_fb = 0;
    memset(m_textures, 0, sizeof(m_textures));
}

GBuffer::~GBuffer()
{
    if (g_fb != 0) {
        glDeleteFramebuffers(1, &g_fb);
    }

    if (m_textures[0] != 0) {
        glDeleteTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
    }
}



bool GBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    glGenFramebuffers (1, &g_fb);

    auto gen_texture = [WindowHeight, WindowWidth](GLuint* tex, GLint internalformat, GLenum format) {
        glGenTextures (1, tex);
        glBindTexture (GL_TEXTURE_2D, *tex);
        /* note 16-bit float RGB format used for positions */
        glTexImage2D (
                GL_TEXTURE_2D,
                0,
                internalformat,
                WindowWidth,
                WindowHeight,
                0,
                format,
                GL_UNSIGNED_BYTE,
                NULL
        );
        /* no bi-linear filtering required because texture same size as viewport */
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    };
    gen_texture(m_textures + GBUFFER_TEXTURE_TYPE_POSITION, GL_RGB32F, GL_RGB);
    gen_texture(m_textures + GBUFFER_TEXTURE_TYPE_NORMAL, GL_RGB16F, GL_RGB);

    /* attach textures to framebuffer. the attachment numbers 0 and 1 don't
    automatically corresponed to frament shader output locations 0 and 1, so we
    specify that afterwards */
    glBindFramebuffer (GL_FRAMEBUFFER, g_fb);
    glFramebufferTexture2D (
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION], 0
    );
    glFramebufferTexture2D (
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_NORMAL], 0
    );
    /* the first item in this array matches fragment shader output location 0 */
    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers (2, draw_bufs);

    GLuint depth_tex;
    glGenRenderbuffers(1, &depth_tex);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_tex);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_tex);
    
    /* validate g_fb and return false on error */
    GLenum status = glCheckFramebufferStatus (GL_DRAW_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != status) {
        fprintf (stderr, "ERROR: incomplete framebuffer %d\n", status);
        return false;
    }
    return true;
}


void GBuffer::BindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, g_fb);
}

void GBuffer::BindForReading()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_fb);
    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION + i]);
    }
}
