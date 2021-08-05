#include "mat-to-texture.hpp"

// From https://gist.github.com/insaneyilin/038a022f2ece61c923315306ddcea081
// With fix from https://stackoverflow.com/a/53566791
void matToTexture(const cv::Mat &mat, GLuint *outTexture) {
  // Generate a number for our textureID's unique handle
  GLuint textureID;
  glGenTextures(1, &textureID);

  // Bind to our texture handle
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set texture interpolation methods for minification and magnification
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Set texture clamping method
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Use fast 4-byte alignment (default anyway) if possible
  glPixelStorei(GL_UNPACK_ALIGNMENT, (mat.cols % 4 == 0) ? 4 : 1);

  // Set length of one complete row in data (doesn't need to equal mat.cols)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, mat.step / mat.elemSize());

  // Set incoming texture format to:
  // GL_BGR     for CV_CAP_OPENNI_BGR_IMAGE,
  // GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
  // Work out other mappings as required ( there's a list in comments in main()
  // )
  GLenum inputColourFormat = GL_BGR;
  if (mat.channels() == 1) {
    inputColourFormat = GL_RED;
  }

  if (!mat.isContinuous()) {
    throw std::runtime_error("Matrix not continuous");
  }

  // Create the texture
  glTexImage2D(GL_TEXTURE_2D, // Type of texture
               0,        // Pyramid level (for mip-mapping) - 0 is the top level
               GL_RGB,   // Internal colour format to convert to
               mat.cols, // Image width  i.e. 640 for Kinect in standard mode
               mat.rows, // Image height i.e. 480 for Kinect in standard mode
               0,        // Border width in pixels (can either be 1 or 0)
               inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA,
                                  // GL_BGR etc.)
               GL_UNSIGNED_BYTE,  // Image data type
               mat.data);         // The actual image data itself

  *outTexture = textureID;
}
