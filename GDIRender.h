#pragma once

#include <windows.h>
#include <memory>
#include "desktop_capture/desktop_cpature_include.h"

using namespace webrtc;

class CGDIRender
{
public:
    CGDIRender();
    ~CGDIRender();
    enum class pix_fmt {
        BGR24,
        RGB24,
        RGBA,
        BGRA
    };

    void render(HWND hWnd, std::unique_ptr<DesktopFrame> &frame);

public:
    static void ConstructBMPHeader(int width, int height, pix_fmt format, BITMAPINFOHEADER& bitmapHeader);

protected:
    void reset();

private:
    BITMAPINFOHEADER _bmpHeader = {0};
    HDC _hDC = nullptr;
    HDC _hMemDC = nullptr;
    DesktopSize _Size;
    HBITMAP _hBitmap = nullptr;
    HWND _hWnd = nullptr;
};

