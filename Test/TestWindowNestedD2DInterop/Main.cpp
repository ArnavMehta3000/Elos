#include <Elos/Utils/Timer.h>
#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/MessageBox.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include "DeviceResources.h"
#include <DirectXColors.h>
#include <d2d1_3helper.h>
#include <SimpleMath.h>
#include "GeometricPrimitive.h"
#include <format>
#include <print>

#pragma comment(lib, "d2d1.lib")

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

struct DXData
{
	std::unique_ptr<DX::DeviceResources> d3d;
	std::unique_ptr<GeometricPrimitive>  shape;
	ComPtr<IDXGISurface1>                dxgiSurface;
	ComPtr<ID2D1RenderTarget>            d2dRenderTarget;
	Matrix                               world;
	Matrix                               view;
	Matrix                               proj;
};

static void Create2DResources(DXData* dx, const Elos::Window& window)
{
	HRESULT hr{ S_OK };
	hr = dx->d3d->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&dx->dxgiSurface));

	ComPtr<ID2D1Factory1> d2dFactory;
	D2D1_FACTORY_OPTIONS opts{};
	opts.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &opts, &d2dFactory);

	float dpi = static_cast<float>(::GetDpiForWindow(window.GetHandle()));

	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpi, dpi);

	hr = d2dFactory->CreateDxgiSurfaceRenderTarget(dx->dxgiSurface.Get(), &props, &dx->d2dRenderTarget);
}

static void InitD3D(DXData* dx, Elos::Window& window)
{
	dx->d3d = std::make_unique<DX::DeviceResources>(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT, 2,
		D3D_FEATURE_LEVEL_11_1,
		DX::DeviceResources::c_AllowTearing | DX::DeviceResources::c_FlipPresent);

	dx->d3d->SetWindow(window.GetHandle(), window.GetSize().Width, window.GetSize().Height);
	dx->d3d->CreateDeviceResources();
	dx->d3d->CreateWindowSizeDependentResources();

	dx->shape = GeometricPrimitive::CreateSphere(dx->d3d->GetD3DDeviceContext());
	dx->world = Matrix::Identity;

	auto size = dx->d3d->GetOutputSize();
	dx->view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f),
		Vector3::Zero, Vector3::UnitY);
	dx->proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
		float(size.right) / float(size.bottom), 0.1f, 10.f);

	Create2DResources(dx, window);
}

static void ResizeDX(DXData* dx, const Elos::Event::Resized& e)
{
	if (dx->d3d->WindowSizeChanged(static_cast<int>(e.Size.Width), static_cast<int>(e.Size.Height)))
	{
		auto size = dx->d3d->GetOutputSize();
		dx->view = Matrix::CreateLookAt(Vector3(2.f, 2.f, 2.f), Vector3::Zero, Vector3::UnitY);
		dx->proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, float(size.right) / float(size.bottom), 0.1f, 10.f);
	}
}

