/*
 *  CSCI 441, Computer Graphics, Fall 2024
 *
 *  Project: mp
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with texturing.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2024
 *
 */

#include "FPEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

///*****************************************************************************
//
// Our main function
int main() {

    auto labEngine = new FPEngine();
    labEngine->initialize();
    if (labEngine->getError() == CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
        labEngine->run();
    }
    labEngine->shutdown();
    delete labEngine;

	return EXIT_SUCCESS;
}
