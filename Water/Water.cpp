#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>


#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int waterHeight = 0;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader waterShader("water.vs", "water.fs");
    Shader poolShader("basic_shader.vs", "basic_shader.fs");
    Shader screenShader("test.vs", "test.fs");
    Shader skyShader("sky.vs", "sky.fs");

    // load models
    //------------
    Model poolModel("resources/fountain/horniman-fountain-edit.obj");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float squareVerts[] = {
        // First triangle
        -0.5f,  0.5f, 0.0f,  // Top-left
        -0.5f, -0.5f, 0.0f,  // Bottom-left
         0.5f,  0.5f, 0.0f,  // Top-right
         // Second triangle
          0.5f,  0.5f, 0.0f,  // Top-right
         -0.5f, -0.5f, 0.0f,  // Bottom-left
          0.5f, -0.5f, 0.0f   // Bottom-right
    };
    float quadVerts[] = {
        // positions   // texCoords
        -0.3f,  1.0f,  0.0f, 1.0f,
        -0.3f,  0.7f,  0.0f, 0.0f,
         0.3f,  0.7f,  1.0f, 0.0f,

        -0.3f,  1.0f,  0.0f, 1.0f,
         0.3f,  0.7f,  1.0f, 0.0f,
         0.3f,  1.0f,  1.0f, 1.0f
    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    //waterVAO
    unsigned int squareVBO, waterVAO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &squareVBO);

    glBindVertexArray(waterVAO);

    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVerts), squareVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //screen quad
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    //skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load and create a texture 
    // -------------------------
  

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------


    // framebuffer configuration
    // -------------------------
    //reflection
    //create frameebuffer
    unsigned int reflectionFramebuffer;
    glGenFramebuffers(1, &reflectionFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFramebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //// create a color attachment texture
    unsigned int reflectionTextureColorbuffer;
    glGenTextures(1, &reflectionTextureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, reflectionTextureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflectionTextureColorbuffer, 0);
    //// create a depth attachment texture
    unsigned int reflectionTextureDepthbuffer;
    glGenTextures(1, &reflectionTextureDepthbuffer);
    glBindTexture(GL_TEXTURE_2D, reflectionTextureDepthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, reflectionTextureDepthbuffer, 0);
    //// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int reflectionRBO;
    glGenRenderbuffers(1, &reflectionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflectionRBO); // now actually attach it
    //// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ////refraction
    //create framebuffer
    unsigned int refractionFramebuffer;
    glGenFramebuffers(1, &refractionFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFramebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //// create a color attachment texture
    unsigned int refractionTextureColorbuffer;
    glGenTextures(1, &refractionTextureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, refractionTextureColorbuffer, 0);
    //// create a depth attachment texture
    unsigned int refractionTextureDepthbuffer;
    glGenTextures(1, &refractionTextureDepthbuffer);
    glBindTexture(GL_TEXTURE_2D, refractionTextureDepthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, refractionTextureDepthbuffer, 0);
    //// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int refractionRBO;
    glGenRenderbuffers(1, &refractionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflectionRBO); // now actually attach it
    //// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    vector<std::string> faces
    {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_CLIP_DISTANCE0);

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        //render reflection texture
        // ------------------------
            //modify camera
            float distance = 2 * (camera.Position.y - waterHeight);
            glm::vec3 newPosition = camera.Position;
            newPosition.y -= distance;
            float newPitch = -1.0 * camera.Pitch;
            glm::vec3 newFront;
            newFront.x = cos(glm::radians(camera.Yaw)) * cos(glm::radians(newPitch));
            newFront.y = sin(glm::radians(newPitch));
            newFront.z = sin(glm::radians(camera.Yaw)) * cos(glm::radians(newPitch));
            newFront = glm::normalize(newFront);
            glm::vec3 newRight = glm::normalize(glm::cross(newFront, glm::vec3(0,1,0)));
            glm::vec3 newUp = glm::normalize(glm::cross(newRight, newFront));

        // bind to framebuffer and draw scene as we normally would to color texture 
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFramebuffer);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        poolShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2, 2, 2));
        model = glm::rotate(model, glm::radians(00.0f), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -3.65, 0));
        glm::mat4 view = glm::lookAt(newPosition, newPosition + newFront, newUp);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        poolShader.setMat4("view", view);
        poolShader.setMat4("projection", projection);
        poolShader.setMat4("model", model);
        poolShader.setVec4("plane", glm::vec4(0, 1, 0, waterHeight)); //set clip plane
        poolModel.Draw(poolShader);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyShader.use();
        view = glm::mat4(glm::mat3(glm::lookAt(newPosition, newPosition + newFront, newUp))); // remove translation from the view matrix
        skyShader.setMat4("view", view);
        skyShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //render refraction texture
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, refractionFramebuffer);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        poolShader.use();
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2, 2, 2));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -3.65, 0));
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        poolShader.setMat4("view", view);
        poolShader.setMat4("projection", projection);
        poolShader.setMat4("model", model);
        poolShader.setVec4("plane", glm::vec4(0, -1, 0, waterHeight)); //set clip plane
        poolModel.Draw(poolShader);


        // now bind back to default framebuffer
        //-------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render main scene
        // -----------------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render pool
        poolShader.use();
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(2, 2, 2));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -3.65, 0));
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        poolShader.setMat4("view", view);
        poolShader.setMat4("projection", projection);
        poolShader.setMat4("model", model);
        poolShader.setVec4("plane", glm::vec4(0, 0, 0, 0)); //set clip plane
        poolModel.Draw(poolShader);
        //render water
        waterShader.use();
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);
        glBindVertexArray(waterVAO);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflectionTextureColorbuffer);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);

        glUniform1i(glGetUniformLocation(waterShader.ID, "reflectionTexture"), 0); // Texture unit 0
        glUniform1i(glGetUniformLocation(waterShader.ID, "refractionTexture"), 1); // Texture unit 1

        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i - 2, waterHeight, j - 2));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
                waterShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        //fill
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0, waterHeight, 2.75));
            model = glm::scale(model, glm::vec3(3, 1.0, 0.5));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            waterShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);


            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0, waterHeight, -3 + .1));
            model = glm::scale(model, glm::vec3(4.2, 1.0, .8));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            waterShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(3 - .15, waterHeight, -0.15f));
            model = glm::scale(model, glm::vec3(0.7, 1.0, 4.0));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            waterShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);


            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-3 + .1f, waterHeight, -0.2f));
            model = glm::scale(model, glm::vec3(0.8f, 1.0, 3.7));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            waterShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);


        glBindVertexArray(0);


        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyShader.setMat4("view", view);
        skyShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        //screenShader.use();
        //glBindVertexArray(quadVAO);
        //glBindTexture(GL_TEXTURE_2D, refractionTextureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        //glDrawArrays(GL_TRIANGLES, 0, 6);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &squareVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}