#ifndef CAMERA_H
#define CAMERA_H

//#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

//possible camera movements
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    TURNL,
    TURNR,
    TURNU,
    TURND
};

//camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    //attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    //amngles
    float Yaw;
    float Pitch;

    //camera
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    //constructor with vecs
    Camera(glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) :
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {


        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    //constructor with scalar
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {


        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    //return view matrix calcualated with angles and look at matrix
    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    //processes input
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        float turnSpeed = velocity * 10.0f;

        if (direction == FORWARD) {
            Position += Front * velocity;
        }
        if (direction == BACKWARD) {
            Position -= Front * velocity;
        }
        if (direction == LEFT) {
            Position -= Right * velocity;
        }
        if (direction == RIGHT) {
            Position += Right * velocity;
        }
        if (direction == UP) {
            Position += Up * velocity;
        }
        if (direction == DOWN) {
            Position -= Up * velocity;
        }
        if (direction == TURNL) {
            Yaw -= turnSpeed;
            updateCameraVectors();
        }
        if (direction == TURNR) {
            Yaw += turnSpeed;
            updateCameraVectors();
        }
        if (direction == TURNU && Pitch >-89.0f ) {
            Pitch += turnSpeed;
            updateCameraVectors();
        }
        if (direction == TURND && Pitch < 89.0f) {
            Pitch -= turnSpeed;
            updateCameraVectors();
        }
    }

    //process input from mouse poisition
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        //when pitch is out of bounds screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        //update camera vectors using the updated angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        MovementSpeed += yoffset;

        if (MovementSpeed < 0.5f) {
            MovementSpeed = 0.5f;
        }
        if (MovementSpeed > 50.0f) {
            MovementSpeed = 50.0f;
        }
    }

private:
    //calculate fron vector from cameras anfles
    void updateCameraVectors()
    {
        //calculate the Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        //calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif