#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include "Math.h"
#include "Timer.h"
#include "Matrix.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float _width, float _height) :
			width{ _width },
			height{ _height },
			origin{ _origin },
			fovAngle{ _fovAngle },
			isProjectionMatrixDirty{ true }
		{
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
		}

		float width;
		float height;
		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};
		bool isProjectionMatrixDirty{ true };

		void Initialize(float _width, float _height, float _fovAngle = 90.f, Vector3 _origin = { 0.f, 0.f, 0.f })
		{
			width = _width;
			height = _height;
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			origin = _origin;

			isProjectionMatrixDirty = true; // Mark dirty on initialization
		}

		void CalculateViewMatrix()
		{
			auto right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			auto up = Vector3::Cross(forward, right).Normalized();

			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
		}

		Matrix GetViewMatrix()
		{
			return viewMatrix;
		}
		void CalculateProjectionMatrix()
		{
			if (isProjectionMatrixDirty)
			{
				projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, width / height, .1f, 100.f);
				isProjectionMatrixDirty = false; // Reset flag after update
			}
		}

		Matrix GetProjectionMatrix()
		{
			return projectionMatrix;
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			// Camera Update Logic
			Vector3 velocity{ 5.f, 5.f, 10.f };
			constexpr float rotationVelocity{ 0.1f * PI / 180.0f };

			// Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			// Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (pKeyboardState[SDL_SCANCODE_W]) origin += forward * velocity.z * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_S]) origin -= forward * velocity.z * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_A]) origin -= right * velocity.x * deltaTime;
			if (pKeyboardState[SDL_SCANCODE_D]) origin += right * velocity.x * deltaTime;

			bool leftButtonPressed = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
			bool rightButtonPressed = mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);

			if (leftButtonPressed)
			{
				origin += forward * mouseY * velocity.z * 0.01f;
				totalPitch += mouseX * rotationVelocity;
			}
			if (rightButtonPressed)
			{
				totalPitch += mouseX * rotationVelocity;
				totalYaw += mouseY * rotationVelocity;
			}

			Matrix finalRotation = finalRotation.CreateRotation(totalYaw, totalPitch, 0.f);
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			right = finalRotation.TransformVector(Vector3::UnitX);

			forward.Normalize();
			right.Normalize();

			// Update Matrices
			CalculateViewMatrix();

			// Calculate projection matrix only if dirty
			CalculateProjectionMatrix();
		}

		void SetFovAngle(float newFovAngle)
		{
			if (fovAngle != newFovAngle)
			{
				fovAngle = newFovAngle;
				fov = tanf((fovAngle * TO_RADIANS) / 2.f);
				isProjectionMatrixDirty = true; // Mark dirty if FOV changes
			}
		}

		void SetViewportSize(float newWidth, float newHeight)
		{
			if (width != newWidth || height != newHeight)
			{
				width = newWidth;
				height = newHeight;
				isProjectionMatrixDirty = true; // Mark dirty if aspect ratio changes
			}
		}
	};
}
