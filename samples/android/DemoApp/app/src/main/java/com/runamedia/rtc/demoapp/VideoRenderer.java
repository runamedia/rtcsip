/*

 Copyright (c) 2015, Runa Media LLC
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

package com.runamedia.rtc.demoapp;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class VideoRenderer {

  public static class I420Frame {
    public final int width;
    public final int height;
    public final int[] yuvStrides;
    public final ByteBuffer[] yuvPlanes;
    public final boolean yuvFrame;
    public Object textureObject;
    public int textureId;

    public int rotationDegree;

    public I420Frame(
            int width, int height, int rotationDegree,
            int[] yuvStrides, ByteBuffer[] yuvPlanes) {
      this.width = width;
      this.height = height;
      this.yuvStrides = yuvStrides;
      if (yuvPlanes == null) {
        yuvPlanes = new ByteBuffer[3];
        yuvPlanes[0] = ByteBuffer.allocateDirect(yuvStrides[0] * height);
        yuvPlanes[1] = ByteBuffer.allocateDirect(yuvStrides[1] * height / 2);
        yuvPlanes[2] = ByteBuffer.allocateDirect(yuvStrides[2] * height / 2);
      }
      this.yuvPlanes = yuvPlanes;
      this.yuvFrame = true;
      this.rotationDegree = rotationDegree;
      if (rotationDegree % 90 != 0) {
        throw new IllegalArgumentException("Rotation degree not multiple of 90: " + rotationDegree);
      }
    }

    public I420Frame(
            int width, int height, int rotationDegree,
            Object textureObject, int textureId) {
      this.width = width;
      this.height = height;
      this.yuvStrides = null;
      this.yuvPlanes = null;
      this.textureObject = textureObject;
      this.textureId = textureId;
      this.yuvFrame = false;
      this.rotationDegree = rotationDegree;
      if (rotationDegree % 90 != 0) {
        throw new IllegalArgumentException("Rotation degree not multiple of 90: " + rotationDegree);
      }
    }

    public int rotatedWidth() {
      return (rotationDegree % 180 == 0) ? width : height;
    }

    public int rotatedHeight() {
      return (rotationDegree % 180 == 0) ? height : width;
    }

    public I420Frame copyFrom(I420Frame source) {
      if (source.yuvFrame && yuvFrame) {
        if (width != source.width || height != source.height) {
          throw new RuntimeException("Mismatched dimensions!  Source: " +
                  source.toString() + ", destination: " + toString());
        }
        nativeCopyPlane(source.yuvPlanes[0], width, height,
                source.yuvStrides[0], yuvPlanes[0], yuvStrides[0]);
        nativeCopyPlane(source.yuvPlanes[1], width / 2, height / 2,
                source.yuvStrides[1], yuvPlanes[1], yuvStrides[1]);
        nativeCopyPlane(source.yuvPlanes[2], width / 2, height / 2,
                source.yuvStrides[2], yuvPlanes[2], yuvStrides[2]);
        rotationDegree = source.rotationDegree;
        return this;
      } else if (!source.yuvFrame && !yuvFrame) {
        textureObject = source.textureObject;
        textureId = source.textureId;
        rotationDegree = source.rotationDegree;
        return this;
      } else {
        throw new RuntimeException("Mismatched frame types!  Source: " +
                source.toString() + ", destination: " + toString());
      }
    }

    public I420Frame copyFrom(byte[] yuvData, int rotationDegree) {
      if (yuvData.length < width * height * 3 / 2) {
        throw new RuntimeException("Wrong arrays size: " + yuvData.length);
      }
      if (!yuvFrame) {
        throw new RuntimeException("Can not feed yuv data to texture frame");
      }
      int planeSize = width * height;
      ByteBuffer[] planes = new ByteBuffer[3];
      planes[0] = ByteBuffer.wrap(yuvData, 0, planeSize);
      planes[1] = ByteBuffer.wrap(yuvData, planeSize, planeSize / 4);
      planes[2] = ByteBuffer.wrap(yuvData, planeSize + planeSize / 4,
              planeSize / 4);
      for (int i = 0; i < 3; i++) {
        yuvPlanes[i].position(0);
        yuvPlanes[i].put(planes[i]);
        yuvPlanes[i].position(0);
        yuvPlanes[i].limit(yuvPlanes[i].capacity());
      }
      this.rotationDegree = rotationDegree;
      return this;
    }

    @Override
    public String toString() {
      return width + "x" + height + ":" + yuvStrides[0] + ":" + yuvStrides[1] +
              ":" + yuvStrides[2];
    }
  }

  private static native void nativeCopyPlane(ByteBuffer src, int width,
                                             int height, int srcStride, ByteBuffer dst, int dstStride);

  public static interface Callbacks {
    public void renderFrame(I420Frame frame);
  }

  long nativeVideoRenderer;

  public VideoRenderer(Callbacks callbacks) {
    nativeVideoRenderer = nativeWrapVideoRenderer(callbacks);
  }

  public void dispose() {
    if (nativeVideoRenderer == 0) {
      // Already disposed.
      return;
    }
    freeWrappedVideoRenderer(nativeVideoRenderer);
    nativeVideoRenderer = 0;
  }

  private static native long nativeWrapVideoRenderer(Callbacks callbacks);

  private static native void freeWrappedVideoRenderer(long nativeVideoRenderer);
}