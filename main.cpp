#include <gfd.h>
#include <gx2/draw.h>
#include <gx2/shaders.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2r/draw.h>
#include <gx2r/buffer.h>
#include <string.h>
#include <stdio.h>
#include <whb/file.h>
#include <whb/proc.h>
#include <whb/sdcard.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <cstdlib>

#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

#include "CafeGLSLCompiler.h"
#include "shaders.h"

static const float sPositionData[] =
{
    1.0f, -1.0f,  0.0f, 1.0f,
    0.0f,  1.0f,  0.0f, 1.0f,
   -1.0f, -1.0f,  1.0f, 1.0f,
};

static const float sColourData[] =
{
   1.0f,  0.0f,  0.0f, 1.0f,
   0.0f,  1.0f,  0.0f, 1.0f,
   0.0f,  0.0f,  1.0f, 1.0f,
};

WHBGfxShaderGroup *GLSL_CompileShader(const char *vsSrc, const char *psSrc)
{
   char infoLog[1024];
   GX2VertexShader *vs = GLSL_CompileVertexShader(vsSrc, infoLog, sizeof(infoLog), GLSL_COMPILER_FLAG_NONE);
   if (!vs)
   {
      OSReport("Failed to compile vertex shader. Infolog: %s\n", infoLog);
      return NULL;
   }
   GX2PixelShader *ps = GLSL_CompilePixelShader(psSrc, infoLog, sizeof(infoLog), GLSL_COMPILER_FLAG_NONE);
   if (!ps)
   {
      OSReport("Failed to compile pixel shader. Infolog: %s\n", infoLog);
      return NULL;
   }
   WHBGfxShaderGroup *shaderGroup = (WHBGfxShaderGroup *)malloc(sizeof(WHBGfxShaderGroup));
   memset(shaderGroup, 0, sizeof(*shaderGroup));
   shaderGroup->vertexShader = vs;
   shaderGroup->pixelShader = ps;
   return shaderGroup;
}

int main(int argc, char **argv)
{
   GX2RBuffer positionBuffer = {};
   GX2RBuffer colourBuffer = {};
   WHBGfxShaderGroup *group = {};
   void *buffer = NULL;
   int result = 0;

   WHBLogUdpInit();
   WHBProcInit();
   WHBGfxInit();

   GLSL_Init();


   group = GLSL_CompileShader(vertexShader, fragmentShader);
   if (!group) {
      result = -1;
      WHBLogPrintf("Shader compilation failed");
      goto exit;
   }

   WHBGfxInitShaderAttribute(group, "in_position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);
   WHBGfxInitShaderAttribute(group, "in_color", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);
   WHBGfxInitFetchShader(group);

   // Set vertex position
   positionBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
                          GX2R_RESOURCE_USAGE_CPU_READ |
                          GX2R_RESOURCE_USAGE_CPU_WRITE |
                          GX2R_RESOURCE_USAGE_GPU_READ;
   positionBuffer.elemSize = 4 * 4;
   positionBuffer.elemCount = 3;
   GX2RCreateBuffer(&positionBuffer);
   buffer = GX2RLockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
   memcpy(buffer, sPositionData, positionBuffer.elemSize * positionBuffer.elemCount);
   GX2RUnlockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);

   // Set vertex colour
   colourBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER |
                        GX2R_RESOURCE_USAGE_CPU_READ |
                        GX2R_RESOURCE_USAGE_CPU_WRITE |
                        GX2R_RESOURCE_USAGE_GPU_READ;
   colourBuffer.elemSize = 4 * 4;
   colourBuffer.elemCount = 3;
   GX2RCreateBuffer(&colourBuffer);
   buffer = GX2RLockBufferEx(&colourBuffer, GX2R_RESOURCE_BIND_NONE);
   memcpy(buffer, sColourData, colourBuffer.elemSize * colourBuffer.elemCount);
   GX2RUnlockBufferEx(&colourBuffer, GX2R_RESOURCE_BIND_NONE);

   WHBLogPrintf("Begin rendering...");
   while (WHBProcIsRunning()) {
      // Animate colours...
      float *colours = (float *)GX2RLockBufferEx(&colourBuffer, GX2R_RESOURCE_BIND_NONE);
      colours[0] = 1.0f;
      colours[1] = colours[1] >= 1.0f ? 0.0f : (colours[1] + 0.01f);
      colours[2] = colours[2] >= 1.0f ? 0.0f : (colours[2] + 0.01f);
      colours[3] = 1.0f;

      colours[4] = colours[4] >= 1.0f ? 0.0f : (colours[4] + 0.01f);
      colours[5] = 1.0f;
      colours[6] = colours[6] >= 1.0f ? 0.0f : (colours[6] + 0.01f);
      colours[7] = 1.0f;

      colours[8] = colours[8] >= 1.0f ? 0.0f : (colours[8] + 0.01f);
      colours[9] = colours[9] >= 1.0f ? 0.0f : (colours[9] + 0.01f);
      colours[10] = 1.0f;
      colours[11] = 1.0f;
      GX2RUnlockBufferEx(&colourBuffer, GX2R_RESOURCE_BIND_NONE);

      // Render!
      WHBGfxBeginRender();

      WHBGfxBeginRenderTV();
      WHBGfxClearColor(0.0f, 0.0f, 1.0f, 1.0f);
      GX2SetFetchShader(&group->fetchShader);
      GX2SetVertexShader(group->vertexShader);
      GX2SetPixelShader(group->pixelShader);
      GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
      GX2RSetAttributeBuffer(&colourBuffer, 1, colourBuffer.elemSize, 0);
      GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLES, 3, 0, 1);
      WHBGfxFinishRenderTV();

      WHBGfxBeginRenderDRC();
      WHBGfxClearColor(1.0f, 0.0f, 1.0f, 1.0f);
      GX2SetFetchShader(&group->fetchShader);
      GX2SetVertexShader(group->vertexShader);
      GX2SetPixelShader(group->pixelShader);
      GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
      GX2RSetAttributeBuffer(&colourBuffer, 1, colourBuffer.elemSize, 0);
      GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLES, 3, 0, 1);
      WHBGfxFinishRenderDRC();

      WHBGfxFinishRender();
   }

exit:
   WHBLogPrintf("Exiting...");
   GX2RDestroyBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
   GX2RDestroyBufferEx(&colourBuffer, GX2R_RESOURCE_BIND_NONE);
   WHBUnmountSdCard();
   WHBGfxShutdown();
   WHBProcShutdown();
   WHBLogUdpDeinit();
   return result;
}
