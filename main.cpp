#include <cstdlib>

#include <iostream>
#include <glm/glm.hpp>

#include "Program.h"
#include "TexturedQuad.h"
#include "Trace.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include <SDL2/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>

//GLuint windowWidth = 1600;
//GLuint windowHeight = 900;

GLuint windowWidth = 900;
GLuint windowHeight = 900;

double t = 0;


int main(int argv, char** args) {
    // === SDL and OpenGl (GLEW) setup. ===
    if( SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO ) != 0 ){ std::cout << "Couldn't initialize SDL!"; return -1; }
    SDL_Window *window = SDL_CreateWindow("Path Tracing", 20, 40, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE  );
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if(  glewInit() != GLEW_OK ) { std::cout << "Couldn't initialize GLEW!"; return -1; }


    // === ImGui setup. ===
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL( window, gl_context );
    ImGui_ImplOpenGL3_Init("#version 150");


    // Shader setup.
    Program program;
    program.createFromFile( "shader.vert", "shader.frag" ); 
    
    TexturedQuad quad;
    quad.init();

    // Scene.
    Trace trace;
    trace.width = windowWidth; trace.height = windowHeight;
    trace.initScene();
    
    std::vector<glm::vec4> image(windowWidth * windowHeight); // Image for rendering in one go.
    std::vector<glm::vec4> blackPixels(windowWidth * windowHeight); // Black pixels to clear texture.
    for (int y = 0; y < windowHeight; y++)
        for (int x = 0; x < windowWidth; x++)
            blackPixels[y * windowWidth + x] = glm::vec4(0, 0, 0, 1);



    quad.setTexture( windowWidth, windowHeight, blackPixels );
    trace.startRenderLoop();

    char saveFileName[1000] = "image";
    float gamma = 2.0;

    glUseProgram(program.getId());
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    bool running = true;
    while (running) {
        unsigned int ticks = SDL_GetTicks();
        t = ticks / 1000.0;

        // Get events.
        SDL_Event event;
        while( SDL_PollEvent(&event) ){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if( event.type == SDL_QUIT )
                running = false;
            if( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED ){
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;

                trace.resize(windowWidth, windowHeight);
                image.resize(windowWidth * windowHeight);
                blackPixels.resize(windowWidth * windowHeight);
                glViewport(0, 0, windowWidth, windowHeight);
            }
        }

        // ====== Render image ======
        glClear(GL_COLOR_BUFFER_BIT);

        trace.renderLoop(quad.texture);
        program.setUniform(quad.texture, "texture1");
        quad.draw();

        // ====== Render ImGui ======
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        ImGui::Begin("Raytracing");

        if (ImGui::Button("Stop")){
            trace.rendering = false;
        }


        if (ImGui::Button("Render")){
            std::cout << "Started Rendering!" << std::endl;
            trace.render( image );
            quad.setTexture( windowWidth, windowHeight, image );
            std::cout << "Render Time: " << trace.renderTime << std::endl;
        }

        if (ImGui::Button("Render Loop")){
            std::cout << "Render Loop started!" << std::endl;
            quad.setTexture( windowWidth, windowHeight, blackPixels );
            trace.startRenderLoop();
        }

        ImGui::DragFloat("gamma correction", &gamma, 0.01, 0.01, 10.0);
        program.setUniform(gamma, "gamma");

        ImGui::Text( ("Size: " + to_string( trace.width ) + " x " + to_string( trace.height )).c_str()  );
        ImGui::Text( ("Number of objects: " + to_string( trace.objects.size() )).c_str()  );
        ImGui::Text( ("Render t: " + to_string( trace.renderTime )).c_str()  );

        // === Remaining Time ===
        float percent = float(trace.ry) / trace.height;
        float remainingTime = (1 - percent) / percent * trace.renderTime;
        percent *= 100;
        ImGui::Text( ("Percent Complete: " + to_string( percent )).c_str()  );
        ImGui::Text( ("Remaining Time: " + to_string( remainingTime )).c_str()  );


        ImGui::DragInt("samples", &trace.samples, 0.5f, 1, 1000000);

        ImGui::SliderInt("Tracefunc", &trace.traceFunctionType, 0, 1);
        if( trace.traceFunctionType == 0 ) {
            ImGui::SliderInt("MaxDepth", &trace.maxDepth, 1, 50);
        }

        float adjustStep = 0.01;
        ImGui::Text( "Camera" );
        ImGui::DragFloat3("pos", reinterpret_cast<float *>(&trace.camera.eye), adjustStep);
        ImGui::DragFloat3("lookat", reinterpret_cast<float *>(&trace.camera.lookat), adjustStep);
        ImGui::DragFloat("fov", &trace.camera.fov, adjustStep, 0.01, 3.12);
        ImGui::DragFloat("aperture", &trace.camera.aperture, 0.005, 0.0, 100.0);
        trace.camera.set();

        ImGui::Text("Background Color");
        ImGui::DragFloat3("color1", reinterpret_cast<float *>(&trace.backGroundColor1), 0.001, 0, 1);
        ImGui::DragFloat3("color2", reinterpret_cast<float *>(&trace.backGroundColor2), 0.001, 0, 1);

        ImGui::Text( "Direct Lights" );
        for( int i = 0; i < trace.lights.size(); ++i) {
            ImGui::DragFloat3(("pos" + std::to_string(i)).c_str(),
                              reinterpret_cast<float *>(&trace.lights[i].position), adjustStep);
            ImGui::DragFloat3(("power" + std::to_string(i)).c_str(),
                              reinterpret_cast<float *>(&trace.lights[i].power), adjustStep);
        }

        ImGui::End();

        ImGui::Begin("Materials");

        for( const auto& it : trace.materials ){
            if(it.second->transparent()){
                ImGui::DragFloat(it.first.c_str(), &reinterpret_cast<TransparentMaterial*>(it.second)->refIndex, 0.001 );
            }else {
                ImGui::DragFloat3(it.first.c_str(), reinterpret_cast<float *>(&it.second->albedo), 0.001, 0, 1);
            }
        }

        ImGui::End();

        ImGui::Begin("Screenshot");
        ImGui::InputText("filename", saveFileName, 999);
        if(ImGui::Button("Write jpg")){
            auto* data = new GLubyte [3 * windowWidth * windowHeight];
            glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_flip_vertically_on_write(1);

            char saveFileNameJpg[1000] = {'\0'};
            strcat(saveFileNameJpg, "screenshot/");
            strcat(saveFileNameJpg, saveFileName);
            strcat(saveFileNameJpg, ".jpg");
            stbi_write_jpg(saveFileNameJpg, windowWidth, windowHeight, 3, data, 100);
            delete[] data;
        }
        ImGui::End();

        ImGui::Begin("Init Scene");
        for( auto it : trace.initFunctions){
            if(ImGui::Button(it.first.c_str())){
                trace.resetScene();
                (trace.*(it.second))();
                trace.makeBVH();
            }
        }
        ImGui::End();


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // ====== ImGui. ======


        SDL_GL_SwapWindow(window);        
    }

    // ===== Cleanup =====
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}