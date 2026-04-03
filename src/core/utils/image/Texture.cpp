#include "Texture.h"

namespace core::utils::image {

SDL_FRect GetImageRect(
    int targetWidth,
    int targetHeight,
    float maxWidthRatio,
    float maxHeightRatio,
    float aspectRatio,
    Pivot pivot,
    ScaleMode scaleMode
) {
    // Área disponible según ratio
    float availW = targetWidth * maxWidthRatio;
    float availH = targetHeight * maxHeightRatio;

    float drawW = availW;
    float drawH = availH;

    switch (scaleMode) {
        case ScaleMode::Stretch:
            // Se deforma para ocupar todo
            drawW = availW;
            drawH = availH;
            break;

        case ScaleMode::Fit:
            if (availW / aspectRatio <= availH) {
                drawW = availW;
                drawH = drawW / aspectRatio;
            } else {
                drawH = availH;
                drawW = drawH * aspectRatio;
            }
            break;

        case ScaleMode::Fill:
            if (availW / aspectRatio >= availH) {
                drawW = availW;
                drawH = drawW / aspectRatio;
            } else {
                drawH = availH;
                drawW = drawH * aspectRatio;
            }
            break;
    }

    // Posición según pivot
    float x = 0.0f;
    float y = 0.0f;

    switch (pivot) {
        case Pivot::TopLeft:
            x = 0; y = 0;
            break;
        case Pivot::TopCenter:
            x = (targetWidth - drawW) / 2.0f; y = 0;
            break;
        case Pivot::TopRight:
            x = targetWidth - drawW; y = 0;
            break;
        case Pivot::CenterLeft:
            x = 0; y = (targetHeight - drawH) / 2.0f;
            break;
        case Pivot::Center:
            x = (targetWidth - drawW) / 2.0f;
            y = (targetHeight - drawH) / 2.0f;
            break;
        case Pivot::CenterRight:
            x = targetWidth - drawW;
            y = (targetHeight - drawH) / 2.0f;
            break;
        case Pivot::BottomLeft:
            x = 0; y = targetHeight - drawH;
            break;
        case Pivot::BottomCenter:
            x = (targetWidth - drawW) / 2.0f;
            y = targetHeight - drawH;
            break;
        case Pivot::BottomRight:
            x = targetWidth - drawW;
            y = targetHeight - drawH;
            break;
    }

    return SDL_FRect{x, y, drawW, drawH};
}

// Funciones prácticas

SDL_FRect GetImageRect_CenterFit(int w, int h, float aspectRatio) {
    return GetImageRect(w, h, 1.0f, 1.0f, aspectRatio, Pivot::Center, ScaleMode::Fit);
}

SDL_FRect GetImageRect_CenterFill(int w, int h, float aspectRatio) {
    return GetImageRect(w, h, 1.0f, 1.0f, aspectRatio, Pivot::Center, ScaleMode::Fill);
}

SDL_FRect GetImageRect_CenterStretch(int w, int h) {
    return GetImageRect(w, h, 1.0f, 1.0f, 1.0f, Pivot::Center, ScaleMode::Stretch);
}

} // namespace core::utils::image
