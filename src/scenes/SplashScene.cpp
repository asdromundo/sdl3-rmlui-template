#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <filesystem>
#include <cmath>

#include "SplashScene.h"
#include "core/scene/Events.h"
#include "core/utils/image/Texture.h"

SplashScene::SplashScene(AppContext *context)
    : Scene("Splash", context) {}

SplashScene::~SplashScene()
{
    CleanUp();
}

bool SplashScene::Init()
{
    std::filesystem::path basePath = SDL_GetBasePath();
    if (basePath.empty())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get base path: %s", SDL_GetError());
        return false;
    }
    return LoadImageTexture((basePath / "resources/logo.svg").string());
}

void SplashScene::Ready()
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Splash scene ready.");
}

static Uint32 SceneFinishedTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
    // Return whether the signal was emited or not
    core::scene::events::EmitSceneFinishedEvent();
    return 0;
}

void SplashScene::RenderLogo(SDL_Renderer *renderer)
{
    // Clean background color
    SDL_SetRenderDrawColor(app->renderer, 36, 18, 36, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);
    
    int targetWidth, targetHeight;
    SDL_GetCurrentRenderOutputSize(app->renderer, &targetWidth, &targetHeight);

    SDL_FRect dstRect = core::utils::image::GetImageRect(targetWidth, targetHeight, 0.5f, 0.5f);

    SDL_RenderTexture(app->renderer, logoTexture, nullptr, &dstRect);

    // Actualizar el rendering target
    SDL_RenderPresent(app->renderer);
}

void SplashScene::OnEnter()
{ // Solo renderizamos la textura si está cargada
    if (logoTexture)
    {
        RenderLogo(app->renderer);
        // End scene after timer
        SDL_AddTimer(200, SceneFinishedTimerCallback, nullptr);
    }
}

SDL_AppResult SplashScene::HandleEvent(SDL_Event *event)
{
    if (logoTexture)
    {
        if (event->type == SDL_EVENT_WINDOW_RESIZED)
        {
            RenderLogo(app->renderer);
            return SDL_APP_CONTINUE;
        }
    }
    return SDL_APP_CONTINUE;
}

void SplashScene::Update(float deltatime)
{
}

void SplashScene::Render()
{
}

void SplashScene::OnExit()
{
}

void SplashScene::CleanUp()
{
    if (logoTexture)
    {
        SDL_DestroyTexture(logoTexture);
        logoTexture = nullptr;
    }
}

bool SplashScene::LoadImageTexture(const std::string &path)
{
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (!surface)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image: %s", SDL_GetError());
        return false;
    }

    logoTexture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_DestroySurface(surface);

    if (!logoTexture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create image texture: %s", SDL_GetError());
        return false;
    }

    return true;
}