int main()
{
	try
	{
		Elos::Timer gameTimer;

		auto mainWindow = std::make_shared<Elos::Window>(
			Elos::WindowCreateInfo::Default("Main Window", { 1280, 720 }));

		auto settingsWindow = mainWindow->CreateChild(
			Elos::WindowCreateInfo::ChildModal(mainWindow, "Test Modal", { 400, 300 }));

		auto gameView = mainWindow->CreateChild(
			Elos::WindowCreateInfo::ChildEmbedded(mainWindow, "Game View", { 800, 600 }));
		gameView->SetMinimumSize({ 100, 100 });
		gameView->SetBackgroundColor(RGB(16, 18, 24));

		bool isDarkTheme = false;
		bool hasRoundCorners = true;
		bool isTransparent = true;

		DXData dx{};
		InitD3D(&dx, *gameView);


#pragma region Event Handlers
		const auto OnTextInput = [&mainWindow](const Elos::Event::TextInput& e)
			{
				std::println("Text Input: {}", e.AsChar());
			};

		const auto OnWindowClose = [&mainWindow](const Elos::Event::Closed&)
			{
				const Elos::MessageBoxDesc desc
				{
					.Window = mainWindow->GetHandle(),
					.Title = "Close Window Requested",
					.Text = "Are you sure you want to close the window?",
					.Flags = Elos::MessageBoxFlags{.Button = Elos::MessageBoxButton::OkCancel}
				};

				const Elos::MessageBoxReturnValue value = Elos::ShowMessageBox(desc);
				if (value == Elos::MessageBoxReturnValue::Ok)
				{
					mainWindow->Close();
				}
			};

		const auto OnKeyPressed = [&](const Elos::Event::KeyPressed& e)
			{
				if (e.Key == Elos::KeyCode::Escape)
				{
					std::println("Escape Pressed. Closing...");
					mainWindow->Close();
				}

				if (e.Key == Elos::KeyCode::D)
				{
					isDarkTheme = !isDarkTheme;
					std::println("Set window dark theme: {}", isDarkTheme);
					Elos::WindowExtensions::EnableDarkMode(mainWindow->GetHandle(), isDarkTheme);
				}

				if (e.Key == Elos::KeyCode::C)
				{
					hasRoundCorners = !hasRoundCorners;
					std::println("Set window round corners: {}", hasRoundCorners);
					Elos::WindowExtensions::RoundCorners(mainWindow->GetHandle(), hasRoundCorners ?
						Elos::WindowExtensions::CornerPreference::Round : Elos::WindowExtensions::CornerPreference::DoNotRound);
				}

				if (e.Key == Elos::KeyCode::P)
				{
					isTransparent = !isTransparent;
					std::println("Set window transparent: {}", isTransparent);
					Elos::WindowExtensions::SetTransparency(mainWindow->GetHandle(), static_cast<Elos::byte>(isTransparent ? 128 : 255));
				}

				if (e.Key == Elos::KeyCode::F)
				{
					// Toggle FPS cap
					gameTimer.SetTargetFPS(gameTimer.GetTargetFPS() < 0.0f ? 60.0f : -1.0f);
				}
			};

		const auto OnWindowResize = [&mainWindow, &gameView, &dx](const Elos::Event::Resized& e)
			{
				dx.d2dRenderTarget.Reset();
				dx.dxgiSurface.Reset();

				ResizeDX(&dx, e);
				Create2DResources(&dx, *gameView);  // Should be same as InitD3D window
			};
#pragma endregion


		float animationTime = 0.0f;

		while (mainWindow->IsOpen())
		{
			mainWindow->HandleEvents(OnTextInput, OnWindowClose, OnKeyPressed, OnWindowResize,
				[](const Elos::Event::MouseButtonPressed& e)
				{
					std::println("Main Window clicked at position: ({}, {})", e.X, e.Y);
				});

			// Handle game view events
			if (gameView && gameView->IsOpen())
			{
				if (gameView->IsOpen())
				{
					gameView->HandleEvents(
						[](const Elos::Event::MouseButtonPressed& e)
						{
							std::println("Game view clicked at position: ({}, {})", e.X, e.Y);
						},
						[&OnWindowResize](const Elos::Event::Resized& e)
						{
							std::println("Game view resized to: ({}, {})", e.Size.Width, e.Size.Height);
							OnWindowResize(e);
						}
					);
				}
			}

			// Handle settings window events
			if (settingsWindow && settingsWindow->IsOpen())
			{
				settingsWindow->HandleEvents(
					[&](const Elos::Event::Closed&)
					{
						settingsWindow->Close();
					},
					[](const Elos::Event::KeyPressed&)
					{
						// Handle settings window key presses
						std::println("Settings window key pressed");
					}
				);
			}

			// Update game logic and render using the timer
			gameTimer.Tick([&](const Elos::Timer::TimeInfo& timeInfo) 
			{
				// Update animation time based on actual elapsed time
				animationTime += static_cast<float>(timeInfo.DeltaTime);

				// Log frame stats every 60 frames
				if (timeInfo.FrameCount % 60 == 0) 
				{
					const auto msg = std::format("Frame Count: {} | FPS: {} | Frame Time: {:.4f}ms | Total Time: {:.3f}s",
						timeInfo.FrameCount,
						timeInfo.FPS,
						timeInfo.DeltaTime * 1000.0,
						timeInfo.TotalTime);

					mainWindow->SetTitle(msg);
				}

				// Render the scene
				auto context = dx.d3d->GetD3DDeviceContext();
				auto renderTarget = dx.d3d->GetRenderTargetView();
				auto depthStencil = dx.d3d->GetDepthStencilView();

				context->ClearRenderTargetView(renderTarget, Colors::DarkSlateBlue);
				context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
				context->OMSetRenderTargets(1, &renderTarget, depthStencil);

				// Set the viewport.
				auto viewport = dx.d3d->GetScreenViewport();
				context->RSSetViewports(1, &viewport);

				// Rotate cube based on elapsed time
				dx.world = Matrix::CreateRotationY(animationTime * 0.5f);
				dx.shape->Draw(dx.world, dx.view, dx.proj);

				HRESULT hr{ S_OK };
				DXGI_SWAP_CHAIN_DESC desc;
				hr = dx.d3d->GetSwapChain()->GetDesc(&desc);

				if (SUCCEEDED(hr) && dx.d2dRenderTarget)
				{
					D2D1_SIZE_F targetSize = dx.d2dRenderTarget->GetSize();
					ComPtr<ID2D1SolidColorBrush> brush;
					D2D1_COLOR_F color{ 1.0f, 0.0f, 0.0f, 1.0f };
					hr = dx.d2dRenderTarget->CreateSolidColorBrush(color, &brush);

					dx.d2dRenderTarget->BeginDraw();

					brush->SetTransform(D2D1::Matrix3x2F::Scale(targetSize));
					D2D1_RECT_F rect = D2D1::RectF(
						100.0f,
						100.0f,
						std::abs(std::sinf(animationTime) * 500.0f),
						std::abs(std::cosf(animationTime) * 500.0f));

					D2D1_ELLIPSE ellipse = D2D1::Ellipse(
						{ 500.0f, 500.0f },
						std::abs(std::sinf(animationTime) * 500.0f),
						std::abs(std::sinf(animationTime) * 500.0f));

					dx.d2dRenderTarget->DrawRectangle(&rect, brush.Get(), 5.0f);
					dx.d2dRenderTarget->DrawEllipse(ellipse, brush.Get(), 7.0f);

					dx.d2dRenderTarget->EndDraw();
				}

				dx.d3d->Present();

				if (mainWindow) mainWindow->Redraw();
				if (gameView) gameView->Redraw();
				if (settingsWindow) settingsWindow->Redraw();
			});
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		std::println("{}", e.what());
	}
	return 0;
}