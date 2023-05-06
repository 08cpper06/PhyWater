#include "SDL_render.h"
#include "SDL_timer.h"
#include "SDL_video.h"
#include <SDL.h>
#include <cmath>
#include <random>
#include <list>


#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480
#define VELOCITY_SIGMA		0.001


int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
		status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
		status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
		status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
	int offsetx, offsety, d;
	int status;

	offsetx = 0;
	offsety = radius;
	d = radius -1;
	status = 0;

	while (offsety >= offsetx) {

		status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
									 x + offsety, y + offsetx);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
									 x + offsetx, y + offsety);
		status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
									 x + offsetx, y - offsety);
		status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
									 x + offsety, y - offsetx);

		if (status < 0) {
			status = -1;
			break;
		}

		if (d >= 2*offsetx) {
			d -= 2*offsetx + 1;
			offsetx +=1;
		}
		else if (d < 2 * (radius - offsety)) {
			d += 2 * offsety - 1;
			offsety -= 1;
		}
		else {
			d += 2 * (offsety - offsetx - 1);
			offsety -= 1;
			offsetx += 1;
		}
	}

	return status;
}

class Vector {
public:
	double X;
	double Y;
public:
	Vector() {}
	Vector(double X, double Y) :
		X(X),
		Y(Y)
	{}
	~Vector() {}

	Vector operator+(const Vector& Rhs) const
	{
		return Vector(X + Rhs.X, Y + Rhs.Y);
	}
	Vector operator*(const double& Rhs) const
	{
		return Vector(X * Rhs, Y * Rhs);
	}
};

class Ball {
public:
	static Vector WindowSize;
public:
	Vector Position;
	Vector Velocity;
	float Radius;
	float Friction;
	Uint64 Red;
	Uint64 Green;
	Uint64 Blue;
public:
	Ball() {}

	void Update(float DeltaTime)
	{
		ApplyGravity(DeltaTime);
		Clipping();
	}

	void Draw(SDL_Renderer* Render)
	{
		SDL_SetRenderDrawColor(Render, Red, Green, Blue, 0);
		SDL_RenderFillCircle(Render, Position.X, Position.Y, Radius);
	}

private:
	void ApplyGravity(float DeltaTime)
	{
		static const Vector Gravity(.0, 9.8 * 0.001);
		Vector OldVelocity = Velocity;
		OldVelocity = OldVelocity + Gravity * DeltaTime;
		Velocity = OldVelocity;
		Position = Position + Velocity;
	}

	void Clipping()
	{
		if (Position.X < Radius) {
			Position.X = Radius;
			Velocity.X *= -Friction;
		}
		if (Position.X > WindowSize.X - Radius) {
			Position.X = WindowSize.X - Radius;
			Velocity.X *= -Friction;
		}
		if (Position.Y < Radius) {
			Position.Y = Radius;
			Velocity.Y *= -Friction;
		}
		if (Position.Y > WindowSize.Y - Radius) {
			Position.Y = WindowSize.Y - Radius;
			Velocity.Y *= -Friction;
		}
		if (abs(Velocity.X) < VELOCITY_SIGMA) {
			Velocity.X = 0.;
		}
		if (abs(Velocity.Y) < VELOCITY_SIGMA) {
			Velocity.Y = 0.;
		}
	}
};

Vector Ball::WindowSize = Vector(WINDOW_WIDTH, WINDOW_HEIGHT);

class Application {
public:
	Application() :
		RequestedFinish(false),
		PerSec(SDL_GetPerformanceFrequency()),
		LastTime(SDL_GetPerformanceCounter()),
		DeltaTime(0.f),
		Window(nullptr),
		Render(nullptr),
		WindowPos(0., 0.)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
	}
	~Application() 
	{
		SDL_Quit();
	}

	void ShowWindow()
	{
		Window = SDL_CreateWindow(
				"PhyWater",
				SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				WINDOW_WIDTH, WINDOW_HEIGHT,
				SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
		Render = SDL_CreateRenderer(Window, -1, 0);
		int X, Y;
		SDL_GetWindowPosition(Window, &X, &Y);
		WindowPos = Vector(X, Y);
	}

	void Execute()
	{
		Uint64 Now;
		while (!RequestedFinish) {
			PollEvent();
			Now = SDL_GetPerformanceCounter();

			DeltaTime = (Now - LastTime) / PerSec * 1000.f;

			Update(DeltaTime);

			Draw();

			LastTime = Now;
		}
		if (Window) {
			SDL_DestroyRenderer(Render);
			SDL_DestroyWindow(Window);
		}
	}

	void Update(float DeltaTime)
	{
		for (auto& ball : BallList) {
			ball.Update(DeltaTime);
		}
	}

	void PollEvent()
	{
		int X, Y;
		Vector Velocity;
		SDL_Event Event;
		if (SDL_PollEvent(&Event)) {
			switch (Event.type) {
			case SDL_QUIT:
				RequestedFinish = true;
				break;
			case SDL_WINDOWEVENT:
				switch (Event.window.event) {
				case SDL_WINDOWEVENT_MOVED:
					SDL_GetWindowPosition(Window, &X, &Y);
					Velocity = Vector((int(WindowPos.X - X) % WINDOW_WIDTH  * 0.5),
									  (int(WindowPos.Y - Y) % WINDOW_HEIGHT * 0.5));
					for (auto& ball : BallList) {
						ball.Velocity = ball.Velocity + Velocity;
					}
					break;
				case SDL_WINDOWEVENT_RESIZED:
					Ball::WindowSize = Vector(Event.window.data1, Event.window.data2);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					Draw();
					break;
				}
				break;
			}
		}
	}

	void Draw()
	{
		SDL_SetRenderDrawColor(Render, 100, 100, 100, 0);
		SDL_RenderClear(Render);

		for (auto& ball : BallList) {
			ball.Draw(Render);
		}

		SDL_RenderPresent(Render);
	}

public:
	std::list<Ball> BallList;

private:
	bool RequestedFinish;
	const float PerSec;
	Uint64 LastTime;
	float DeltaTime;
	SDL_Window* Window;
	SDL_Renderer* Render;
	Vector WindowPos;
	Vector WindowSize;
};

void HSVtoRGB(Uint64 H, Uint64 S, Uint64 V, Uint64& R, Uint64& G, Uint64& B)
{
	double H_dash = double(H) / 60.;
	H_dash = H_dash - int(H / 60);
	double S_dash = S / 100.;
	double V_dash = V / 100.;

	Uint64 tmp_A = V_dash * 255;
	Uint64 tmp_B = V_dash * (1 - S_dash) * 255;
	Uint64 tmp_C = V_dash * (1 - S_dash * H_dash) * 255;
	Uint64 tmp_D = V_dash * (1 - S_dash * (1 - H_dash)) * 255;

	if (S == 0) {
		R = G = B = tmp_A;
		return;
	}
	if (H < 60) {
		R = tmp_A;
		G = tmp_D;
		B = tmp_B;
		return;
	} else if (H < 120) {
		R = tmp_C;
		G = tmp_A;
		B = tmp_B;
		return;
	} else if (H < 180) {
		R = tmp_B;
		G = tmp_A;
		B = tmp_D;
		return;
	} else if (H < 240) {
		R = tmp_B;
		G = tmp_C;
		B = tmp_A;
		return;
	} else if (H < 300) {
		R = tmp_D;
		G = tmp_B;
		B = tmp_A;
		return;
	} else if (H < 360) {
		R = tmp_A;
		G = tmp_B;
		B = tmp_C;
		return;
	}
}

int main(int argc, char** argv)
{

	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());

	Application App;
	App.ShowWindow();
	Uint64 H, S, V;
	/* H : 150 - 280 */
	/* S : 40-100 */
	V = 100;

	Ball ball;
	for (int i = 0; i < 1000; ++i) {
		H = engine() % 130 + 150;
		S = engine() % 40 + 60;
		HSVtoRGB(H, S, V, ball.Red, ball.Green, ball.Blue);
		ball.Position = Vector(engine() % WINDOW_WIDTH, engine() % WINDOW_HEIGHT);
		ball.Velocity = Vector(engine() % 10, engine() % 10);
		ball.Friction = 0.9f;
		ball.Radius = engine() % 20;
		App.BallList.push_back(ball);
	}

	App.Execute();
}
